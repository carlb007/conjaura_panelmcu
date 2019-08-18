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
#include "dma.h"
#include "spi.h"
#include "colour.h"
#include "timers.h"

uint8_t ledR[MAX_PANEL_LEDS];
uint8_t ledG[MAX_PANEL_LEDS];
uint8_t ledB[MAX_PANEL_LEDS];

uint16_t bamBuffer1[OUTPUTBUFFERSIZE];			//FLIP FLOP LED STREAM ARRAYS
uint16_t bamBuffer2[OUTPUTBUFFERSIZE];			//FLIP FLOP LED STREAM ARRAYS

uint16_t * renderOutput;						//FLIP FLOP POINTERS BETWEEN STREAM AND RENDER
uint8_t * streamOutput;							//FLIP FLOP POINTERS BETWEEN STREAM AND RENDER
uint8_t edgeCompiled[((MAX_EDGE_LEDS*3)*8)/2];	//BIT ARRAY FOR STREAMING TO WS2812 LEDS. 1 BYTE = 2 BITS
uint8_t edgeData[MAX_EDGE_LEDS*3];				//MAX SIZE OF EDGE DATA

void ConvertRawPixelData(void);
void BamifyData(void);

void LEDDataTransmit(void);
void FinaliseLEDData(void);

void EnableEdgeLights(void);
void DisableEdgeLights(void);
void TXEdgeLights(void);

#endif /* LED_DATA_H_ */
