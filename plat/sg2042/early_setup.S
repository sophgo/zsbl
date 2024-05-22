#include <platform.h>
#include <sbi/riscv_encoding.h>

	.global plat_early_setup
	.func plat_early_setup
plat_early_setup:
	li	t0, 0x70013
	csrw	CSR_MCOR, t0

	/* disable l0btb to workround auipc bug */
	li	t0, 0x1ff
	csrw	CSR_MHCR, t0

	li	t0, 0xe0410009
	csrw	CSR_MCCR2, t0

	li	t0, 0xe6e30c
	csrw	CSR_MHINT, t0

	csrr	t0, CSR_MHINT2
	li	t1, 3
	slli	t1, t1, 7
	or	t0, t0, t1
	li	t1, 1
	slli	t1, t1, 22
	or	t0, t0, t1
	csrw	CSR_MHINT2, t0

	li	t0, 0x638000
	csrw	CSR_MXSTATUS, t0

	/* disable dvm */
	li	t0, 0x20
	csrw	CSR_MHINT4, t0

	li	t0, 1
	csrw	CSR_MSMPR, t0

	/* delegate 0-7 exceptions to S-mode */
	csrr	t0, CSR_MEDELEG
	ori	t0, t0, 0xff
	csrw	CSR_MEDELEG, t0

	ret

