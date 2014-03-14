	.global _schedule_asm
	.global _enable_IRQ

; PRECONDITIONS =====================
; Mode: SVC
; ===================================
; ARGUMENTS =========================
;	r0 	->	uint32_t* userPC
;	r1	->	uint32_t userCpsr
;	r2 	->	uint32_t* userRegs
; ===================================
_schedule_asm:
	STMFD	SP, { R0 - R2 }		; store arguments on stack, don't move SP

	SUB		SP, SP, #4			; move SP up 4 bytes, SP points now to R2 on stack
	LDMFD	SP, { R0 - R14 }^	; restore user-registers

	SUB		SP, SP, #4			; move SP up 4 bytes, SP points now to R1 on stack
	LDR 	LR, [ SP ] 			; transfer content of address SP points to SP
	MSR     cpsr_cf, LR			; restore user-mode cpsr

	;CPS		#0x10			; switch processor back to user-mode

	SUB		SP, SP, #4			; move SP down 4 bytes, SP points now to R0 on stack, original SP restored
	LDR 	LR, [ SP ]			; load address stored in arg-R0 to which the SP points now to the SP itself
	BX		LR					; jump back to user-task
