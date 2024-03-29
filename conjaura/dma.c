/*
 * dma.c
 *
 *  Created on: 2 Aug 2019
 *      Author: me
 */

#include "dma.h"

extern DMA_HandleTypeDef hdma_usart3_tx;
extern UART_HandleTypeDef huart3;

void DMAInit(){
	__HAL_RCC_DMA1_CLK_ENABLE();

	  /* DMA interrupt init */
	  /* DMA1_Channel1_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
	  /* DMA1_Channel2_3_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA1_Channel2_3_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);
	  /* DMA1_Ch4_7_DMAMUX1_OVR_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA1_Ch4_7_DMAMUX1_OVR_IRQn, 3, 0);
	HAL_NVIC_EnableIRQ(DMA1_Ch4_7_DMAMUX1_OVR_IRQn);

	DMA1_1_Init();
	DMA1_23_Init();
	DMA1_4_Init();
	DMA1_7_Init();
}

void DMA1_1_Init(){
	//CHANNEL 1 IS USED FOR SPI1 TX MODE ONLY

	DMA1_Channel1->CCR = 0;				//CLEAR ALL BITS
	DMAMUX1_ChannelStatus->CFR |= 1;	//CLEAR CHANNEL 1 OVERRUN FLAG
	DMA1->IFCR |= (1|2);				//CLEAR CHANNEL 1 INTERUPTS
	DMA1_Channel1->CNDTR = 0;			//SET DATA LEN TO ZERO
	DMA1_Channel1->CPAR = (uint32_t)&SPI1->DR;		//PERIPHERAL ADDRESS TARGET (SPI DATA REGISTER)
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
	DMA1->IFCR |= (16|32);				//CLEAR CHANNEL 4 INTERUPTS
	DMA1_Channel2->CNDTR = 0;			//SET DATA LEN TO ZERO
	DMA1_Channel2->CPAR = (uint32_t)&SPI2->DR;					//ADDRESS OF SRC DATA
	DMA1_Channel2->CMAR = (uint32_t)bufferSPI_RX;				//PERIPHERAL ADDRESS SOURCE (SPI DATA REGISTER)
	DMA1_Channel2->CCR |= 2;			//SET BIT 2 TO 1. TRANSFER COMPLETE INTERUPT ENABLED
	DMA1_Channel2->CCR &= ~16;			//SET BIT 4 TO 0. DIRECTION - PERIPH TO MEM;
	DMA1_Channel2->CCR |= 128;			//SET BIT 8 TO 1. MEM INCREMENT MODE.
	DMA1_Channel2->CCR |= 12288;		//SET 13th > 12th BITS TO 1,0. HIGHEST PRIORITY MODE (LEV 4/4)

	DMAMUX1_Channel1->CCR = 18;			//SET MUX CHANNEL TO SPI 2 RX.
}

void DMA1_4_Init(){
	//CHANNEL 4 IS USED FOR ADC RX ONLY
	DMA1_Channel4->CCR = 0;				//CLEAR ALL BITS
	DMAMUX1_ChannelStatus->CFR |= 8;	//CLEAR CHANNEL 4 OVERRUN FLAG
	DMA1->IFCR |= (4096|8192);			//CLEAR CHANNEL 1 INTERUPTS
	DMA1_Channel4->CNDTR = 0;			//SET DATA LEN TO ZERO

	DMA1_Channel4->CCR |= 2;			//SET BIT 2 TO 1. TRANSFER COMPLETE INTERUPT ENABLED
	DMA1_Channel4->CCR &= ~16;			//SET BIT 4 TO 0. DIRECTION - PERIPH TO MEM;
	DMA1_Channel4->CCR |= 128;			//SET BIT 8 TO 1. MEM INCREMENT MODE.
	DMA1_Channel4->CCR |= 4096;			//SET 13th > 12th BITS TO 0,1. MED PRIORITY MODE (LEV 2/4)

	DMAMUX1_Channel3->CCR = 5;			//SET MUX CHANNEL TO ADC.
}

void DMA1_7_Init(){
	//CHANNEL 7 IS USED FOR EDGE DATA TX ONLY
	DMA1_Channel7->CCR = 0;				//CLEAR ALL BITS
	DMAMUX1_ChannelStatus->CFR |= 64;	//CLEAR CHANNEL 4 OVERRUN FLAG
	DMA1->IFCR |= (16777216|33554432);	//CLEAR CHANNEL 1 INTERUPTS
	DMA1_Channel7->CNDTR = 0;			//SET DATA LEN TO ZERO

	DMA1_Channel7->CCR |= 2;			//SET BIT 2 TO 1. TRANSFER COMPLETE INTERUPT ENABLED
	DMA1_Channel7->CCR |= 16;			//SET BIT 4 TO 0. DIRECTION - PERIPH TO MEM;
	DMA1_Channel7->CCR |= 128;			//SET BIT 8 TO 1. MEM INCREMENT MODE.
	DMA1_Channel7->CCR |= 4096;			//SET 13th > 12th BITS TO 0,1. MED PRIORITY MODE (LEV 2/4)

	DMA1_Channel7->CPAR = (uint32_t)&USART3->TDR;		//PERIPHERAL ADDRESS TARGET (SPI DATA REGISTER) - SET ON INIT
	DMA1_Channel7->CMAR = (uint32_t)edgeCompiled;		//ADDRESS OF SRC DATA

	DMAMUX1_Channel6->CCR = 55;			//SET MUX CHANNEL TO USART3 TX.
}

void DMA1_1_IRQ(){
	DMA1_Channel1->CCR &= ~3;	//CLEAR TRANSFER COMPLETE FLAG AND DISABLE
	DMA1->IFCR |= (1|2);		//CLEAR ALL CHANNEL 1 INTERUPT
	//DMA1_Channel1->CCR &= ~1;	//DISABLE DMA

	if(globalVals.dataState == PANEL_DATA_STREAM){
		//if(renderState.immediateJump==FALSE){
			FinaliseLEDData();
		//}
	}
	else if(globalVals.dataState == SENDING_DATA_STREAM){
		FinishDataSend();
	}
	else if(globalVals.dataState == SENDING_ADDRESS_CALL){
		globalVals.dataState = AWAITING_ADDRESS_CALLS;
		HeaderMode(FALSE);
	}
}


void DMA1_23_IRQ(){
	DMA1_Channel2->CCR &= ~3;	//CLEAR TRANSFER COMPLETE FLAG AND DISABLE
	DMA1->IFCR |= (16|32);		//CLEAR ALL CHANNEL 2 INTERUPT
	//DMA1_Channel2->CCR &= ~1;	//DISABLE DMA

	if(globalVals.dataState == PANEL_DATA_STREAM){			//WAITING TO SEE NEXT PANELS LED DATA
		HandlePanelData();
	}
	else{
		if(globalVals.dataState == AWAITING_HEADER){
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
}

void DMA1_47_IRQ(){

	if(globalVals.touchRunning == TRUE){
		DMA1_Channel4->CCR &= ~3;	//CLEAR TRANSFER COMPLETE FLAG AND DISABLE
		DMA1->IFCR |= (4096|8192);	//CLEAR ALL CHANNEL 1 INTERUPT
		//DMA1_Channel4->CCR &= ~1;	//DISABLE DMA
		ADCConversionComplete();
	}
	else{
		renderState.edgeComplete = TRUE;
		DMA1_Channel7->CCR &= ~3;	//CLEAR TRANSFER COMPLETE FLAG AND DISABLE
		DMA1->IFCR |= (16777216|33554432);	//CLEAR ALL CHANNEL 1 INTERUPT
		//DMA1_Channel7->CCR &= ~1;	//DISABLE DMA
	}
}


void ReceiveSPI2DMA(uint16_t len){
	//FLIP FLOP OUR RX BUFFER POINTER SO WE DONT OVER WRITE BEFORE WEVE SAVED AND PARSED
	//if(globalVals.dataState == PANEL_DATA_STREAM){
	//	globalVals.bufferFlipFlopState = !globalVals.bufferFlipFlopState;
	//	if(globalVals.bufferFlipFlopState==0){
	//		bufferSPI_RX = spiBufferRX;
	//	}
	//	else{
	//		bufferSPI_RX = spiBufferRXAlt;
			//Runs First Data
	//	}
	//}

	//DMA1_Channel2->CCR &= ~769;								//SET BIT 0 TO 0 TO DISABLE DMA. SET BITS 8 and 9 to 0. CLEAR THE PERIPHERAL DATA SIZE. 00 SET US IN 8BIT MODE.

	DMA1_Channel2->CNDTR = len;									//DATA LENGTH OF TRANSFER
	//DMA1_Channel2->CPAR = (uint32_t)&SPI2->DR;				//ADDRESS OF SRC DATA - SET ON INIT
	DMA1_Channel2->CMAR = (uint32_t)bufferSPI_RX;				//PERIPHERAL ADDRESS SOURCE (SPI DATA REGISTER) - SET ON INIT. ALWAYS BUFFER ADDR 0.
	DMA1_Channel2->CCR |= 3;									//SET BIT 0 TO 1 TO ENABLE THE DMA TRANSFER. SET BIT 1 TO 1 TO ENABLE TRANSFER COMPLETE INTERUPT
	WatchdogRefresh();

}

void TransmitSPI1DMA(uint8_t *ptr, uint16_t len){
	//DMA1_Channel1->CCR &= ~769;								//SET BIT 0 TO 0 TO DISABLE DMA. SET BITS 8 and 9 to 0. CLEAR THE PERIPHERAL DATA SIZE. 00 SET US IN 8BIT MODE.
	//DMA1_Channel1->CCR |= 256;									//ENABLE 16 BIT MODE
	DMA1_Channel1->CNDTR = len;									//DATA LENGTH OF TRANSFER
	//DMA1_Channel1->CPAR = (uint32_t)&SPI1->DR;		//PERIPHERAL ADDRESS TARGET (SPI DATA REGISTER) - SET ON INIT
	DMA1_Channel1->CMAR = (uint32_t)ptr;			//ADDRESS OF SRC DATA
	//DMA1->IFCR |= 1;											//FORCE BIT 0 TO A 1 TO CLEAR ALL CHANNEL 1 INTERUPT FLAGS
	DMA1_Channel1->CCR |= 3;									//SET BIT 0 TO 1 TO ENABLE THE DMA TRANSFER. SET BIT 1 TO 1 TO ENABLE TRANSFER COMPLETE INTERUPT
}
