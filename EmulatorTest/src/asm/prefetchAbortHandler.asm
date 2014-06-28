	.global _prefetch_abort_handler_asm
	;.global prefetchAbortHandler

	.sect "._prefetch_abort_handler_asm"

_prefetch_abort_handler_asm:

	;BL		prefetchAbortHandler					; branch AND link to prefetch-abort parent handler

	SUBS	PC, R14, #4
