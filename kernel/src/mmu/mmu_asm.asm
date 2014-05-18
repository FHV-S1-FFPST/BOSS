	.global _ttb_set
	.global _tlb_flush
	.global _pid_set

_ttb_set:
	MCR p15, #0, r0, c2, c0, #0   ; TTB -> CP15:c2:c0
	MOV PC, LR

_tlb_flush:
	MCR p15, #0, r0, c8, c7, #0
	MOV PC, LR

_pid_set:
	MCR p15, #0, r0, c13, c0, #0
	MOV PC, LR

