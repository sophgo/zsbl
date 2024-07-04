/* Copyright 2018-2022 SiFive, Inc */
/* SPDX-License-Identifier: Apache-2.0 */

#include <asm_macros.h>
#include <sifive_platform.h>
#include <sbi/riscv_encoding.h>
#include <sbi/sbi_trap.h>

#define CLINT_MSIP (METAL_RISCV_CLINT0_0_BASE_ADDRESS + \
		METAL_RISCV_CLINT0_MSIP_BASE)
#define CLINT_MTIMECMP (METAL_RISCV_CLINT0_0_BASE_ADDRESS + \
			METAL_RISCV_CLINT0_MTIMECMP_BASE)

/* This code executes before _start, which is contained inside the C library.
 * In embedded systems we want to ensure that _start, which contains the first
 * code to be executed, can be loaded at a specific address. */
.section .vector, "ax"
.global _start
_start:
	.cfi_startproc

	/* Inform the debugger that there is nowhere to backtrace past _start. */
	.cfi_undefined ra

	/* The absolute first thing that must happen is configuring the global
	* pointer register, which must be done with relaxation disabled because
	* it's not valid to obtain the address of any symbol without GP
	* configured.  The C environment might go ahead and do this again, but
	* that's safe as it's a fixed register. */

.option	push
.option	norelax
#ifdef __riscv_cmodel_compact
1:
	auipc a0, %pcrel_hi(__global_pointer__)
	addi a0, a0, %pcrel_lo(1b)
	ld   gp, 0(a0)
	add  gp, gp, a0
#else /* !__riscv_cmodel_compact */
1:
	auipc gp, %pcrel_hi(__global_pointer$)
	addi gp, gp, %pcrel_lo(1b)
#endif /* !__riscv_cmodel_compact */
.option pop

	/* Set TLS pointer */
	.weak __tls_base
	load_a tp, __tls_base

	/* Disable and clear all interrupt sources */
	li   a3, -1
	csrc mie, a3
	csrc mip, a3

	/* The delegation CSRs exist if user mode interrupts (N extension) or
	* supervisor mode (S extension) are supported */
	csrr a5, misa
	lui  a4, 0x42
	and  a4, a4, a5
	beqz a4, 1f
	csrc mideleg, a3
	csrc medeleg, a3
1:

	/* The satp CSR exists if supervisor mode (S extension) is supported */
	lui  a4, 0x40
	and  a4, a4, a5
	beqz a4, 1f
	csrc satp, a3
1:

	/* Check RISC-V isa and enable FS bits if Floating Point architecture. */
	li   a4, 0x10028
	and  a5, a5, a4
	beqz a5, 1f
	csrr a5, mstatus
	lui  a4, 0x2
	or   a5, a5, a4
	csrw mstatus, a5
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
	li a5, 0x600
	csrs mstatus, a5
	csrr a5, mstatus
	andi a5, a5, 0x600
	beqz a5, 1f
	vsetivli x0, 0, e8, m1, ta, ma
	csrwi vcsr, 0
1:
#endif

	/* set a0 to mhartid */
	csrr a0, mhartid

	/* Set __metal_init_hart to __metal_boot_hart */
	la t0, __metal_boot_hart
	load_a t1, __metal_init_hart
	store_x t0, 0(t1)

	/* If we're __metal_init_hart, go to initialization step (a0 contains mhartid) */
	beq a0, t0, .Linit

	/* We're not __metal_init_hart */

	/* If the __metal_chicken_bit symbol is set to 0, do not change the Feature
	* Disable registers. */
	la t0, __metal_chicken_bit
	beqz t0, 1f
	/* Trap over the Feature Disable registers' clearing since some SiFive chips
	* don't have them (for example, FE310 and FU540) */
	la t0, 1f
	csrw mtvec, t0
	/* Clear the Feature Disable registers' bits */
	csrw 0x7C1, x0
	csrw 0x7C2, x0
.align 4
1:

	/* Put hart in WFI loop until signaled using IPI from __metal_init_hart */
	/* enable software interrupt for M-mode */
	csrsi mie, 1 << 3
1:
	/* Wait for interrupt */
	wfi
	/* Check if the interrupt was software interrupt, otherwise go back to wfi */
	csrr t1, mip
	andi t1, t1, 1 << 3
	beqz t1, 1b
	/* disable software interrupt for M-mode*/
	csrci mie, 1 << 3
	/* Set t0 to CLINT/CLIC MSIP */

	li t0, CLINT_MSIP

	/* Add hart offset (a0 contains mhartid) */
	slli t1, a0, 2
	add t0, t0, t1
	/* Reset MSIP[hartid] */
	sw x0, 0(t0)
	j .Lpost_sync

.Linit:
	/* If the __metal_chicken_bit symbol is set to 0, do not change the Feature
	* Disable registers. */
	la t0, __metal_chicken_bit
	beqz t0, 1f
	/* Trap over the Feature Disable registers' clearing since some SiFive chips
	* don't have them (for example, FE310 and FU540) */
	la t0, 1f
	csrw mtvec, t0
	/* Clear the Feature Disable registers' bits, except the
	* suppressCorruptOnGrantData bit (this one will be cleared later in the
	* boot sequence). */
	li t0, ~(1 << 9)
	csrc 0x7C1, t0
	csrw 0x7C2, x0
.align 4
1:

.Lpost_sync:
	/* Set up a simple trap vector to catch anything that goes wrong early in
	* the boot process. */
	la t0, early_trap_vector
	csrw mtvec, t0

	/* The METAL is designed for a bare-metal environment and therefore is expected
	* to define its own stack pointer. We also align the stack pointer here
	* because the only RISC-V ABI that's currently defined, mandates 16-byte
	* stack alignment. */
	load_a sp, __ld_stack_top
1:
	/* Increment by hartid number of stack sizes */
	csrr a0, mhartid
	li t0, 0
	la t1, __ld_stack_size
1:
	andi sp, sp, -16
	beq t0, a0, 1f
	sub sp, sp, t1
	addi t0, t0, 1
	j 1b
1:
	/* If we're not __metal_init_hart, skip the initialization work */
	csrr a0, mhartid
	load_a t0, __metal_init_hart
	load_x t0, 0(t0)
	bne a0, t0, .Lskip_init

	/* Check for an initialization routine and call it if one exists, otherwise
	* just skip over the call entirely.   Note that __metal_before_start isn't
	* actually a full C function, as it doesn't end up with the .bss or .data
	* segments having been initialized.  This is done to avoid putting a
	* burden on systems that can be initialized without having a C environment
	* set up. */
	la ra, __metal_before_start
	beqz ra, 1f
	jalr ra
1:

	/* If the __metal_chicken_bit symbol is set to 0, do not change the Feature
	* Disable registers. */
	la t0, __metal_chicken_bit
	beqz t0, 1f
	/* Trap over the Feature Disable registers' clearing since some SiFive chips
	* don't have them (for example, FE310 and FU540) */
	la t0, 1f
	csrw mtvec, t0
	/* Clear the Feature Disable registers' suppressCorruptOnGrantData bit */
	li t0, (1 << 9)
	csrc 0x7C1, t0
.align 4
1:
	/* Put back early trap vector */
	la t0, _trap_handler
	csrw mtvec, t0

	/* Embedded systems frequently require relocating the data segment before C
	* code can be run -- for example, the data segment may exist in flash upon
	* boot and then need to get relocated into a non-persistant writable memory
	* before C code can execute.  If this is the case we do so here.  This step
	* is optional: if the METAL provides an environment in which this relocation
	* is not necessary then it must simply set metal_segment_data_source_start to
	* be equal to metal_segment_data_target_start. */
	load_a a0, __ld_data_load_start
	load_a a1, __ld_data_start
	load_a a2, __ld_data_end
	call metal_init_copy_segment

	fence.i

	/* Zero the BSS segment. */
	load_a a0, __ld_bss_start
	load_a a1, __ld_bss_end
	call metal_init_clear_segment

	/* At this point we're in an environment that can execute C code.  The first
	* thing to do is to make the callback to the parent environment if it's
	* been requested to do so. */

.Lskip_init:

	jal	system_init
	la 	t0, die
	jalr 	t0

	.cfi_endproc

.weak __metal_before_start
.type __metal_before_start, @function
__metal_before_start:
	/* Save caller ra */
	mv      a5, ra

	/* Disable machine interrupts to be safe */
	li      a3, 8
	csrc    mstatus, a3

	/* Restore caller ra */
	mv      ra, a5
	ret

.global metal_init_clear_segment
.section .text.metal.init.clear_segment
.balign 8
.type   metal_init_clear_segment, @function
metal_init_clear_segment:
	/*
	* void metal_init_clear_segment(uintptr_t *start, uintptr_t *end)
	*
	* Zero a memory segment.
	*/
	bge a0, a1, 2f

1:
	store_x x0, 0(a0)
	addi a0, a0, REG_SIZE
	blt a0, a1, 1b
2:
	ret

.global metal_init_copy_segment
.section .text.metal.init.init_copy_segment
.balign 8
.type   metal_init_copy_segment, @function
metal_init_copy_segment:
	/*
	* void metal_init_copy_segment(uintptr_t *source_start,
	*                              uintptr_t *target_start,
	*                              uintptr_t *target_end);
	*
	* Embedded systems frequently require relocating the data segment before C
	* code can be run -- for example, the data segment exist in ROM upon it need
	* to get relocated into a non-persistant writable memory before C code can
	* execute.  If this is the case we do so here.  This step is optional:
	* If this relocation is not necessary then it must simply set
	* metal_segment_data_source_start to equal to metal_segment_data_target_start.
	*/
	beq a0, a1, 2f
	bge a1, a2, 2f

1:
	load_x t0, 0(a0)
	addi a0, a0, REG_SIZE
	store_x t0, 0(a1)
	addi a1, a1, REG_SIZE
	blt a1, a2, 1b
2:
	ret

/* RISC-V systems always use __libc_{init,fini}_array, but for compatibility we
 * define _{init,fini} to do nothing. */
.global _init
.type   _init, @function
.global _fini
.type   _fini, @function
_init:
_fini:
	ret
.size _init, .-_init
.size _fini, .-_fini

#ifdef __riscv_cmodel_compact
/* Compact stub.  */
	.section .text.__global_pointer__, "aMG",@progbits, 8, __global_pointer__, comdat
	.align 3
	.global __global_pointer__
	.type   __global_pointer__, object
__global_pointer__:
	.quad   __global_pointer$ -.
#endif

/* The global variable __metal_init_hart contains the hart that was selected
 * for the boot sequence (and WG initialization for WG-aware platforms) */
.section .startup_data.__metal_init_hart, "aw", @progbits
.global __metal_init_hart
.balign 8
__metal_init_hart:
	.quad  0x0

.align 4

/*
 * For sanity's sake we set up an early trap vector that just does nothing.
 * If you end up here then there's a bug in the early boot code somewhere.
 */
.section .text.metal.init.trapvec
.global early_trap_vector
.align 2
early_trap_vector:
	.cfi_startproc
	csrr t0, mcause
	csrr t1, mepc
	csrr t2, mtval
	j early_trap_vector
	.cfi_endproc


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

die:
	wfi
	j die
