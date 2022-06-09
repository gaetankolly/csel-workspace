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
	pwrite (gpio, "1", sizeof("1"), 0);
	pwrite (gpio, "0", sizeof("0"), 0);

	int fd = open ("/dev/mem", O_RDWR);
	if (fd == -1) return -1;
	off_t psz = getpagesize();
	volatile uint32_t*  pio = mmap (0, psz, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0x01c20000);
	volatile uint32_t* dat_pio  =  pio + (0x810+6*0x24)/4;

	struct timespec start_time;
	struct timespec stop_time;
	clock_gettime (CLOCK_MONOTONIC, &start_time);  // setup...

	// --> measurement with internal timers on gpio 
	clock_gettime (CLOCK_MONOTONIC, &start_time);
	
	for (int i=0; i<1000; i++)
		*dat_pio ^= (1<<11);

	clock_gettime (CLOCK_MONOTONIC, &stop_time);
	long long t = (stop_time.tv_nsec - start_time.tv_nsec) +
		    (stop_time.tv_sec - start_time.tv_sec) * 1000000000;
	printf ("*dat_pio ^= (1<<11) --> %6lld ns\n", t/1000);


	// --> measurement on internal data with internal timers
	volatile uint32_t data = 0;
	clock_gettime (CLOCK_MONOTONIC, &start_time);
	
	for (int i=0; i<1000; i++)
		data ^= (1<<11);
	
	clock_gettime (CLOCK_MONOTONIC, &stop_time);
	t = (stop_time.tv_nsec - start_time.tv_nsec) +
		    (stop_time.tv_sec - start_time.tv_sec) * 1000000000;
	printf ("data     ^= (1<<11) --> %6lld ns\n", t/1000);


	// --> measurement with oscilloscope
	while(true) {
		*dat_pio ^= (1<<11);
	}
	
	return 0;
}
