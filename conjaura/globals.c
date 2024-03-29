/*
 * globals.c
 *
 *  Created on: 15 Jul 2019
 *      Author: me
 */
#include "globals.h"

uint8_t * bufferSPI_RX = spiBufferRX;	//spiBufferRXAlt
uint8_t * bufferSPI_TX = spiBufferTX;



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
	globalVals.watchdogRunning = FALSE;
	thisPanel.addressSet = FALSE;
	thisPanel.gammaSize = 0;
	globalVals.touchRunning = FALSE;
	globalVals.touchCalibrated = FALSE;
	GPIOB->BSRR |= ROW_SEL_EN_GLK_Pin;  //DISABLE ALL OUTPUTS ON LED DRIVER. LABELLED "OE" ON CHIP
	GPIOA->BRR |= LED_LATCH_Pin;		//ENSURE LATCH IS DEFAULTED LOW

	renderState.firstRender = TRUE;		//MARK AS FIRST RENDER
	renderState.drawBufferSwitchPending = FALSE;	//NO SWITCH NEEDED
	renderState.edgeComplete = TRUE;
	globalVals.currentPanelID=0;
	renderState.returnDataMode=FALSE;
}

void Initialise(){
	DisableEdgeLights();
	EnsureDefaults();
	InitTimers();
	InitSPI();
	DMAInit();
	ADC1->CR |= ADC_CR_ADEN;
	DataToEXT();
	HeaderMode(TRUE);
	LoadAddress();
	InitTouch_ADC();
	EnableEdgeLights();
	debugPrint("Ready\n","");
}

void InitWatchdog(){
	//ENABLE WATCHDOG
	IWDG->KR = IWDG_KEY_ENABLE;
	//PROVIDE WRITE ACCESS
	IWDG->KR = IWDG_KEY_WRITE_ACCESS_ENABLE;
	//PRESCALER SET. 32KHZ CLK. 0 = PSC /4, 1 = PSC /8;
	IWDG->PR = 1;
	//RELOAD VALUE;
	IWDG->RLR = 800;	//PSC 4 AND 800 = 0.1 seconds watchdog reset.
	//WAIT TIL READY
	while (IWDG->SR != 0x00u);
	//WatchdogRefresh();
	globalVals.watchdogRunning = TRUE;
}

void WatchdogRefresh(){
	//REFRESH COUNTER VALUE
	IWDG->KR = IWDG_KEY_RELOAD;
}
