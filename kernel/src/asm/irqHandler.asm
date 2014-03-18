	.global _irq_handler_asm

	.global irqHandler

INTCPS_SIR_IRQ_ADDR .word 0x48200040

_irq_handler_asm:
	STMFD	SP, { R0 - R14 }^			; store (user) registers on stack
	SUB		SP, SP, #60					; decrement stack-pointer: 15 * 4 bytes = 60bytes
	MOV		R3, SP						; need pointer to user-registers which are stored in in R3

	STMFD	SP!, { LR }					; store link-register because will BL to irqHandler c function and need to return from it

	MOV 	R2, LR						; store LR in R2 to pass it as 'user-pc' to the irqHandler c-function

	LDR		R0, INTCPS_SIR_IRQ_ADDR		; load address of IRQ_ADDR to R0
	LDR 	R0, [ R0 ]					; load content of address to R0

	MRS		R1, CPSR					; need user cpsr in r1

	; irqHandler( uint32_t irqNr, uint32_t userCpsr, uint32_t* userPC, uint32_t* userRegs )
	BL		irqHandler					; branch AND link to irq parent handler

	LDMFD	SP!, { LR }					; restore link-register (overridden by BL to irqHandler)
	ADD		SP, SP, #60					; increment stack-pointer

 	SUBS	PC, LR, #4					; return from IRQ
