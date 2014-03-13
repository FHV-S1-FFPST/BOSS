/*
 * gptimer.h
 *
 *  Created on: 26.02.2014
 *      Author: Michael
 */

#ifndef GPTIMER_H_
#define GPTIMER_H_

#define GPTIMER1_BASE 0x48318000
#define GPTIMER2_BASE 0x49032000
#define GPTIMER_TMAR 0x038 // compare value
#define GPTIMER_TIER 0x01C // controls interrupt events (match, overflow, capture)
#define GPTIMER_TCLR 0x024 //
#define GPTIMER_TCRR 0x028 // timer counter
#define GPTIMER_TLDR 0x02C // timer load value
#define GPTIMER_TWER 0x020 //
#define GPTIMER_TISR 0x018
#define GPTIMER_TTGR 0x030 // timer reloads if written

#define GPTIMER_MATCH 1
#define GPTIMER_OVF (1<<1)
#define GPTIMER_CAPT (1<<2)


#endif /* GPTIMER_H_ */
