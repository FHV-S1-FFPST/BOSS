	.global _irq_handler_asm

	.global irqHandler

INTCPS_SIR_IRQ_ADDR .word 0x48200040

_irq_handler_asm:
	STMFD	SP, { R0 - R14 }^			; store user registers
	SUB		SP, SP, #60					; decrement stack-pointer: 15 * 4 bytes = 60bytes
	MOV		R2, SP						; need pointer to user-registers which are stored in  in r2

	PUSH	{ LR }						; store IRQ link-register because will BL to irqHandler c function

	LDR		R0, INTCPS_SIR_IRQ_ADDR		; load address of IRQ_ADDR to R0
	LDR 	R0, [ R0 ]					; load content of address to R0

	MRS		R1, CPSR					; need user cpsr in r1

	BL		irqHandler					; branch AND link to irq parent handler

	LDMFD	SP!, { R0 - R2, LR }		; Restore registers and return.

 	SUBS	PC, LR, #4					; return from IRQ THIS IS WRONG
