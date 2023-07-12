/*Theodoros Goltsios 
        AM 1991          */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#define KSIZE (16)
#define VSIZE (1000)

#define LINE "+-----------------------------+----------------+------------------------------+-------------------+\n"
#define LINE1 "---------------------------------------------------------------------------------------------------\n"

/* To use as arguments, for the worker threads */
typedef struct {
    int r;
    int thread_num;					 // To track each thread
	int count;						// To know how many reads/writes each thread should complete
	int num_of_threads_user_given;	// To know how many threads the user wants to use
} thread_arg;



long long get_ustime_sec(void);
void _random_key(char *key,int length);
