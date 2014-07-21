#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>

#define handle_error_en(en, msg) \
       do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

#define handle_error(msg) \
       do { perror(msg); exit(EXIT_FAILURE); } while (0)

void usage(){
    fprintf(stderr,"Usage: ./make_threads <no_threads> <sum_up_to>\n");
}

unsigned long long global_sum;

void * up_to_n(void *args){
    int *n = args;
    int i;
    for (i =1 ; i <= *n; i++){
       __sync_add_and_fetch(&global_sum, i);
    }
    //printf("global sum = %lld\n", global_sum);
    return NULL;
}

int main(int argc, char ** argv){
    int t, n, i, s;
    pthread_t* threads;
    pthread_attr_t attr;
    void * res;
    if (argc < 2) {
        fprintf(stderr,"Need to specify two arguments to the command line\n");
        usage();
        exit(1);
    }
    // asign t and n
    t = atoi(argv[1]);
    n = atoi(argv[2]);
    threads = calloc(t,sizeof(*threads));
    global_sum = 0;
    s= pthread_attr_init(&attr);
    if ( s != 0)
        handle_error_en(s, "pthread_attr_init");
    s = pthread_attr_destroy(&attr);
    if (s != 0)
        handle_error_en(s, "pthread_attr_destroy");
    for (i = 0; i < t; i ++){
        pthread_create(&threads[i], &attr, &up_to_n, (void *) &n);
    }
    //for (i = 0; i < t; i++){
       // s = pthread_join(threads[i], &res);
        //if ( s != 0 )
         //   handle_error_en(s, "pthread_join");
    //    printf("Joined thread %d \n",
      //          i);
   // }
    for (i = 0; i < t; i++){
        s = pthread_join(threads[i], &res);
        if ( s != 0 )
            handle_error_en(s, "pthread_join");
    }
    printf("%lld\n", global_sum);
    free(threads);
    exit(EXIT_SUCCESS);
}
