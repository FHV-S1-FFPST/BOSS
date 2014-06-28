	.global _memcopy

_memcopy:
	PUSH {r4-r10} 
LDMloop: 
	LDMIA r1!, {r3- r10} 
	STMIA r0!, {r3- r10} 
	SUBS r2, r2,#32 
	BGE LDMloop 
	POP {r4-r10}
