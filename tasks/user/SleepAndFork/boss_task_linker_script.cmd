
-stack           0x00002000
-heap            0x00F00000

MEMORY
{
	v_ram:			ORIGIN = 0x00000000		LENGTH = 0x40000000
}

SECTIONS
{
	.const      > v_ram
	.data       > v_ram
	.text       > v_ram

	.cinit		> v_ram

	.heap		> v_ram
	.stack      > v_ram
}
