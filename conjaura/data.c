/*
 * data.c
 *
 *  Created on: 24 Jul 2019
 *      Author: me
 */
#include "data.h"

extern SPI_HandleTypeDef hspi1,hspi2;

void Initialise(){
	debugPrint("Ready\n","");
	DataToEXT();
	HeaderMode(TRUE);
}

void HeaderMode(uint8_t changeState){
	EnableRS485RX();
	if(changeState){
		globalVals.dataState = AWAITING_HEADER;
	}
	HAL_SPI_Receive_DMA(&hspi2, bufferSPI_RX, 2);
}

void EnableRS485RX(){
	GPIOA->BRR = SEL_READ_WRITE_Pin;
	globalVals.rs485RXMode = TRUE;
}

void EnableRS485TX(){
	GPIOA->BSRR = SEL_READ_WRITE_Pin;
	globalVals.rs485RXMode = FALSE;
}

void DataToLEDs(){
	GPIOA->BSRR = SEL_MEM_LED_Pin;
	globalVals.dataToLEDs = TRUE;
}

void DataToEXT(){
	GPIOA->BRR = SEL_MEM_LED_Pin;
	globalVals.dataToLEDs = FALSE;
}

void ParseHeader(){
	globalVals.dataMode = *bufferSPI_RX>>6;
	if (globalVals.dataMode == COLOUR_MODE){
		globalSettings.colourMode = (*bufferSPI_RX>>4) & 0x3;
		globalSettings.biasHC = (*bufferSPI_RX>>2) & 0x3;
		globalSettings.bamBits = *bufferSPI_RX>>2 & 0x3;
		globalSettings.paletteSize = 0;
		if(globalSettings.colourMode == PALETTE_COLOUR){
			globalSettings.paletteSize = *(bufferSPI_RX+1);
			globalVals.dataMode = AWAITING_PALETTE_DATA;
			HAL_SPI_Receive_DMA(&hspi2, bufferSPI_RX, globalSettings.paletteSize);
		}
		else{
			HeaderMode(TRUE);
		}
		debugPrint("GOT COLOUR MODE %d \n",(uint16_t*)globalSettings.colourMode);
	}
	else if (globalVals.dataMode == ADDRESS_MODE){
		AddressHeader();
	}
	else if (globalVals.dataMode == CONFIG_MODE){
		globalVals.configSubMode = (*bufferSPI_RX>>4) & 0x3;
		globalSettings.totalPanels  = (*bufferSPI_RX+1);
		if(globalVals.configSubMode == PANEL_INF){
			globalVals.dataMode = AWAITING_CONF_DATA;
			HAL_SPI_Receive_DMA(&hspi2, bufferSPI_RX, globalSettings.totalPanels*4);
		}
		else if(globalVals.configSubMode == GAMMA){
			globalVals.dataMode = AWAITING_GAMMA_DATA;
			uint16_t gamSize = 768;
			if(globalSettings.colourMode==HIGH_COLOUR){
				if(globalSettings.biasHC==3){
					gamSize = 96;	// 5/5/5
				}
				else{
					gamSize = 128;	// 5/6/5 or 6/5/5 or 5/5/6
				}
			}
			HAL_SPI_Receive_DMA(&hspi2, bufferSPI_RX, gamSize);
		}
	}
	else{
		debugPrint("unknown %d \n",(uint16_t*)*bufferSPI_RX);
	}
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi){
	if(globalVals.dataState == AWAITING_HEADER){
		ParseHeader();
	}
}


void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi){
	if(globalVals.dataState == SENDING_ADDRESS_CALL){
		HeaderMode(TRUE);
		debugPrint("Sent Address\n","");
	}
}

void selectRow(uint8_t row){
	GPIOA->BSRR = ROW_SEL_EN_Pin;	//SET ROW SEL HIGH TO DISABLE OUTPUT

	switch(row){
		case 0:														//ALL LOW - PIN 13 - 2,0 & 2,2 TOUCH
			GPIOA->BRR = (ROW_SEL1_Pin | ROW_SEL3_Pin);
			ROW_SEL2_GPIO_Port->BRR = ROW_SEL2_Pin;
			break;
		case 1:														//1 HIGH - PIN 14 - 1,0 & 1,2 TOUCH
			ROW_SEL2_GPIO_Port->BRR = ROW_SEL2_Pin;
			GPIOA->BRR = ROW_SEL3_Pin;
			GPIOA->BSRR = ROW_SEL1_Pin;
			break;
		case 2:														//2 HIGH - PIN 15 - 3,0 & 3,2 TOUCH*
			GPIOA->BRR = (ROW_SEL1_Pin | ROW_SEL3_Pin);
			ROW_SEL2_GPIO_Port->BSRR = ROW_SEL2_Pin;
			break;
		case 3:														//1 + 2 HIGH - PIN 12 - 0,0 & 0,2 TOUCH*
			GPIOA->BRR = ROW_SEL3_Pin;
			GPIOA->BSRR = ROW_SEL1_Pin;
			ROW_SEL2_GPIO_Port->BSRR = ROW_SEL2_Pin;
			break;
		case 4:														//3 HIGH - PIN 1 - 0,1 & 0,3 TOUCH
			ROW_SEL2_GPIO_Port->BRR = ROW_SEL2_Pin;
			GPIOA->BRR = ROW_SEL1_Pin;
			GPIOA->BSRR = ROW_SEL3_Pin;
			break;
		case 5:														//3 + 1 HIGH - PIN 5 - 2,1 & 2,3 TOUCH
			ROW_SEL2_GPIO_Port->BRR = ROW_SEL2_Pin;
			GPIOA->BSRR = (ROW_SEL1_Pin | ROW_SEL3_Pin);
			break;
		case 6:														//3 + 2 HIGH - PIN 2 - 1,1 & 1,3 TOUCH*
			GPIOA->BRR = ROW_SEL1_Pin;
			GPIOA->BSRR = ROW_SEL3_Pin;
			ROW_SEL2_GPIO_Port->BSRR = ROW_SEL2_Pin;
			break;
		case 7:														//ALL HIGH - PIN 4 - 3,1 & 3,3 TOUCH*
			GPIOA->BSRR = (ROW_SEL1_Pin | ROW_SEL3_Pin);
			ROW_SEL2_GPIO_Port->BSRR = ROW_SEL2_Pin;
			break;
	}

	GPIOA->BSRR  = ROW_SEL_EN_Pin;	//SET ROW SEL LOW TO ENABLE OUTPUT
}
