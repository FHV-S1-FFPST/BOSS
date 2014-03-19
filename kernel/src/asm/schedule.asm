	.global _schedule_asm
	.global _enable_IRQ

; PRECONDITIONS =====================
; Mode: IRQ
; ===================================
; ARGUMENTS =========================
;	r0 	->	uint32_t* userPC
;	r1	->	uint32_t userCpsr
;	r2 	->	uint32_t* userRegs
; ===================================
_schedule_asm:
	STMFD	SP, { R0 - R1 }		; save R0 and R1 on stack, will be overwritten when restoring user-registers, DONT move SP

	LDMFD	R2, { R0 - R14 }^	; restore user-registers, will overwrite arguments R0, R1 and R2

	SUB		SP, SP, #4			; move SP down 4 bytes, SP points now to R1 on stack which is the user-CPSR
	LDR 	LR, [ SP ] 			; load content of address in SP to LR, which holds the previous R1 (user-cpsr)
	MSR		SPSR_cxsf, LR		; set stored cpsr from user to the current CPSR - will be restored later during SUBS
	;MSR     cpsr_cf, LR		; set stored cpsr from user to the current CPSR - will be restored later during SUBS

	SUB		SP, SP, #4			; move SP down 4 bytes, SP points now to R0 on stack which is the user-pc
	LDR 	LR, [ SP ]			; load address stored in arg-R0 to which the SP points now to the SP itself

	; TODO: need to restore SP correctly, OR set it when entering IRQ
 	SUBS	PC, LR, #4			; return from IRQ
