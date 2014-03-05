	.global _swi_handler_asm
	.global swiHandler

_swi_handler_asm:
 	PUSH	{ r0-r3, r12, lr}	; Store registers.
	MOV		r1, sp        		; Second parameter to C routine ...is pointer to register values.

    LDR		r0, [ lr, #-4 ]			; Calculate address of SVC instruction and load it into R0.
    BIC		r0, r0, #0xFF000000 	; Mask off top 8 bits of instruction to give SVC number.

	BL		swiHandler

	LDM		sp!, { r0-r3, r12, pc }^ ; Restore registers and return.
