/*
 * led_data.c
 *
 *  Created on: 30 Jul 2019
 *      Author: me
 */

#include "led_data.h"
extern SPI_HandleTypeDef hspi1,hspi2;
uint16_t * renderOutput = bamBuffer1;
uint16_t timeDelays[8] = {100,200,400,800,1600,3200,6400,12800};
uint8_t testData[12] = {255,0,0,0,0,0,0,0,0,0,0,0};

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
		uint8_t curRow = thisPanel.height;
		uint8_t colCount = 0;
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
	if(renderState.streamingData==FALSE){
		BamifyData();
	}
	else{
		renderState.awaitingSwitch = TRUE;
	}
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

	uint8_t bitSelect;
	uint32_t dataOffsetRowStart = 0;
	uint32_t dataOffset = 0;
	uint8_t topOffset = (thisPanel.width * thisPanel.height) / 2;
	uint16_t colourIdx1,colourIdx2;
	uint8_t bytesPerFullBamRow = thisPanel.bamBits*6;
	uint8_t shiftLeftBits[16] = {15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0};

	for(uint8_t row=0;row<thisPanel.scanlines;row++){
		uint16_t colourIdx2Base = (row*thisPanel.width);
		uint16_t colourIdx1Base = colourIdx2Base+topOffset;
		dataOffsetRowStart =  (row*bytesPerFullBamRow);

		for(uint8_t bamBit=0;bamBit<thisPanel.bamBits;bamBit++){
			bitSelect = 1 << bamBit;
			dataOffset = dataOffsetRowStart;
			for(uint8_t col=0;col<thisPanel.width;col++){
				/*
				ROW 7...
				IDX 1 (TOP) = 240 > 255
				IDX 2 (BOTTOM) = 112 > 127

				ROW 1...
				IDX 1 = 128 + 16
				IDX 2 = 16
				*/
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
			}

			dataOffsetRowStart += 6;
		}
	}
	renderState.awaitingSwitch = TRUE;
	if(renderState.streamingData==FALSE){
		renderState.streamingData = TRUE;
		StartBamTimer();
	}
}


void StartBamTimer(){
	DataToLEDs();
	//TIM6->ARR = timeDelays[renderState.currentBamBit];
	//TIM6->CR1 |= TIM_CR1_CEN;		//START TIMER
	uint8_t blank[12] = {0,0,0,0,0,0,0,0,0,0,0,0};

	//CONFIGURE SPI FOR DATA TO LEDS (WE SEND FASTER AND WITH DIFFERENT CPOL AND BIT ORDER TO THE LEDS)
	//SPI1->CR1;
	//SPI2->CR1
	globalVals.dataState = DEBUG;
	SPI1->CR1 &= ~64;						//SET 6th BIT TO 0 TO DISABLE SPI
	SPI1->CR1 &= ~32768;					//SET 15th BIT TO 0 TO SET TO ZERO FOR UNIDIRECTIONAL MODE (MOSI MISO)
	SPI1->CR1 &= ~56; 					//CLEAR 3rd > 5th BITS TO WIPE SPEED SETTING
	SPI1->CR1 &= ~2;						//SET 2nd BIT TO 0 TO ENABLE CPOL LOW
	SPI1->CR1 |= 8;						//SET 4th BIT TO 1 TO SET SPEED TO CLK/4
	SPI1->CR1 |= 128;						//SET 7th BIT TO 1 TO ENABLE LSB MODE
	SPI1->CR1 |= 16384; 					//SET 14th BIT TO 1 TO ENABLE TX MODE


	SPI1->CR2 &= ~3840; 					//SET 8th > 11th BITS TO 0 - CLEAR THE DATA SIZE BITS
	SPI1->CR2 |= 1732; 					//SET 8th > 11th BITS TO 0111 TO ENABLE 8 BIT TRANSFER MODE
	SPI1->CR1 |= 64;						//SET 6th BIT TO 1 TO ENABLE SPI
	SPI1->CR2 |= 2;						//SET BIT 1 TO 1. ENABLE DMA TX (BIT 0 ENABLES RX DMA)


	//hspi1.hdmatx->XferCpltCallback = Tester;
	//hspi1.Instance->CR2 |= 128;					//ENABLE TX BUFFER EMPTY INTERUPT


	/*
	DMA1_Channel1->CCR &= ~1;						//SET BIT 0 TO 0 TO DISABLE DMA.
	DMA1_Channel1->CCR |= 2;						//SET BIT 1 TO 1 TO ENABLE TRANSFER COMPLETE INTERUPT
	DMA1_Channel1->CCR &= ~768;					//SET BITS 8 and 9 to 0. CLEAR THE PERIPHERAL DATA SIZE. 00 SET US IN 8BIT MODE.

	DMA1_Channel1->CNDTR = 12;						//DATA LENGTH OF TRANSFER
	DMA1_Channel1->CPAR = (uint32_t)&hspi1.Instance->DR;		//PERIPHERAL ADDRESS TARGET (SPI DATA REGISTER)
	DMA1_Channel1->CMAR = (uint32_t)&blank;					//ADDRESS OF SRC DATA

	DMA1->IFCR |= 1;								//FORCE BIT 0 TO A 1 TO CLEAR ALL CHANNEL 1 INTERUPT FLAGS
	DMA1_Channel1->CCR |= 1;						//SET BIT 0 TO 1 TO ENABLE THE DMA TRANSFER
	*/
	TIM6->ARR = timeDelays[renderState.currentBamBit];
	TIM6->CR1 |= TIM_CR1_CEN;		//START TIMER
	//HAL_DMA_Start_IT(hspi1.hdmatx, (uint32_t)&blank, (uint32_t)&hspi1.Instance->DR, 12);

	//debugPrint("DMA ATTEMPTED! \n","");

	//HAL_SPI_Transmit_DMA(&hspi1, blank, 12);

}

void Tester(){
	debugPrint("CALLED \n","");
}

void TIM6_IRQHandler(){
	TIM6->CR1 &= ~TIM_CR1_CEN;										//PAUSE TIMER
	TIM6->CNT = 0;													//ZERO TIMER
	TIM6->SR = 0;													//CLEAR THE UPDATE EVENT FLAG
	//printf("timer callback \n");




	//if(renderState.immediateJump != TRUE){
		//EnableRS485RX();
		//DataToLEDs();

		//uint8_t bytesPerFullBamRow = 2*thisPanel.bamBits*3;
		//uint8_t bytesPerFullBamRow = 2*thisPanel.bamBits*6;
		//uint8_t dataOffsetRowStart =  (renderState.currentRow*bytesPerFullBamRow);

		uint16_t dataPos8Bit = (renderState.currentRow*(12*thisPanel.bamBits))+(renderState.currentBamBit*12);
		//uint16_t dataPos16Bit = (renderState.currentRow*(6*thisPanel.bamBits))+(renderState.currentBamBit*6);


		//printf("16 Bit %d %d %d %d %d %d \n",bamBuffer1[12],bamBuffer1[13],bamBuffer1[14],bamBuffer1[15],bamBuffer1[16],bamBuffer1[17]);
		//uint8_t * bitSmall = (uint8_t *)bamBuffer1;
		//printf("8 Bit %d %d %d %d %d %d %d %d %d %d %d %d \n",*(bitSmall+23),*(bitSmall+24),*(bitSmall+25),*(bitSmall+26),*(bitSmall+27),*(bitSmall+28),*(bitSmall+29),*(bitSmall+30),*(bitSmall+31),*(bitSmall+32),*(bitSmall+33),*(bitSmall+34));

		//
		//hspi1.Init.FirstBit = SPI_FIRSTBIT_LSB;
		//if(renderState.currentRow==0){
		//	printf("Off: %d ROW %d \n",dataPos16Bit,renderState.currentRow);
		//}
		uint8_t *data = (uint8_t *)bamBuffer1;

		//HAL_SPI_Transmit_DMA(&hspi1, (data+dataPos8Bit), 12);


		DMA1_Channel1->CCR &= ~769;								//SET BIT 0 TO 0 TO DISABLE DMA. SET BITS 8 and 9 to 0. CLEAR THE PERIPHERAL DATA SIZE. 00 SET US IN 8BIT MODE.

		DMA1_Channel1->CNDTR = 12;								//DATA LENGTH OF TRANSFER
		DMA1_Channel1->CPAR = (uint32_t)&hspi1.Instance->DR;	//PERIPHERAL ADDRESS TARGET (SPI DATA REGISTER)
		DMA1_Channel1->CMAR = (uint32_t)(data+dataPos8Bit);		//ADDRESS OF SRC DATA

		DMA1->IFCR |= 1;										//FORCE BIT 0 TO A 1 TO CLEAR ALL CHANNEL 1 INTERUPT FLAGS
		DMA1_Channel1->CCR |= 3;								//SET BIT 0 TO 1 TO ENABLE THE DMA TRANSFER. SET BIT 1 TO 1 TO ENABLE TRANSFER COMPLETE INTERUPT

		TIM6->ARR = timeDelays[renderState.currentBamBit];
		TIM6->CR1 |= TIM_CR1_CEN;		//START TIMER
		renderState.streamInProgress = TRUE;

		//hspi1.Init.DataSize = SPI_DATASIZE_16BIT;
		//HAL_SPI_Transmit_DMA(&hspi1, &bamBuffer1[dataPos16Bit], 6);
		GPIOA->BRR |= LED_LATCH_Pin;		//LATCH DATA
	//}
	//else{
	//	renderState.immediateJump = FALSE;
	//	FinaliseLEDData();
	//}

	//HAL_SPI_Transmit_DMA(&hspi1, testData, 12);
	//hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
	//HAL_SPI_Transmit_DMA(&hspi1, testData, 12);


}

void FinaliseLEDData(){
	HAL_GPIO_WritePin(GPIOA,LED_LATCH_Pin,GPIO_PIN_SET);//GPIO_PIN_RESET
	while((hspi1.Instance->SR & SPI_SR_BSY));
	//GPIOA->BSRR |= LED_LATCH_Pin;	//PREPARE CHIP TO LATCH

	HAL_GPIO_WritePin(GPIOA,LED_LATCH_Pin,GPIO_PIN_RESET);

	//DisableRowEn(); 					//DISABLE ALL OUTPUTS ON MOSFET VIA MULTIPLEXER
	HAL_GPIO_WritePin(GPIOB,ROW_SEL_EN_GLK_Pin,GPIO_PIN_SET);
	//GPIOB->BSRR |= ROW_SEL_EN_GLK_Pin;  //DISABLE ALL OUTPUTS ON LED DRIVER. LABELLED "OE" ON CHIP

	SelectRow(renderState.currentRow);

	HAL_GPIO_WritePin(GPIOB,ROW_SEL_EN_GLK_Pin,GPIO_PIN_RESET);
	//GPIOB->BRR |= ROW_SEL_EN_GLK_Pin; //ENABLE LED DRIVER OUTPUT LABELLED "OE" ON CHIP
	//EnableRowEn();

	//GPIOA->BRR |= LED_LATCH_Pin;	//PREPARE CHIP TO LATCH






	//printf("led data callback bam %d %d \n",renderState.currentBamBit,timeDelays[renderState.currentBamBit]);
	//TIM6->ARR = timeDelays[renderState.currentBamBit];


	renderState.currentBamBit++;
	if(renderState.currentBamBit == thisPanel.bamBits){
		renderState.currentBamBit=0;
		renderState.currentRow++;
		if(renderState.currentRow == thisPanel.scanlines){
			renderState.currentRow = 0;
		}
	}
	//if(renderState.currentBamBit ==0){
		//SEND DATA NOW AHEAD OF TIME
	//	uint16_t dataPos8Bit = (renderState.currentRow*(12*thisPanel.bamBits))+(renderState.currentBamBit*12);
	//	uint8_t *data = (uint8_t *)bamBuffer1;
	//	renderState.immediateJump = TRUE;
	//	HAL_SPI_Transmit_DMA(&hspi1, (data+dataPos8Bit), 12);
	//	TIM6->CR1 |= TIM_CR1_CEN;		//START TIMER
	//}

}
