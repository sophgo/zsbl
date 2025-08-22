#include <stdio.h>
#include <timer.h>
#include <arch.h>
#include <common/module.h>
#include <common/common.h>

enum {
	FAB_TEST_NT		= 0,
	FAB_TEST_PASS,
	FAB_TEST_FAIL,
};

struct region {
	unsigned long addr;
	int64_t write;
	int64_t expected;
	int (*func)(struct region *region);
	char *name;
	int result;
};

static int test_check(unsigned long addr, uint32_t expected)
{
	uint32_t read_back = readl(addr);
	if (expected != read_back) {
		pr_err("expected 0x%08x but 0x%08x get\n", expected, read_back);
		return 1;
	}

	return 0;
}

static inline const char *test_result(struct region *r)
{
	if (r->result == FAB_TEST_PASS)
		return "PASS";
	else if (r->result == FAB_TEST_FAIL)
		return "FAIL";
	else
		return "NOT-TESTED";
}

void show_region(struct region *r)
{
	pr_info("%08lx %16s",
		r->addr, r->name);
}

static int test_region(struct region *r)
{
	int err;

	err = 0;

	show_region(r);

	/* basic read should normally return */
	readl(r->addr);

	if (r->func)
		err |= r->func(r);

	if (r->write >= 0)
		writel(r->write, r->addr);

	if (r->expected >= 0)
		err |= test_check(r->addr, r->expected);

	r->result = err ? FAB_TEST_FAIL : FAB_TEST_PASS;

	pr_info("    [%s]\n", test_result(r));

	return err;
}

#define WRITE_CHECK(addr, name) \
	{addr, 0xdeadbeef, 0xdeadbeef, NULL, name}
#define READ_ONLY(addr, name) \
	{addr, -1, -1, NULL, name}
#define READ_CHECK(addr, expected, name) \
	{addr, -1, expected, NULL, name}
#define WRITE_CHECK_VALUE(addr, value, name) \
	{addr, 0x5a, 0x5a, NULL, name}

struct region regions[] = {
	WRITE_CHECK(0x10000000, "AXI SRAM"),
	READ_ONLY(0x07000000, "AHB ROM"),
	READ_ONLY(0x06000000, "SPI flash"),
	WRITE_CHECK(0x08000000, "TPU local"),
	WRITE_CHECK(0x09000000, "TPU static"),
	READ_ONLY(0x58001000, "TPU"),
	READ_CHECK(0x58000000, 4, "GDMA"),
	READ_CHECK(0x5800397c, 0, "CDMA"),
	READ_CHECK(0x02008010, 4, "GDE"),
	READ_ONLY(0x02009000, "SORT"),
	READ_ONLY(0x0200a000, "NMS"),
	READ_CHECK(0x0200b008, 0x3130312a, "TSDMA"),
	READ_CHECK(0x5f800000, 0x16861f1c, "PCIE config"),
	READ_ONLY(0x58002000, "MMU"),
	WRITE_CHECK_VALUE(0x50028004, 0x5a, "eFuse"),
	WRITE_CHECK(0x50100000, "eMMC"),
	WRITE_CHECK(0x50101000, "SDIO"),
	READ_CHECK(0x50108110, 0x50, "ETH0"),
	READ_CHECK(0x5010c110, 0x50, "ETH1"),
	READ_ONLY(0x50027050, "GPIO0"),
	READ_ONLY(0x50027400, "GPIO1"),
	READ_ONLY(0x50027800, "GPIO2"),
	READ_ONLY(0x50118000, "UART0"),
	READ_CHECK(0x50010000, 0x16860000, "TOP"),
	READ_ONLY(0x5001a000, "I2C0"),
	READ_ONLY(0x5001c000, "I2C1"),
	READ_ONLY(0x5001e000, "I2C2"),
	READ_CHECK(0x500230e0, 0x3230352a, "INTC0"),
	READ_CHECK(0x5002b0e0, 0x3230352a, "INTC1"),
	READ_CHECK(0x5002c0e0, 0x3230352a, "INTC2"),
	READ_CHECK(0x5002d0e0, 0x3230352a, "INTC3"),
	READ_CHECK(0x50022000, 0, "TIMER"),
	READ_ONLY(0x50026000, "WDT"),
	READ_CHECK(0x50029000, 0, "PWM"),
	READ_CHECK(0x12008000, 0, "SPACC"),
	READ_CHECK(0x12000000, 0, "PKA"),
	READ_CHECK(0x50018008, 0x28, "TRNG"),
	READ_CHECK(0x68000000, 0x83080020, "DDR2A"),
	READ_CHECK(0x6a000000, 0x83080020, "DDR0A"),
	READ_CHECK(0x6c000000, 0x83080020, "DDR0B"),
	READ_CHECK(0x6e000000, 0x83080020, "DDR1"),
	READ_CHECK(0x50008000, 0, "FAB24PMU"),
	READ_CHECK(0x50008400, 0, "FAB25PMU"),
	READ_CHECK(0x50008800, 0, "AXI Fab5 PMU"),
	READ_CHECK(0x50008c00, 0, "VD1 AXI Fab5 PMU"),
	READ_CHECK(0x5000e000, 7, "TOP SE"),
	READ_CHECK(0x5000e400, 7, "PCIe SE"),
	READ_CHECK(0x5000e800, 7, "TPU SE"),
	READ_CHECK(0x5000ec00, 7, "Video SE"),
	READ_CHECK(0x5000f000, 7, "VD1 Video SE"),
	READ_CHECK(0x50126000, 0, "WDE Wave512"),
	READ_CHECK(0x50136000, 0, "WDE ATT"),
	READ_CHECK(0x50030000, 0, "JPEG"),
	READ_CHECK(0x50050000, 0, "WAVE"),
	READ_CHECK(0x50070100, 0x19051686, "VPP"),
	READ_CHECK(0x50111000, 0, "JPEG ATT REG"),
	READ_CHECK(0x50112000, 0, "WAVE ATT REG"),
	READ_CHECK(0x50113000, 0, "VPP ATT REG"),
	READ_CHECK(0x50040000, 0, "VD1 JPEG"),
	READ_CHECK(0x50060000, 0, "VD1 WAVE"),
	READ_CHECK(0x500f0100, 0x19051686, "VD1 VPP"),
	READ_CHECK(0x50121000, 0, "VD1 JPEG ATT REG"),
	READ_CHECK(0x50122000, 0, "VD1 WAVE ATT REG"),
	READ_CHECK(0x50123000, 0, "VD1 VPP ATT REG"),
};

struct perf_region {
	unsigned long addr;
	int gran;
	char *name;
};

struct perf_region perf_regions[] = {
	{0x08000000, 16, "TPU local"},
	{0x09000000, 16, "TPU static"},
	{0x10000000, 16, "AXI SRAM"},
	{0x58001100, 4, "TPU"},
	{0x58000000, 4, "GDMA"},
	{0x58003000, 4, "CDMA"},
	{0x02008000, 4, "GDE"},
	{0x02009000, 4, "SORT"},
	{0x0200a000, 4, "NMS"},
};

#define PERF_DEVICE_COUNT	(1 * 1024 * 1024)

int perf_device_region(struct perf_region *r)
{
	unsigned long ptr;
	unsigned long i = 0;
	uint64_t time;
	register uint32_t __attribute__((unused)) tmp32;
	register uint128_t __attribute__((unused)) tmp128;

	ptr = r->addr;

	pr_info("**************************************\n");
	pr_info("%08lx %16s\n",
		r->addr, r->name);

	time = timer_get_tick();

	if (r->gran == 4) {
		for (i = 0; i < PERF_DEVICE_COUNT; ++i)
			*(volatile uint32_t *)ptr = 0UL;
	} else if (r->gran == 16) {
		for (i = 0; i < PERF_DEVICE_COUNT; ++i)
			*(volatile uint128_t *)ptr = 0xdeadbeefUL;
	} else {
		pr_err("unsupported write granularity %d\n",
				r->gran);
		return -1;
	}

	time = timer_get_tick() - time;
	time = timer_tick2us(time);

	pr_info("%ld times write%d in %ldus\n", i, r->gran * 8, time);
	pr_info("%ldwrite-ops/s %ldB/s\n",
		i * 1000 * 1000 / time,
		i * 1000 * 1000 * r->gran / time);


	time = timer_get_tick();

	if (r->gran == 4) {
		for (i = 0; i < PERF_DEVICE_COUNT; ++i)
			tmp32 = *(volatile uint32_t *)ptr;
	} else if (r->gran == 16) {
		for (i = 0; i < PERF_DEVICE_COUNT; ++i)
			tmp128 = *(volatile uint128_t *)ptr;
	} else {
		pr_err("unsupported read granularity %d\n",
		       r->gran);
		return -1;
	}

	time = timer_get_tick() - time;
	time = timer_tick2us(time);

	pr_info("%ld times read%d in %ldus\n", i, r->gran * 8, time);
	pr_info("%ldread-ops/s %ldB/s\n",
		i * 1000 * 1000 / time,
		i * 1000 * 1000 * r->gran / time);
	pr_info("**************************************\n");

	return 0;
}

static int test_fab(void)
{
	int i;

	pr_info("FAB Access Test\n");

	pr_info("enable TPU local sram\n");
	writel(readl(0x58001100) | 1, 0x58001100);
	pr_info("enable emmc clock\n");
	writel(3, 0x5010002c);
	pr_info("enable sdio clock\n");
	writel(3, 0x5010102c);

	for (i = 0; i < ARRAY_SIZE(regions); ++i)
		test_region(&regions[i]);


	pr_info("\n\nperformance test\n\n");
	for (i = 0; i < ARRAY_SIZE(perf_regions); ++i)
		perf_device_region(&perf_regions[i]);

	return 0;
}

test_case(test_fab);
