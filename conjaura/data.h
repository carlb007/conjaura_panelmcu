/*
 * data.h
 *
 *  Created on: 24 Jul 2019
 *      Author: me
 */
#include "globals.h"

#ifndef DATA_H_
#define DATA_H_

void Initialise(void);
void HeaderMode(uint8_t);

void EnableRS485RX(void);
void EnableRS485TX(void);

void DataToLEDs(void);
void DataToEXT(void);

void ParseHeader(void);

void selectRow(uint8_t);

#endif /* DATA_H_ */
