#define _GNU_SOURCE 
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <assert.h>
#include <stdatomic.h>
#include <sched.h>
#include <math.h>
#include <signal.h>

int num_thread=0;
int num_point=0;
double upper_area=0.0;
double point_width=0.0;
double area_width=0.0;
double first_area=0.0;
pthread_mutex_t mutex;
atomic_int assigned_cpuid=0;



void ctrC_handler(int sigNumber) {
	printf("upper_area = %lf  lower_area = %lf \n" , (double)4*upper_area , (double)4*(upper_area-first_area));
}


void thread(double *init_point)
{
	double local_area;
	double total_area;
	double i , j , k;

	for(i = *init_point ; i < *init_point + area_width && i < (double)1.0 ; i+=point_width) 
	{
		if(i>=0.99999999)
			break;	
		printf("i=%lf  \n",i);
		j = (double)1.0 - (i*i);
		for(k=1.0 ; (k*k) > j ; k-=0.000001);	
		j = k;
		local_area = j * point_width ;
		total_area += local_area;

		if(i==(double)0.0)
			first_area=local_area;

	}
	
	pthread_mutex_lock(&mutex);
	upper_area += total_area;
	pthread_mutex_unlock(&mutex);
}

void main(int argc,char*argv[]) {
	pthread_t id[4096];
	int i;
	double upper_pi = 0.0;
	double lower_pi = 0.0;

	signal(SIGINT, ctrC_handler);

	num_point=atol(argv[1]);
	num_thread=atoi(argv[2]);
	printf("usage ./pi_drand48r [number_point = %d] [number_thread = %d]\n",num_point,num_thread);
	point_width = (float)1.0/(num_point-1);
	area_width = (float)1.0/(num_thread);
	
	pthread_mutex_init(&mutex,NULL);
	
	double temp[4096] , *ptr ;
	
	for(i=0 ; i < num_thread ; i++)
	{
		temp[i] = area_width*(double)i;
		int j=0;
		for(j=0;j*point_width<temp[i];j++);
		temp[i]=j*point_width;
		ptr=&temp[i];
		pthread_create(&id[i] , NULL , (void *)thread , ptr);
	}
	
	for(i=0;i<num_thread;i++)
		pthread_join(id[i],NULL);


	upper_pi = (double)4*upper_area;
	lower_pi = (double)4*(upper_area-first_area);
	printf("upper_pi = %.8lf\n",upper_pi);
	printf("lower_pi = %.8lf\n",lower_pi);
}
