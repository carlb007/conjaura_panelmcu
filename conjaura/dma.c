/*
 * dma.c
 *
 *  Created on: 2 Aug 2019
 *      Author: me
 */

#include "dma.h"


void DMAInit(){
	DMA1_1_Init();
	DMA1_23_Init();
}

void DMA1_1_Init(){
	//CHANNEL 1 IS USED FOR SPI1 TX MODE ONLY

	DMA1_Channel1->CCR = 0;				//CLEAR ALL BITS
	DMAMUX1_ChannelStatus->CFR |= 1;	//CLEAR CHANNEL 1 OVERRUN FLAG
	DMA1->IFCR |= (1|2);				//CLEAR CHANNEL 1 INTERUPTS
	DMA1_Channel1->CNDTR = 0;			//SET DATA LEN TO ZERO

	DMA1_Channel1->CCR |= 2;			//SET BIT 2 TO 1. TRANSFER COMPLETE INTERUPT ENABLED
	DMA1_Channel1->CCR |= 16;			//SET BIT 4 TO 1. DIRECTION - MEM TO PERIPH;
	DMA1_Channel1->CCR |= 128;			//SET BIT 8 TO 1. MEM INCREMENT MODE.
	DMA1_Channel1->CCR |= 8192;			//SET 13th > 12th BITS TO 1,0. HIGH PRIORITY MODE (LEV 3/4)

	DMAMUX1_Channel0->CCR = 17;			//SET MUX CHANNEL TO SPI 1 TX.
}

void DMA1_23_Init(){
	//CHANNEL 2 IS USED FOR SPI2 RX ONLY
	DMA1_Channel2->CCR = 0;				//CLEAR ALL BITS
	DMAMUX1_ChannelStatus->CFR |= 2;	//CLEAR CHANNEL 2 OVERRUN FLAG
	DMA1->IFCR |= (16|32);				//CLEAR CHANNEL 1 INTERUPTS
	DMA1_Channel2->CNDTR = 0;			//SET DATA LEN TO ZERO

	DMA1_Channel2->CCR |= 2;			//SET BIT 2 TO 1. TRANSFER COMPLETE INTERUPT ENABLED
	DMA1_Channel2->CCR &= ~16;			//SET BIT 4 TO 0. DIRECTION - PERIPH TO MEM;
	DMA1_Channel2->CCR |= 128;			//SET BIT 8 TO 1. MEM INCREMENT MODE.
	DMA1_Channel2->CCR |= 8192;			//SET 13th > 12th BITS TO 1,0. HIGH PRIORITY MODE (LEV 3/4)

	DMAMUX1_Channel1->CCR = 18;			//SET MUX CHANNEL TO SPI 2 RX.
}

void DMA1_1_IRQ(){
	//printf("Called\n");
	DMA1_Channel1->CCR &= ~2;	//CLEAR TRANSFER COMPLETE FLAG
	DMA1->IFCR |= (1|2);		//CLEAR ALL CHANNEL 1 INTERUPT
	DMA1_Channel1->CCR &= ~1;	//DISABLE DMA

	if(renderState.immediateJump==FALSE){
		FinaliseLEDData();
	}
}


void DMA1_23_IRQ(){
	DMA1_Channel2->CCR &= ~2;	//CLEAR TRANSFER COMPLETE FLAG
	DMA1->IFCR |= (16|32);		//CLEAR ALL CHANNEL 2 INTERUPT
	DMA1_Channel2->CCR &= ~1;	//DISABLE DMA

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
