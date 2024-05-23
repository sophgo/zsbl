#include <arch.h>
#include <macro.h>
	.section .vector, "ax"
	.global _start
_start:
	/* for eda simulation */
	mov     x0,  xzr
	mov     x1,  xzr
	mov     x2,  xzr
	mov     x3,  xzr
	mov     x4,  xzr
	mov     x5,  xzr
	mov     x6,  xzr
	mov     x7,  xzr
	mov     x8,  xzr
	mov     x9,  xzr
	mov     x10, xzr
	mov     x11, xzr
	mov     x12, xzr
	mov     x13, xzr
	mov     x14, xzr
	mov     x15, xzr
	mov     x16, xzr
	mov     x17, xzr
	mov     x18, xzr
	mov     x19, xzr
	mov     x20, xzr
	mov     x21, xzr
	mov     x22, xzr
	mov     x23, xzr
	mov     x24, xzr
	mov     x25, xzr
	mov     x26, xzr
	mov     x27, xzr
	mov     x28, xzr
	mov     x29, xzr
	mov     x30, xzr
	mov     sp,  x0

	/* select sp, using sp_elx */
	ldr	x0, =1
	msr	spsel, x0

	bl	smp_core_id
	cmp	x0, 0
	beq	1f
park:
	b	smp_wait
1:
	/* setup stack */
	ldr	x0, =__ld_stack_top
	mov	sp, x0
#ifndef CONFIG_TARGET_EMULATOR
	/* emulator can force all memory to 0 after platform reset */
	/* so clean bss is not needed */
	bl	clear_bss
#endif
	bl	load_data
	bl	system_init
	bl	main
	bl	_exit
_end:
	wfi
	b	_end

	.section .exception_handler, "ax"
	.global exception_handler
	.balign 2048
exception_handler:
current_el_sp0_sync:
	b	.

	.balign 0x80
current_el_sp0_irq:
	b	.

	.balign	0x80
current_el_sp0_fiq:
	b	.

	.balign	0x80
current_el_sp0_serror:
	b	.

	.balign	0x80

current_el_spx_sync:
	b	handle_exception

	.balign 0x80
current_el_spx_irq:
	b	handle_irq

	.balign	0x80
current_el_spx_fiq:
	b	handle_fiq

	.balign	0x80
current_el_spx_serror:
	b	handle_exception

	.balign	0x80

/* lower el using aa64 and aa32 cannot happen */
handle_exception:
	/* save context */
	sub	sp, sp, #CONTEXT_FRAME_SIZE
	stp	x0, x1, [sp, #16 * 0]
	stp	x2, x3, [sp, #16 * 1]
	stp	x4, x5, [sp, #16 * 2]
	stp	x6, x7, [sp, #16 * 3]
	stp	x8, x9, [sp, #16 * 4]
	stp	x10, x11, [sp, #16 * 5]
	stp	x12, x13, [sp, #16 * 6]
	stp	x14, x15, [sp, #16 * 7]
	stp	x16, x17, [sp, #16 * 8]
	stp	x18, x19, [sp, #16 * 9]
	stp	x20, x21, [sp, #16 * 10]
	stp	x22, x23, [sp, #16 * 11]
	stp	x24, x25, [sp, #16 * 12]
	stp	x26, x27, [sp, #16 * 13]
	stp	x28, x29, [sp, #16 * 14]
	add	x1, sp, #CONTEXT_FRAME_SIZE
	stp	x30, x1, [sp, #16 * 15]

	mov	x0, sp
	bl	exception_dump
	b	.

handle_serror:
	/* save context */
	sub	sp, sp, #CONTEXT_FRAME_SIZE
	stp	x0, x1, [sp, #16 * 0]
	stp	x2, x3, [sp, #16 * 1]
	stp	x4, x5, [sp, #16 * 2]
	stp	x6, x7, [sp, #16 * 3]
	stp	x8, x9, [sp, #16 * 4]
	stp	x10, x11, [sp, #16 * 5]
	stp	x12, x13, [sp, #16 * 6]
	stp	x14, x15, [sp, #16 * 7]
	stp	x16, x17, [sp, #16 * 8]
	stp	x18, x19, [sp, #16 * 9]
	stp	x20, x21, [sp, #16 * 10]
	stp	x22, x23, [sp, #16 * 11]
	stp	x24, x25, [sp, #16 * 12]
	stp	x26, x27, [sp, #16 * 13]
	stp	x28, x29, [sp, #16 * 14]
	add	x1, sp, #CONTEXT_FRAME_SIZE
	stp	x30, x1, [sp, #16 * 15]

	mov	x0, sp
	bl	serror_dump
	b	.


handle_irq:
	/* save context */
	sub	sp, sp, #CONTEXT_FRAME_SIZE

	stp	x0, x1, [sp, #16 * 0]
	stp	x2, x3, [sp, #16 * 1]
	stp	x4, x5, [sp, #16 * 2]
	stp	x6, x7, [sp, #16 * 3]
	stp	x8, x9, [sp, #16 * 4]
	stp	x10, x11, [sp, #16 * 5]
	stp	x12, x13, [sp, #16 * 6]
	stp	x14, x15, [sp, #16 * 7]
	stp	x16, x17, [sp, #16 * 8]
	stp	x18, x19, [sp, #16 * 9]
	stp	x20, x21, [sp, #16 * 10]
	stp	x22, x23, [sp, #16 * 11]
	stp	x24, x25, [sp, #16 * 12]
	stp	x26, x27, [sp, #16 * 13]
	stp	x28, x29, [sp, #16 * 14]
	mrs	x0, elr_el3
	stp	x30, x0, [sp, #16 * 15]	/* lr and elr */

	mov	x0, sp
	bl	irq_entry

	/* restore elr first, for we need a general purpose register to hold the
	 * intermediate data
	 */
	ldp	x30, x0, [sp, #16 * 15]
	msr	elr_el3, x0

	ldp	x0, x1, [sp, #16 * 0]
	ldp	x2, x3, [sp, #16 * 1]
	ldp	x4, x5, [sp, #16 * 2]
	ldp	x6, x7, [sp, #16 * 3]
	ldp	x8, x9, [sp, #16 * 4]
	ldp	x10, x11, [sp, #16 * 5]
	ldp	x12, x13, [sp, #16 * 6]
	ldp	x14, x15, [sp, #16 * 7]
	ldp	x16, x17, [sp, #16 * 8]
	ldp	x18, x19, [sp, #16 * 9]
	ldp	x20, x21, [sp, #16 * 10]
	ldp	x22, x23, [sp, #16 * 11]
	ldp	x24, x25, [sp, #16 * 12]
	ldp	x26, x27, [sp, #16 * 13]
	ldp	x28, x29, [sp, #16 * 14]

	add	sp, sp, #CONTEXT_FRAME_SIZE

	eret

handle_fiq:
	/* save context */
	sub	sp, sp, #CONTEXT_FRAME_SIZE

	stp	x0, x1, [sp, #16 * 0]
	stp	x2, x3, [sp, #16 * 1]
	stp	x4, x5, [sp, #16 * 2]
	stp	x6, x7, [sp, #16 * 3]
	stp	x8, x9, [sp, #16 * 4]
	stp	x10, x11, [sp, #16 * 5]
	stp	x12, x13, [sp, #16 * 6]
	stp	x14, x15, [sp, #16 * 7]
	stp	x16, x17, [sp, #16 * 8]
	stp	x18, x19, [sp, #16 * 9]
	stp	x20, x21, [sp, #16 * 10]
	stp	x22, x23, [sp, #16 * 11]
	stp	x24, x25, [sp, #16 * 12]
	stp	x26, x27, [sp, #16 * 13]
	stp	x28, x29, [sp, #16 * 14]
	mrs	x0, elr_el3
	stp	x30, x0, [sp, #16 * 15]	/* lr and elr */

	mov	x0, sp
	bl	fiq_entry

	/* restore elr first, for we need a general purpose register to hold the
	 * intermediate data
	 */
	ldp	x30, x0, [sp, #16 * 15]
	msr	elr_el3, x0

	ldp	x0, x1, [sp, #16 * 0]
	ldp	x2, x3, [sp, #16 * 1]
	ldp	x4, x5, [sp, #16 * 2]
	ldp	x6, x7, [sp, #16 * 3]
	ldp	x8, x9, [sp, #16 * 4]
	ldp	x10, x11, [sp, #16 * 5]
	ldp	x12, x13, [sp, #16 * 6]
	ldp	x14, x15, [sp, #16 * 7]
	ldp	x16, x17, [sp, #16 * 8]
	ldp	x18, x19, [sp, #16 * 9]
	ldp	x20, x21, [sp, #16 * 10]
	ldp	x22, x23, [sp, #16 * 11]
	ldp	x24, x25, [sp, #16 * 12]
	ldp	x26, x27, [sp, #16 * 13]
	ldp	x28, x29, [sp, #16 * 14]

	add	sp, sp, #CONTEXT_FRAME_SIZE

	eret

	.align  8
	.global smp_core_id
	.type   smp_core_id, %function

smp_core_id:
	mrs x0, mpidr_el1
	and x1, x0, #MPIDR_CPU_MASK
	and x0, x0, #MPIDR_CLUSTER_MASK
	add x0, x1, x0, LSR #6
	ret
	.size   smp_core_id, . - smp_core_id
#ifdef CONFIG_SUPPORT_SMP
smp_wait:
	bl	smp_core_id
	mov x2, x0
	adr_l x1, smp_context
	lsr x2, x2, #SMP_CONTEXT_SIZE_SHIFT
	add x2, x2, x1
	str xzr, [x2, #SMP_CONTEXT_SP_OFFSET]
	str xzr, [x2, #SMP_CONTEXT_FN_OFFSET]
	str xzr, [x2, #SMP_CONTEXT_PRIV_OFFSET]

1:
	wfe
	ldr	x3, [x2, #SMP_CONTEXT_FN_OFFSET]
	cmp x3, 0
	beq 1b
	ldr x0, [x2, #SMP_CONTEXT_SP_OFFSET]
	ldr x1, [x2, #SMP_CONTEXT_STATCKSIZE_OFFSET]
	add x0, x0, x1
	mov sp, x0
	ldr x0, [x2, #SMP_CONTEXT_PRIV_OFFSET]
	blr x3
	b smp_wait
#else
smp_wait:
	b smp_wait
#endif
