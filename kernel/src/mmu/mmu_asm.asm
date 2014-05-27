	.global _control_set
	.global _ttb_set
	.global _tlb_flush
	.global _pid_set
	.global _mmu_initPT
	.global _mmu_setDomainAccess
	.global _ttbr_set_ctrl_bits
	.global _ttbr_enable_write_buffer
	.global _mmu_activate
	.global test

_control_set:
	MRC p15, #0, r2, c1, c0, #0
	BIC r2, r2, r1
	ORR r2, r2, r0
	MCR p15, #0, r2, c1, c0, #0

_ttbr_set_ctrl_bits:
	MCR p15, #0, R0, c2, c0, #2
	MOV PC, LR

_ttbr_enable_write_buffer:
	MRC p15, #0, r0, c1, c0, #0
	ORR r0, r0, #0x8 ; Write buffer
	MRC p15, #0, r0, c1, c0, #0

_ttb_set:
	MCR p15, #0, r0, c2, c0, #0   ; TTB -> CP15:c2:c0
	MOV PC, LR

_tlb_flush:
	MCR p15, #0, r0, c8, c7, #0
	MOV PC, LR

_pid_set:
	MCR p15, #0, r0, c13, c0, #0
	MOV PC, LR

_mmu_initPT:
	STMFD	SP, { R0 - R14 }^   	; store user registers on stack
	SUB		SP, SP, #60				; decrement stack-pointer: 15 * 4 bytes = 60bytes

	MOV r4, r0
	MOV r5, r0
	MOV r6, r0
	MOV r7, r0

L1:	CMP r1, #0						; check if index is 0, if yes jump to L2
	BEQ L2

	STMIA r2!, {r4-r7}
	STMIA r2!, {r4-r7}
	STMIA r2!, {r4-r7}
	STMIA r2!, {r4-r7}
	STMIA r2!, {r4-r7}
	STMIA r2!, {r4-r7}
	STMIA r2!, {r4-r7}
	STMIA r2!, {r4-r7}

	SUB r1, r1, #1
	B L1							; jump back to loop start

L2: LDMFD	SP, { R0 - R14 }^			; restore user-registers, if changed by scheduler
	ADD		SP, SP, #60					; increment stack-pointer: 15 * 4 bytes = 60bytes

	MOV PC, LR

_mmu_setDomainAccess:
	MRC p15, #0, r2, c3, c0, #0
	BIC r2, r2, r1
	ORR r2, r2, r0
	MCR p15, #0, r2, c3, c0, #0
	MOV PC, LR

test:
	MRC p15, #0, r1, c1, c0, #0
 	BIC r1, r1, #(0x1 << 12)     ; Enable Instruction cache
  	BIC r1, r1, #(0x1 << 2)      ; Enable Data cache
  	MCR p15, #0, r1, c1, c0, #0
  	mcr p15, #0, r1, c8, c7, #0    ; Invalidate TLB
  	mov r0, #0x5555
  	mcr p15, #0, r0, c3, c0, #0    ; Set DACR to all "manager" - no permissions checking

  	mov r0, #0
  	mcr p15, #0, r0, c7, c5, #4		; flush prefetch cache
  	mcr p15, #0, r0, c7, c5, #6
  	mcr p15, #0, r0, c7, c5, #0
  	mcr p15, #0, r0, c8, c7, #0

  	MOV PC, LR

_mmu_activate:
	mrc p15, #0, r0, c1, c0, #0
 	orr r0, r0, #0x3
	mcr p15, #0, r0, c1, c0, #0
	MOV PC, LR
