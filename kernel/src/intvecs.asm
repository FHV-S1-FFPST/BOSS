; *********************************************************************
; * Interrupt Vector Table for BOSS
; *
; *********************************************************************

	.global _c_int00
	.global SWI_Handler

	.sect ".intvecs"
	B _c_int00 		; reset interrupt
	.word 0 		; undefined instruction interrupt
	B SWI_Handler 	; software interrupt
	.word 0 		; abort (prefetch) interrupt
	.word 0 		; abort (data) interrupt		TODO: register page-fault handler here
	.word 0 		; reserved
	.word 0 		; IRQ interrupt NORMAL INTERRUPT: will save context registers (r0-r7)
	.word 0 		; FIQ interrupt	FAST INTERRUPT REQUEST: won't save context registers (r0-r7)
