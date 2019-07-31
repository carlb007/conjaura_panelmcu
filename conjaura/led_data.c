/*
 * led_data.c
 *
 *  Created on: 30 Jul 2019
 *      Author: me
 */

#include "led_data.h"

uint16_t * dataOut = bamBuffer1;


void ConvertRawPixelData(){
	uint16_t curLED = thisPanel.pixelCount;
	uint16_t srcOffset;
	if(thisPanel.colourMode == PALETTE_COLOUR){
		srcOffset = thisPanel.pixelCount;
		do{
			ledB[--curLED] = paletteB[spiBufferRX[--srcOffset]];
			ledG[curLED] = paletteG[spiBufferRX[srcOffset]];
			ledR[curLED] = paletteR[spiBufferRX[srcOffset]];
		}
		while(curLED>0);
	}
	else if(thisPanel.colourMode == HIGH_COLOUR){
		//DECOMPRESS DATA INTO PURE RGB.
		srcOffset = thisPanel.pixelCount*2;
		if(thisPanel.biasHC==0){		//GREEN BIAS 565
			do{
				ledB[--curLED] = spiBufferRX[--srcOffset] & 0x1F;
				ledG[curLED] = (spiBufferRX[srcOffset] >>5) | ((spiBufferRX[--srcOffset] & 0x7) <<3) ;
				ledR[curLED] = spiBufferRX[srcOffset] & 0xF8;
			}while(curLED>0);
		}
		else if(thisPanel.biasHC==1){	//RED BIAS 655
			do{
				ledB[--curLED] = spiBufferRX[--srcOffset] & 0x1F;
				ledG[curLED] = (spiBufferRX[srcOffset] >>5) | ((spiBufferRX[--srcOffset] & 0x3) <<3) ;
				ledR[curLED] = spiBufferRX[srcOffset] & 0xFC;
			}while(curLED>0);
		}
		else if(thisPanel.biasHC==2){	//BLUE BIAS	556
			do{
				ledB[--curLED] = spiBufferRX[--srcOffset] & 0x3F;
				ledG[curLED] = (spiBufferRX[srcOffset] >>6) | ((spiBufferRX[--srcOffset] & 0x3) <<2) ;
				ledR[curLED] = spiBufferRX[srcOffset] & 0xF8;
			}while(curLED>0);
		}
		else{							//EQUAL 15BIT COLOUR
			do{
				ledB[--curLED] = (spiBufferRX[--srcOffset] & 0x3E) >> 1;
				ledG[curLED] = (spiBufferRX[srcOffset] >>6) | ((spiBufferRX[--srcOffset] & 0x3) <<2) ;
				ledR[curLED] = spiBufferRX[srcOffset] & 0xF8;
			}while(curLED>0);
		}
	}
	else{
		srcOffset = thisPanel.pixelCount*3;
		//debugPrint("LED Count %d\n",curLED);
		do{
			//debugPrint("SRC %d\n",srcOffset);
			ledB[--curLED] = spiBufferRX[--srcOffset];
			ledG[curLED] = spiBufferRX[--srcOffset];
			ledR[curLED] = spiBufferRX[--srcOffset];
		}
		while(curLED>0);
	}
	BamifyData();
}

void BamifyData(){
	/*
	THIS SECTION OF CODE IS SPECIFIC TO THE 16x16 PANEL.
	ITS NOT FULLY COMPATABLE WITH OTHER SIZES YET.
	*/

	/*
	ONCE WE HAVE OUR STANDARDISED RGB ARRAYS WE NEED TO CONVERT THAT DATA INTO A PRE-COMPILED SET OF SPI STREAMABLE
	BAM DATA.
	SPI DATA IS SENT AS 12 BYTES TO THE SHIFT REGISTERS IN R,R,G,G,B,B(TOP),R,R,G,G,B,B(BOTTOM) ORDER. WE NEED TO COMPILE
	THE DATA WITH THAT IN MIND.
	OUR DATA GETS STORED IN 16BIT INTEGERS RATHER THAN 8 BIT - SO A CAST IS USED ON TX. THE 16BITS MATCHES THE PANEL WIDTH
	AND AIDS IN SIMPLER BIT SHIFTING.
	A SINGLE TX OF DATA WILL ALWAYS INCLUDE THE TOP AND BOTTOM PARTS OF OUR SCANLINES (IN 16X16)
	IE ROW 8 (TOP SECTION ROW 0) AND ROW 0(BOTTOM SECTION ROW 0) ARE HELD TOGETHER AS THEYRE STREAMED IN THE SAME SCANLINE:
	BAM BIT 1: RGB(TOP)RGB(BOTTOM) (NOTE 16BIT RATHER THAN 8 BIT)
	BAM BIT 2: RGB(TOP)RGB(BOTTOM)
	BAM BIT 3: RGB(TOP)RGB(BOTTOM)
	...Nth BAM BIT.

	EACH BAM BIT DATA IS A FLAG OF 1 OR 0 FOR WHETHER THE LIGHT IS TURNED ON OR OFF. FOR EXAMPLE:
	IF PIXEL 0(BOTTOM ROW 0) IS SENT AN RGB VALUE OF (255,0,0) OUR OUTPUT BUFFER BAM DATA WOULD BE STORED AS:
	BAM BIT 1: 0,0,0,32768,0,0
	BAM BIT 2: 0,0,0,32768,0,0
	BAM BIT 3: 0,0,0,32768,0,0
	...
	IN THIS INSTANCE THE LED IS ALWAYS ON (255 = FULLY ON). 32768 INT 16 TO BINARY = 1000 0000 0000 0000

	BAM BIT 1 IS THE SHORTEST DURATION CYCLE.
	*/

	uint8_t bitSelect,col;
	uint32_t dataOffsetRowStart = 0;
	uint32_t dataOffset = 0;
	uint8_t topOffset = (thisPanel.width * thisPanel.height) / 2;
	uint16_t colourIdx1,colourIdx2;
	uint8_t bytesPerFullBamRow = 2*thisPanel.bamBits*3;
	uint8_t shiftLeftBits[16] = {15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0};

	for(uint8_t row=0;row<thisPanel.scanlines;row++){
		uint16_t colourIdx2Base = (row*thisPanel.width);
		uint16_t colourIdx1Base = colourIdx2Base+topOffset;
		dataOffsetRowStart =  (row*bytesPerFullBamRow);

		for(uint8_t bamBit=0;bamBit<thisPanel.bamBits;bamBit++){
			bitSelect = 1 << bamBit;
			col=thisPanel.width;
			dataOffset = dataOffsetRowStart;
			do{
				col--;
				colourIdx1 = colourIdx1Base + col;
				colourIdx2 = colourIdx2Base + col;
				//RED ROW TOP
				*(dataOut+dataOffset) |= ((ledR[colourIdx1] & bitSelect)>>bamBit)<<shiftLeftBits[col];
				//GREEN ROW TOP
				*(dataOut+dataOffset+1) |= ((ledG[colourIdx1] & bitSelect)>>bamBit)<<shiftLeftBits[col];
				//BLUE ROW TOP
				*(dataOut+dataOffset+2) |= ((ledB[colourIdx1] & bitSelect)>>bamBit)<<shiftLeftBits[col];

				//RED ROW BOTTOM
				*(dataOut+dataOffset+3) |= ((ledR[colourIdx2] & bitSelect)>>bamBit)<<shiftLeftBits[col];
				//GREEN ROW BOTTOM
				*(dataOut+dataOffset+4) |= ((ledG[colourIdx2] & bitSelect)>>bamBit)<<shiftLeftBits[col];
				//BLUE ROW BOTTOM
				*(dataOut+dataOffset+5) |= ((ledB[colourIdx2] & bitSelect)>>bamBit)<<shiftLeftBits[col];

			}while(col>0);

			dataOffsetRowStart += 6;
		}
	}
}
