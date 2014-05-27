////////////////////////////////////////////////////////
// Linker command file for BeagleBoard)
//
// Basic configuration using only external memory
//

-stack           0x00002000
-heap            0x00F00000

MEMORY
{
	int_ram:		ORIGIN = 0x40200000		LENGTH = 0x0000FFFF
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

	// TODO: add .heap section with meaningful value
	.heap		> ext_ddr_rest

	.text       > ext_ddr_os
	.sysmem     > ext_ddr_os

	// stack-address for irq-mode 0x40200100
	.irqstack	> ext_ddr_os
	// stack-address for abort-mode(s) 0x40200300
	.abortstack	> ext_ddr_os
	// stack-address for supervisor-mode
	.svcstack	> ext_ddr_os
	// TODO: add here further stack-addresses and initialize them in boot.asm as required


	// map interrupt-vectors to 0x40200000 instead of 0x4020FFC8 (OMAP35x.pdf at page 3438) because would not
	// fit to memory (overshoot length). so in boot.asm the c12 register is set to 0x40200000
	.intvecs    > ext_ddr_os

}
