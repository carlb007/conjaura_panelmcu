/*
 * data.c
 *
 *  Created on: 24 Jul 2019
 *      Author: me
 */
#include "data.h"

void HeaderMode(uint8_t changeState){
	EnableRS485RX();
	if(changeState){
		globalVals.dataState = AWAITING_HEADER;
	}
	ReceiveSPI2DMA(2);
}

void EnableRS485RX(){
	GPIOA->BRR |= SEL_READ_WRITE_Pin;
	globalVals.rs485RXMode = TRUE;
}

void EnableRS485TX(){
	GPIOA->BSRR |= SEL_READ_WRITE_Pin;
	globalVals.rs485RXMode = FALSE;
}

void DataToLEDs(){
	GPIOB->BSRR |= SEL_MEM_LED_Pin;
	globalVals.dataToLEDs = TRUE;
	while((SPI1->SR & SPI_SR_BSY));
	ConfigLEDDataSPI();
}

void DataToEXT(){
	GPIOB->BRR |= SEL_MEM_LED_Pin;
	globalVals.dataToLEDs = FALSE;
	while((SPI1->SR & SPI_SR_BSY));
	ConfigReturnDataSPI();
}

void DisableRowEn(){
	GPIOA->BSRR |= ROW_SEL_EN_Pin;	//SET ROW SEL HIGH TO DISABLE OUTPUT
}

void EnableRowEn(){
	GPIOA->BRR  |= ROW_SEL_EN_Pin;	//SET ROW SEL LOW TO ENABLE OUTPUT
}



void ParseHeader(){
	globalVals.headerMode = *bufferSPI_RX>>6;
	if (globalVals.headerMode == DATA_MODE){
		globalVals.currentPanelID = 0;
		globalVals.dataState = PANEL_DATA_STREAM;
		DataReceive();
	}
	else if (globalVals.headerMode == COLOUR_MODE){
		ColourHeader();
	}
	else if (globalVals.headerMode == ADDRESS_MODE){
		AddressHeader();
	}
	else if (globalVals.headerMode == CONFIG_MODE){
		ConfigHeader();
	}
}


void DataReceive(){
	//WE WATCH FOR ALL DATA FLYING BY EVEN IF WE DONT NEED IT. ITS THE ONLY WAY WE HAVE OF TRACKING WHERE WERE AT WITHIN THE FLOW
	if(renderState.returnDataMode==FALSE){
		//globalVals.currentPanelSize = panelInfoLookup[globalVals.currentPanelID].edgeByteSize + panelInfoLookup[globalVals.currentPanelID].ledByteSize;
		ReceiveSPI2DMA(panelInfoLookup[globalVals.currentPanelID].edgeByteSize + panelInfoLookup[globalVals.currentPanelID].ledByteSize);
	}
	else{
		ReceiveSPI2DMA(globalVals.currentPanelReturnSize);
	}
}


void HandlePanelData(){
	uint8_t panelIDCache = globalVals.currentPanelID;
	uint8_t totPanels = globalVals.totalPanels;
	renderState.framesSeen++;

	if(renderState.returnDataMode==FALSE){
		if(panelIDCache == thisPanel.address){
			//IMMEDIATELY SWITCH OUR RX BUFFER POINTER SO WE CAN PRESERVE OUR DATA WITHOUT OVERWRITING
			//WE NEED A BIT OF TIME BEFORE WE CAN FULLY PARSE IT BUT STILL NEED TO KEEP TABS ON OTHER DATA FLYING BY
			globalVals.rxBufferLocation = !globalVals.rxBufferLocation;
			if(globalVals.rxBufferLocation==0){
				bufferSPI_RX = spiBufferRX;
			}
			else{
				bufferSPI_RX = spiBufferRXAlt;
			}
			renderState.storedData=FALSE;
			renderState.parsedData=FALSE;
			renderState.waitingProcessing = TRUE;
			renderState.framesReceived++;
		}
		if((panelIDCache+1)==totPanels){
			panelIDCache=0;
			renderState.returnDataMode = TRUE;
		}
	}
	//CAN BECOME ACTIVE STRAIGHT AWAY
	if(renderState.returnDataMode==TRUE){
		uint8_t panelIDCache2 = panelIDCache;
		uint16_t returnSize = 0;

		while(returnSize==0 && panelIDCache2<totPanels){
			panelIDCache = panelIDCache2;
			returnSize = panelInfoLookup[panelIDCache2].touchByteSize + panelInfoLookup[panelIDCache2].periperalByteSize;
			panelIDCache2++;
		}

		if(returnSize>0){
			globalVals.currentPanelReturnSize = returnSize;
			if(panelIDCache == thisPanel.address){
				globalVals.currentPanelID = panelIDCache+1;	//FWD 1 READY FOR AFTER IRQ CALLBACK.
				renderState.waitingToReturn = TRUE;
			}
			else{
				DataReceive();
				globalVals.currentPanelID = panelIDCache+1;
			}
		}
		else{
			globalVals.currentPanelID = 0;
			renderState.returnDataMode = FALSE;
			DataReceive();
		}
	}
	else{
		globalVals.currentPanelID++;
		DataReceive();
	}
}


void SendReturnData(){
	globalVals.dataState = SENDING_DATA_STREAM;
	renderState.waitingToReturn = FALSE;
	renderState.returnedFrames++;
	TIM6->CR1 &= ~1;	//PAUSE LED TIMER
	DataToEXT();
	EnableRS485TX();
	TransmitSPI1DMA(bufferSPI_TX, thisPanel.touchChannels);
}

void FinishDataSend(){
	globalVals.dataState = PANEL_DATA_STREAM;
	while((SPI1->SR & SPI_SR_BSY));
	EnableRS485RX();
	DataToLEDs();
	TIM6->CR1 |= 1;	//RESUME LED TIMER
	HandlePanelData();
	//DataReceive();

}

void SelectRow(uint8_t row){
	switch(row){
		case 0:														//ALL LOW - PIN 13 - 2,0 & 2,2 TOUCH, ROW 0 (BOTTOM)
			GPIOA->BRR |= (ROW_SEL1_Pin | ROW_SEL3_Pin);
			ROW_SEL2_GPIO_Port->BRR |= ROW_SEL2_Pin;
			break;
		case 1:														//1 HIGH - PIN 14 - 1,0 & 1,2 TOUCH, ROW 1
			ROW_SEL2_GPIO_Port->BRR |= ROW_SEL2_Pin;
			GPIOA->BRR |= ROW_SEL3_Pin;
			GPIOA->BSRR |= ROW_SEL1_Pin;
			break;
		case 2:														//2 HIGH - PIN 15 - 3,0 & 3,2 TOUCH, ROW 2
			GPIOA->BRR |= (ROW_SEL1_Pin | ROW_SEL3_Pin);
			ROW_SEL2_GPIO_Port->BSRR |= ROW_SEL2_Pin;
			break;
		case 3:														//3 HIGH - PIN 1 - 0,1 & 0,3 TOUCH, ROW 3
			ROW_SEL2_GPIO_Port->BRR |= ROW_SEL2_Pin;
			GPIOA->BRR |= ROW_SEL1_Pin;
			GPIOA->BSRR |= ROW_SEL3_Pin;
			break;
		case 4:														//3 + 2 HIGH - PIN 2 - 1,1 & 1,3 TOUCH, ROW 4
			GPIOA->BRR |= ROW_SEL1_Pin;
			GPIOA->BSRR |= ROW_SEL3_Pin;
			ROW_SEL2_GPIO_Port->BSRR |= ROW_SEL2_Pin;
			break;
		case 5:														//ALL HIGH - PIN 4 - 3,1 & 3,3 TOUCH, ROW 5
			GPIOA->BSRR |= (ROW_SEL1_Pin | ROW_SEL3_Pin);
			ROW_SEL2_GPIO_Port->BSRR |= ROW_SEL2_Pin;
			break;
		case 6:														//3 + 1 HIGH - PIN 5 - 2,1 & 2,3 TOUCH, ROW 6
			ROW_SEL2_GPIO_Port->BRR |= ROW_SEL2_Pin;
			GPIOA->BSRR |= (ROW_SEL1_Pin | ROW_SEL3_Pin);
			break;
		case 7:														//1 + 2 HIGH - PIN 12 - 0,0 & 0,2 TOUCH, ROW 7
			GPIOA->BRR |= ROW_SEL3_Pin;
			GPIOA->BSRR |= ROW_SEL1_Pin;
			ROW_SEL2_GPIO_Port->BSRR |= ROW_SEL2_Pin;
			break;
	}
}
