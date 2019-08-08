/*
 * colour.c
 *
 *  Created on: 26 Jul 2019
 *      Author: me
 */
#include "colour.h"

extern SPI_HandleTypeDef hspi2;

void ColourHeader(){
	thisPanel.colourMode = (*bufferSPI_RX>>4) & 0x3;
	thisPanel.biasHC = (*bufferSPI_RX>>2) & 0x3;
	thisPanel.bamBits = 5+(*bufferSPI_RX & 0x3);
	thisPanel.paletteSize = 0;
	if(thisPanel.colourMode == PALETTE_COLOUR){
		thisPanel.paletteSize = *(bufferSPI_RX+1);
		globalVals.dataState = AWAITING_PALETTE_DATA;
		//HAL_SPI_Receive_DMA(&hspi2, bufferSPI_RX, (thisPanel.paletteSize+1)*3);
		ReceiveSPI2DMA((thisPanel.paletteSize+1)*3);
	}
	else{
		HeaderMode(TRUE);
	}
	//debugPrint("GOT COLOUR MODE %d \n",(uint16_t*)thisPanel.colourMode);
}

void HandlePaletteData(){
	for(uint16_t thisColour=0; thisColour<thisPanel.paletteSize+1; thisColour++){
		paletteR[thisColour] = *(bufferSPI_RX+(thisColour*3));
		paletteG[thisColour] = *(bufferSPI_RX+(thisColour*3)+1);
		paletteB[thisColour] = *(bufferSPI_RX+(thisColour*3)+2);
	}
	HeaderMode(TRUE);
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
	for(uint16_t gamR=0; gamR<thisPanel.gammaRLength; gamR++){
		gammaR[gamR] = *(bufferSPI_RX+gamR);
	}
	for(uint16_t gamG=0; gamG<thisPanel.gammaGLength; gamG++){
		gammaG[gamG] = *(bufferSPI_RX+thisPanel.gammaRLength+gamG);
	}
	for(uint16_t gamB=0; gamB<thisPanel.gammaBLength; gamB++){
		gammaB[gamB] = *(bufferSPI_RX+thisPanel.gammaRLength+thisPanel.gammaGLength+gamB);
	}
	HeaderMode(TRUE);
}
