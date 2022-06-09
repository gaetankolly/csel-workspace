#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sched.h>
#include <sys/mman.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#define GPIO_EXPORT		"/sys/class/gpio/export"
#define GPIO_UNEXPORT	"/sys/class/gpio/unexport"
#define GPIO_GPIOG11	"/sys/class/gpio/gpio203"
#define GPIOG11			"203"

static int open_gpio()
{
	// unexport pin out of sysfs (reinitialization)
	int f = open (GPIO_UNEXPORT, O_WRONLY);
	write (f, GPIOG11, strlen(GPIOG11));
	close (f);

	// export pin to sysfs
	f = open (GPIO_EXPORT, O_WRONLY);
	write (f, GPIOG11, strlen(GPIOG11));
	close (f);	

	// config pin
	f = open (GPIO_GPIOG11 "/direction", O_WRONLY);
	write (f, "out", 3);
	close (f);

	// open gpio value attribute
 	f = open (GPIO_GPIOG11 "/value", O_RDWR);
	return f;
}

/**
 * main program...
 */
int main()
{
	cpu_set_t my_set;
	CPU_ZERO(&my_set);
	CPU_SET(2, &my_set);
	sched_setaffinity(0, sizeof(cpu_set_t), &my_set);

	int gpio = open_gpio();
	bool on = false;
	struct timespec start_time;
	struct timespec stop_time;
	clock_gettime (CLOCK_MONOTONIC, &start_time);  // setup...


	// --> measurement with internal timers
	clock_gettime (CLOCK_MONOTONIC, &start_time);
	for (int i=0; i<1000; i++) {
		if (on) {
			pwrite (gpio, "1", sizeof("1"), 0);
		} else {
			pwrite (gpio, "0", sizeof("0"), 0);
		}
		on = !on;
	}
	clock_gettime (CLOCK_MONOTONIC, &stop_time);
	long long t = (stop_time.tv_nsec - start_time.tv_nsec) +
		    (stop_time.tv_sec - start_time.tv_sec) * 1000000000;
		printf ("pwrite (gpio, \"1\", sizeof(\"1\"), 0) -> %lld ns\n", t/1000);


	// --> measurement with oscilloscope
	while(true) {
		if (on) {
			pwrite (gpio, "1", sizeof("1"), 0);
		} else {
			pwrite (gpio, "0", sizeof("0"), 0);
		}
		on = !on;
	}
	
	return 0;
}
