	.global _schedule_asm

; assumes we are in system-mode
; called with arguments:
;	r0 	->	uint32_t userCpsr
;	r1	->	uint32_t* userPC
;	r2 	->	uint32_t* userRegs
_schedule_asm:
	LDMFD	R0, { R0 - R14 }^	; restore user-registers

	MSR     cpsr_cf, R0			; restore user-mode cpsr
	;CPS		#0x1F				; switch processor back to user-mode

	BX		R1			; jump back to user-task
