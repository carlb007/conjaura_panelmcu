/*
 * data.h
 *
 *  Created on: 24 Jul 2019
 *      Author: me
 */
#include "globals.h"
#include "led_data.h"

#ifndef DATA_H_
#define DATA_H_

void HeaderMode(uint8_t);

void EnableRS485RX(void);
void EnableRS485TX(void);

void DataToLEDs(void);
void DataToEXT(void);

void DisableRowEn(void);
void EnableRowEn(void);

void ParseHeader(void);

void HandlePanelData(void);
void HandleReturnData(void);
void DataReceive(void);

void DMA1_1_IRQ(void);

void SelectRow(uint8_t);

#endif /* DATA_H_ */
