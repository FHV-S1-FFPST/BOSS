; *********************************************************************
; * Interrupt Vector Table for BOSS
; *
; *********************************************************************

	.global resetHandler
	.global undefInstrHandler
	.global _swi_handler_asm
	.global prefetchAbortHandler
	.global dataAbortHandler
	.global irqHandler
	.global fiqHandler

	.sect ".intvecs"
	B resetHandler			; reset interrupt
	B undefInstrHandler 	; undefined instruction interrupt
	B _swi_handler_asm 		; software interrupt
	B prefetchAbortHandler	; abort (prefetch) interrupt
	B dataAbortHandler		; abort (data) interrupt
	.word 0 				; reserved
	B irqHandler			; IRQ interrupt NORMAL INTERRUPT: will save context registers (r8-r16)
	B fiqHandler			; FIQ interrupt	FAST INTERRUPT REQUEST: won't save context registers (r8-r16)
