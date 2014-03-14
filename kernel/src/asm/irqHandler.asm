	.global _irq_handler_asm

	.global irqHandler

INTCPS_SIR_IRQ_ADDR .word 0x48200040

_irq_handler_asm:
	CPS		#0x13						; switch to supervisor-mode

	STMFD	SP, { R0 - R14 }^			; store (user) registers on stack
	SUB		SP, SP, #60					; decrement stack-pointer: 15 * 4 bytes = 60bytes
	MOV 	R2, LR						; store link-register because will BL to irqHandler c function AND its the "user-pc"
	MOV		R3, SP						; need pointer to user-registers which are stored in  in r2

	LDR		R0, INTCPS_SIR_IRQ_ADDR		; load address of IRQ_ADDR to R0
	LDR 	R0, [ R0 ]					; load content of address to R0

	MRS		R1, CPSR					; need user cpsr in r1

	; irqHandler( uint32_t irqNr, uint32_t userCpsr, uint32_t* userPC, uint32_t* userRegs )
	BL		irqHandler					; branch AND link to irq parent handler

	; NOTE: in case of scheduling we won't arrive here
	; TODO: stack-pointer is fucked up then, because wont be resetted!

	MOV		LR, R2						; restore link-register (overridden by BL to irqHandler)
	LDMFD	SP!, { R0 - R3 }			; restore registers and return.

	CPS		#0x12						; back to IRQ-mode

 	SUBS	PC, LR, #4					; return from IRQ
