/*
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>


enum { arr_size=0xffff };
static long arr[arr_size+1];

static double bogo_sps [2][arr_size+1] = {};
static double bogo_time_psec[2][arr_size+1] = {};

static inline unsigned long delay(unsigned long loops, unsigned qty, unsigned chet_nechet)
{
  unsigned long i, s = 0;

  --qty;

  for (i = 0; !!(i < loops); ++i) {
	if (!(i & chet_nechet))
		s += arr[i & qty];
	else
		s += arr[s & qty];
  }

  return s;
}


static int
calibrating_delay_loop(unsigned qty, unsigned chet_nechet)
{
  unsigned long loops_per_sec = 1;

  printf("[%05u] Calibrating delay loop.. ", qty);
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

      printf("ok - %09llu BogoSps (s = %019lu)\n",
             lps, s);
      fflush(stdout);

	  bogo_sps [chet_nechet][qty-1] = lps;
	  bogo_time_psec[chet_nechet][qty-1] = 1000000000000.0 / bogo_sps[chet_nechet][qty-1];
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

#define MIN(x,y) ((x) < (y) ? (x) : (y))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

int
main(void)
{
	int res = 0;
	int res_L1 = 0;
	unsigned i;
	unsigned n = 0;

	double min_thput = 1000000000000.0;
	double avg_thput = 0;
	double max_thput = 0;

	double min_latency = 1000000000000.0;
	double avg_latency = 0;
	double max_latency = 0;

	double min_perf = 1000000000000.0;
	double avg_perf = 0;
	double max_perf = 0;

	srandom(time(NULL));

	for (i=0; !!(i<=arr_size); ++i)
		arr[i] = random();

	res |= calibrating_delay_loop_arr(0);
	res |= calibrating_delay_loop_arr(1);

	printf(" *** SUMMARY\n");

	for (i=1; i<=(arr_size+1); i*=2) {
		double latency, performance;

		latency     = bogo_time_psec[1][i-1] - bogo_time_psec[0][i-1];
		performance = sqrt(bogo_sps [0][i-1] * bogo_sps[1][i-1]);

		printf ("[%05u] thput=%.3f BogoMsps, latency=%.3f psec, perf=%.3f BogoMsps\n",
				i, bogo_sps[0][i-1]/1000000, latency, performance/1000000);

#define THRESHOLD (0.05)

		if (0==n || 1==n || (fabs((avg_thput/n)-bogo_sps[0][i-1]) < (THRESHOLD * (avg_thput/n)))) {
			min_thput = MIN(min_thput, bogo_sps[0][i-1]);
			avg_thput += bogo_sps[0][i-1];
			max_thput = MAX(max_thput, bogo_sps[0][i-1]);

			min_latency = MIN(min_latency, latency);
			avg_latency += latency;
			max_latency = MAX(max_latency, latency);

			min_perf = MIN(min_perf, performance);
			avg_perf += performance;
			max_perf = MAX(max_perf, performance);

			n++;
		} else {
			printf("\t==== Results: ====\n");
			printf("It looks like we have L1 cache size: %u * %zu (sizeof(long)) = %zu (bytes)\n",
			       i/2, sizeof(long), i*sizeof(long)/2);
			printf("thput=(min, avg, max) BogoMsps, \tlatency=(min, avg, max) psec, \tperf=(min, avg, max) BogoMsps\n");
			printf("thput=(%.3f, %.3f, %.3f) BogoMsps, latency=(%.3f, %.3f, %.3f) psec, perf=(%.3f, %.3f, %.3f) BogoMsps\n",
			       min_thput/1000000, avg_thput/(n*1000000), max_thput/1000000,
				   min_latency, avg_latency/n, max_latency,
				   min_perf/1000000, avg_perf/(n*1000000), max_perf/1000000);
			res_L1 = 1; //We have found results successfully
			break;
		}
	}

	if(!res_L1) {
		res |= 1;

		printf("\t==== Results: ====\n");
		printf("We run on architecture with unexpected behaviour (like Qemu or other simulator).\n");
		printf("thput=(min, avg, max) BogoMsps, \tlatency=(min, avg, max) psec, \tperf=(min, avg, max) BogoMsps\n");
		printf("thput=(%.3f, %.3f, %.3f) BogoMsps, latency=(%.3f, %.3f, %.3f) psec, perf=(%.3f, %.3f, %.3f) BogoMsps\n",
			   min_thput/1000000, avg_thput/(n*1000000), max_thput/1000000,
			   min_latency, avg_latency/n, max_latency,
			   min_perf/1000000, avg_perf/(n*1000000), max_perf/1000000);
	}

	fflush(stdout);

	return res;
}
