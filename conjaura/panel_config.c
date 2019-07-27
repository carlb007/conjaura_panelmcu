/*
 * panel_config.c
 *
 *  Created on: 26 Jul 2019
 *      Author: me
 */

#include "panel_config.h"

extern SPI_HandleTypeDef hspi2;

void ConfigHeader(){
	globalVals.configSubMode = (*bufferSPI_RX>>4) & 0x3;
	if(globalVals.configSubMode == PANEL_INF){
		globalVals.totalPanels  = *(bufferSPI_RX+1);
		globalVals.dataState = AWAITING_CONF_DATA;
		HAL_SPI_Receive_DMA(&hspi2, bufferSPI_RX, globalVals.totalPanels*4);
	}
	else if(globalVals.configSubMode == GAMMA){
		globalVals.dataState = AWAITING_GAMMA_DATA;
		GammaSetup();
		HAL_SPI_Receive_DMA(&hspi2, bufferSPI_RX, thisPanel.gammaSize);
	}
}

void HandleConfigData(){
	uint8_t bytesPerLED;
	if(thisPanel.colourMode == TRUE_COLOUR){
		bytesPerLED = 3;
	}
	else if(thisPanel.colourMode == HIGH_COLOUR){
		bytesPerLED = 2;
	}
	else if(thisPanel.colourMode == PALETTE_COLOUR){
		bytesPerLED = 1;
	}
	for(uint16_t pid=0;pid<globalVals.totalPanels;pid++){
		uint32_t offset = pid*4;
		uint8_t w = *(bufferSPI_RX+offset) >>6;			//0 = 8, 1 = 16, 2 = 24, 3 = 32
		uint8_t h = (*(bufferSPI_RX+offset) >>4)&0x3;
		w = (w+1)*8;
		h = (h+1)*8;
		if(pid==thisPanel.address){
			thisPanel.width = w;
			thisPanel.height = h;
			thisPanel.pixelCount = thisPanel.width * thisPanel.height;
			thisPanel.orientation = (*(bufferSPI_RX+offset) >>2)&0x3;
			thisPanel.scanlines = (*(bufferSPI_RX+offset) >>1)&0x1;

			offset++;
			thisPanel.outputEn  = *(bufferSPI_RX+offset) >>7;
			thisPanel.outputThrottle = (*(bufferSPI_RX+offset) >>5)&0x3;

			offset++;
			thisPanel.touchActive  = *(bufferSPI_RX+offset) >>7;
			uint8_t tmpTouchChannels = (*(bufferSPI_RX+offset)>>5) & 0x3;
			if(tmpTouchChannels==0){
				thisPanel.touchChannels = (w/4) * (h/4);
			}
			thisPanel.touchBits = (*(bufferSPI_RX+offset) >>4)&0x1;
			thisPanel.edgeActive = (*(bufferSPI_RX+offset) >>3)&0x1;
			thisPanel.edgeThrottle = (*(bufferSPI_RX+offset) >>2)&0x1;
			thisPanel.edgeDensity = *(bufferSPI_RX+offset)&0x3;

			offset++;
			thisPanel.periperalActive  = *(bufferSPI_RX+offset) >>5;
			thisPanel.peripheralSettings  = (*(bufferSPI_RX+offset) >>3)&0x3;
			thisPanel.peripheralSizeFlag  = (*(bufferSPI_RX+offset) >>1)&0x3;

			//RESET OFFSET:
			offset = pid*4;
		}

		offset+=2;

		uint8_t touchActive = *(bufferSPI_RX+offset)>>7;
		uint8_t touchChannels = (*(bufferSPI_RX+offset)>>5) & 0x3;
		uint8_t touchBits = (*(bufferSPI_RX+offset)>>4) & 0x1;
		uint8_t edgeActive = (*(bufferSPI_RX+offset) >>3)&0x1;
		uint8_t edgeDensity = *(bufferSPI_RX+offset)&0x3;

		uint8_t touchBytes = 0;
		uint8_t touchChannelCount = 0;
		uint8_t edgeBytes = 0;

		if(touchActive){
			if(touchChannels==0){
				touchChannelCount = (w/4) * (h/4);
			}
			if(touchBits == 1){		//BYTE SIZE
				touchBytes = touchChannelCount;
			}
			else{					//NIBBLE SIZE
				touchBytes = touchChannelCount/2;
			}
		}

		if(edgeActive){
			if(edgeDensity == 0){		//3 PER 8
				edgeBytes = ((((w * 2)+(h*2))/8)*3)*bytesPerLED;
			}
			else if(edgeDensity == 1){	//6 PER 8
				edgeBytes = ((((w * 2)+(h*2))/8)*6)*bytesPerLED;
			}
		}

		offset++;
		uint8_t peripheralType = *(bufferSPI_RX+offset) >>5;
		uint8_t periphBytes = 0;
		//PERIPHERAL SIZING STILL TO BE IMPLEMENTED.

		panelInfoLookup[pid].ledByteSize = bytesPerLED*(w*h);
		panelInfoLookup[pid].edgeByteSize = edgeBytes;
		panelInfoLookup[pid].periperalByteSize = periphBytes;
		panelInfoLookup[pid].touchByteSize = touchBytes;
	}
	HeaderMode(TRUE);
}
