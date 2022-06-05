#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sched.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void measure (int mode, int samples)
{
	struct timespec start_time;
	struct timespec stop_time;
	clock_gettime (mode, &start_time);  // setup...
	clock_gettime (mode, &start_time);
	for (int i = 0; i<samples; i++)
	{
		clock_gettime (mode, &start_time);
		clock_gettime (mode, &stop_time);
		long long t = (stop_time.tv_nsec - start_time.tv_nsec) +
		    (stop_time.tv_sec - start_time.tv_sec) * 1000000000;
		printf ("%lld\n", t);
	}

}


/**
 * main program...
 */
int main(int argc, char* argv[])
{
	cpu_set_t my_set;
	CPU_ZERO(&my_set);
	CPU_SET(2, &my_set);
	sched_setaffinity(0, sizeof(cpu_set_t), &my_set);

	printf ("clocks_per_sec=%ld\n", CLOCKS_PER_SEC);

	int samples = 1000;
	if (argc == 2)
		samples = atol(argv[1]);

	struct timespec start_time;
	clock_getres (CLOCK_MONOTONIC_RAW, &start_time);
	long t = start_time.tv_sec * 1000000000 + start_time.tv_nsec;
	printf ("time=%ld'%03ld'%03ld ns\n", t/1000000, (t/1000)%1000, t%1000);

	measure (CLOCK_MONOTONIC_RAW, samples);

	return 0;
}
