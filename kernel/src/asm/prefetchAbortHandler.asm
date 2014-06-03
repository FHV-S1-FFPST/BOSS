	.global _prefetchAbortHandler
	.global prefetchAbortHandler

_prefetchAbortHandler:

	BL		prefetchAbortHandler					; branch AND link to prefetch-abort parent handler

	SUBS	PC, R14, #4
