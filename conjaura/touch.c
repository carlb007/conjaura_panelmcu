/*
 * touch.c
 *
 *  Created on: 23 July 2019
 *  Author: Carl Barnett
 */

#include "touch.h"

extern ADC_HandleTypeDef hadc1;
uint8_t * ADCRead = ADCReadings;
uint8_t touchCalibrated = FALSE;
uint16_t calibrationSampleCount = 0;
uint8_t currentTouchPoint = 0;

void InitTouch_ADC(){
	HAL_StatusTypeDef resp = HAL_ADC_Start_DMA(&hadc1, ADCRead, ADCHANNELS);
}

HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){
	if(!touchCalibrated){
		touchChannel[currentTouchPoint].baseReading += *(ADCRead);
		touchChannel[currentTouchPoint+8].baseReading += *(ADCRead+1);
		if(calibrationSampleCount==CALIBRATIONSAMPLES){
			for (uint8_t ch=0;ch<MAXTOUCHCHANNELS;ch++){
				touchChannel[ch].baseReading /= CALIBRATIONSAMPLES;
			}
			touchCalibrated = TRUE;
			//printf("ADC: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d \n",touchChannel[0].baseReading, touchChannel[1].baseReading, touchChannel[2].baseReading, touchChannel[3].baseReading,touchChannel[4].baseReading, touchChannel[5].baseReading, touchChannel[6].baseReading, touchChannel[7].baseReading,touchChannel[8].baseReading, touchChannel[9].baseReading, touchChannel[10].baseReading, touchChannel[11].baseReading,touchChannel[12].baseReading, touchChannel[13].baseReading, touchChannel[14].baseReading, touchChannel[15].baseReading);
			printf("ADC CALIB: BL %d, BR %d, TL %d, TR %d \n",touchChannel[1].baseReading, touchChannel[5].baseReading, touchChannel[11].baseReading, touchChannel[15].baseReading);
		}
	}
	else{
		touchChannel[currentTouchPoint].value = *(ADCRead) - (touchChannel[currentTouchPoint].baseReading-1);
		touchChannel[currentTouchPoint+8].value = *(ADCRead+1) - (touchChannel[currentTouchPoint+8].baseReading-1);
		globalVals.peripheralDataPoint++;
		*(returnData+TOUCH_BUFFER_SIZE+1+globalVals.peripheralDataPoint) = *(ADCRead+2);
		if(globalVals.peripheralDataPoint==PERIPHERAL_SIZE-2){
			globalVals.peripheralDataPoint = 0;
		}
		if(currentTouchPoint==7){
			if(touchChannel[10].value>2 || touchChannel[8].value>2 || touchChannel[13].value>2 || touchChannel[15].value>2){
				if (globalVals.dataMode == ADDRESS_MODE && thisPanel.addressSet==FALSE){
					SetAndSendAddress();
				}
				//printf("ADC: BL %d, BR %d, TL %d, TR %d \n",touchChannel[8].value, touchChannel[10].value, touchChannel[13].value, touchChannel[15].value);
			}
			//printf("ADC: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d \n",touchChannel[0].value, touchChannel[1].value, touchChannel[2].value, touchChannel[3].value,touchChannel[4].value, touchChannel[5].value, touchChannel[6].value, touchChannel[7].value,touchChannel[8].value, touchChannel[9].value, touchChannel[10].value, touchChannel[11].value,touchChannel[12].value, touchChannel[13].value, touchChannel[14].value, touchChannel[15].value);
		}
	}
	currentTouchPoint++;
	if(currentTouchPoint==MAXTOUCHCHANNELS/2){
		currentTouchPoint = 0;
		if(!touchCalibrated){
			calibrationSampleCount++;
		}
	}
	selectRow(currentTouchPoint);
	InitTouch_ADC();
}
