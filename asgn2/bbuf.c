/* If you don't want N=30, T=int, #define those in the command line. */

#include "bbuf.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
int const Number_Count = 1000*1000*1000;
void bounded_buffer_init(bounded_buffer *p) {
    pthread_mutexattr_t m;
    pthread_condattr_t  c;

    pthread_mutexattr_init(&m);
    pthread_mutexattr_settype(&m, PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutex_init(&p->mutex, &m);
    pthread_condattr_init(&c);
    pthread_condattr_setpshared(&c, PTHREAD_PROCESS_PRIVATE);
    pthread_cond_init(&p->non_full, &c);
    pthread_cond_init(&p->non_empty, &c);
    p->head = p->tail = p->size = 0;
    pthread_condattr_destroy(&c);
    pthread_mutexattr_destroy(&m);
}

void bounded_buffer_destroy(bounded_buffer *p) {
    pthread_cond_destroy(&p->non_empty);
    pthread_cond_destroy(&p->non_full);
    pthread_mutex_destroy(&p->mutex);
}

void bounded_buffer_add_last(bounded_buffer *p, T x) {
    pthread_mutex_lock(&p->mutex);
    while (p->size == N) {
//	DBG(printf(PTRFMT " wait non_full\n", (size_t)pthread_self()));
	pthread_cond_wait(&p->non_full, &p->mutex);
//	DBG(printf(PTRFMT " wake non_full\n", (size_t)pthread_self()));
    }
    p->item[p->tail++] = x;
 //   DBG(printf(PTRFMT " add %d\n", (size_t)pthread_self(), x));
    if (p->tail == N) p->tail = 0;
    if (0 == p->size++) pthread_cond_broadcast(&p->non_empty);
    pthread_mutex_unlock(&p->mutex);
}

T bounded_buffer_remove_first(bounded_buffer *p) {
    T x;

    pthread_mutex_lock(&p->mutex);
    while (p->size == 0) {
//	DBG(printf(PTRFMT " wait non_empty\n", (size_t)pthread_self()));
	pthread_cond_wait(&p->non_empty, &p->mutex);
//	DBG(printf(PTRFMT " wake non_empty\n", (size_t)pthread_self()));
    }
    x = p->item[p->head++];
  //  DBG(printf(PTRFMT " remove %x\n", (size_t)pthread_self(), x));
    if (p->head == N) p->head = 0;
    if (N == p->size--) pthread_cond_broadcast(&p->non_full);
    pthread_mutex_unlock(&p->mutex);
    return x;
}


#define N_PRODUCERS 1
#define N_ITEMS_PER_PRODUCER 1
#define N_CONSUMERS 1
#define N_ITEMS_PER_CONSUMER ((N_PRODUCERS*N_ITEMS_PER_PRODUCER)/N_CONSUMERS)

static bounded_buffer bbuf;

static void *producer(void *dummy) {
    int i;
    int a =  632559378;  // \ Wichmann-Hill (2006)
    int b = 1436901506;  // | 4-cycle random 
    int c =  244914893;  // | generator
    int d = 1907460368;  // / state.

    double const recip_a /* 1/2 147 483 579.0 */ =
        0.00000000046566130226972971885062316464865503867958;
    double const recip_b /* 1/2 147 483 543.0 */ =
        0.00000000046566131007598599324865699331694482745566;
    double const recip_c /* 1/2 147 483 423.0 */ =
        0.00000000046566133609684213147940094716158374741503;
    double const recip_d /* 1/2 147 483 123.0 */ =
        0.00000000046566140114899519981000567798175892812360;

    double w;   // The random number

    for (i = 0; i < Number_Count; i++) {
    w  = recip_a * (double)(a = (int)((a * 11600LL) % 2147483579));
    w += recip_b * (double)(b = (int)((b * 47003LL) % 2147483543));
    w += recip_c * (double)(c = (int)((c * 23000LL) % 2147483423));
    w += recip_d * (double)(d = (int)((d * 33000LL) % 2147483123));
    if (w >= 2.0) w -= 2.0;
    if (w >= 1.0) w -= 1.0;
	bounded_buffer_add_last(&bbuf, w);
    }

}

static void *consumer(void *dummy) {
    int i;
    double x, v;
    double mean = 0.0, sum2 = 0.0;

    for (i = 0; i < Number_Count; i++) {
    // Recieve from the other thread
    x = bounded_buffer_remove_first(&bbuf);
    v = (x - mean)/(i+1);
    sum2 += ((i+1)*v)*(i*v);
    mean += v;
    }
    printf("Mean = %g, standard deviation = %g\n",
           mean, sqrt(sum2/(i-1)));
}

int main(void) {
    pthread_t producer_id[N_PRODUCERS];
    pthread_t consumer_id[N_CONSUMERS];
    int i;

    bounded_buffer_init(&bbuf);
    printf("starting\n");
    for (i = 0; i < N_CONSUMERS; i++) {
	pthread_create(&consumer_id[i], 0, consumer, 0);
    }
    printf("consumers created\n");
    for (i = 0; i < N_PRODUCERS; i++) {
	pthread_create(&producer_id[i], 0, producer, 0);
    }
    printf("producers created\n");
    for (i = 0; i < N_CONSUMERS; i++) {
	pthread_join(consumer_id[i], 0);
    }
    for (i = 0; i < N_PRODUCERS; i++) {
	pthread_join(producer_id[i], 0);
    }
    printf("finished\n");
    bounded_buffer_destroy(&bbuf);
    return 0;
}

