/*
 * timers.c
 *
 *  Created on: 3 Aug 2019
 *      Author: me
 */

#include "timers.h"

uint16_t timeDelays[8] = {65,120,240,480,960,1920,3840,7680};
uint8_t timersEnabled = FALSE;

void InitTimers(){
	__HAL_RCC_TIM6_CLK_ENABLE();
	__HAL_RCC_TIM7_CLK_ENABLE();

	TIM6->PSC = 2;
	TIM6->ARR = 0;
	TIM6->CR1 = 0;				//HALT AND STOP COUNTER AT EVENT
	TIM6->EGR |= TIM_EGR_UG;	//NEED TO FIRE EVENT REGISTER TO LATCH IN UPDATED PRESCALER.
	TIM6->SR = 0;				//CLEAR INTERUPT FLAG
	TIM6->DIER = 0;				//DISABLE INTERUPT

	TIM7->PSC = 9;
	TIM7->ARR = 0;
	TIM7->CR1 = 0;				//HALT AND STOP COUNTER AT EVENT
	TIM7->EGR |= TIM_EGR_UG;	//NEED TO FIRE EVENT REGISTER TO LATCH IN UPDATED PRESCALER.
	TIM7->SR = 0;				//CLEAR INTERUPT FLAG
	TIM7->DIER = 0;				//DISABLE INTERUPT

	TIM6->DIER = 1;				//ENABLE INTERUPT FOR TIMER 6

	HAL_NVIC_SetPriority(TIM6_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(TIM6_IRQn);
	timersEnabled = TRUE;
}

void SetAndStartTimer6(uint16_t duration){
	TIM6->ARR = duration;
	TIM6->CNT = 0;				//ZERO TIMER
	TIM6->CR1 = 1;				//START TIMER
}

void ClearAndPauseTimer6(){
	TIM6->CR1 = 0;				//PAUSE TIMER
	TIM6->CNT = 0;				//ZERO TIMER
	TIM6->SR = 0;				//CLEAR THE UPDATE EVENT FLAG
}


void TIM7_IRQHandler(){
	ClearAndPauseTimer7();
}

void SetAndStartTimer7(uint16_t duration){
	TIM7->ARR = duration;
	TIM7->CNT = 0;				//ZERO TIMER
	TIM7->CR1 |= TIM_CR1_CEN;	//START TIMER
}

void ClearAndPauseTimer7(){
	TIM7->CR1 &= ~TIM_CR1_CEN;	//PAUSE TIMER
	TIM7->CNT = 0;				//ZERO TIMER
	TIM7->SR = 0;				//CLEAR THE UPDATE EVENT FLAG
}
