#include <arch.h>
#include <smp.h>
#include <sbi/riscv_encoding.h>
#include <sbi/sbi_trap.h>
#include <sbi/riscv_asm.h>
	.section .vector, "ax"
	.global _start
_start:
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

	fence.i
	fence iorw, iorw

	/* for eda simulation */
	add     x0,  zero, zero
	add     x1,  zero, zero
	add     x2,  zero, zero
	add     x3,  zero, zero
	add     x4,  zero, zero
	add     x5,  zero, zero
	add     x6,  zero, zero
	add     x7,  zero, zero
	add     x8,  zero, zero
	add     x9,  zero, zero
	add     x10, zero, zero
	add     x11, zero, zero
	add     x12, zero, zero
	add     x13, zero, zero
	add     x14, zero, zero
	add     x15, zero, zero
	add     x16, zero, zero
	add     x17, zero, zero
	add     x18, zero, zero
	add     x19, zero, zero
	add     x20, zero, zero
	add     x21, zero, zero
	add     x22, zero, zero
	add     x23, zero, zero
	add     x24, zero, zero
	add     x25, zero, zero
	add     x26, zero, zero
	add     x27, zero, zero
	add     x28, zero, zero
	add     x29, zero, zero
	add     x30, zero, zero
	add     sp,  zero, zero

	/* Setup trap handler */
	la	a4, _trap_handler
	csrw	CSR_MTVEC, a4

#ifdef CONFIG_SUPPORT_SMP
	la	t0, smp_context
	csrr	t1, mhartid
	slli	t1, t1, SMP_CONTEXT_SIZE_SHIFT
	add	t0, t0, t1
	li	t1, SMP_CONTEXT_SIZE
	li	t2, 0
clean_smp_context:
	slt	t3, t2, t1
	beqz	t3, clean_smp_context_done
	add	t3, t0, t2
	sd	zero, (t3)
	addi	t2, t2, 8

	j	clean_smp_context
#endif
clean_smp_context_done:
	la	a0, smp_boot_spin_lock
	li	t1, 1
	amoadd.w	a0, t1, (a0)
	bnez	a0, 2f

1:
	/* setup stack */
	la	t0, __ld_stack_top
	add	sp, zero, t0
#ifndef CONFIG_TARGET_EMULATOR
	/* emulator can force all memory to 0 after platform reset */
	/* so clean bss is not needed */
	jal	clear_bss
#endif
	jal	load_data
	jal	system_init
	jal	__libc_init_array
	jal	main
	jal	_exit
_end:
	wfi
	beqz	zero, _end

poll_core:
	amoswap.w.rl	x0, x0, (a0)

not_support_smp:
#ifndef CONFIG_SUPPORT_SMP
	wfi
	beqz	zero, not_support_smp
#else
2:
secondary_core_poll:

	la	t0, smp_context
	csrr	t1, mhartid
	slli	t1, t1, SMP_CONTEXT_SIZE_SHIFT
	add	t1, t0, t1
	add	t2, t1, SMP_CONTEXT_FN_OFFSET
	add	t3, t1, SMP_CONTEXT_SP_OFFSET
	add	t4, t1, SMP_CONTEXT_PRIV_OFFSET
	ld	a0, (t4)
	ld	t5, (t3)
	add	t3, t1, SMP_CONTEXT_STATCKSIZE_OFFSET
	ld	t6, (t3)
	add	sp, t6, t5
	ld	t5, (t2)
	bnez	t5, mul_core_wake_up
	j	secondary_core_poll
mul_core_wake_up:
	la      t0, smp_context
	csrr    t1, mhartid
	slli    t1, t1, SMP_CONTEXT_SIZE_SHIFT
	add     t1, t0, t1
	add     t2, t1, SMP_CONTEXT_FN_OFFSET
	add     t3, t1, SMP_CONTEXT_SP_OFFSET
	add     t4, t1, SMP_CONTEXT_PRIV_OFFSET
	ld      a0, (t4)
	ld      t5, (t3)
	add     t3, t1, SMP_CONTEXT_STATCKSIZE_OFFSET
	ld      t6, (t3)
	add     sp, t6, t5
	ld      t5, (t2)

	jalr    t5

	la	t0, smp_context
	csrr	t1, mhartid
	slli	t1, t1, SMP_CONTEXT_SIZE_SHIFT
	add	t1, t0, t1
	add	t1, t1, SMP_CONTEXT_FN_OFFSET
	sd	zero, (t1)
	j	secondary_core_poll
#endif

_trap_handler:
	/* Re-use current SP as exception stack */
	add	sp, sp, -(SBI_TRAP_REGS_SIZE)

_trap_handler_all_mode:
	/* Save T0 on stack */
	REG_S	t0, SBI_TRAP_REGS_OFFSET(t0)(sp)

	add	t0, sp, (SBI_TRAP_REGS_SIZE)
	/* Save original SP (from T0) on stack */
	REG_S	t0, SBI_TRAP_REGS_OFFSET(sp)(sp)

	/* Swap TP and MSCRATCH */
	csrrw	tp, CSR_MSCRATCH, tp

	/* Save MEPC and MSTATUS CSRs */
	csrr	t0, CSR_MEPC
	REG_S	t0, SBI_TRAP_REGS_OFFSET(mepc)(sp)
	csrr	t0, CSR_MSTATUS
	REG_S	t0, SBI_TRAP_REGS_OFFSET(mstatus)(sp)
	REG_S	zero, SBI_TRAP_REGS_OFFSET(mstatusH)(sp)

	/* Save all general regisers except SP and T0 */
	REG_S	zero, SBI_TRAP_REGS_OFFSET(zero)(sp)
	REG_S	ra, SBI_TRAP_REGS_OFFSET(ra)(sp)
	REG_S	gp, SBI_TRAP_REGS_OFFSET(gp)(sp)
	REG_S	tp, SBI_TRAP_REGS_OFFSET(tp)(sp)
	REG_S	t1, SBI_TRAP_REGS_OFFSET(t1)(sp)
	REG_S	t2, SBI_TRAP_REGS_OFFSET(t2)(sp)
	REG_S	s0, SBI_TRAP_REGS_OFFSET(s0)(sp)
	REG_S	s1, SBI_TRAP_REGS_OFFSET(s1)(sp)
	REG_S	a0, SBI_TRAP_REGS_OFFSET(a0)(sp)
	REG_S	a1, SBI_TRAP_REGS_OFFSET(a1)(sp)
	REG_S	a2, SBI_TRAP_REGS_OFFSET(a2)(sp)
	REG_S	a3, SBI_TRAP_REGS_OFFSET(a3)(sp)
	REG_S	a4, SBI_TRAP_REGS_OFFSET(a4)(sp)
	REG_S	a5, SBI_TRAP_REGS_OFFSET(a5)(sp)
	REG_S	a6, SBI_TRAP_REGS_OFFSET(a6)(sp)
	REG_S	a7, SBI_TRAP_REGS_OFFSET(a7)(sp)
	REG_S	s2, SBI_TRAP_REGS_OFFSET(s2)(sp)
	REG_S	s3, SBI_TRAP_REGS_OFFSET(s3)(sp)
	REG_S	s4, SBI_TRAP_REGS_OFFSET(s4)(sp)
	REG_S	s5, SBI_TRAP_REGS_OFFSET(s5)(sp)
	REG_S	s6, SBI_TRAP_REGS_OFFSET(s6)(sp)
	REG_S	s7, SBI_TRAP_REGS_OFFSET(s7)(sp)
	REG_S	s8, SBI_TRAP_REGS_OFFSET(s8)(sp)
	REG_S	s9, SBI_TRAP_REGS_OFFSET(s9)(sp)
	REG_S	s10, SBI_TRAP_REGS_OFFSET(s10)(sp)
	REG_S	s11, SBI_TRAP_REGS_OFFSET(s11)(sp)
	REG_S	t3, SBI_TRAP_REGS_OFFSET(t3)(sp)
	REG_S	t4, SBI_TRAP_REGS_OFFSET(t4)(sp)
	REG_S	t5, SBI_TRAP_REGS_OFFSET(t5)(sp)
	REG_S	t6, SBI_TRAP_REGS_OFFSET(t6)(sp)

	/* Call C routine */
	add	a0, sp, zero
	call	sbi_trap_handler

	/* Restore all general regisers except SP and T0 */
	REG_L	ra, SBI_TRAP_REGS_OFFSET(ra)(sp)
	REG_L	gp, SBI_TRAP_REGS_OFFSET(gp)(sp)
	REG_L	tp, SBI_TRAP_REGS_OFFSET(tp)(sp)
	REG_L	t1, SBI_TRAP_REGS_OFFSET(t1)(sp)
	REG_L	t2, SBI_TRAP_REGS_OFFSET(t2)(sp)
	REG_L	s0, SBI_TRAP_REGS_OFFSET(s0)(sp)
	REG_L	s1, SBI_TRAP_REGS_OFFSET(s1)(sp)
	REG_L	a0, SBI_TRAP_REGS_OFFSET(a0)(sp)
	REG_L	a1, SBI_TRAP_REGS_OFFSET(a1)(sp)
	REG_L	a2, SBI_TRAP_REGS_OFFSET(a2)(sp)
	REG_L	a3, SBI_TRAP_REGS_OFFSET(a3)(sp)
	REG_L	a4, SBI_TRAP_REGS_OFFSET(a4)(sp)
	REG_L	a5, SBI_TRAP_REGS_OFFSET(a5)(sp)
	REG_L	a6, SBI_TRAP_REGS_OFFSET(a6)(sp)
	REG_L	a7, SBI_TRAP_REGS_OFFSET(a7)(sp)
	REG_L	s2, SBI_TRAP_REGS_OFFSET(s2)(sp)
	REG_L	s3, SBI_TRAP_REGS_OFFSET(s3)(sp)
	REG_L	s4, SBI_TRAP_REGS_OFFSET(s4)(sp)
	REG_L	s5, SBI_TRAP_REGS_OFFSET(s5)(sp)
	REG_L	s6, SBI_TRAP_REGS_OFFSET(s6)(sp)
	REG_L	s7, SBI_TRAP_REGS_OFFSET(s7)(sp)
	REG_L	s8, SBI_TRAP_REGS_OFFSET(s8)(sp)
	REG_L	s9, SBI_TRAP_REGS_OFFSET(s9)(sp)
	REG_L	s10, SBI_TRAP_REGS_OFFSET(s10)(sp)
	REG_L	s11, SBI_TRAP_REGS_OFFSET(s11)(sp)
	REG_L	t3, SBI_TRAP_REGS_OFFSET(t3)(sp)
	REG_L	t4, SBI_TRAP_REGS_OFFSET(t4)(sp)
	REG_L	t5, SBI_TRAP_REGS_OFFSET(t5)(sp)
	REG_L	t6, SBI_TRAP_REGS_OFFSET(t6)(sp)

	/* Restore MEPC and MSTATUS CSRs */
	REG_L	t0, SBI_TRAP_REGS_OFFSET(mepc)(sp)
	csrw	CSR_MEPC, t0
	REG_L	t0, SBI_TRAP_REGS_OFFSET(mstatus)(sp)
	csrw	CSR_MSTATUS, t0
#if __riscv_xlen == 32
	csrr	t0, CSR_MISA
	srli	t0, t0, ('H' - 'A')
	andi	t0, t0, 0x1
	beq	t0, zero, _skip_mstatush_restore
	REG_L	t0, SBI_TRAP_REGS_OFFSET(mstatusH)(sp)
	csrw	CSR_MSTATUSH, t0
_skip_mstatush_restore:
#endif

	/* Restore T0 */
	REG_L	t0, SBI_TRAP_REGS_OFFSET(t0)(sp)

	/* Restore SP */
	REG_L	sp, SBI_TRAP_REGS_OFFSET(sp)(sp)

	mret

.align 3
smp_boot_spin_lock:
.dword 0
main_boot_core:
.dword 0
