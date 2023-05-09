#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/unistd.h>

#include <timer.h>

#include <arch.h>

int firmware_start(void);

int main(void)
{
	firmware_start();
	return 0;
}
