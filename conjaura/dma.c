/*
 * dma.c
 *
 *  Created on: 2 Aug 2019
 *      Author: me
 */

#include "dma.h"

void DMA1_1_IRQ(){
	DMA1_Channel1->CCR &= ~2;	//CLEAR TRANSFER COMPLETE FLAG
	DMA1->IFCR |= (1|2);		//CLEAR ALL CHANNEL 1 INTERUPT
	DMA1_Channel1->CCR &= ~1;	//DISABLE DMA

	FinaliseLEDData();
}
