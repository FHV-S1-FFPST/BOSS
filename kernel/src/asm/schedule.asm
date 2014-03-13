	.global _schedule_asm

; assumes we are in system-mode
; called with arguments:
;	r0 	->	uint32_t* userPC
;	r1	->	uint32_t userCpsr
;	r2 	->	uint32_t* userRegs
_schedule_asm:
	; TODO: fix error: need to take banked SP
	STMFD	SP!, { R0 - R2 }	; store arguments on stack

	SUB		SP, SP, #8			; move SP up 8 bytes, SP points now to R2 on stack
	LDMFD	SP, { R0 - R14 }^	; restore user-registers

	ADD		SP, SP, #4			; move SP down 4 bytes, SP points now to R1 on stack
	MSR     cpsr_cf, SP			; restore user-mode cpsr
	;CPS		#0x1F			; switch processor back to user-mode

	ADD		SP, SP, #4			; move SP down 4 bytes, SP points now to R0 on stack, original SP restored
	BX		SP					; jump back to user-task
