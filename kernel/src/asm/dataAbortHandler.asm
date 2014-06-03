	.global _dataAbortHandler
	.global dataAbortHandler

_dataAbortHandler:
	STMFD	SP!, { R0, R1, R14 }			; save registers on stack

	MRC p15, #0, r0, c5, c0, #0			; fetch FSR into R0 as argument to dataAbortHandler
	MRC p15, #0, r1, c6, c0, #0			; fetch FAR into R1 as argument to dataAbortHandler

	BL		dataAbortHandler			; branch AND link to data-abort parent handler

	CMP	r0, #0							; check if dataAbortHandler returned 0 if so Z-flag will be set to 1. This will signal a non-repeat exit

	LDMFD	SP!, { R0, R1, R14 }		; restore registers

	BEQ	exitNonRepeat					; branch to non-repeating exit if previous compare is true

exitWithRepeat:
	SUBS PC, R14, #8					; repeat instruction which was aborted

exitNonRepeat:
	SUBS PC, R14, #4					; DONT repeat instruction which was aborted
