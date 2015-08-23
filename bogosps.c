/*
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>


enum { arr_size=0x1f };
static long arr[arr_size+1];

static inline long sum(unsigned qty, unsigned chet_nechet)
{
	unsigned i;

	long s = 0;

	for (i=0; !!(i<qty); ++i)
		if (!chet_nechet)
			s += arr[i];
		else
			s += arr[chet_nechet];

	return s;
}

/* portable version */
static inline long delay(long long loops, unsigned qty, unsigned chet_nechet)
{
  long s;

  for (; !!(loops > 0); --loops)
	  s = sum(qty, loops & chet_nechet);

  return s;
}

double bogo_sps[2] = {};
double bogo_time[2] = {};

int
calibrating_delay_loop(unsigned qty, unsigned chet_nechet)
{
  unsigned long long loops_per_sec = 1;

  printf("[%02u] Calibrating delay loop.. ", qty);
  fflush(stdout);
  
  while ((loops_per_sec <<= 1)) {
    unsigned long long ticks;
    long s;

    ticks = clock();
    s = delay(loops_per_sec, qty, chet_nechet);
    ticks = clock() - ticks;
    if (ticks >= CLOCKS_PER_SEC) {
      loops_per_sec = (loops_per_sec * CLOCKS_PER_SEC * qty) / ticks;
      printf("ok - %09llu BogoSps (s = %+018ld)\n",
 	     loops_per_sec,
		 s
	     );
      fflush(stdout);

	  if (4 == qty) {
		  bogo_sps[chet_nechet] = loops_per_sec;
		  bogo_time[chet_nechet] = 1.0 / bogo_sps[chet_nechet];
	  }
      return 0;
    }
  }

  printf("failed\n");
  fflush(stdout);
  return -1;
}

static int calibrating_delay_loop_arr(int chet_nechet)
{
	int res = 0;
	unsigned i;

	printf(" *** chet_nechet = %02u\n", chet_nechet);
	fflush(stdout);

	for (i=1; i<=(arr_size+1); i*=2) {
		res |= calibrating_delay_loop(i, chet_nechet);
	}

	return res;
}

int
main(void)
{
	int res = 0;
	unsigned i;
	double latency, performance;

	srandom(time(NULL));

	for (i=0; !!(i<=arr_size); ++i)
		arr[i] = random();

	res |= calibrating_delay_loop_arr(0);
	res |= calibrating_delay_loop_arr(1);

	latency = bogo_time[1] - bogo_time[0];
	performance = bogo_sps[0] / latency;

	printf ("thput=%g BogoSps, latency=%g sec, perf=%g BogoSps^2\n",
	        bogo_sps[0], latency, performance);
	fflush(stdout);

	return res;
}
