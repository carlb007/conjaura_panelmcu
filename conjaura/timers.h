/*
 * timers.h
 *
 *  Created on: 3 Aug 2019
 *      Author: me
 */

#include "globals.h"

#ifndef TIMERS_H_
#define TIMERS_H_

uint16_t timeDelays[8];				//WAIT TIME FOR EACH BAM BIT CYCLE

void InitTimers(void);
void SetAndStartTimer6(uint16_t);
void ClearAndPauseTimer6();
void SetAndStartTimer7(uint16_t);
void ClearAndPauseTimer7();

#endif /* TIMERS_H_ */
