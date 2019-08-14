/*
 * touch.c
 *
 *  Created on: 23 July 2019
 *  Author: Carl Barnett
 */

#include "touch.h"

extern ADC_HandleTypeDef hadc1;
uint8_t * ADCRead = ADCReadings;
uint16_t calibrationSampleCount = 0;
uint8_t currentTouchPoint = 0;
uint8_t DMASet = 0;

void InitTouch_ADC(){
	//SetAndStartTimer7(60000);
	globalVals.touchRunning = TRUE;
	//HAL_ADC_Start_DMA(&hadc1, ADCRead, ADCHANNELS);
	while((ADC1->CR & 4) == ADC_CR_ADSTART);	//WAIT TIL ADC NOT BUSY
	if(DMASet==0){
		//if((ADC1->CR & 1) == ADC_CR_ADEN){
			ADC1->CR |= ADC_CR_ADDIS;
			while((ADC1->CR & 2) == ADC_CR_ADDIS);
			ADC1->CFGR1 |= ADC_CFGR1_DMAEN;
			ADC1->CR |= ADC_CR_ADEN;
			ADC1->IER |= (ADC_IT_OVR | ADC_IT_EOS);
			DMA1_Channel4->CPAR = (uint32_t)&ADC1->DR;				//ADDRESS OF SRC DATA
			DMA1_Channel4->CMAR = (uint32_t)ADCRead;				//PERIPHERAL ADDRESS SOURCE (ADC DATA REGISTER)
			DMASet = 1;
		//}
	}
	//CLEAR OUT INTERUPT REGISTER.
	//ADC1->IER &= ~((HAL_ADC_STATE_READY | HAL_ADC_STATE_REG_EOC | HAL_ADC_STATE_REG_OVR | HAL_ADC_STATE_REG_EOSMP | HAL_ADC_STATE_REG_BUSY));

	//CLEAR INTERUPT FLAGS.
	ADC1->ISR &= ~(ADC_FLAG_EOC | ADC_FLAG_EOS | ADC_FLAG_OVR);

	//DMA CONF...
	//DMA1_Channel4->CCR &= ~769;								//SET BIT 0 TO 0 TO DISABLE DMA. SET BITS 8 and 9 to 0. CLEAR THE PERIPHERAL DATA SIZE. 00 SET US IN 8BIT MODE.
	DMA1_Channel4->CNDTR = 3;								//DATA LENGTH OF TRANSFER
	DMA1_Channel4->CCR |= 3;								//SET BIT 0 TO 1 TO ENABLE THE DMA TRANSFER. SET BIT 1 TO 1 TO ENABLE TRANSFER COMPLETE INTERUPT

	//printf("called adc start\n");
	ADC1->CR |= ADC_CR_ADSTART;

}

void DeInitTouch_ADC(){
	globalVals.touchRunning = FALSE;
	ADC1->ISR &= ~(ADC_FLAG_EOC | ADC_FLAG_EOS | ADC_FLAG_OVR);		//CLEAR INTERUPT FLAGS.
	DMA1_Channel4->CCR &= ~1;										//DISABLE DMA
	ADC1->CR &= ~ADC_CR_ADEN;
}


ADCConversionComplete(){
	DMA1_Channel4->CCR &= ~2;	//CLEAR TRANSFER COMPLETE FLAG
	DMA1->IFCR |= (4096|8192);	//CLEAR ALL CHANNEL 1 INTERUPT
	DMA1_Channel4->CCR &= ~1;	//DISABLE DMA

	if(!globalVals.touchCalibrated){
		thisPanel.touchChannel[currentTouchPoint].baseReading += *(ADCRead);
		thisPanel.touchChannel[currentTouchPoint+8].baseReading += *(ADCRead+1);
		if(calibrationSampleCount==CALIBRATIONSAMPLES){
			for (uint8_t ch=0;ch<MAXTOUCHCHANNELS;ch++){
				thisPanel.touchChannel[ch].baseReading /= CALIBRATIONSAMPLES;
			}
			globalVals.touchCalibrated = TRUE;
			//printf("ADC CALIB: BL %d, BR %d, TL %d, TR %d \n",thisPanel.touchChannel[1].baseReading, thisPanel.touchChannel[5].baseReading, thisPanel.touchChannel[11].baseReading, thisPanel.touchChannel[15].baseReading);
		}
	}
	else{
		spiBufferTX[currentTouchPoint] = *(ADCRead) - (thisPanel.touchChannel[currentTouchPoint].baseReading-1);
		spiBufferTX[currentTouchPoint+8] = *(ADCRead+1) - (thisPanel.touchChannel[currentTouchPoint+8].baseReading-1);

		//COLLECT PERIPHERAL DATA IF NEEDED. WE CAN COLLECT 1 PERIPH BYTE PER ROWSCAN.
		if(panelInfoLookup[thisPanel.address].periperalByteSize>0){
			spiBufferTX[TOUCH_BUFFER_SIZE+globalVals.peripheralDataPoint] = *(ADCRead+2);
			globalVals.peripheralDataPoint++;
			if(globalVals.peripheralDataPoint==PERIPHERAL_SIZE-2){
				globalVals.peripheralDataPoint = 0;
			}
		}

		if(currentTouchPoint==7){
			if(spiBufferTX[10]>2 || spiBufferTX[8]>2 || spiBufferTX[13]>2 || spiBufferTX[15]>2){
				if (globalVals.headerMode == ADDRESS_MODE && thisPanel.addressSet==FALSE){
					SetAndSendAddress();
				}
				//uint16_t val = TIM7->CNT;
				//printf("ADC TIME: %d \n",val);
				//printf("ADC: BL %d, BR %d, TL %d, TR %d \n",spiBufferTX[8], spiBufferTX[10], spiBufferTX[13], spiBufferTX[15]);
			}
			//printf("ADC: BL %d, BR %d, TL %d, TR %d \n",thisPanel.touchChannel[8].value, thisPanel.touchChannel[10].value, thisPanel.touchChannel[13].value, thisPanel.touchChannel[15].value);
		}
	}
	currentTouchPoint++;
	if(!globalVals.touchCalibrated || (globalVals.headerMode == ADDRESS_MODE && thisPanel.addressSet==FALSE)){
		DisableRowEn();
		SelectRow(currentTouchPoint);
		EnableRowEn();
		InitTouch_ADC();
	}
	if(currentTouchPoint==MAXTOUCHCHANNELS/2){
		currentTouchPoint = 0;
		if(!globalVals.touchCalibrated){
			calibrationSampleCount++;
		}
	}
	globalVals.touchRunning = FALSE;
}
