	.global _swi_handler_asm
	.global swiHandler

_swi_handler_asm:
 	STMFD	SP!, { R0 - R1, LR }	; Store registers.
	MOV		R1, SP        			; Second parameter to C routine ...is pointer to register values.

    LDR		R0, [ LR, #-4 ]			; Calculate address of SVC instruction and load it into R0.
    BIC		R0, R0, #0xFF000000 	; Mask off top 8 bits of instruction to give SVC number.

	BL		swiHandler				; jump AND link, otherwise stackpointer would not be restored

	LDMFD	SP!, { R0 - R1, LR }	; Restore registers and return.

	MOVS	PC, LR					; return from software interrupt-mode
