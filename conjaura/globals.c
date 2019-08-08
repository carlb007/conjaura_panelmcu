/*
 * globals.c
 *
 *  Created on: 15 Jul 2019
 *      Author: me
 */
#include "globals.h"

uint8_t * bufferSPI_RX = spiBufferRX;
uint8_t * bufferSPI_TX = spiBufferTX;
uint8_t * returnData = panelReturnData;



//*(bufferSPI_TX) = 1;
//*(bufferSPI_TX+1) = 2;

//*(returnedPanelData) = 254;
//*(returnedPanelData+1) = 255;

void debugPrint(char *data, uint16_t *params){
	#if DEBUGMODE
		printf(data,params);
	#endif
}

//DEFAULT VALS:
void EnsureDefaults(){
	thisPanel.addressSet = FALSE;
	thisPanel.gammaSize = 0;
	globalVals.touchRunning = FALSE;
	globalVals.touchCalibrated = FALSE;
	GPIOB->BSRR |= ROW_SEL_EN_GLK_Pin;  //DISABLE ALL OUTPUTS ON LED DRIVER. LABELLED "OE" ON CHIP
	GPIOA->BRR |= LED_LATCH_Pin;		//ENSURE LATCH IS DEFAULTED LOW
	renderState.firstRender = TRUE;		//MARK AS FIRST RENDER
	renderState.awaitingSwitch = FALSE;	//NO SWITCH NEEDED
}

void Initialise(){
	EnsureDefaults();
	InitTimers();
	InitSPI();
	DMAInit();
	DataToEXT();
	HeaderMode(TRUE);
	LoadAddress();
	InitTouch_ADC();
	debugPrint("Ready\n","");
}
