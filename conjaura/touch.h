/*
 * touch.h
 *
 *  Created on: 23 July 2019
 *  Author: Carl Barnett
 */
#ifndef TOUCH_H_
#define TOUCH_H_

#include "globals.h"

#define CALIBRATIONSAMPLES 160
#define MAXTOUCHCHANNELS 16
#define ADCHANNELS 3

//ADC READINGS ARRAY IS SET TO 4 BYTES. 1 BYTE FOR EACH ADC CHANNEL IN BACKWARDS ORDER: ADC1, ADC2, EXT2.
//NOTE ADC2 RELATES TO TOP HALF OF 16x16/8x32 PANEL OR THE RIGHT SIDE OF A 32x8. IN 16x8 OR 8x8 ITS NOT PRESENT.
uint8_t ADCReadings[ADCHANNELS];

uint8_t touchCalibrated;
uint16_t calibrationSampleCount;

struct touchCh{
	uint8_t value;
	uint32_t baseReading;
} touchChannel[MAXTOUCHCHANNELS];

uint8_t *ADCRead;

void InitTouch_ADC(void);

#endif /* TOUCH_H_ */
