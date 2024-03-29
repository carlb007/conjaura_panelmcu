/*
 * spi.h
 *
 *  Created on: 2 Aug 2019
 *      Author: me
 */

#include "globals.h"
#include "main.h"

#ifndef SPI_H_
#define SPI_H_

#define SPI_SPEED_CLK2 7<<3;
#define SPI_SPEED_CLK4 1<<3;
#define SPI_SPEED_CLK8 2<<3;
#define SIZE_8BIT 7<<8;
#define SPI_LSBFIRST 1<<7;

void InitSPI(void);
void SPI1Defaults(void);
void SPI2Defaults(void);
void ConfigLEDDataSPI(void);
void ConfigReturnDataSPI(void);

#endif /* SPI_H_ */
