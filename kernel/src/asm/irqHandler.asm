	.global _irq_handler_asm

	.global irqHandler
	.global identify_and_clear_source

_irq_handler_asm:
 	SUB     lr, lr, #4             ; construct the return address
    PUSH    {lr}                   ; and push the adjusted lr_IRQ
    MRS     lr, SPSR               ; copy spsr_IRQ to lr
    PUSH    {R0-R4,R12,lr}         ; save AAPCS regs and spsr_IRQ
    BL      identify_and_clear_source
    MSR     CPSR_c, #0x9F          ; switch to SYS mode, IRQ is
                                   ; still disabled. USR mode
                                   ; registers are now current.
    AND     R1, sp, #4             ; test alignment of the stack
    SUB     sp, sp, R1             ; remove any misalignment (0 or 4)
    PUSH    {R1,lr}                ; store the adjustment and lr_USR
    MSR     CPSR_c, #0x1F          ; enable IRQ
    BL      irqHandler
    MSR     CPSR_c, #0x9F          ; disable IRQ, remain in SYS mode
    POP     {R1,lr}                ; restore stack adjustment and lr_USR
    ADD     sp, sp, R1             ; add the stack adjustment (0 or 4)
    MSR     CPSR_c, #0x92          ; switch to IRQ mode and keep IRQ
                                   ; disabled. FIQ is still enabled.
    POP     {R0-R4,R12,lr}         ; restore registers and
    MSR     SPSR_cxsf, lr          ; spsr_IRQ
    LDM     sp!, {pc}^             ; return from IRQ.
