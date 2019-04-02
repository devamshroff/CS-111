#include <getopt.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>

#include "SortedList.h"

int num_threads = 1;
int num_iter = 1;
char* test_name;
int opt_yield = 0;
static char opt_sync = 'u';
int spin_lock = 0;
pthread_mutex_t pthread_lock = PTHREAD_MUTEX_INITIALIZER;
int old_count;
int new_count;
SortedList_t* head;
SortedList_t** elements;
int list_len = 0;
int option_index = 0;
//create additional yield flags
#define ID_YIELD    0x03
#define IL_YIELD    0x05
#define DL_YIELD    0x06
#define IDL_YIELD   0x07

void handler(int num){
    if(num == SIGSEGV){
        write(2,"Segmentation fault caught!\n", 50);
    }
    exit(2);
}

void* run_thread(void* pos){
    int start_ele = *((int *) pos);

    //insert elements into list pointed to by head
    for(int i = start_ele; i < list_len; i+=num_threads){
        switch (opt_sync) {
            case 'u':
                SortedList_insert(head, elements[i]);
                break;
            case 'm':
                pthread_mutex_lock(&pthread_lock);
                SortedList_insert(head, elements[i]);
                pthread_mutex_unlock(&pthread_lock);
                break;
            case 's':
                while (__sync_lock_test_and_set(&spin_lock,1))
                    continue;
                SortedList_insert(head, elements[i]);
                __sync_lock_release(&spin_lock);
                break;
            default:
                break;
        }
    }

    //get list length
    int len = 0;
    switch(opt_sync){
        case 'u':
            len = SortedList_length(head);
            break;
        case 'm':
            pthread_mutex_lock(&pthread_lock);
            len = SortedList_length(head);
            pthread_mutex_unlock(&pthread_lock);
            break;
        case 's':
            while (__sync_lock_test_and_set(&spin_lock,1))
                    continue;
            len = SortedList_length(head);
            __sync_lock_release(&spin_lock);
            break;
        default:
            break;
    }
    if(len < 0){
        fprintf(stderr, "SortedList_length returned a negative number. Possible corruption. \n");
        exit(2);
    }
    
    //delete keys
    SortedListElement_t* selement = NULL;
    for(int i = start_ele; i < list_len; i+=num_threads){
        switch (opt_sync) {
            case 'u':
                 selement = SortedList_lookup(head, elements[i]->key);
                if (selement == NULL) {
                    write(2, "element does not exist.\n", 30);
                    exit(2);
                }
                if (SortedList_delete(selement)) {
                   write(2, "element could not be deleted.\n", 50);
                   exit(2);
                }
                break;
            case 'm':
                pthread_mutex_lock(&pthread_lock);
                selement = SortedList_lookup(head, elements[i]->key);
                if (selement == NULL) {
                    write(2, "element does not exist.\n", 30);
                    exit(2);
                }
                if (SortedList_delete(selement)) {
                    write(2, "element could not be deleted.\n", 50);
                    exit(2);
                }
                pthread_mutex_unlock(&pthread_lock);
                break;
            case 's':
                while (__sync_lock_test_and_set(&spin_lock,1))
                    continue;
                selement = SortedList_lookup(head, elements[i]->key);
                if (selement == NULL) {
                    write(2, "element does not exist.\n", 30);
                    exit(2);
                }
                if (SortedList_delete(selement)) {
                    write(2, "element could not be deleted.\n", 50);
                    exit(2);
                }
                __sync_lock_release(&spin_lock);
                break;
            default:
               break;
        }
    }    
    pthread_exit(NULL);
}

int main(int argc, char **argv){
    
     static struct option long_options[] = 
    {
        {"threads", required_argument, 0, 't',},
        {"iterations", required_argument, 0, 'i'},
        {"sync", required_argument, 0, 's'},
        {"yield", optional_argument, 0, 'y'},
        {0,0,0,0}
    };
    test_name = (char*)malloc(sizeof(char)*20);
    strcat(test_name, "list");
    int ch;

    //create option parser
    while((ch = getopt_long(argc, argv, "", long_options, &option_index)) != -1){
        switch(ch){
            case 't': ;
            num_threads = atoi(optarg);
            if(num_threads < 0){
                fprintf(stderr, "Invalid number of threads. \n");
                exit(1);
            }
            break;
            case 'i': ; 
            num_iter = atoi(optarg);
            if(num_iter < 0){
                fprintf(stderr, "Invalid number of iterations. \n");
                exit(1);
            }
            break;
            case 's': ;
            opt_sync = optarg[0]; //only need the first char
            if(opt_sync != 'm' && opt_sync != 's'){
                fprintf(stderr, "Invalid argument to sync.\n");
                exit(1);
            }
            break;
            case 'y': ; 
            int len = strlen(optarg);
            if(len > 3){
                fprintf(stderr, "Too many critical section yields specified. \n");
                exit(1);
            }
            for(int i = 0; i < len; i++){
                if(optarg[i] == 'i'){
                    opt_yield |= INSERT_YIELD;
                }else if(optarg[i] == 'd'){
                    opt_yield |= DELETE_YIELD;
                }else if(optarg[i] == 'l'){
                    opt_yield |= LOOKUP_YIELD;
                }else{
                    fprintf(stderr, "wrong argument provided for yield. \n");
                    exit(1);
                }
            }
            break;
            default: 
                fprintf(stderr, "invalid argument. error: %d, message: %s\n", errno, strerror(errno));
                exit(1);
                break;
        }
     }

    //initialize head 
    head = (SortedList_t*)malloc(sizeof(SortedList_t)*10);
    head->key = NULL;
    head->next = head;
    head->prev = head;

    //initialize list
    int num_elem = num_iter * num_threads;
    list_len = num_elem;
    elements = (SortedList_t**)malloc(sizeof(SortedList_t*)*num_elem);

    //generate random keys
    char* alphanum ="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    size_t alpha_len = strlen(alphanum);
    int key_len = 5;
    for(int i = 0; i < num_elem; i++){
        char* string = malloc(sizeof(char)*(key_len+1));
        for (int j = 0; j < key_len; j++) {
            int rand_letter =rand() % alpha_len;
            string[j] = alphanum[rand_letter];
        }
        string[key_len] = '\0';
        elements[i] = (SortedList_t*)malloc(sizeof(SortedList_t));
        elements[i]->key = string;
    }


    //update test-name
    switch( opt_yield ){
        case INSERT_YIELD:
            strcat(test_name, "-i");
            break;     
        case DELETE_YIELD:
            strcat(test_name, "-d");
            break;
        case LOOKUP_YIELD:
            strcat(test_name, "-l");
            break;
        case ID_YIELD:
            strcat(test_name, "-id");
            break;
        case DL_YIELD:
            strcat(test_name, "-dl");
            break;
        case IL_YIELD:
            strcat(test_name, "-il");
            break;
        case IDL_YIELD:
            strcat(test_name, "-idl");
            break;
        default:
            strcat(test_name, "-none");
            break;
    }

    //check if sync was never changed (no yield option)
    if(opt_sync == 'u') {
        strcat(test_name, "-none");
    } else {
        strcat(test_name, "-");
        strcat(test_name, &opt_sync);
    }
    //catch segfaults
    signal(SIGSEGV, handler);

    //start timer
    struct timespec time_start;
    clock_gettime(CLOCK_MONOTONIC, &time_start);

    //allocate memory for pthread IDs
    pthread_t* ptid = (pthread_t*) malloc(sizeof(pthread_t) * num_threads);
    int* pthread_ids =(int*) malloc(sizeof(int) * num_threads);
    //create pthreads
    for( int i = 0 ; i < num_threads; i++){
        pthread_ids[i] = i;
        if(pthread_create(&ptid[i], NULL, run_thread, &pthread_ids[i])){
            fprintf(stderr, "Unable to create threads. error: %d, message: %s \n", errno, strerror(errno));
            exit(1);
        }
    }

    //join pthreads
    for(int i = 0; i < num_threads; i++){
        if(pthread_join(ptid[i], NULL)){
            fprintf(stderr, "Unable to join threads. error: %d, message: %s \n", errno, strerror(errno));
            exit(1);
        }
    }

    //end time
    struct timespec time_end;
    clock_gettime(CLOCK_MONOTONIC, &time_end);

    // check if list length is 0
    list_len = SortedList_length(head);
    if (list_len != 0) {
        if (list_len == -1) {
            fprintf(stderr, "Error: List is corrupted.\n");
        } else {
            fprintf(stderr, "Error: List is not size 0.\n");
        }
        free(ptid);
        free(head);
        free(elements);
        exit(2);
    }

    long long runtime = time_end.tv_sec - time_start.tv_sec;
    runtime *= 1000000000;
    runtime += time_end.tv_nsec;
    runtime -= time_start.tv_nsec;
    int num_oper = num_threads * num_iter * 3;
    int avg_runtime = runtime / num_oper;
    int num_lists = 1;
    fprintf(stdout, "%s,%d,%d,%d,%d,%lld,%d\n", test_name, num_threads, num_iter, num_lists, num_oper, runtime, avg_runtime);
    fflush(stdout);

    free(ptid);
    free(head);
    free(elements);
    exit(0);
}