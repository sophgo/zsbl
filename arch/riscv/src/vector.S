#include <arch.h>
#include <smp.h>
#include <encoding.h>
#include <trap.h>
#include <asm.h>

#define BOOT_STATUS_BOOT_HART_DONE	2

	.func plat_early_setup
	.weak plat_early_setup
plat_early_setup:
	ret

	.section .vector, "ax"
	.global _start
_start:
	la	t0, plat_early_setup
	jalr	t0

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

_try_lottery:
#if 0
	la	a0, smp_boot_spin_lock
	li	t1, 1
	amoadd.w	a0, t1, (a0)
#else
	csrr	a0, CSR_MHARTID
#endif
	bnez	a0, _wait_relocate_copy_done

	/* Save load address */
	lla	t0, _load_start
	lla	t1, _fw_start
	REG_S	t1, 0(t0)

	/* relocate the global table content */
	lla	t0, _link_start
	REG_L	t0, 0(t0)
	/* t1 shall has the address of _fw_start */
	sub	t2, t1, t0
	lla	t3, _runtime_offset
	REG_S	t2, (t3)
	lla	t0, __rel_dyn_start
	lla	t1, __rel_dyn_end
	beq	t0, t1, _relocate_done
	j	5f
2:
	REG_L	t5, -(REGBYTES*2)(t0)	/* t5 <-- relocation info:type */
	li	t3, R_RISCV_RELATIVE	/* reloc type R_RISCV_RELATIVE */
	bne	t5, t3, 3f
	REG_L	t3, -(REGBYTES*3)(t0)
	REG_L	t5, -(REGBYTES)(t0)	/* t5 <-- addend */
	add	t5, t5, t2
	add	t3, t3, t2
	REG_S	t5, 0(t3)		/* store runtime address to the GOT entry */
	j	5f

3:
	lla	t4, __dyn_sym_start

4:
	REG_L	t5, -(REGBYTES*2)(t0)	/* t5 <-- relocation info:type */
	srli	t6, t5, SYM_INDEX	/* t6 <--- sym table index */
	andi	t5, t5, 0xFF		/* t5 <--- relocation type */
	li	t3, RELOC_TYPE
	bne	t5, t3, 5f

	/* address R_RISCV_64 or R_RISCV_32 cases*/
	REG_L	t3, -(REGBYTES*3)(t0)
	li	t5, SYM_SIZE
	mul	t6, t6, t5
	add	s5, t4, t6
	REG_L	t6, -(REGBYTES)(t0)	/* t0 <-- addend */
	REG_L	t5, REGBYTES(s5)
	add	t5, t5, t6
	add	t5, t5, t2		/* t5 <-- location to fix up in RAM */
	add	t3, t3, t2		/* t3 <-- location to fix up in RAM */
	REG_S	t5, 0(t3)		/* store runtime address to the variable */

5:
	addi	t0, t0, (REGBYTES*3)
	ble	t0, t1, 2b

	/* mark boot hart done */
	li	t0, BOOT_STATUS_BOOT_HART_DONE
	lla	t1, _boot_status
	REG_S	t0, 0(t1)
	fence	rw, rw

	j	_relocate_done

_relocate_done:
	/* Setup trap handler */
	la	a4, _trap_handler
	csrw	CSR_MTVEC, a4
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

_wait_relocate_copy_done:
	li	t0, BOOT_STATUS_BOOT_HART_DONE
	lla	t1, _boot_status
	REG_L	t1, 0(t1)
	/* Reduce the bus traffic so that boot hart may proceed faster */
	nop
	nop
	nop
	bne	t0, t1, _wait_relocate_copy_done

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

	.balign	4
_trap_handler:
	/* Re-use current SP as exception stack */
	add	sp, sp, -(TRAP_REGS_SIZE)

_trap_handler_all_mode:
	/* Save T0 on stack */
	REG_S	t0, TRAP_REGS_OFFSET(t0)(sp)

	add	t0, sp, (TRAP_REGS_SIZE)
	/* Save original SP (from T0) on stack */
	REG_S	t0, TRAP_REGS_OFFSET(sp)(sp)

	/* Swap TP and MSCRATCH */
	csrrw	tp, CSR_MSCRATCH, tp

	/* Save MEPC and MSTATUS CSRs */
	csrr	t0, CSR_MEPC
	REG_S	t0, TRAP_REGS_OFFSET(mepc)(sp)
	csrr	t0, CSR_MSTATUS
	REG_S	t0, TRAP_REGS_OFFSET(mstatus)(sp)
	REG_S	zero, TRAP_REGS_OFFSET(mstatusH)(sp)

	/* Save all general regisers except SP and T0 */
	REG_S	zero, TRAP_REGS_OFFSET(zero)(sp)
	REG_S	ra, TRAP_REGS_OFFSET(ra)(sp)
	REG_S	gp, TRAP_REGS_OFFSET(gp)(sp)
	REG_S	tp, TRAP_REGS_OFFSET(tp)(sp)
	REG_S	t1, TRAP_REGS_OFFSET(t1)(sp)
	REG_S	t2, TRAP_REGS_OFFSET(t2)(sp)
	REG_S	s0, TRAP_REGS_OFFSET(s0)(sp)
	REG_S	s1, TRAP_REGS_OFFSET(s1)(sp)
	REG_S	a0, TRAP_REGS_OFFSET(a0)(sp)
	REG_S	a1, TRAP_REGS_OFFSET(a1)(sp)
	REG_S	a2, TRAP_REGS_OFFSET(a2)(sp)
	REG_S	a3, TRAP_REGS_OFFSET(a3)(sp)
	REG_S	a4, TRAP_REGS_OFFSET(a4)(sp)
	REG_S	a5, TRAP_REGS_OFFSET(a5)(sp)
	REG_S	a6, TRAP_REGS_OFFSET(a6)(sp)
	REG_S	a7, TRAP_REGS_OFFSET(a7)(sp)
	REG_S	s2, TRAP_REGS_OFFSET(s2)(sp)
	REG_S	s3, TRAP_REGS_OFFSET(s3)(sp)
	REG_S	s4, TRAP_REGS_OFFSET(s4)(sp)
	REG_S	s5, TRAP_REGS_OFFSET(s5)(sp)
	REG_S	s6, TRAP_REGS_OFFSET(s6)(sp)
	REG_S	s7, TRAP_REGS_OFFSET(s7)(sp)
	REG_S	s8, TRAP_REGS_OFFSET(s8)(sp)
	REG_S	s9, TRAP_REGS_OFFSET(s9)(sp)
	REG_S	s10, TRAP_REGS_OFFSET(s10)(sp)
	REG_S	s11, TRAP_REGS_OFFSET(s11)(sp)
	REG_S	t3, TRAP_REGS_OFFSET(t3)(sp)
	REG_S	t4, TRAP_REGS_OFFSET(t4)(sp)
	REG_S	t5, TRAP_REGS_OFFSET(t5)(sp)
	REG_S	t6, TRAP_REGS_OFFSET(t6)(sp)

	/* Call C routine */
	add	a0, sp, zero
	call	trap_handler

	/* Restore all general regisers except SP and T0 */
	REG_L	ra, TRAP_REGS_OFFSET(ra)(sp)
	REG_L	gp, TRAP_REGS_OFFSET(gp)(sp)
	REG_L	tp, TRAP_REGS_OFFSET(tp)(sp)
	REG_L	t1, TRAP_REGS_OFFSET(t1)(sp)
	REG_L	t2, TRAP_REGS_OFFSET(t2)(sp)
	REG_L	s0, TRAP_REGS_OFFSET(s0)(sp)
	REG_L	s1, TRAP_REGS_OFFSET(s1)(sp)
	REG_L	a0, TRAP_REGS_OFFSET(a0)(sp)
	REG_L	a1, TRAP_REGS_OFFSET(a1)(sp)
	REG_L	a2, TRAP_REGS_OFFSET(a2)(sp)
	REG_L	a3, TRAP_REGS_OFFSET(a3)(sp)
	REG_L	a4, TRAP_REGS_OFFSET(a4)(sp)
	REG_L	a5, TRAP_REGS_OFFSET(a5)(sp)
	REG_L	a6, TRAP_REGS_OFFSET(a6)(sp)
	REG_L	a7, TRAP_REGS_OFFSET(a7)(sp)
	REG_L	s2, TRAP_REGS_OFFSET(s2)(sp)
	REG_L	s3, TRAP_REGS_OFFSET(s3)(sp)
	REG_L	s4, TRAP_REGS_OFFSET(s4)(sp)
	REG_L	s5, TRAP_REGS_OFFSET(s5)(sp)
	REG_L	s6, TRAP_REGS_OFFSET(s6)(sp)
	REG_L	s7, TRAP_REGS_OFFSET(s7)(sp)
	REG_L	s8, TRAP_REGS_OFFSET(s8)(sp)
	REG_L	s9, TRAP_REGS_OFFSET(s9)(sp)
	REG_L	s10, TRAP_REGS_OFFSET(s10)(sp)
	REG_L	s11, TRAP_REGS_OFFSET(s11)(sp)
	REG_L	t3, TRAP_REGS_OFFSET(t3)(sp)
	REG_L	t4, TRAP_REGS_OFFSET(t4)(sp)
	REG_L	t5, TRAP_REGS_OFFSET(t5)(sp)
	REG_L	t6, TRAP_REGS_OFFSET(t6)(sp)

	/* Restore MEPC and MSTATUS CSRs */
	REG_L	t0, TRAP_REGS_OFFSET(mepc)(sp)
	csrw	CSR_MEPC, t0
	REG_L	t0, TRAP_REGS_OFFSET(mstatus)(sp)
	csrw	CSR_MSTATUS, t0
#if __riscv_xlen == 32
	csrr	t0, CSR_MISA
	srli	t0, t0, ('H' - 'A')
	andi	t0, t0, 0x1
	beq	t0, zero, _skip_mstatush_restore
	REG_L	t0, TRAP_REGS_OFFSET(mstatusH)(sp)
	csrw	CSR_MSTATUSH, t0
_skip_mstatush_restore:
#endif

	/* Restore T0 */
	REG_L	t0, TRAP_REGS_OFFSET(t0)(sp)

	/* Restore SP */
	REG_L	sp, TRAP_REGS_OFFSET(sp)(sp)

	mret

.align 3
_boot_status:
	RISCV_PTR	0
smp_boot_spin_lock:
	.dword 0
main_boot_core:
	.dword 0
_load_start:
	RISCV_PTR	_fw_start
_link_start:
	RISCV_PTR	CONFIG_RAM_START
_runtime_offset:
	RISCV_PTR	0
