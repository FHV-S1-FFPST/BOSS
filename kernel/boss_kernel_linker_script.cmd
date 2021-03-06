////////////////////////////////////////////////////////
// Linker command file for BeagleBoard)
// 
// Basic configuration using only external memory
//

-stack           0x00002000
-heap            0x00F00000

MEMORY
{
	int_ram_vecs:	ORIGIN = 0x40200000		LENGTH = 0x00000100
	int_ram_hndl:	ORIGIN = 0x40200100		LENGTH = 0x0000FEFF
	ext_ddr_pt:		ORIGIN = 0x80000000		LENGTH = 0x00500000
	ext_ddr_os:		ORIGIN = 0x80500000		LENGTH = 0x02000000
	ext_ddr_rest: 	ORIGIN = 0x82500000		LENGTH = 0x3DB00000
}

SECTIONS
{
	.const      > ext_ddr_os
	.bss        > ext_ddr_os
	.far        > ext_ddr_os

	.stack      > ext_ddr_os
	.data       > ext_ddr_os
	.cinit      > ext_ddr_os
	.cio        > ext_ddr_os

	.text       > ext_ddr_os
	.sysmem     > ext_ddr_os

	// stack-address for irq-mode 0x40200100
	.irqstack	> ext_ddr_os
	// stack-address for abort-mode(s) 0x40200300
	.abortstack	> ext_ddr_os
	// stack-address for supervisor-mode
	.svcstack	> ext_ddr_os

	// map interrupt-vectors to 0x40200000 instead of 0x4020FFC8 (OMAP35x.pdf at page 3438) because would not
	// fit to memory (overshoot length). so in boot.asm the c12 register is set to 0x40200000
	.intvecs    > int_ram_vecs

	._data_abort_handler_asm 		> int_ram_hndl
	._irq_handler_asm 				> int_ram_hndl
	._prefetch_abort_handler_asm 	> int_ram_hndl
	._swi_handler_asm				> int_ram_hndl
}
