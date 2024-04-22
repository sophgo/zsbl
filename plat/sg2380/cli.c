#include <timer.h>
#include <string.h>
#include <driver/serial/ns16550.h>
#include <stdio.h>
#include <console.h>

#define CLI_HOT_KEY 'f'
#define TIMEOUT 50000000

extern int console_exit;

static int abortboot_single_key(int bootdelay)
{
	int abort = 0;
	/*
	 * Check if key already pressed
	 */
	if (uart_getc() == CLI_HOT_KEY || bootdelay == 0) {	/* we got a key press	*/
		abort = 1;	/* don't auto boot	*/
		goto out;
	}

	if (bootdelay == -1)
		goto out;	/* don't wait, just boot	*/

	while ((bootdelay > 0) && (!abort)) {
		--bootdelay;
		/* delay 1000 ms */
		unsigned long time_start = timer_get_tick();
		do {
			if (uart_getc() == CLI_HOT_KEY) {	/* we got a key press	*/
				abort  = 1;	/* don't auto boot	*/
				bootdelay = 0;	/* no more delay	*/
				break;
			}
		} while (!abort && timer_get_tick() - time_start <= TIMEOUT);

		printf("\b\b\b%02d ", bootdelay);
	}

out:
	return abort;
}

/*
 * bootdelay:
 * 0: enter command line directly
 * >0: print promt and wait for input by give seconds
 * -1: only check input once, no promt
 */
void cli_loop(int bootdelay)
{
	console_init();
	if (abortboot_single_key(bootdelay)) {
		while(!console_exit) {
			console_poll();
		}
	}

}
