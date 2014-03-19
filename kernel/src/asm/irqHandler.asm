	.global _irq_handler_asm

	.global irqHandler

INTCPS_SIR_IRQ_ADDR .word 0x48200040

_irq_handler_asm:
	STMFD	SP, { R0 - R14 }^			; store (user) registers on stack
	SUB		SP, SP, #60					; decrement stack-pointer: 15 * 4 bytes = 60bytes
	MOV		R3, SP						; pointer to user-registers in R3, content will be changed by scheduler in case of scheduling

	STMFD	SP!, { LR }					; store LR in stack to either restore it after BL to allow scheduler change it (and restore it after return from BL)
	MOV 	R2, SP						; pointer to user-pc in R2, content will be changed by scheduler in case of scheduling

	MRS		R1, SPSR					; fetch user-cpsr (SPSR) value to R1
	STMFD	SP!, { R1 }					; store SPSR in stack to allow scheduler change it
	MOV 	R1, SP						; pointer to user-cpsr in R1, content will be changed by scheduler in case of scheduling

	LDR		R0, INTCPS_SIR_IRQ_ADDR		; load address of IRQ_ADDR to R0
	LDR 	R0, [ R0 ]					; load content of address to R0 => irq-number

	; irqHandler( uint32_t irqNr, uint32_t* userCpsr, uint32_t* userPC, uint32_t* userRegs )
	BL		irqHandler					; branch AND link to irq parent handler

	; TODO: do a BEQ on R0:
	; if R0 == 1 then scheduling was performed

	LDMFD	SP!, { R1 }					; restore SPSR, if changed by scheduler
	MSR		SPSR_cxsf, R1				; set stored cpsr from user to the current CPSR - will be restored later during SUBS

	LDMFD	SP!, { LR }					; restore LR, if changed by scheduler

	LDMFD	SP, { R0 - R14 }^			; restore user-registers, if changed by scheduler
	ADD		SP, SP, #60					; increment stack-pointer: 15 * 4 bytes = 60bytes

	; NOTE: at this point SP should be at starting address again

 	SUBS	PC, LR, #4					; return from IRQ
