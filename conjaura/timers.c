/*
 * timers.c
 *
 *  Created on: 3 Aug 2019
 *      Author: me
 */

#include "timers.h"

void InitTimers(){
	__HAL_RCC_TIM6_CLK_ENABLE();
	TIM6->PSC = 4;
	TIM6->ARR = 0;
	TIM6->CR1 = 0;
	TIM6->EGR |= TIM_EGR_UG;
	TIM6->SR = ~1;
	TIM6->DIER = 1;
	HAL_NVIC_EnableIRQ(TIM6_IRQn);
}

void SetAndStartTimer6(uint16_t duration){
	TIM6->ARR = duration;
	TIM6->CR1 |= TIM_CR1_CEN;										//START TIMER
}

void ClearAndPauseTimer6(){
	TIM6->CR1 &= ~TIM_CR1_CEN;										//PAUSE TIMER
	TIM6->CNT = 0;													//ZERO TIMER
	TIM6->SR = 0;													//CLEAR THE UPDATE EVENT FLAG
}

void TIM6_IRQHandler(){
	ClearAndPauseTimer6();
	LEDDataTransmit();
	//printf("timer callback \n");





		//renderState.streamInProgress = TRUE;

		//hspi1.Init.DataSize = SPI_DATASIZE_16BIT;
		//HAL_SPI_Transmit_DMA(&hspi1, &bamBuffer1[dataPos16Bit], 6);

	//}
	//else{
	//	renderState.immediateJump = FALSE;
	//	FinaliseLEDData();
	//}

	//HAL_SPI_Transmit_DMA(&hspi1, testData, 12);
	//hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
	//HAL_SPI_Transmit_DMA(&hspi1, testData, 12);
}
