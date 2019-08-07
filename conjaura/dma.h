/*
 * dma.h
 *
 *  Created on: 2 Aug 2019
 *      Author: me
 */

#include "globals.h"
#include "led_data.h"
#include "data.h"

#ifndef DMA_H_
#define DMA_H_

void DMAInit(void);
void DMA1_1_Init(void);
void DMA1_23_Init(void);
void DMA1_1_IRQ(void);
void DMA1_23_IRQ(void);

#endif /* DMA_H_ */
