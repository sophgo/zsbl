/* Copyright 2019-2022 SiFive, Inc */
/* SPDX-License-Identifier: Apache-2.0 */
/* ----------------------------------- */
/* ----------------------------------- */

#ifndef METAL_MACHINE_H
#define METAL_MACHINE_H

#include <sifive_metal.h>
#include <sifive_buserror0.h>
#include <stddef.h>
#include <cpu.h>


#define METAL_CACHE_DRIVER_PREFIX sifive_extensiblecache0

#define METAL_SIFIVE_EXTENSIBLECACHE0_PERFMON_COUNTERS 0

#define __METAL_DT_SIFIVE_EXTENSIBLECACHE0_HANDLE (struct metal_cache *)NULL


#define METAL_CACHE_DRIVER_PREFIX sifive_extensiblecache0

#define METAL_SIFIVE_EXTENSIBLECACHE0_PERFMON_COUNTERS 0

#define __METAL_DT_SIFIVE_EXTENSIBLECACHE0_HANDLE (struct metal_cache *)NULL


#define METAL_CACHE_DRIVER_PREFIX sifive_extensiblecache0

#define METAL_SIFIVE_EXTENSIBLECACHE0_PERFMON_COUNTERS 0

#define __METAL_DT_SIFIVE_EXTENSIBLECACHE0_HANDLE (struct metal_cache *)NULL


#define METAL_CACHE_DRIVER_PREFIX sifive_extensiblecache0

#define METAL_SIFIVE_EXTENSIBLECACHE0_PERFMON_COUNTERS 0

#define __METAL_DT_SIFIVE_EXTENSIBLECACHE0_HANDLE (struct metal_cache *)NULL


#define METAL_CACHE_DRIVER_PREFIX sifive_extensiblecache0

#define METAL_SIFIVE_EXTENSIBLECACHE0_PERFMON_COUNTERS 0

#define __METAL_DT_SIFIVE_EXTENSIBLECACHE0_HANDLE (struct metal_cache *)NULL


#define METAL_CACHE_DRIVER_PREFIX sifive_extensiblecache0

#define METAL_SIFIVE_EXTENSIBLECACHE0_PERFMON_COUNTERS 0

#define __METAL_DT_SIFIVE_EXTENSIBLECACHE0_HANDLE (struct metal_cache *)NULL


#define METAL_CACHE_DRIVER_PREFIX sifive_extensiblecache0

#define METAL_SIFIVE_EXTENSIBLECACHE0_PERFMON_COUNTERS 0

#define __METAL_DT_SIFIVE_EXTENSIBLECACHE0_HANDLE (struct metal_cache *)NULL


#define METAL_CACHE_DRIVER_PREFIX sifive_extensiblecache0

#define METAL_SIFIVE_EXTENSIBLECACHE0_PERFMON_COUNTERS 0

#define __METAL_DT_SIFIVE_EXTENSIBLECACHE0_HANDLE (struct metal_cache *)NULL


#define METAL_CACHE_DRIVER_PREFIX sifive_extensiblecache0

#define METAL_SIFIVE_EXTENSIBLECACHE0_PERFMON_COUNTERS 0

#define __METAL_DT_SIFIVE_EXTENSIBLECACHE0_HANDLE (struct metal_cache *)NULL


#define METAL_CACHE_DRIVER_PREFIX sifive_extensiblecache0

#define METAL_SIFIVE_EXTENSIBLECACHE0_PERFMON_COUNTERS 0

#define __METAL_DT_SIFIVE_EXTENSIBLECACHE0_HANDLE (struct metal_cache *)NULL


#define METAL_CACHE_DRIVER_PREFIX sifive_extensiblecache0

#define METAL_SIFIVE_EXTENSIBLECACHE0_PERFMON_COUNTERS 0

#define __METAL_DT_SIFIVE_EXTENSIBLECACHE0_HANDLE (struct metal_cache *)NULL


#define METAL_CACHE_DRIVER_PREFIX sifive_extensiblecache0

#define METAL_SIFIVE_EXTENSIBLECACHE0_PERFMON_COUNTERS 0

#define __METAL_DT_SIFIVE_EXTENSIBLECACHE0_HANDLE (struct metal_cache *)NULL


#define METAL_CACHE_DRIVER_PREFIX sifive_extensiblecache0

#define METAL_SIFIVE_EXTENSIBLECACHE0_PERFMON_COUNTERS 0

#define __METAL_DT_SIFIVE_EXTENSIBLECACHE0_HANDLE (struct metal_cache *)NULL


#define METAL_CACHE_DRIVER_PREFIX sifive_extensiblecache0

#define METAL_SIFIVE_EXTENSIBLECACHE0_PERFMON_COUNTERS 0

#define __METAL_DT_SIFIVE_EXTENSIBLECACHE0_HANDLE (struct metal_cache *)NULL


#define METAL_CACHE_DRIVER_PREFIX sifive_extensiblecache0

#define METAL_SIFIVE_EXTENSIBLECACHE0_PERFMON_COUNTERS 0

#define __METAL_DT_SIFIVE_EXTENSIBLECACHE0_HANDLE (struct metal_cache *)NULL


#define METAL_CACHE_DRIVER_PREFIX sifive_extensiblecache0

#define METAL_SIFIVE_EXTENSIBLECACHE0_PERFMON_COUNTERS 0

#define __METAL_DT_SIFIVE_EXTENSIBLECACHE0_HANDLE (struct metal_cache *)NULL


#define METAL_SIFIVE_EXTENSIBLECACHE0_SLICES {\
				{METAL_SIFIVE_EC_SLICE_2010000_BASE_ADDRESS, METAL_SIFIVE_EC_SLICE_2010000_SIZE}, \
				{METAL_SIFIVE_EC_SLICE_2014000_BASE_ADDRESS, METAL_SIFIVE_EC_SLICE_2014000_SIZE}, \
				{METAL_SIFIVE_EC_SLICE_2018000_BASE_ADDRESS, METAL_SIFIVE_EC_SLICE_2018000_SIZE}, \
				{METAL_SIFIVE_EC_SLICE_201C000_BASE_ADDRESS, METAL_SIFIVE_EC_SLICE_201C000_SIZE}, \
				{METAL_SIFIVE_EC_SLICE_2020000_BASE_ADDRESS, METAL_SIFIVE_EC_SLICE_2020000_SIZE}, \
				{METAL_SIFIVE_EC_SLICE_2024000_BASE_ADDRESS, METAL_SIFIVE_EC_SLICE_2024000_SIZE}, \
				{METAL_SIFIVE_EC_SLICE_2028000_BASE_ADDRESS, METAL_SIFIVE_EC_SLICE_2028000_SIZE}, \
				{METAL_SIFIVE_EC_SLICE_202C000_BASE_ADDRESS, METAL_SIFIVE_EC_SLICE_202C000_SIZE}, \
				{METAL_SIFIVE_EC_SLICE_2030000_BASE_ADDRESS, METAL_SIFIVE_EC_SLICE_2030000_SIZE}, \
				{METAL_SIFIVE_EC_SLICE_2034000_BASE_ADDRESS, METAL_SIFIVE_EC_SLICE_2034000_SIZE}, \
				{METAL_SIFIVE_EC_SLICE_2038000_BASE_ADDRESS, METAL_SIFIVE_EC_SLICE_2038000_SIZE}, \
				{METAL_SIFIVE_EC_SLICE_203C000_BASE_ADDRESS, METAL_SIFIVE_EC_SLICE_203C000_SIZE}, \
				{METAL_SIFIVE_EC_SLICE_2040000_BASE_ADDRESS, METAL_SIFIVE_EC_SLICE_2040000_SIZE}, \
				{METAL_SIFIVE_EC_SLICE_2044000_BASE_ADDRESS, METAL_SIFIVE_EC_SLICE_2044000_SIZE}, \
				{METAL_SIFIVE_EC_SLICE_2048000_BASE_ADDRESS, METAL_SIFIVE_EC_SLICE_2048000_SIZE}, \
				{METAL_SIFIVE_EC_SLICE_204C000_BASE_ADDRESS, METAL_SIFIVE_EC_SLICE_204C000_SIZE}, \
		}

#define METAL_MAX_LOCAL_EXT_INTERRUPTS 0

#define __METAL_GLOBAL_EXTERNAL_INTERRUPTS_INTERRUPTS 255

#define METAL_MAX_GLOBAL_EXT_INTERRUPTS 255

#define METAL_HWPF1_WINDOWBITS 6

#define METAL_HWPF1_DISTANCEBITS 6

#define METAL_HWPF1_MAXPFDISTANCEBITS 9

#define METAL_HWPF1_QFULLNESSTHRDBITS 4

#define METAL_HWPF1_HITCACHETHRDBITS 5

#define METAL_HWPF1_HITMSHRTHRDBITS 4

#define METAL_HWPF1_NSTREAMS 8

#define METAL_HWPF1_NBPMSTREAMS 16

#define METAL_HWPF1_NISSQENT 6

#define METAL_HWPF1_L2PFPOOLSIZE 10

#define METAL_HWPF1_NPREPFETCHQUEUEENTRIES 2


#define HWPFCSR_ENABLETWJ_OFFSET 0

#define HWPFCSR_WINDOW_OFFSET 1

#define HWPFCSR_DIST_OFFSET 7

#define HWPFCSR_MAXALLOWEDDIST_OFFSET 13

#define HWPFCSR_LINTOEXPTHRD_OFFSET 22

#define HWPFCSR_QFULLNESSTHRDL1_OFFSET 28

#define HWPFCSR_HITCACHETHRDL1_OFFSET 32

#define HWPFCSR_HITMSHRTHRDL1_OFFSET 37

#define HWPFCSR_ISSUEBUBBLE_OFFSET 41

#define HWPFCSR_MAXL1PFDIST_OFFSET 42

#define HWPFCSR_NTLL1_OFFSET 48

#define HWPFCSR_ENABLEBPM_OFFSET 49

#define HWPFCSR_NUML1PFLSSQENT_OFFSET 50

#define HWPFCSR_FORGIVETHRD_OFFSET 53

#define HWPFCSR_FORGIVEMAXDELTA_OFFSET 57


#define HWPF2CSR_L2PFENABLE_OFFSET 0

#define HWPF2CSR_QFULLNESSTHRDL2_OFFSET 1

#define HWPF2CSR_HITCACHETHRDL2_OFFSET 5

#define HWPF2CSR_HITMSHRTHRDL2_OFFSET 10

#define HWPF2CSR_NUML2PFLSSQENT_OFFSET 14


#define METAL_PL2CACHE_DRIVER_PREFIX sifive_pl2cache0

#define METAL_SIFIVE_PL2CACHE0_BASE_ADDR {\
				METAL_SIFIVE_PL2CACHE2_0_BASE_ADDRESS,\
				METAL_SIFIVE_PL2CACHE2_1_BASE_ADDRESS,\
				METAL_SIFIVE_PL2CACHE2_2_BASE_ADDRESS,\
				METAL_SIFIVE_PL2CACHE2_3_BASE_ADDRESS,\
				METAL_SIFIVE_PL2CACHE2_4_BASE_ADDRESS,\
				METAL_SIFIVE_PL2CACHE2_5_BASE_ADDRESS,\
				METAL_SIFIVE_PL2CACHE2_6_BASE_ADDRESS,\
				METAL_SIFIVE_PL2CACHE2_7_BASE_ADDRESS,\
				METAL_SIFIVE_PL2CACHE2_8_BASE_ADDRESS,\
				METAL_SIFIVE_PL2CACHE2_9_BASE_ADDRESS,\
				METAL_SIFIVE_PL2CACHE2_10_BASE_ADDRESS,\
				METAL_SIFIVE_PL2CACHE2_11_BASE_ADDRESS,\
				METAL_SIFIVE_PL2CACHE2_12_BASE_ADDRESS,\
				METAL_SIFIVE_PL2CACHE2_13_BASE_ADDRESS,\
				METAL_SIFIVE_PL2CACHE2_14_BASE_ADDRESS,\
				METAL_SIFIVE_PL2CACHE2_15_BASE_ADDRESS,\
				}

#define METAL_SIFIVE_PL2CACHE0_PERFMON_COUNTERS 6

#define __METAL_DT_SIFIVE_PL2CACHE0_HANDLE (struct metal_cache *)NULL

/* From cpu@0 */
extern struct __metal_driver_cpu __metal_dt_cpu_0;

/* From cpu@1 */
extern struct __metal_driver_cpu __metal_dt_cpu_1;

/* From cpu@2 */
extern struct __metal_driver_cpu __metal_dt_cpu_2;

/* From cpu@3 */
extern struct __metal_driver_cpu __metal_dt_cpu_3;

/* From cpu@4 */
extern struct __metal_driver_cpu __metal_dt_cpu_4;

/* From cpu@5 */
extern struct __metal_driver_cpu __metal_dt_cpu_5;

/* From cpu@6 */
extern struct __metal_driver_cpu __metal_dt_cpu_6;

/* From cpu@7 */
extern struct __metal_driver_cpu __metal_dt_cpu_7;

/* From cpu@8 */
extern struct __metal_driver_cpu __metal_dt_cpu_8;

/* From cpu@9 */
extern struct __metal_driver_cpu __metal_dt_cpu_9;

/* From cpu@a */
extern struct __metal_driver_cpu __metal_dt_cpu_a;

/* From cpu@b */
extern struct __metal_driver_cpu __metal_dt_cpu_b;

/* From cpu@c */
extern struct __metal_driver_cpu __metal_dt_cpu_c;

/* From cpu@d */
extern struct __metal_driver_cpu __metal_dt_cpu_d;

/* From cpu@e */
extern struct __metal_driver_cpu __metal_dt_cpu_e;

/* From cpu@f */
extern struct __metal_driver_cpu __metal_dt_cpu_f;

/* From bus_error_unit@1700000 */
extern struct metal_buserror __metal_dt_bus_error_unit_1700000;

/* From bus_error_unit@1704000 */
extern struct metal_buserror __metal_dt_bus_error_unit_1704000;

/* From bus_error_unit@1708000 */
extern struct metal_buserror __metal_dt_bus_error_unit_1708000;

/* From bus_error_unit@170c000 */
extern struct metal_buserror __metal_dt_bus_error_unit_170c000;

/* From bus_error_unit@1710000 */
extern struct metal_buserror __metal_dt_bus_error_unit_1710000;

/* From bus_error_unit@1714000 */
extern struct metal_buserror __metal_dt_bus_error_unit_1714000;

/* From bus_error_unit@1718000 */
extern struct metal_buserror __metal_dt_bus_error_unit_1718000;

/* From bus_error_unit@171c000 */
extern struct metal_buserror __metal_dt_bus_error_unit_171c000;

/* From bus_error_unit@1720000 */
extern struct metal_buserror __metal_dt_bus_error_unit_1720000;

/* From bus_error_unit@1724000 */
extern struct metal_buserror __metal_dt_bus_error_unit_1724000;

/* From bus_error_unit@1728000 */
extern struct metal_buserror __metal_dt_bus_error_unit_1728000;

/* From bus_error_unit@172c000 */
extern struct metal_buserror __metal_dt_bus_error_unit_172c000;

/* From bus_error_unit@1730000 */
extern struct metal_buserror __metal_dt_bus_error_unit_1730000;

/* From bus_error_unit@1734000 */
extern struct metal_buserror __metal_dt_bus_error_unit_1734000;

/* From bus_error_unit@1738000 */
extern struct metal_buserror __metal_dt_bus_error_unit_1738000;

/* From bus_error_unit@173c000 */
extern struct metal_buserror __metal_dt_bus_error_unit_173c000;

static __inline__ uintptr_t __metal_driver_sifive_buserror0_control_base(const struct metal_buserror *buserror)
{
	if ((uintptr_t)buserror == (uintptr_t)&__metal_dt_bus_error_unit_1700000) {
		return METAL_SIFIVE_BUSERROR0_1700000_BASE_ADDRESS;
	}
	else if ((uintptr_t)buserror == (uintptr_t)&__metal_dt_bus_error_unit_1704000) {
		return METAL_SIFIVE_BUSERROR0_1704000_BASE_ADDRESS;
	}
	else if ((uintptr_t)buserror == (uintptr_t)&__metal_dt_bus_error_unit_1708000) {
		return METAL_SIFIVE_BUSERROR0_1708000_BASE_ADDRESS;
	}
	else if ((uintptr_t)buserror == (uintptr_t)&__metal_dt_bus_error_unit_170c000) {
		return METAL_SIFIVE_BUSERROR0_170C000_BASE_ADDRESS;
	}
	else if ((uintptr_t)buserror == (uintptr_t)&__metal_dt_bus_error_unit_1710000) {
		return METAL_SIFIVE_BUSERROR0_1710000_BASE_ADDRESS;
	}
	else if ((uintptr_t)buserror == (uintptr_t)&__metal_dt_bus_error_unit_1714000) {
		return METAL_SIFIVE_BUSERROR0_1714000_BASE_ADDRESS;
	}
	else if ((uintptr_t)buserror == (uintptr_t)&__metal_dt_bus_error_unit_1718000) {
		return METAL_SIFIVE_BUSERROR0_1718000_BASE_ADDRESS;
	}
	else if ((uintptr_t)buserror == (uintptr_t)&__metal_dt_bus_error_unit_171c000) {
		return METAL_SIFIVE_BUSERROR0_171C000_BASE_ADDRESS;
	}
	else if ((uintptr_t)buserror == (uintptr_t)&__metal_dt_bus_error_unit_1720000) {
		return METAL_SIFIVE_BUSERROR0_1720000_BASE_ADDRESS;
	}
	else if ((uintptr_t)buserror == (uintptr_t)&__metal_dt_bus_error_unit_1724000) {
		return METAL_SIFIVE_BUSERROR0_1724000_BASE_ADDRESS;
	}
	else if ((uintptr_t)buserror == (uintptr_t)&__metal_dt_bus_error_unit_1728000) {
		return METAL_SIFIVE_BUSERROR0_1728000_BASE_ADDRESS;
	}
	else if ((uintptr_t)buserror == (uintptr_t)&__metal_dt_bus_error_unit_172c000) {
		return METAL_SIFIVE_BUSERROR0_172C000_BASE_ADDRESS;
	}
	else if ((uintptr_t)buserror == (uintptr_t)&__metal_dt_bus_error_unit_1730000) {
		return METAL_SIFIVE_BUSERROR0_1730000_BASE_ADDRESS;
	}
	else if ((uintptr_t)buserror == (uintptr_t)&__metal_dt_bus_error_unit_1734000) {
		return METAL_SIFIVE_BUSERROR0_1734000_BASE_ADDRESS;
	}
	else if ((uintptr_t)buserror == (uintptr_t)&__metal_dt_bus_error_unit_1738000) {
		return METAL_SIFIVE_BUSERROR0_1738000_BASE_ADDRESS;
	}
	else if ((uintptr_t)buserror == (uintptr_t)&__metal_dt_bus_error_unit_173c000) {
		return METAL_SIFIVE_BUSERROR0_173C000_BASE_ADDRESS;
	}
	else {
		return 0;
	}
}


static __inline__ struct metal_buserror * __metal_driver_cpu_buserror(struct metal_cpu *cpu)
{
	if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_0) {
		return &__metal_dt_bus_error_unit_1700000;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_1) {
		return &__metal_dt_bus_error_unit_1704000;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_2) {
		return &__metal_dt_bus_error_unit_1708000;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_3) {
		return &__metal_dt_bus_error_unit_170c000;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_4) {
		return &__metal_dt_bus_error_unit_1710000;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_5) {
		return &__metal_dt_bus_error_unit_1714000;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_6) {
		return &__metal_dt_bus_error_unit_1718000;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_7) {
		return &__metal_dt_bus_error_unit_171c000;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_8) {
		return &__metal_dt_bus_error_unit_1720000;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_9) {
		return &__metal_dt_bus_error_unit_1724000;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_a) {
		return &__metal_dt_bus_error_unit_1728000;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_b) {
		return &__metal_dt_bus_error_unit_172c000;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_c) {
		return &__metal_dt_bus_error_unit_1730000;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_d) {
		return &__metal_dt_bus_error_unit_1734000;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_e) {
		return &__metal_dt_bus_error_unit_1738000;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_f) {
		return &__metal_dt_bus_error_unit_173c000;
	}
	else {
		return NULL;
	}
}

struct __metal_driver_cpu *__metal_cpu_table[] __attribute__((weak))  = {
					&__metal_dt_cpu_0,
					&__metal_dt_cpu_1,
					&__metal_dt_cpu_2,
					&__metal_dt_cpu_3,
					&__metal_dt_cpu_4,
					&__metal_dt_cpu_5,
					&__metal_dt_cpu_6,
					&__metal_dt_cpu_7,
					&__metal_dt_cpu_8,
					&__metal_dt_cpu_9,
					&__metal_dt_cpu_a,
					&__metal_dt_cpu_b,
					&__metal_dt_cpu_c,
					&__metal_dt_cpu_d,
					&__metal_dt_cpu_e,
					&__metal_dt_cpu_f
};

#endif
