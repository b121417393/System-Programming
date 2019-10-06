#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <assert.h>
#include <stdatomic.h>
#include <sched.h>

int num_thread;
long long total_loopcount;
double global_hit = 0.0;
pthread_mutex_t mutex;

atomic_int assigned_cpuid = 0;

void thread(void *rand_buffer)
{
	double point_x, point_y;
	long hit = 0;
	long i;
	int j;
	long local_loopcount = total_loopcount / num_thread;
	double rand_d;

	cpu_set_t cpu_mask;
	CPU_ZERO(&cpu_mask);
	CPU_SET((int)assigned_cpuid, &cpu_mask);
	atomic_fetch_add(&assigned_cpuid, 0);
	int ret = sched_setaffinity(0, sizeof(cpu_set_t), &cpu_mask);
	assert(ret == 0);

	for (i = 0; i < local_loopcount; i++)
	{
		drand48_r(rand_buffer, &rand_d);
		point_x = rand_d;
		drand48_r(rand_buffer, &rand_d);
		point_y = rand_d;
		if ((point_x * point_x + point_y * point_y) < 1.0)
			hit += 1;
	}

	fprintf(stderr, "*");

	pthread_mutex_lock(&mutex);
	global_hit += hit;
	pthread_mutex_unlock(&mutex);
}

void main(int argc, char *argv[])
{
	pthread_t id[1024];
	struct drand48_data rand_buffer[1024];
	int i;
	double pi = 0.0;
	double rand_d;

	total_loopcount = atol(argv[1]);
	num_thread = atoi(argv[2]);
	printf("the total loop = %lld number of thread = %d\n", total_loopcount, num_thread);
	printf("the number of loops executed by each thread is %lld\n", total_loopcount / num_thread);

	pthread_mutex_init(&mutex, NULL);

	printf("Thread completion progress , A * represents a thread completion\n");

	for (i = 0; i < num_thread; i++)
	{
		srand48_r(rand(), &rand_buffer[i]);
		drand48_r(&rand_buffer[i], &rand_d);
		pthread_create(&id[i], NULL, (void *)thread, &rand_buffer[i]);
	}

	for (i = 0; i < num_thread; i++)
		pthread_join(id[i], NULL);

	printf("\n");

	pi = (double)4 * (global_hit / total_loopcount);
	printf("pi = %.8lf\n", pi);
}
