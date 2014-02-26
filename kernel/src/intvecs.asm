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
	.word 0 		; abort (data) interrupt
	.word 0 		; reserved
	.word 0 		; IRQ interrupt
	.word 0 		; FIQ interrupt
