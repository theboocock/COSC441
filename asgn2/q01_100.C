
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
int const Number_Count = 1000*1000*1000;
int pipe_variable[2];

#define handle_error_en(en, msg) \
       do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

#define handle_error(msg) \
       do { perror(msg); exit(EXIT_FAILURE); } while (0)

void usage(){
    fprintf(stderr,"Usage: ./comm_test <message passing> <number of samples>\n");
}


void *  producer(void *dummy) {
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
    double buffer[100];
    int i;               // Loop counter
    for (i = 0; i < Number_Count; i++) {
    w  = recip_a * (double)(a = (int)((a * 11600LL) % 2147483579));
    w += recip_b * (double)(b = (int)((b * 47003LL) % 2147483543));
    w += recip_c * (double)(c = (int)((c * 23000LL) % 2147483423));
    w += recip_d * (double)(d = (int)((d * 33000LL) % 2147483123));
    if (w >= 2.0) w -= 2.0;
    if (w >= 1.0) w -= 1.0;
    if ( i != 0 && i % 100 == 0){
            write(pipe_variable[1],&buffer, sizeof(*buffer)* 100); 
        }
    buffer[i % 100] = w;
    }
}

void * consumer(void *dummy) {
    int i;
    double x, v;
    double mean = 0.0, sum2 = 0.0;
    double buffer[100];
    for (i = 0; i < Number_Count; i++) {
    // Recieve from the other thread
    if ( i %100 == 0){
    read(pipe_variable[0], &buffer,sizeof(*buffer) * 100);
    }
    x = buffer[i % 100];
    v = (x - mean)/(i+1);
    sum2 += ((i+1)*v)*(i*v);
    mean += v;
    }
    printf("Mean = %g, standard deviation = %g\n",
           mean, sqrt(sum2/(i-1)));
}
int main(void){
    int t, s, i;
    pthread_t* threads;
    pthread_attr_t attr;
    void *dummy = NULL;
    threads = calloc(2, sizeof *threads);
    // must be pipe
    pipe(pipe_variable);
    s= pthread_attr_init(&attr);
    if ( s != 0)
        handle_error_en(s, "pthread_attr_init");
    pthread_create(&threads[0],&attr,&consumer,NULL); 
    pthread_create(&threads[1],&attr,&producer,NULL);

    for (i = 0; i < 2; i++){
    s = pthread_join(threads[i], NULL); 
        if ( s != 0 )
            handle_error_en(s, "pthread_join");
    }
    exit(EXIT_SUCCESS);
    free(threads);
    exit(EXIT_SUCCESS);
}





