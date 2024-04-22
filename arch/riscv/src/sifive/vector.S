/* Copyright 2018-2023 SiFive, Inc */
/* SPDX-License-Identifier: Apache-2.0 */

#include <arch.h>
#include <asm_macros.h>
#include <sifive_metal.h>
#include <sifive_cpu.h>
#include <sbi/riscv_encoding.h>
#include <sbi/sbi_trap.h>
#include <sbi/riscv_asm.h>
#include <sbi/riscv_elf.h>
#include <memmap.h>

/* This code executes before _start, which is contained inside the C library.
 In embedded systems we want to ensure that _enter, which contains the first
 code to be executed, can be loaded at a specific address.  To enable this
 feature we provide the '.text.metal.init.enter' section, which is
 defined to have the first address being where execution should start. */
.global _start
_start:

	/* Disable and clear all interrupt sources */
	li	    a3, -1
	csrc	    mie, a3
	csrc	    mip, a3

	/* Check RISC-V isa and enable FS bits if Floating Point architecture. */
	csrr	    a5, misa
	li	    a4, 0x10028
	and	    a5, a5, a4
	beqz	    a5, 1f
	csrr	    a5, mstatus
	lui	    a4, 0x2
	or	    a5, a5, a4
	csrw	    mstatus, a5
	csrwi fcsr, 0
1:

	/* Check for "vector" extension support and enable the vector unit if found.
	* Omit if the toolchain doesn't support any vector extension.
	*
	* Here the tree of the extension support:
	*       V
	*       |
	*     Zve64d
	*       |
	*     Zve64f
	*    /      \
	* Zve64x   Zve32f
	*    \      /
	*     Zve32x
	*
	* So if any extension is present we should have __riscv_zve32x
	*/

#ifdef __riscv_zve32x

	/* Unfortunately, Zve* (embedded vector extensions) do not set misa.V,
	* so, until the RISC-V standard discovery mechanism is finalized, we use
	* a trick: we set mstatus.VS to Dirty, and then read it back. It's set to
	* Off iff there's no vector unit. This should detect the presence of any
	* standard vector extension (V or Zve*) or the absence of all. */
	li	    a5, 0x600
	csrs	    mstatus, a5
	csrr	    a5, mstatus
	andi	    a5, a5, 0x600
	beqz	    a5, 1f
	vsetivli    x0, 0, e8, m1, ta, ma
	csrwi	    vcsr, 0
1:
#endif

#ifdef CONFIG_SUPPORT_SMP
	li t0, MULTI_CORE_FLAG
	li t1, 0x1
	fence ow, ow
	sd t1, 0(t0)
#endif

1:
	csrr	    a5, misa
	li	    a4, METAL_ISA_U_EXTENSIONS
	and	    a4, a4, a5
	beqz	    a4, 1f

	la	    t0, __metal_wgcpu_mwidlist_masks_table
	csrr	    a0, mhartid
	slli	    t1, a0, REG_SIZE_LOG2
	add	    t0, t0, t1

	load_x	    t2, (t0)
	beqz	    t2, 1f
	mv	    a3, t2
	li	    t0, 0

.Lmlwidloop:
	andi	    t1, t2, 1
	bne	    t1, x0, .Lmlwid_found
	addi	    t0, t0, 1
	srli	    t2, t2, 1
	j	    .Lmlwidloop

.Lmlwid_found:
	la	    t1, 1f
	csrw	    mtvec, t1
	csrw	    METAL_WG_CSR_MLWID, t0

	csrr	    a5, misa
	li	    a4, METAL_ISA_S_EXTENSIONS
	and	    a4, a4, a5
	beqz	    a4, 1f

	csrw	    METAL_WG_CSR_MWIDDELEG, a3
	csrw	    METAL_WG_CSR_SLWID, t0

.align 4
1:
	la	    t0, __metal_wgchecker2_base_address_table

.Lchecker2loop:
	load_x	    t1, (t0)
	beqz	    t1, .Lchecker2end

	lw	    s1, 0x8(t1)
	addi	    t1, t1, 0x20
	li	    s0, (1<<31)
	sw	    s0, 0x10(t1)

.Lcheckeraddrloop:
	addi	    t1, t1, 0x20
	addi	    s1, s1, -1
	bnez	    s1, .Lcheckeraddrloop

	li	    s0, -1
	sw	    s0, 0x8(t1)
	sw	    s0, 0xc(t1)

	li	    s0, 0x80000301
	sw	    s0, 0x10(t1)
	addi	    t0, t0, REG_SIZE
	j	    .Lchecker2loop

.Lchecker2end:
	la	    t0, __metal_wgmarker2_base_address_table
	li	    s0, METAL_SIFIVE_WORLDGUARD_TRUSTEDWID
	li	    s1, 0x2 // Valid bit

.Lmarker2loop:
	load_x	    t1, (t0)
	beqz	    t1, .Linit

	sw	    s0, 0x08(t1)

	sw	    s1, 0x0C(t1)

	addi	    t0, t0, REG_SIZE
	j	    .Lmarker2loop

.align 4
1:
_try_lottery:
	la	a0, smp_boot_spin_lock
	li	t1, 1
	amoadd.w	a0, t1, (a0)
	bnez a0, _wait_relocate_copy_done

.Linit:
	/* If the __metal_chicken_bit symbol is set to 0, do not change the Feature
	* Disable registers. */
	la	    t0, __metal_chicken_bit
	beqz	    t0, 1f
	/* Trap over the Feature Disable registers' clearing since some SiFive chips
	* don't have them (for example, FE310 and FU540) */
	la	    t0, 1f
	csrw	    mtvec, t0
	/* Clear the Feature Disable registers' bits, except the
	* suppressCorruptOnGrantData bit (this one will be cleared later in the
	* boot sequence). */
	li	    t0, ~(1 << 9)
	csrc	    0x7C1, t0
	csrw	    0x7C2, x0

.align 4

1:
	/* Set up a simple trap vector to catch anything that goes wrong early in
	* the boot process. */
	la	    t0, _trap_handler
	csrw	    mtvec, t0

1:
	la	    t0, __ld_stack_top
	add	    sp, zero, t0

	/* Check for an initialization routine and call it if one exists, otherwise
	* just skip over the call entirely.   Note that __metal_before_start isn't
	* actually a full C function, as it doesn't end up with the .bss or .data
	* segments having been initialized.  This is done to avoid putting a
	* burden on systems that can be initialized without having a C environment
	* set up. */
1:
	/* If the __metal_chicken_bit symbol is set to 0, do not change the Feature
	* Disable registers. */
	la	    t0, __metal_chicken_bit
	beqz	    t0, 1f
	/* Trap over the Feature Disable registers' clearing since some SiFive chips
	* don't have them (for example, FE310 and FU540) */
	la	    t0, 1f
	csrw	    mtvec, t0
	/* Clear the Feature Disable registers' suppressCorruptOnGrantData bit */
	li	    t0, (1 << 9)
	csrc	    0x7C1, t0

	li	t0, BOOT_STATUS_BOOT_HART_DONE
	lla	t1, _boot_status
	REG_S	t0, 0(t1)
	fence	rw, rw

.align 4
1:
	/* Put back early trap vector */
	la	    t0, _trap_handler
	csrw	    mtvec, t0
#ifndef CONFIG_TARGET_EMULATOR
	/*
	* emulator can force all memory to 0
	* after platform reset so clean bss
	* is not needed
	*/
	jal	    clear_bss
#endif
	jal	    load_data
	jal	    system_init
_end:
	wfi
	beqz	    zero, _end

_wait_relocate_copy_done:
	wfi

#if 1
	li	t0, BOOT_STATUS_BOOT_HART_DONE
	lla	t1, _boot_status
	REG_L	t1, 0(t1)
	/* Reduce the bus traffic so that boot hart may proceed faster */
	nop
	nop
	nop
	bne	t0, t1, _wait_relocate_copy_done

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

.align 4

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
_boot_status:
	RISCV_PTR	0
smp_boot_spin_lock:
	.dword 0
