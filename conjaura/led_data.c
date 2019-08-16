/*
 * led_data.c
 *
 *  Created on: 30 Jul 2019
 *      Author: me
 */

#include "led_data.h"

extern UART_HandleTypeDef huart3;
uint16_t * renderOutput = bamBuffer1;
uint8_t * streamOutput = (uint8_t *)bamBuffer1;

uint8_t t = 0;
uint16_t conversions = 0;
uint16_t rowTime = 0;

void ConvertRawPixelData(){
	conversions++;
	uint16_t curLED = thisPanel.pixelCount;
	uint16_t srcOffset = 0;
	uint8_t *bufferPtr;
	if(globalVals.rxBufferLocation==0){
		bufferPtr = spiBufferRXAlt;
	}
	else{
		bufferPtr = spiBufferRX;
	}

	if(thisPanel.colourMode == PALETTE_COLOUR){
		srcOffset = thisPanel.pixelCount;
		do{
			ledB[--curLED] = paletteB[bufferPtr[--srcOffset]];
			ledG[curLED] = paletteG[bufferPtr[srcOffset]];
			ledR[curLED] = paletteR[bufferPtr[srcOffset]];
		}
		while(curLED>0);
	}
	else if(thisPanel.colourMode == HIGH_COLOUR){
		//DECOMPRESS DATA INTO PURE RGB.
		srcOffset = thisPanel.pixelCount*2;
		if(thisPanel.biasHC==0){		//GREEN BIAS 565
			do{
				ledB[--curLED] = bufferPtr[--srcOffset] & 0x1F;
				ledG[curLED] = (bufferPtr[srcOffset] >>5) | ((bufferPtr[--srcOffset] & 0x7) <<3) ;
				ledR[curLED] = bufferPtr[srcOffset] & 0xF8;
			}while(curLED>0);
		}
		else if(thisPanel.biasHC==1){	//RED BIAS 655
			do{
				ledB[--curLED] = bufferPtr[--srcOffset] & 0x1F;
				ledG[curLED] = (bufferPtr[srcOffset] >>5) | ((bufferPtr[--srcOffset] & 0x3) <<3) ;
				ledR[curLED] = bufferPtr[srcOffset] & 0xFC;
			}while(curLED>0);
		}
		else if(thisPanel.biasHC==2){	//BLUE BIAS	556
			do{
				ledB[--curLED] = bufferPtr[--srcOffset] & 0x3F;
				ledG[curLED] = (bufferPtr[srcOffset] >>6) | ((bufferPtr[--srcOffset] & 0x3) <<2) ;
				ledR[curLED] = bufferPtr[srcOffset] & 0xF8;
			}while(curLED>0);
		}
		else{							//EQUAL 15BIT COLOUR
			do{
				ledB[--curLED] = (bufferPtr[--srcOffset] & 0x3E) >> 1;
				ledG[curLED] = (bufferPtr[srcOffset] >>6) | ((bufferPtr[--srcOffset] & 0x3) <<2) ;
				ledR[curLED] = bufferPtr[srcOffset] & 0xF8;
			}while(curLED>0);
		}
	}
	else{
		//OPTIMISED
		srcOffset = thisPanel.pixelCount*3;
		uint8_t *bfr = bufferPtr+srcOffset;
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

	renderState.drawBufferLocation = !renderState.drawBufferLocation;
	if(renderState.drawBufferLocation==0){
		renderOutput = bamBuffer1;
		streamOutput = (uint8_t *)bamBuffer2;
	}
	else{
		renderOutput = bamBuffer2;
		streamOutput = (uint8_t *)bamBuffer1;
	}

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
			*(renderOutput+dataOffset) = 0;
			*(renderOutput+dataOffset+1) = 0;
			*(renderOutput+dataOffset+2) = 0;
			*(renderOutput+dataOffset+3) = 0;
			*(renderOutput+dataOffset+4) = 0;
			*(renderOutput+dataOffset+5) = 0;
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
	renderState.waitingProcessing = FALSE;
	renderState.firstRender = FALSE;
	renderState.drawBufferSwitchPending = TRUE;
	//47362 CYCLES TO GET HERE IN TC MODE
	if(renderState.bamTimerStarted==FALSE){
		renderState.bamTimerStarted = TRUE;
		DataToLEDs();
		#if DEBUGMODE
		uint16_t timd = 64000;
		SetAndStartTimer7(timd);
		#endif
		LEDDataTransmit();
	}
}


void LEDDataTransmit(){
	//ClearAndPauseTimer6();
	TIM6->CR1 = 0;		//PAUSE TIMER
	//TIM6->CNT = 0;		//ZERO TIMER
	TIM6->SR = 0;		//CLEAR THE UPDATE EVENT FLAG
	uint16_t dataPos8Bit = renderState.rowOffset+renderState.bamOffset;
	//uint8_t *data = (uint8_t *)streamOutput;
	TransmitSPI1DMA(streamOutput+dataPos8Bit,12);

	//WE CAN DO A FEW OTHER LITTLE BITS AHEAD OF TIME
	GPIOA->BSRR |= LED_LATCH_Pin;			//SET LATCH PIN HIGH READY TO LATCH
	GPIOB->BSRR |= ROW_SEL_EN_GLK_Pin;		//DISABLE ALL OUTPUTS OF LED DRIVER. PLACE HERE FOR DIMMER LEVELS.
	renderState.bamOffset += 12;
}


void FinaliseLEDData(){
	//DO THIS WHILST WAITING FOR THE SPI TO CLOCK IN...DMA WILL ALWAYS END BEFORE SPI HAS FINISHED SPITTING OUT ITS BITS

	//GPIOB->BSRR |= ROW_SEL_EN_GLK_Pin;			//DISABLE ALL OUTPUTS OF LED DRIVER - MOVED TO ALLOW DIMMER LEVELS.

	SelectRow(renderState.currentRow);				//CHANGE ROW. NOTE WE DONT BOTHER SHUTTING OFF THE MULTIPLEXER AS THE LED DRIVER IS OFF ANYWAY.
	uint8_t bamCache = renderState.currentBamBit++;

	if(renderState.currentBamBit == thisPanel.bamBits){
		renderState.currentBamBit=0;
		renderState.bamOffset = 0;
		renderState.currentRow++;
		if(renderState.currentRow == thisPanel.scanlines){
			renderState.currentRow = 0;
			renderState.requireEdgeUpdate = TRUE;
			if(renderState.drawBufferSwitchPending==TRUE){
				if(renderState.drawBufferLocation==0){
					streamOutput = (uint8_t *)bamBuffer1;
				}
				else{
					streamOutput = (uint8_t *)bamBuffer2;
				}
				renderState.drawBufferSwitchPending= FALSE;
			}
		}
		renderState.rowOffset = renderState.currentRow*(12*thisPanel.bamBits);
	}
	//NOT NEEDED IF WE FILL OUT TIME WITH SOMETHING USEFUL FOR A FEW CLOCKS...
	//while((SPI1->SR & SPI_SR_BSY));

	GPIOA->BRR |= LED_LATCH_Pin;		//SET LATCH LOW TO COMPLETE THE LATCHING PROCESS. DATA IS DISPLAYED ON OE (EN_GLK) LOW.
	GPIOB->BRR |= ROW_SEL_EN_GLK_Pin;	//ENABLE ALL OUTPUTS OF LED DRIVER AND SHIFT LATCHED DATA TO OUTPUT

	SetAndStartTimer6(timeDelays[bamCache]);

	#if DEBUGMODE
	if(renderState.currentRow == 1){
		if(renderState.currentBamBit==1 && t==0){
			rowTime = TIM7->CNT;
			ClearAndPauseTimer7();
			//printf("ROW TIME: %d \n",val);
			t=1;
		}
	}
	#endif

	//BECAUSE THE DELAY IS SO SHORT ON BAM 0 WE IMMEDIATELY SEND BAM 1
	//THIS REDUCES THE TIME (BAM TIME + SEND TIME) DOWN TO JUST BAM TIME.
	//WE REPLICATE THIS FUNCTIONALITY ON BAMS 2,3 and 6. THIS LEAVES THE SPI/DMA TX LINE FREE FOR LONGER STINTS
	//if(bamCache==0 || bamCache==2 || bamCache==4 || bamCache==6){	//USE CURRENTROW SO FIRST PASS DOESNT FIRE THIS

	//if(renderState.immediateJump>0){
	//	LEDDataTransmit();
	//}

	//ADC DATA COLLECTION IS DONE DURING BAM4 - WE HAVE APROX 840 CYCLES TO GET THIS DONE.
	if(thisPanel.touchActive==TRUE && bamCache==4){
		InitTouch_ADC();
	}
	if(renderState.waitingToReturn==TRUE && bamCache==5){
		SendReturnData();
	}
	if(renderState.requireEdgeUpdate==TRUE && bamCache==6){
		if(renderState.edgeComplete == TRUE){
			renderState.requireEdgeUpdate = FALSE;
			renderState.edgeComplete = FALSE;
			TXEdgeLights();
		}
	}
}

void EnableEdgeLights(){
	GPIOA->BSRR |= EDGE_EN_Pin;
}


void DisableEdgeLights(){
	GPIOA->BRR |= EDGE_EN_Pin;
}

//GRB ORDER
uint8_t data[72] = {1,0,0,0,32,0,0,0,1,1,0,0,0,1,0,0,0,1,1,0,0,0,1,0,0,0,1,1,0,0,0,1,0,0,0,1,1,0,0,0,1,0,0,0,1,1,0,0,0,1,0,0,0,1,1,0,0,0,1,0,0,0,1,1,0,0,0,1,0,0,0,1};
//uint8_t data[72] = {210,0,0,0,255,0,0,0,255,128,0,0,0,128,0,0,0,128,128,0,0,0,128,0,0,0,128,128,0,0,0,128,0,0,0,128,128,0,0,0,128,0,0,0,128,128,0,0,0,128,0,0,0,128,128,0,0,0,128,0,0,0,128,128,0,0,0,128,0,0,0,128};

//uint8_t data[72] = {255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255};
//uint8_t edgeCompiled[576];
uint8_t gg =0;
void TXEdgeLights(){

	uint8_t data2[72] = {1,0,0,0,1,0,0,0,1,1,0,0,0,1,0,0,0,1,1,0,0,0,1,0,0,0,1,1,0,0,0,1,0,0,0,1,1,0,0,0,1,0,0,0,1,1,0,0,0,1,0,0,0,1,1,0,0,0,1,0,0,0,1,1,0,0,0,1,0,0,0,1};
	data2[gg] = 64;
	gg++;
	if(gg>71){
		gg=0;
	}
	uint32_t offset = 0;
	uint8_t result;
	for(uint8_t led=0;led<72;led++){
		uint8_t bits=8;
		do{
			bits--;
			result = (data2[led] & (1<<bits));	//IS A 1 - WRITE OUT 3 BITS FOR 1 (110) BUT INVERTED

			if(result){
				edgeCompiled[offset] = 14;
			}
			else{
				edgeCompiled[offset] = 15;
			}
			bits--;
			result = (data2[led] & (1<<bits));	//IS A 1 - WRITE OUT 3 BITS FOR 1 (110) BUT INVERTED
			if(result){
				edgeCompiled[offset] |= 192;
			}
			else{
				edgeCompiled[offset] |= 224;
			}

			offset++;
			//bits--;
		}while(bits>0);
	}


	//USART3->CR3 |= 2684354560;
	//USART3->CR3 |= 8388608;
	DMA1_Channel7->CNDTR = offset;						//DATA LENGTH OF TRANSFER
	DMA1_Channel7->CPAR = (uint32_t)&USART3->TDR;		//PERIPHERAL ADDRESS TARGET (SPI DATA REGISTER) - SET ON INIT
	DMA1_Channel7->CMAR = (uint32_t)edgeCompiled;		//ADDRESS OF SRC DATA


	DMA1_Channel7->CCR |= 3;									//SET BIT 0 TO 1 TO ENABLE THE DMA TRANSFER. SET BIT 1 TO 1 TO ENABLE TRANSFER COMPLETE INTERUPT



	//HAL_HalfDuplex_EnableTransmitter(&huart3);
	//uint8_t t = HAL_UART_Transmit(&huart3, edgeCompiled, 3, 1000);
	//printf("Dat %d \n",edgeCompiled[0]);

//printf("send %d\n",t);
	//TXEdgeLights();
}


void UART_DMATransmitCplt(UART_HandleTypeDef *huart){
	printf("uart called\n");
}
