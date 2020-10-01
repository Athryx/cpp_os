ENTRY(start)

KERNEL_VMA = 0xffffff8000000000;

SECTIONS
{
	. = 1M;

	.boot_text ALIGN(4K) :
	{
		*(.multiboot_header)
		*(.boot_text)
	}

	.boot_rodata ALIGN(4K) :
	{
		*(.boot_rodata)
	}

	.boot_bss ALIGN(4K) :
	{
		*(.boot_bss)
	}

	KERNEL_LMA = .;
	. += KERNEL_VMA;

	.text ALIGN(4K) : AT(ADDR(.text) - KERNEL_VMA)
	{
		text_start = .;
		*(.text)
		text_end = .;
	}

	.rodata ALIGN(4K) : AT(ADDR(.rodata) - KERNEL_VMA)
	{
		rodata_start = .;
		*(.rodata)
		rodata_end = .;
	}

	.data ALIGN(4K) : AT(ADDR(.data) - KERNEL_VMA)
	{
		data_start = .;
		*(.data)
		data_end = .;
	}

	.bss ALIGN(4K) : AT(ADDR(.bss) - KERNEL_VMA)
	{
		bss_start = .;
		*(.bss)
		bss_end = .;
	}
	. = ALIGN(4K);
	kernel_end = .;

	/DISCARD/ :
	{
		*(.comment)
	}
}
