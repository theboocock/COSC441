/* If you don't want N=30, T=int, #define those in the command line. */


#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <pthread.h>
#include <errno.h>

int const Number_Count = 1000*1000*1000;

static int queue;

struct my_msgbuf {
    long mtype;
    double x;
};

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
    struct my_msgbuf buffer;
    buffer.mtype = 1;
    for (i = 0; i < Number_Count; i++) {
    w  = recip_a * (double)(a = (int)((a * 11600LL) % 2147483579));
    w += recip_b * (double)(b = (int)((b * 47003LL) % 2147483543));
    w += recip_c * (double)(c = (int)((c * 23000LL) % 2147483423));
    w += recip_d * (double)(d = (int)((d * 33000LL) % 2147483123));
    if (w >= 2.0) w -= 2.0;
    if (w >= 1.0) w -= 1.0;
    buffer.x = w;
    printf("old %f\n",buffer.x);
    if(msgsnd(queue,&buffer,sizeof(double),0) == -1){
        printf("failed send\n");
        exit(1);    
        }
    }
    return NULL;
}

static void *consumer(void *dummy) {
    int i;
    double x, v;
    double mean = 0.0, sum2 = 0.0;
    
    struct my_msgbuf buffer;
    int status; 
    for (i = 0; i < Number_Count; i++) {
    if(msgrcv(queue, &buffer, sizeof(double),0,0) == -1){
        printf("failed\n");
        exit(1);   
    }
    printf("%f\n",buffer.x);
    x = buffer.x;
    v = (x - mean)/(i+1);
    sum2 += ((i+1)*v)*(i*v);
    mean += v;
    }
    printf("Mean = %g, standard deviation = %g\n",
           mean, sqrt(sum2/(i-1)));
    return NULL;
}

#define N_PRODUCERS 1
#define N_CONSUMERS 1

int main(void) {
    pthread_t producer_id[N_PRODUCERS];
    pthread_t consumer_id[N_CONSUMERS];
    int i;
    struct my_msgbuf buf;
    key_t key;
    if((key = ftok("../", 'B'))== -1){
        perror("mskey");
        exit(1);
    }
    if ((queue = msgget(key, 0644 | IPC_CREAT))== -1) {
        perror("msgget");
        exit(1);
    }
    printf("starting queue_ih %d \n",queue);
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
    // destroy the message queue
    return 0;
}

