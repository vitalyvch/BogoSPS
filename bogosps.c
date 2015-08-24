/*
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>


enum { arr_size=0x1f };
static long arr[arr_size+1];

static double bogo_sps [2][arr_size+1] = {};
static double bogo_time[2][arr_size+1] = {};

/* portable version */
static inline unsigned long delay(unsigned long loops, unsigned qty, unsigned chet_nechet)
{
  unsigned long i, s = 0;

  for (i = 0; !!(i < loops); ++i) {
	if (!(i & chet_nechet))
		s += arr[i % qty];
	else
		s += arr[s % qty];
  }

  return s;
}


int
calibrating_delay_loop(unsigned qty, unsigned chet_nechet)
{
  unsigned long loops_per_sec = 1;

  printf("[%02u] Calibrating delay loop.. ", qty);
  fflush(stdout);
  
  while ((loops_per_sec <<= 1)) {
    unsigned long long ticks;
    unsigned long s;

    ticks = clock();
    s = delay(loops_per_sec, qty, chet_nechet);
    ticks = clock() - ticks;

    if (ticks >= CLOCKS_PER_SEC) {
      unsigned long long lps = loops_per_sec;
      lps = (lps * CLOCKS_PER_SEC) / ticks;

      printf("ok - %09llu BogoSps (s = %018lu)\n",
             lps, s);
      fflush(stdout);

	  bogo_sps [chet_nechet][qty-1] = lps;
	  bogo_time[chet_nechet][qty-1] = 1.0 / bogo_sps[chet_nechet][qty-1];
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
	double prev_thput = 0;

	srandom(time(NULL));

	for (i=0; !!(i<=arr_size); ++i)
		arr[i] = random();

	res |= calibrating_delay_loop_arr(0);
	res |= calibrating_delay_loop_arr(1);

	printf(" *** SUMMARY\n");

	for (i=1; i<=(arr_size+1); i*=2) {
		double latency, performance;

		latency     = bogo_time[1][i-1] - bogo_time[0][i-1];
		performance = bogo_sps [0][i-1] / latency;

		if (prev_thput > bogo_sps[0][i-1])
			break;

		printf ("[%02u] thput=%g BogoSps, latency=%g sec, perf=%g BogoSps^2\n",
				i, bogo_sps[0][i-1], latency, performance);
		prev_thput = bogo_sps[0][i-1];
	}
	fflush(stdout);

	return res;
}
