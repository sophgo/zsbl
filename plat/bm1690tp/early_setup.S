#include <platform.h>
#include <encoding.h>

	.global plat_early_setup
	.func plat_early_setup
plat_early_setup:
	li	t0, 0x70013
	csrw	CSR_MCOR, t0

	li	t0, 0x11ff
	csrw	CSR_MHCR, t0

	// enable L2Cache ECC
	li	t0, 0xe249000b
	csrw	CSR_MCCR2, t0

	li	t0, 1
	csrw	CSR_MSMPR, t0

	// disable fence.i fence sfence.vma broadcast
	li	t0, 0x3fea32c
	csrw	CSR_MHINT, t0

	li      t0, 0x180
	csrw    CSR_MHINT2, t0

	/* disable dvm transaction out of cluster*/
	li      t0, 0x20
	csrw    CSR_MHINT4, t0

	// enable THEADISAEE 	- bit 22
	// enable CLINTEE 	- bit 17
	li	t0, 0x438000
	csrw	CSR_MXSTATUS, t0

	/* delegate 0-7 exceptions to S-mode */
	csrr	t0, CSR_MEDELEG
	ori	t0, t0, 0xff
	csrw	CSR_MEDELEG, t0

	ret

