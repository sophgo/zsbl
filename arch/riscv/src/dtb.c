extern unsigned char __dtb_start[0];
extern unsigned char __dtb_end[0];

void *dtb_get_base(void)
{
	return __dtb_start;
}

unsigned long dtb_get_size(void)
{
	return (unsigned long)__dtb_end - (unsigned long)__dtb_start;
}

