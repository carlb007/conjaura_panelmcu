/*
 * data.c
 *
 *  Created on: 24 Jul 2019
 *      Author: me
 */
#include "data.h"

extern SPI_HandleTypeDef hspi1,hspi2;

void HeaderMode(uint8_t changeState){
	EnableRS485RX();
	if(changeState){
		globalVals.dataState = AWAITING_HEADER;
	}
	HAL_SPI_Receive_DMA(&hspi2, bufferSPI_RX, 2);
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
}

void DataToEXT(){
	GPIOB->BRR |= SEL_MEM_LED_Pin;
	globalVals.dataToLEDs = FALSE;
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
	globalVals.currentPanelSize = panelInfoLookup[globalVals.currentPanelID].edgeByteSize + panelInfoLookup[globalVals.currentPanelID].ledByteSize;
	HAL_SPI_Receive_DMA(&hspi2, bufferSPI_RX, globalVals.currentPanelSize);
	InitTouch_ADC();
	globalVals.dataState = PANEL_DATA_STREAM;
	globalVals.currentPanelReturnSize = panelInfoLookup[globalVals.currentPanelID].touchByteSize + panelInfoLookup[globalVals.currentPanelID].periperalByteSize;
}

void HandlePanelData(){
	//debugPrint("GOT DATA \n","");
	if(globalVals.currentPanelID == thisPanel.address){

		//RETURN OUR DATA IF NEEDED...
		if(thisPanel.touchActive){
			//EnableRS485TX();
			//DataToEXT();
			//for(uint8_t ch=0;ch<thisPanel.touchChannels;ch++){
			//	bufferSPI_TX[ch] = thisPanel.touchChannel[ch].value;
			//}
			//globalVals.dataState = SENDING_DATA_STREAM;
			//globalVals.pauseOutput = TRUE;
			//HAL_SPI_Transmit_DMA(&hspi1, bufferSPI_TX, 16);
		}
		renderState.storedData=FALSE;
		renderState.parsedData=FALSE;
		renderState.framesReceived++;
		//WE NEED TO STORE OUR DATA ASAP BEFORE NEXT DATA ARRIVES...
		ConvertRawPixelData();
	}
	else{
		if(globalVals.currentPanelReturnSize){
			//KEEP AN EYE OUT FOR THE RETURN DATA WHIZZING BY IF THERES GOING TO BE ANY PRESENT
			//EVEN THOUGH WE DONT NEED IT WE NEED TO TRACK WHEN THE NEXT PANEL DATA IS GOING TO ARRIVE
			globalVals.dataState = PANEL_RETURN_STREAM;
			HAL_SPI_Receive_DMA(&hspi2, bufferSPI_RX, globalVals.currentPanelReturnSize);
		}
		else{
			HandleReturnData();
		}
	}

}

void HandleReturnData(){
	globalVals.currentPanelID++;
	if(globalVals.currentPanelID==globalVals.totalPanels){
		globalVals.currentPanelID=0;
	}
	DataReceive();
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi){
	if(globalVals.dataState == PANEL_DATA_STREAM){
		HandlePanelData();
	}
	else if(globalVals.dataState == PANEL_RETURN_STREAM){
		HandleReturnData();
	}
	else if(globalVals.dataState == AWAITING_HEADER){
		ParseHeader();
	}
	else if(globalVals.dataState == AWAITING_ADDRESS_CALLS){
		AddressHeader();
	}
	else if(globalVals.dataState == AWAITING_PALETTE_DATA){
		HandlePaletteData();
	}
	else if(globalVals.dataState == AWAITING_GAMMA_DATA){
		HandleGammaData();
	}
	else if(globalVals.dataState == AWAITING_CONF_DATA){
		HandleConfigData();
	}
}


void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi){
	if(globalVals.dataState == SENDING_DATA_STREAM){
		EnableRS485RX();
		DataToLEDs();
		globalVals.pauseOutput = FALSE;
		HandleReturnData();
	}
	else if(globalVals.dataState == SENDING_ADDRESS_CALL){
		globalVals.dataState = AWAITING_ADDRESS_CALLS;
		HeaderMode(FALSE);
	}
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
