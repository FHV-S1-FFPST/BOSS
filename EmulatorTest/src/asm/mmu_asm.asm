	.global _ttb_set
	.global _tlb_flush
	.global _pid_set
	.global _mmu_setDomainAccess
	.global _mmu_activate
	.global _mmu_init

_ttb_set:
	MCR p15, #0, r0, c2, c0, #0   ; TTB -> CP15:c2:c0
	MOV PC, LR

_tlb_flush:
	MCR p15, #0, r0, c8, c7, #0
	MOV PC, LR

_pid_set:
	MCR p15, #0, r0, c13, c0, #0
	MOV PC, LR

_mmu_setDomainAccess:
	MRC p15, #0, r2, c3, c0, #0
	BIC r2, r2, r1
	ORR r2, r2, r0
	MCR p15, #0, r2, c3, c0, #0
	MOV PC, LR

_mmu_init:
	MRC p15, #0, r1, c1, c0, #0
 	BIC r1, r1, #(0x1 << 12)     ; Enable Instruction cache
  	BIC r1, r1, #(0x1 << 2)      ; Enable Data cache
  	MCR p15, #0, r1, c1, c0, #0
  	mcr p15, #0, r1, c8, c7, #0    ; Invalidate TLB
  	mov r0, #0xFFFF
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
