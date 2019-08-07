/*
 * timers.c
 *
 *  Created on: 3 Aug 2019
 *      Author: me
 */

#include "timers.h"

uint16_t timeDelays[8] = {75,140,280,560,1120,2240,4480,8960};
uint8_t timersEnabled = FALSE;

void InitTimers(){
	__HAL_RCC_TIM6_CLK_ENABLE();
	__HAL_RCC_TIM7_CLK_ENABLE();

	TIM6->PSC = 2;
	TIM6->ARR = 0;
	TIM6->CR1 = 0;		//HALT AND STOP COUNTER AT EVENT
	TIM6->EGR |= TIM_EGR_UG;
	TIM6->SR = 0;		//CLEAR INTERUPT FLAG
	TIM6->DIER = 1;		//ENABLE INTERUPT
	HAL_NVIC_EnableIRQ(TIM6_IRQn);


	TIM7->PSC = 9;
	TIM7->ARR = 0;
	TIM7->CR1 = 0;				//HALT AND STOP COUNTER AT EVENT
	TIM7->EGR |= TIM_EGR_UG;	//NEED TO FIRE EVENT REGISTER TO LATCH IN UPDATED PRESCALER.
	TIM7->SR = 0;				//CLEAR INTERUPT FLAG
	TIM7->DIER = 0;				//DISABLE INTERUPT
	HAL_NVIC_EnableIRQ(TIM7_IRQn);
	timersEnabled = TRUE;
}

void SetAndStartTimer6(uint16_t duration){
	//printf("Dur %d \n",duration);
	TIM6->ARR = duration;
	TIM6->CNT = 0;										//ZERO TIMER
	TIM6->CR1 = 1;										//START TIMER
}

void ClearAndPauseTimer6(){
	//printf("Dur %d \n",TIM6->CNT);
	TIM6->CR1 = 0;										//PAUSE TIMER
	TIM6->CNT = 0;													//ZERO TIMER
	TIM6->SR = 0;													//CLEAR THE UPDATE EVENT FLAG
}

void TIM6_IRQHandler(){
	ClearAndPauseTimer6();
	if(timersEnabled>0){
		if(renderState.immediateJump==0){
			LEDDataTransmit();
		}
		else{
			FinaliseLEDData();
		}
	}
}

void TIM7_IRQHandler(){
	ClearAndPauseTimer7();
	printf("7 called\n");
}

void SetAndStartTimer7(uint16_t duration){
	//printf("Time 7 Dur %d \n",duration);
	//TIM7->EGR = 1;
	TIM7->ARR = duration;
	TIM7->CNT = 0;													//ZERO TIMER
	TIM7->CR1 |= TIM_CR1_CEN;										//START TIMER
}

void ClearAndPauseTimer7(){
	TIM7->CR1 &= ~TIM_CR1_CEN;										//PAUSE TIMER
	TIM7->CNT = 0;													//ZERO TIMER
	TIM7->SR = 0;													//CLEAR THE UPDATE EVENT FLAG
}
