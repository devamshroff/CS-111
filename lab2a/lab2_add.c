
#include <getopt.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

int option_index = 0;
static long long counter = 0;
int num_threads = 1;
int num_iter = 1;
char* test_name;
int opt_yield= 0;
int sync = 0;
static char opt_sync = 'u';
int spin_lock = 0;
pthread_mutex_t pthread_lock = PTHREAD_MUTEX_INITIALIZER;
int old_count;
int new_count;

void add(long long *pointer, long long value) {
    long long sum = *pointer + value;
    if (opt_yield)
        sched_yield();
    *pointer = sum;
}

void add_helper(int i){
    for(int j = 0; j < num_iter; j++){
        switch(opt_sync){
            case 'm': //mutex
                pthread_mutex_lock(&pthread_lock);
                add(&counter, i);
                pthread_mutex_unlock(&pthread_lock);
                break;
            case 's': //sync-lock
                while(__sync_lock_test_and_set(&spin_lock,1))
                    continue;
                add(&counter, i);
                __sync_lock_release(&spin_lock);
                break;
            case 'c': //cas
                for(long long old_count, new_count; ; ){
                    old_count = counter;
                    new_count = old_count + i;
                    if(__sync_val_compare_and_swap(&counter, old_count, new_count) == old_count)
                        break;
                }
                break;
            default:
                add(&counter, i);
                break;
        }
}
}

void* run_thread(){
    add_helper(1); //add 1
    add_helper(-1); //subtract 1
    return NULL;
}

int main(int argc, char** argv){
    static struct option long_options[] = 
    {
        {"threads", required_argument, 0, 't',},
        {"iterations", required_argument, 0, 'i'},
        {"sync", required_argument, 0, 's'},
        {"yield", no_argument, 0, 'y'},
        {0,0,0,0}
    };

    test_name = (char*)malloc(sizeof(char*)*20);
    int ch;
    //create option parser
    while((ch = getopt_long(argc, argv, "t:i:s:y", long_options, &option_index)) != -1){
        switch(ch){
            case 't': ;
            num_threads = atoi(optarg);
            if(num_threads < 0){
                fprintf(stderr, "Invalid number of threads.");
                exit(1);
            }
            break;
            case 'i': ; 
            num_iter = atoi(optarg);
            if(num_iter < 0){
                fprintf(stderr, "Invalid number of iterations.");
                exit(1);
            }
            break;
            case 's': ;
            sync = 1;
            opt_sync = optarg[0]; //only need the first char
       
            if(opt_sync != 'c' && opt_sync != 'm' && opt_sync != 's'){
                fprintf(stderr, "Invalid argument to sync");
                exit(1);
            }
            break;
            case 'y': ; 
            opt_yield = 1;
            break;
            default: 
                fprintf(stderr, "invalid argument. error: %d, message: %s", errno, strerror(errno));
                exit(1);
        }
    }
    fflush(stdout);
    //build test name
    strcat(test_name,"add");
    if(opt_yield)
        strcat(test_name, "-yield");

    if(opt_sync != 'm' && opt_sync != 's' && opt_sync != 'c'){
        strcat(test_name, "-none");
    }else{
        strcat(test_name, "-");
        strcat(test_name, &opt_sync);
    }

    //start timer
    struct timespec time_start;
    clock_gettime(CLOCK_MONOTONIC, &time_start);

    //allocate memory for pthread IDs
    pthread_t* ptid = (pthread_t*) malloc(sizeof(pthread_t) * num_threads);
    if(ptid == NULL){
        fprintf(stderr, "Unable to allocate memory to store thread IDs. error: %d, message: %s", errno, strerror(errno));
        exit(1);
    }

    //create pthreads
    for( int i = 0 ; i < num_threads; i++){
        if(pthread_create(&ptid[i], NULL, run_thread, NULL)){
            free(ptid);
            fprintf(stderr, "Unable to create threads. error: %d, message: %s", errno, strerror(errno));
            exit(1);
        }
    }

    //join pthreads
    for(int i = 0; i < num_threads; i++){
        if(pthread_join(ptid[i], NULL)){
            fprintf(stderr, "Unable to join threads. error: %d, message: %s", errno, strerror(errno));
            free(ptid);
            exit(1);
        }
    }
    //end time
    struct timespec time_end;
    clock_gettime(CLOCK_MONOTONIC, &time_end);

    long long runtime = time_end.tv_sec - time_start.tv_sec;
    runtime *= 1000000000;
    runtime += time_end.tv_nsec;
    runtime -= time_start.tv_nsec;
    int num_oper = num_threads * num_iter * 2;
    int avg_runtime = runtime / num_oper;
    fflush(stdout);
    //output test in csv format
    fprintf(stdout, "%s,%d,%d,%d,%lld,%d,%lld\n", test_name, num_threads, num_iter, num_oper, runtime, avg_runtime, counter);
    free(ptid);
    fflush(stdout);
    if(counter == 0)
        exit(0);
    exit(1);
}

