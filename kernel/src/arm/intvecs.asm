; *********************************************************************
; * Interrupt Vector Table for BOSS
; *
; *********************************************************************

	.global undefInstrHandler
	.global swiHandler
	.global prefetchAbortHandler
	.global dataAbortHandler
	.global irqHandler
	.global fiqHandler

	.sect ".intvecs"
	;B _c_int00 				; reset interrupt
	B undefInstrHandler 	; undefined instruction interrupt
	B swiHandler 			; software interrupt
	B prefetchAbortHandler	; abort (prefetch) interrupt
	B dataAbortHandler		; abort (data) interrupt
	.word 0 				; reserved
	B irqHandler			; IRQ interrupt NORMAL INTERRUPT: will save context registers (r8-r16)
	B fiqHandler			; FIQ interrupt	FAST INTERRUPT REQUEST: won't save context registers (r8-r16)