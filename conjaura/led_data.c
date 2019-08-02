/*
 * led_data.c
 *
 *  Created on: 30 Jul 2019
 *      Author: me
 */

#include "led_data.h"
extern SPI_HandleTypeDef hspi1,hspi2;
uint16_t * renderOutput = bamBuffer1;



void ConvertRawPixelData(){
	uint16_t curLED = thisPanel.pixelCount;
	uint16_t srcOffset = 0;
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
		//OPTIMISED
		srcOffset = thisPanel.pixelCount*3;
		uint8_t *bfr = spiBufferRX+srcOffset;
		do{
			ledB[--curLED] = *(--bfr);
			ledG[curLED] = *(--bfr);
			ledR[curLED] = *(--bfr);
		}while(curLED>0);
	}
	renderState.storedData = TRUE;
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


	//OPTIMISED
	uint8_t bitSelect,col;
	uint32_t dataOffsetRowStart = 0;
	uint32_t dataOffset = 0;
	uint8_t topOffset = (thisPanel.width * thisPanel.height) / 2;
	uint16_t colourIdx1,colourIdx2,colourIdx2Base,colourIdx1Base;
	uint8_t bytesPerFullBamRow = thisPanel.bamBits*6;
	uint8_t shiftLeftBits[16] = {15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0};

	for(uint8_t row=0;row<thisPanel.scanlines;row++){
		colourIdx2Base = (row*thisPanel.width);
		colourIdx1Base = colourIdx2Base+topOffset;
		dataOffsetRowStart =  (row*bytesPerFullBamRow);
		for(uint8_t bamBit=0;bamBit<thisPanel.bamBits;bamBit++){
			bitSelect = 1 << bamBit;
			dataOffset = dataOffsetRowStart;
			col = thisPanel.width;
			do{
				col--;
				colourIdx1 = colourIdx1Base + col;
				colourIdx2 = colourIdx2Base + col;
				//RED ROW TOP
				*(renderOutput+dataOffset) |= ((ledR[colourIdx1] & bitSelect)>>bamBit)<<shiftLeftBits[col];
				//GREEN ROW TOP
				*(renderOutput+dataOffset+1) |= ((ledG[colourIdx1] & bitSelect)>>bamBit)<<shiftLeftBits[col];
				//BLUE ROW TOP
				*(renderOutput+dataOffset+2) |= ((ledB[colourIdx1] & bitSelect)>>bamBit)<<shiftLeftBits[col];
				//RED ROW BOTTOM
				*(renderOutput+dataOffset+3) |= ((ledR[colourIdx2] & bitSelect)>>bamBit)<<shiftLeftBits[col];
				//GREEN ROW BOTTOM
				*(renderOutput+dataOffset+4) |= ((ledG[colourIdx2] & bitSelect)>>bamBit)<<shiftLeftBits[col];
				//BLUE ROW BOTTOM
				*(renderOutput+dataOffset+5) |= ((ledB[colourIdx2] & bitSelect)>>bamBit)<<shiftLeftBits[col];
			}while(col>0);
			dataOffsetRowStart += 6;
		}
	}
	renderState.parsedData = TRUE;
	//47362 CYCLES TO GET HERE IN TC MODE
	if(renderState.bamTimerStarted==FALSE){
		renderState.bamTimerStarted = TRUE;
		ConfigLEDDataSPI();
		DataToLEDs();
		SetAndStartTimer6(timeDelays[renderState.currentBamBit]);
	}
	else{
		renderState.awaitingSwitch = TRUE;
	}
}

void LEDDataTransmit(){
	uint16_t dataPos8Bit = (renderState.currentRow*(12*thisPanel.bamBits))+(renderState.currentBamBit*12);
	uint8_t *data = (uint8_t *)bamBuffer1;

	DMA1_Channel1->CCR &= ~769;									//SET BIT 0 TO 0 TO DISABLE DMA. SET BITS 8 and 9 to 0. CLEAR THE PERIPHERAL DATA SIZE. 00 SET US IN 8BIT MODE.

	DMA1_Channel1->CNDTR = 12;									//DATA LENGTH OF TRANSFER
	DMA1_Channel1->CPAR = (uint32_t)&hspi1.Instance->DR;		//PERIPHERAL ADDRESS TARGET (SPI DATA REGISTER)
	DMA1_Channel1->CMAR = (uint32_t)(data+dataPos8Bit);	//ADDRESS OF SRC DATA

	DMA1->IFCR |= 1;											//FORCE BIT 0 TO A 1 TO CLEAR ALL CHANNEL 1 INTERUPT FLAGS
	DMA1_Channel1->CCR |= 3;									//SET BIT 0 TO 1 TO ENABLE THE DMA TRANSFER. SET BIT 1 TO 1 TO ENABLE TRANSFER COMPLETE INTERUPT

	SetAndStartTimer6(timeDelays[renderState.currentBamBit]);
}

void FinaliseLEDData(){
	GPIOA->BSRR |= LED_LATCH_Pin;		//SET LATCH PIN HIGH READY TO LATCH
	while((hspi1.Instance->SR & SPI_SR_BSY));


	GPIOA->BRR |= LED_LATCH_Pin;		//SET LATCH LOW TO COMPLETE THE LATCHING PROCESS. DATA IS DISPLAYED ON OE (EN_GLK) LOW.
	GPIOB->BSRR |= ROW_SEL_EN_GLK_Pin;	//DISABLE ALL OUTPUTS OF LED DRIVER
	SelectRow(renderState.currentRow);	//CHANGE ROW. NOTE WE DONT BOTHER SHUTTING OFF THE MULTIPLEXER AS THE LED DRIVER IS OFF ANYWAY.
	GPIOB->BRR |= ROW_SEL_EN_GLK_Pin;	//ENABLE ALL OUTPUTS OF LED DRIVER AND SHIFT LATCHED DATA TO OUTPUT

	renderState.currentBamBit++;
	if(renderState.currentBamBit == thisPanel.bamBits){
		renderState.currentBamBit=0;
		renderState.currentRow++;
		if(renderState.currentRow == thisPanel.scanlines){
			renderState.currentRow = 0;
		}
	}
}
