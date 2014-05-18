////////////////////////////////////////////////////////
// Linker command file for BeagleBoard)
// 
// Basic configuration using only external memory
//

-stack           0x00002000
-heap            0x00002000

MEMORY
{
	int_ram:	ORIGIN = 0x40200000		LENGTH = 0x0000FFFF
	ext_ddr:	ORIGIN = 0x82000000		LENGTH = 0x20000000
}

SECTIONS
{
	.const      > ext_ddr
	.bss        > ext_ddr
	.far        > ext_ddr

	.stack      > ext_ddr
	.data       > ext_ddr
	.cinit      > ext_ddr
	.cio        > ext_ddr

	// TODO: add .heap section with meaningful value

	.text       > ext_ddr
	.sysmem     > ext_ddr

	// stack-address for irq-mode 0x40200100
	.irqstack	> ext_ddr
	// stack-address for abort-mode(s) 0x40200300
	.abortstack	> ext_ddr
	// stack-address for supervisor-mode
	.svcstack	> ext_ddr
	// TODO: add here further stack-addresses and initialize them in boot.asm as required


	// map interrupt-vectors to 0x40200000 instead of 0x4020FFC8 (OMAP35x.pdf at page 3438) because would not
	// fit to memory (overshoot length). so in boot.asm the c12 register is set to 0x40200000
	.intvecs    > int_ram

}
