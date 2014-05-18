/*
 * mmu.c
 *
 *  Created on: 18.04.2014
 *      Author: faisstm
 */

extern void _ttb_set(unsigned int ttb);
extern void _tlb_flush(unsigned int c8format);
extern void _pid_set(unsigned int pid);

void ttbSet(unsigned int ttb) {
	ttb &= 0xffffc000;
	_ttb_set(ttb);
}

void tlbFlush(void) {
	unsigned int c8format = 0;
	_tlb_flush(c8format);
}

void setProcessID(unsigned int pid) {
	pid = pid << 25;
	_pid_set(pid);
}



