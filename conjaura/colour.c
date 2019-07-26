/*
 * colour.c
 *
 *  Created on: 26 Jul 2019
 *      Author: me
 */
#include "colour.h"

extern SPI_HandleTypeDef hspi2;

uint8_t * bufferPalette = paletteBuffer;
uint8_t  * gammaDataR = gammaR;
uint8_t  * gammaDataG = gammaG;
uint8_t  * gammaDataB = gammaB;

void ColourHeader(){
	thisPanel.colourMode = (*bufferSPI_RX>>4) & 0x3;
	thisPanel.biasHC = (*bufferSPI_RX>>2) & 0x3;
	thisPanel.bamBits = *bufferSPI_RX>>2 & 0x3;
	thisPanel.paletteSize = 0;
	if(thisPanel.colourMode == PALETTE_COLOUR){
		thisPanel.paletteSize = *(bufferSPI_RX+1);
		globalVals.dataMode = AWAITING_PALETTE_DATA;
		HAL_SPI_Receive_DMA(&hspi2, bufferSPI_RX, thisPanel.paletteSize);
	}
	else{
		HeaderMode(TRUE);
	}
	debugPrint("GOT COLOUR MODE %d \n",(uint16_t*)thisPanel.colourMode);
}

void HandlePaletteData(){
	for(uint16_t thisColour=0; thisColour<=thisPanel.paletteSize; thisColour++){
		uint8_t R = *(bufferSPI_RX+(thisColour*3));
		uint8_t G = *(bufferSPI_RX+(thisColour*3)+1);
		uint8_t B = *(bufferSPI_RX+(thisColour*3)+2);
		uint16_t colourTarget = thisColour*3;
		*(bufferPalette+colourTarget) = R;
		*(bufferPalette+colourTarget+1) = G;
		*(bufferPalette+colourTarget+2) = B;
	}
	HeaderMode(TRUE);
	debugPrint("GOT PALETTE %d \n",(uint16_t*)thisPanel.paletteSize);
}


void GammaSetup(){
	if(thisPanel.colourMode!=HIGH_COLOUR){
		thisPanel.gammaRLength = 256;
		thisPanel.gammaGLength = 256;
		thisPanel.gammaBLength = 256;
	}
	else{
		thisPanel.gammaRLength = 32;
		thisPanel.gammaGLength = 32;
		thisPanel.gammaBLength = 32;
		if(thisPanel.biasHC==0){		// 5/6/5 LEVELS
			thisPanel.gammaGLength = 64;
		}
		else if(thisPanel.biasHC==1){		// 6/5/5 LEVELS
			thisPanel.gammaRLength = 64;
		}
		else if(thisPanel.biasHC==2){		// 5/5/6 LEVELS
			thisPanel.gammaBLength = 64;
		}
		else if(thisPanel.biasHC==3){	// 5/5/5 LEVELS
			//DO NOTHING
		}
	}
	thisPanel.gammaSize = thisPanel.gammaRLength + thisPanel.gammaGLength + thisPanel.gammaBLength;
}

void HandleGammaData(){
	for(uint8_t gamR=0; gamR<thisPanel.gammaRLength; gamR++){
		*(gammaDataR+gamR) = *(bufferSPI_RX+gamR);
	}
	for(uint8_t gamG=0; gamG<thisPanel.gammaGLength; gamG++){
		*(gammaDataG+gamG) = *(bufferSPI_RX+thisPanel.gammaRLength+gamG);
	}
	for(uint8_t gamB=0; gamB<thisPanel.gammaBLength; gamB++){
		*(gammaDataG+gamB) = *(bufferSPI_RX+thisPanel.gammaRLength+thisPanel.gammaGLength+gamB);
	}
	HeaderMode(TRUE);
}
