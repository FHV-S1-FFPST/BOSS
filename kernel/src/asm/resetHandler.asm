	.global _reset_handler_asm

c_stack		.long    0x1200

_reset_handler_asm:
	;STR		R0, c_stack		; stack_base could be defined above, or located in a scatter file

	;MSR		CPSR_c, R0
	;MOV     sp, r0

	;MRS     r0, cpsr
    ;BIC     r0, r0, #0x12  ; CLEAR MODES
    ;ORR     r0, r0, #0x12  ; SET SYSTEM MODE
    ;MSR     cpsr_cf, r0

	;STR		SP, c_stack
