	.global	__dtb_start
	.global __dtb_end
	.balign	8
__dtb_start:
	.incbin	CONFIG_BUILTIN_DTB
__dtb_end:
