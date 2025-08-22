#include <stdio.h>
#include <timer.h>
#include <errno.h>
#include <common/module.h>

static int __attribute__((unused)) test_timer_delay(void)
{
	int i;

	printf("Test timer delay, delay 1s, repeat 10 times\n");
	for (i = 0; i < 10; ++i) {
		timer_mdelay(1000);
		printf("%d seconds\n", i);
	}

	return 0;
}

static void test_timer_interrupt_isr(void *data)
{
	printf("Timer isr, current tick is %lld\n", timer_get_tick());
	printf("data is %ld\n", *(unsigned long *)data);

	--*(unsigned long *)data;
}

static unsigned long test_timer_interrupt_isr_data;

static int __attribute__((unused)) test_timer_interrupt(void)
{
	uint64_t current_tick = timer_get_tick();
	int err;

	err = timer_enable_irq(timer_s2tick(1), test_timer_interrupt_isr,
			       &test_timer_interrupt_isr_data);

	printf("Test timer interrupt, interrupt will happened 1s in the future\n");
	printf("Current tick is %lld\n", current_tick);

	test_timer_interrupt_isr_data = 10;

	if (err) {
		if (err == -ENODEV) {
			printf("No mtimecmp device\n");
			return 0;
		} else {
			printf("Failed to enable timer interrupt\n");
			return err;
		}
	}

	while (test_timer_interrupt_isr_data) {
	}

	printf("Test done\n");

	return 0;
}

test_case(test_timer_delay);
test_case(test_timer_interrupt);

