
-stack           0x00002000
-heap            0x00F00000

MEMORY
{
	v_ram:			ORIGIN = 0x02100000		LENGTH = 0x20000000
}

SECTIONS
{
	.const      > v_ram
	.data       > v_ram
	.text       > v_ram

	.cinit		> v_ram

	.heap		> v_ram
	.stack      > v_ram

	.bss		> v_ram
	.sysmem		> v_ram
}
