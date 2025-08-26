#include <stdio.h>
#include <timer.h>
#include <string.h>
#include <framework/module.h>
#include <framework/common.h>
#include <lib/libc/errno.h>
#include <atomic.h>
#include <spinlock.h>
#include <smp.h>
#include <asm.h>
#include <platform.h>
#include <memmap.h>
#include <asm.h>
#include <lib/mmio.h>
#include <lib/mac.h>
#include <driver/bootdev.h>
#include <driver/platform.h>
#include <driver/blkdev.h>
#include <sbi.h>
#include <smp.h>
#include <libfdt.h>
#include <fdt_dump.h>
#include <of.h>
#include <lib/cli.h>
#include <lib/akcipher.h>
#include <driver/dtb.h>
#include <stdint.h>

enum c2clat_test_mode{
	TEST_MODE_LRSC2LRSC_NCORE = 0,
	TEST_MODE_LRSC2LRSC_2CORE,
	TEST_MODE_AMO2LRSC,
};
enum c2clat_test_mode test_mode = TEST_MODE_LRSC2LRSC_NCORE;

static atomic_t c2c_counter;
static atomic_t c2c_start_ping_pong;
static atomic_t c2c_value_ping_pong;
static atomic_t atomic_start_flag = ATOMIC_INITIALIZER(0);

static uint32_t test_cycles        = 10;
static uint32_t core_num           = 64;
static uint32_t test_cycles_cycles = 10;

static volatile long start_value = -63;
static volatile long flag_check_value = 0xffffffffffffffffUL;

static unsigned char secondary_core_stack[CONFIG_SMP_NUM][4096];

uint64_t gen_check_flag(int n) {
    if (n == 0) {
        return 0;
    } else if (n == 64) {
        return ~0ULL;
    } else {
        return (1ULL << n) - 1;
    }
}

static uint64_t set_flag(uint32_t hartid, atomic_t *atom)
{
    uint64_t ulTemp, ulFlag = (0x1UL << hartid);
    asm volatile (
        "amoor.d %0, %2, %1\n" "\tfence r , rw\n"
        :"=r"(ulTemp),"+A"(atom->counter)
        :"r"(ulFlag)
        :"memory");
    return(ulTemp | ulFlag);
}

static uint64_t check_flag(atomic_t *atom, uint64_t value)
{
    return(((atomic_read(atom) & value) ^ value) ? 0 : 1 );
}

static void next_core_fun(void *priv)
{
    unsigned int hartid;
	hartid = current_hartid();
    set_flag(hartid, &atomic_start_flag);

    while(!check_flag(&atomic_start_flag, flag_check_value))
        ;

	for (int n = 0; n < test_cycles; ++n) {
		int cmp;
		do {
			cmp = core_num * n - current_hartid();
		} while (atomic_cmpxchg(&c2c_counter, cmp, cmp + 1) != cmp);
	}
}

static int c2clat_lrsc_ncore(struct command *c)
{
	uint64_t start, end;
    int j;

    unsigned int hartid = current_hartid();

    for (j = 0; j < core_num; ++j) {
        if (j == current_hartid())
            continue;
        wake_up_other_core(j, next_core_fun, NULL,
				secondary_core_stack[j], sizeof(secondary_core_stack[j]));

    }

    set_flag(hartid, &atomic_start_flag);
    while(!check_flag(&atomic_start_flag, flag_check_value))
        ;

    start = timer_get_tick();
    for (int n = 0; n < test_cycles; ++n) {
        int cmp;
        do {
            cmp = core_num * n - current_hartid();
        } while (atomic_cmpxchg(&c2c_counter, cmp, cmp + 1) != cmp);
    }
    while (atomic_read(&c2c_counter) != (core_num * (test_cycles - 1) - current_hartid() + 1))
        ;
    end = timer_get_tick();

    console_printf(c->console,"latency: total %04llu ticks, once %03llu ns\n", end - start, (end - start)*20/(core_num*test_cycles));

    return 0;
}

static void init_atomic_vars(void)
{
    atomic_xchg(&c2c_counter, -1);
    atomic_xchg(&c2c_start_ping_pong, -1);
    atomic_xchg(&c2c_value_ping_pong, 0);
}

static void atomic_ping_to_pong(volatile long *ptr)
{
    volatile long old_val;
    volatile long set_val = 1;
    __asm__ __volatile__(
        "1:\n"
        "amoor.d %0, %2, (%1)\n"
        "bnez %0, 1b\n"
        : "=&r" (old_val)
        : "r" (ptr),
          "r" (set_val)
        : "memory"
    );
}

static void c2clat_pong_thread(void *priv)
{
	while (atomic_read(&c2c_start_ping_pong) != 0)
	    ;
	atomic_xchg(&c2c_start_ping_pong,1);

    for (int n = 0; n < test_cycles; ++n) {
		atomic_ping_to_pong(&(c2c_value_ping_pong.counter));
	}
}

static int c2clat_ping_thread(struct command *c)
{
	uint64_t start, end;
    int j;

    for (j = 0; j < CONFIG_SMP_NUM; ++j) {
        if (j == current_hartid())
            continue;

        init_atomic_vars();

        wake_up_other_core(j, c2clat_pong_thread, NULL,
				secondary_core_stack[j], sizeof(secondary_core_stack[j]));

        atomic_xchg(&c2c_start_ping_pong,0);
        while (atomic_read(&c2c_start_ping_pong) == 0)
            ;
        atomic_xchg(&c2c_start_ping_pong,-1);

        start = timer_get_tick();
        for (int n = 0; n < test_cycles; ++n) {
            while (atomic_cmpxchg(&c2c_value_ping_pong, 1, 0) != 1)
                ;
        }

        end = timer_get_tick();
        console_printf(c->console,"%02d <----> %03d latency: total %04llu ticks, once %03llu ns\n", current_hartid(), j, end - start, (end - start)*20/(2 * test_cycles));
    }
    return 0;
}


static void secondary_core_fun(void *priv)
{
	while (atomic_read(&c2c_start_ping_pong) != 0)
	    ;
	atomic_xchg(&c2c_start_ping_pong,1);

	for (int n = 0; n < test_cycles; ++n) {
		int cmp;
		do {
			cmp = 2 * n;
		} while (atomic_cmpxchg(&c2c_counter, cmp, cmp + 1) != cmp);
	}
}

static int c2clat_lrsc_2core(struct command *c)
{
	uint64_t start, end;
    int j;

    for (j = 0; j < CONFIG_SMP_NUM; ++j) {
        if (j == current_hartid())
            continue;

        init_atomic_vars();

        wake_up_other_core(j, secondary_core_fun, NULL,
				secondary_core_stack[j], sizeof(secondary_core_stack[j]));

        atomic_xchg(&c2c_start_ping_pong,0);
        while (atomic_read(&c2c_start_ping_pong) == 0)
            ;
        atomic_xchg(&c2c_start_ping_pong,-1);

        start = timer_get_tick();
        for (int n = 0; n < test_cycles; ++n) {
            int cmp;
            do {
                cmp = 2 * n - 1;
            } while (atomic_cmpxchg(&c2c_counter, cmp, cmp + 1) != cmp);
        }
        while (atomic_read(&c2c_counter) != (2 * test_cycles - 1))
            ;
        end = timer_get_tick();

        console_printf(c->console,"%02d <----> %03d latency: total %04llu ticks, once %03llu ns\n", current_hartid(), j, end - start, (end - start)*20/(2 * test_cycles));
    }
    return 0;
}

static void c2clat_cmd(struct command *c, int argc, const char *argv[])
{
	if (argc != 5) {
		console_printf(c->console, "Invalid arguments\n");
		console_printf(c->console, "Useage: c2clat <Type> <CoreNum> <Cycles> <Cycles-Cycles>\n");
		console_printf(c->console, "        <Type=0: lrsc2lrsc_ncore>\n");
		console_printf(c->console, "        <Type=1: lrsc2lrsc_2core,Only supports 2 cores>\n");
		console_printf(c->console, "        <Type=2: amo2lrsc,Only supports 2 cores>\n");
		return;
	}

	test_mode = strtoul(argv[1], NULL, 0);
	core_num = strtoul(argv[2], NULL, 0);
	test_cycles = strtoul(argv[3], NULL, 0);
	test_cycles_cycles = strtoul(argv[4], NULL, 0);

	if (test_mode > TEST_MODE_AMO2LRSC) {
		console_printf(c->console, "Invalid test type\n");
		return;
	}

	if (core_num > CONFIG_SMP_NUM) {
		console_printf(c->console, "Invalid core number\n");
		return;
	}
	start_value = -((long)core_num - 1);
	flag_check_value = gen_check_flag(core_num);

	if (test_mode == TEST_MODE_LRSC2LRSC_NCORE) {
		console_printf(c->console,"Start %d cores %d cycles %d cycles_cycles test!\n", core_num, test_cycles, test_cycles_cycles);
		console_printf(c->console,"Start value     : %ld\n", start_value);
		console_printf(c->console,"Flag check value: 0x%lx\n", flag_check_value);

		for (int n = 0; n < test_cycles_cycles; ++n) {
			atomic_xchg(&atomic_start_flag, 0);
			atomic_xchg(&c2c_counter, start_value);
			c2clat_lrsc_ncore(c);

		}
	} else if (test_mode == TEST_MODE_LRSC2LRSC_2CORE) {
		console_printf(c->console,"Start 2 cores %d cycles %d cycles_cycles test!\n", test_cycles, test_cycles_cycles);
		for (int n = 0; n < test_cycles_cycles; ++n) {
			c2clat_lrsc_2core(c);
		}
	} else if (test_mode == TEST_MODE_AMO2LRSC) {
		console_printf(c->console,"Start 2 cores %d cycles %d cycles_cycles AMO2LRSC test!\n", core_num, test_cycles, test_cycles_cycles);

		for (int n = 0; n < test_cycles_cycles; ++n) {
			c2clat_ping_thread(c);
		}
	}
}

cli_command(c2clat, c2clat_cmd);

static int  c2clat_test(void)
{
    cli_loop(0);
    return 0;
}

test_case(c2clat_test);




