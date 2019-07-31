/*
 * colour.h
 *
 *  Created on: 26 Jul 2019
 *      Author: me
 */

#include "globals.h"

#ifndef COLOUR_H_
#define COLOUR_H_

uint8_t gammaR[MAX_GAMMA_R_SIZE];
uint8_t gammaG[MAX_GAMMA_G_SIZE];
uint8_t gammaB[MAX_GAMMA_B_SIZE];

uint8_t paletteR[MAX_PALETTE_SIZE];
uint8_t paletteG[MAX_PALETTE_SIZE];
uint8_t paletteB[MAX_PALETTE_SIZE];

void ColourHeader(void);
void HandlePaletteData(void);

void GammaSetup(void);

void HandleGammaData(void);

#endif /* COLOUR_H_ */
