ENTRY(bootstrap32)
OUTPUT_FORMAT("elf64-x86-64")
SECTIONS
{
	. = 0x100000;

    __kernel_start = .;

	.bootstrap :
	{
		*(.bootstrap)
		*(.trampoline)
	}

	. += 0xFFFFFFFF80000000;

    .text ALIGN (0x1000) : AT(ADDR(.text) - 0xFFFFFFFF80000000)
    {
    	*(.text)
	}

    .data ALIGN (0x1000) : AT(ADDR(.data) - 0xFFFFFFFF80000000)
    {
        *(.data)
        *(.rodata)
    }

	.eh_frame ALIGN(0x1000) : AT(ADDR(.eh_frame) - 0xFFFFFFFF80000000)
   	{
   	    *(.eh_frame)
   	}

    .bss ALIGN(0x1000) : AT(ADDR(.bss) - 0xFFFFFFFF80000000)
    {
        __bss_start = .;
        *(.bss)
        *(COMMON)
        __bss_end = .;
    }

    .initcall ALIGN(0x1000) : AT(ADDR(.initcall) - 0xFFFFFFFF80000000)
    {
        __initcall1_start = .;
        *(.initcall1)
        __initcall2_start = .;
        *(.initcall2)
        __initcall3_start = .;
        *(.initcall3)
        __initcall4_start = .;
        *(.initcall4)
        __initcall5_start = .;
        *(.initcall5)
        __initcall6_start = .;
        *(.initcall6)
        __initcall7_start = .;
        *(.initcall7)
        __initcall8_start = .;
        *(.initcall8)
        __initcall_end = .;
    }

    __kernel_end = . - 0xFFFFFFFF80000000;
    /DISCARD/ :
    {
       	*(.comment)
    }
}
