/*
 * addresses.h
 *
 *  Created on: 24 Jul 2019
 *      Author: me
 */
#include "globals.h"
#include "data.h"

#ifndef ADDRESSES_H_
#define ADDRESSES_H_

uint8_t lastSeenAddress;

void AddressHeader(void);
void SetAndSendAddress(void);
void LoadAddress(void);
void FinishAddressMode(void);



#endif /* ADDRESSES_H_ */
