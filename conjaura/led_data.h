/*
 * led_data.h
 *
 *  Created on: 30 Jul 2019
 *      Author: me
 */

#ifndef LED_DATA_H_
#define LED_DATA_H_

#include "globals.h"
#include "data.h"
#include "colour.h"

uint8_t ledR[MAX_PANEL_LEDS];
uint8_t ledG[MAX_PANEL_LEDS];
uint8_t ledB[MAX_PANEL_LEDS];

uint16_t bamBuffer1[OUTPUTBUFFERSIZE];		//FLIP FLOP LED STREAM ARRAYS
uint8_t bamBuffer2[OUTPUTBUFFERSIZE];		//FLIP FLOP LED STREAM ARRAYS

uint16_t * renderOutput;					//FLIP FLOP POINTERS BETWEEN STREAM AND RENDER
uint8_t * streamOutput;						//FLIP FLOP POINTERS BETWEEN STREAM AND RENDER

uint16_t timeDelays[8];
uint8_t testData[12];

void ConvertRawPixelData(void);
void BamifyData(void);
void StartBamTimer(void);
void FinaliseLEDData(void);

void Tester(void);

#endif /* LED_DATA_H_ */
