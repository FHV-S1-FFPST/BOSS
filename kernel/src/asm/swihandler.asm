	.global _swi_handler_asm
	.global swiHandler

_swi_handler_asm:
	STMFD	SP, { R0 - R14 }^		; store user-registers on stack
	SUB		SP, SP, #60				; decrement stack-pointer: 15 * 4 bytes = 60bytes (not possible with ^ in previous op)

	STMFD	SP!, { LR }				; store SWI-LR in stack to either restore it after BL

	MRS		R1, SPSR				; fetch user-cpsr (SPSR) value to R1
	STMFD	SP!, { R1 }				; store SPSR in stack to allow scheduler change it

	MOV 	R1, SP					; pointer to SP in R1, to point to UserContext-struct

    LDR		R0, [ LR, #-4 ]			; Calculate address of SVC instruction and load it into R0.
    BIC		R0, R0, #0xFF000000 	; Mask off top 8 bits of instruction to give SVC number.

	; swiHandler( uint32_t swiId, UserContext* ctx )
	BL		swiHandler				; jump AND link, otherwise stackpointer would not be restored

	LDMFD	SP!, { R1 }				; restore SPSR, if changed by scheduler
	MSR		SPSR_cxsf, R1			; set stored cpsr from user to the current CPSR - will be restored later during MOVS

	LDMFD	SP!, { LR }				; restore LR, if changed by scheduler

	LDMFD	SP, { R0 - R14 }^		; restore user-registers, if changed by scheduler
	ADD		SP, SP, #60				; increment stack-pointer: 15 * 4 bytes = 60bytes

	MOVS	PC, LR					; return from software interrupt-mode
