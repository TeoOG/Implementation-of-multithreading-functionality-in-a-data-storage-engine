/*Theodoros Goltsios 
        AM 1991          */

#include <string.h>
#include "../engine/db.h"
#include "../engine/variant.h"
#include "bench.h"

/* We add those following libraries */

// For threads
#include <pthread.h> 

/* For general use, just to be sure, even if it's not needed */
#include <math.h>

#define DATAS ("testdb")


/*-------------------------------------------------------- VERY IMPORTANT -----------------------------------------------------------------------------*/
// Global variable type DB for opening/closing the database and read and writting for thread use. We make it global to get the state of database , cause
// we use it on many functions
DB* db_for_threads; 


/* Original function, we leave it intact */
void _write_test(long int count, int r)
{
	int i;
	double cost;
	long long start,end;
	Variant sk, sv;
	DB* db;

	char key[KSIZE + 1];
	char val[VSIZE + 1];
	char sbuf[1024];

	memset(key, 0, KSIZE + 1);
	memset(val, 0, VSIZE + 1);
	memset(sbuf, 0, 1024);

	db = db_open(DATAS);

	start = get_ustime_sec();
	for (i = 0; i < count; i++) {
		if (r)
			_random_key(key, KSIZE);
		else
			snprintf(key, KSIZE, "key-%d", i);
		fprintf(stderr, "%d adding %s\n", i, key);
		snprintf(val, VSIZE, "val-%d", i);

		sk.length = KSIZE;
		sk.mem = key;
		sv.length = VSIZE;
		sv.mem = val;

		db_add(db, &sk, &sv);
		if ((i % 10000) == 0) {
			fprintf(stderr,"random write finished %d ops%30s\r", 
					i, 
					"");

			fflush(stderr);
		}
	}

	db_close(db);

	end = get_ustime_sec();
	cost = end -start;

	printf(LINE);
	printf("|Random-Write	(done:%ld): %.6f sec/op; %.1f writes/sec(estimated); cost:%.3f(sec);\n"
		,count, (double)(cost / count)
		,(double)(count / cost)
		,cost);	
}

/* Original function, we leave it intact */
void _read_test(long int count, int r)
{
	int i;
	int ret;
	int found = 0;
	double cost;
	long long start,end;
	Variant sk;
	Variant sv;
	DB* db;
	char key[KSIZE + 1];

	db = db_open(DATAS);
	start = get_ustime_sec();
	for (i = 0; i < count; i++) {
		memset(key, 0, KSIZE + 1);

		/* if you want to test random write, use the following */
		if (r)
			_random_key(key, KSIZE);
		else
			snprintf(key, KSIZE, "key-%d", i);
		fprintf(stderr, "%d searching %s\n", i, key);
		sk.length = KSIZE;
		sk.mem = key;
		ret = db_get(db, &sk, &sv);
		if (ret) {
			//db_free_data(sv.mem);
			found++;
		} else {
			INFO("not found key#%s", 
					sk.mem);
    	}

		
		if ((i % 10000) == 0) {
			fprintf(stderr,"random read finished %d ops%30s\r", 
					i, 
					"");

			fflush(stderr);
		}
	}

	db_close(db);

	end = get_ustime_sec();
	cost = end - start;
	printf(LINE);
	printf("|Random-Read	(done:%ld, found:%d): %.6f sec/op; %.1f reads /sec(estimated); cost:%.3f(sec)\n",
		count, found,
		(double)(cost / count),
		(double)(count / cost),
		cost);
}

/* ============================================== */
/* ADDED FUNCTIONS FOLLOWING HERE */


void *threaded_write_worker(void *arg)
{
    int i, start_key, end_key;
    //int ret;
    char key[KSIZE + 1];
    char val[VSIZE + 1];
    Variant sk, sv;

    thread_arg *targ = (thread_arg*)arg;
    int r = targ->r;
    int thread_num = targ->thread_num;
	int count = targ->count;
	int num_of_threads_user_given = targ->num_of_threads_user_given;

    int keys_per_thread = count / num_of_threads_user_given;
    start_key = thread_num * keys_per_thread;
    end_key = start_key + keys_per_thread - 1;
    if (thread_num == num_of_threads_user_given - 1) {
        end_key = count - 1;
    }

	// Take the sum of writes for each thread
	long int sum_of_writes_for_each_thread = 0;

    for (i = start_key; i <= end_key; i++)
    {
        // Take the sum of writes for each thread
		// int sum_of_writes_for_each_thread = 1;

        memset(key, 0, KSIZE + 1);
        memset(val, 0, VSIZE + 1);
        if (r) {
            _random_key(key, KSIZE);
            snprintf(val, VSIZE, "val-%d", rand() % count);
        } else {
            snprintf(key, KSIZE, "key-%d", i);
            snprintf(val, VSIZE, "val-%d", i);
        }
        sk.length = KSIZE;
        sk.mem = key;
        sv.length = VSIZE;
        sv.mem = val;
        db_add(db_for_threads, &sk, &sv); // changed from db to db_for_threads

		// Increment for every db_add we do for each thread
		sum_of_writes_for_each_thread += 1;
    }

	if ((i % 100000) == 0) { // changed from 10000 to 100000
			fprintf(stderr,"random write finished %d ops%30s\r", 
					i, 
					"");

			fflush(stderr);
		}


	// Print the number of keys each thread written, along with the thread number and id in bold lawn green color
	printf("\033[1;32m #%d Thread with id %lu finished and wrote %ld keys\033[0m\n\n\n", thread_num, pthread_self(), sum_of_writes_for_each_thread);
	

    return NULL;
}

void _threaded_write_test(int count, int r,int num_of_threads_user_given)
{
    int i;
    double cost;
    long long start, end;
    pthread_t threads[num_of_threads_user_given];
    //DB *db;

    //count = write_count;

    db_for_threads = db_open(DATAS); //db_open(DATAS);
    start = get_ustime_sec();

    for (i = 0; i < num_of_threads_user_given; i++)
    {
        thread_arg *targ = malloc(sizeof(thread_arg));
        targ->r = r;
        targ->thread_num = i;
		targ->count = count;
		targ->num_of_threads_user_given = num_of_threads_user_given;

        pthread_create(&threads[i], NULL, threaded_write_worker,(void *)targ);
    }

    for (i = 0; i < num_of_threads_user_given; i++)
    {
        pthread_join(threads[i], NULL);
    }

    db_close(db_for_threads); // db_close(db);
    end = get_ustime_sec();
    cost = end - start;
    printf("\033[0;40m");
    printf(LINE);
    printf("|Threaded-Write (done:%d): %.6f sec/op; %.1f writes/sec(estimated); cost:%.3f(sec);\n"
        ,count, (double)(cost / count * num_of_threads_user_given) 
        ,(double)(count * num_of_threads_user_given / cost),(double)cost); 
}



/* New function, each thread executes this code for parallel reading */
void *threaded_read_worker(void *arg)
{
	//DB* db_for_threads; // For db_get , we set it as global
    int i;
    int ret;

    //int found = 0;
	// we cast it to long, in order to escape warning: "cast to pointer from integer of different size 
    // return (void*)found;" warning , cause in a lot of systems, we have sizeof(long) == sizeof(void *) , maybe we can use also malloc
	long found = 0;

    char key[KSIZE + 1];
    Variant sk, sv;

	
	// RETRIEVING-ACCESSING VALUES
	// Initialize struct for thread arguments
    //thread_arg *targ = (thread_arg*)arg;

	/* we don't need to allocate memory for the thread_arg struct because it is passed to the function as a void pointer (void *arg). 
	We first cast 
	a pointer to a thread_arg pointer with (thread_arg*)arg and then access the r and thread_num fields 
	of the struct with targ->r and targ->thread_num, respectively*/
	thread_arg *targ = (thread_arg*)arg;

    int r = targ->r;
    int thread_num = targ->thread_num;
	int num_of_threads_user_given = targ->num_of_threads_user_given;
	int count = targ->count;

	// Logic for dividing the keys for each thread, in order to cover odd number of keys
	// Check if we are on the last thread and if so, we give the rest of the keys to the last thread to process
    int keys_per_thread = count / num_of_threads_user_given;
    int start_key = thread_num * keys_per_thread;
    int end_key = start_key + keys_per_thread - 1;
	if (thread_num == num_of_threads_user_given - 1) {
        end_key = count - 1;
    }

    for (i = start_key; i <= end_key; i++)
    {
    	memset(key, 0, KSIZE + 1);
        if (r) {
            snprintf(key, KSIZE, "key-%d", rand() % count);
        } else {
            snprintf(key, KSIZE, "key-%d", i);
        }
        sk.length = KSIZE;
        sk.mem = key;
        ret = db_get(db_for_threads, &sk, &sv); // we change it here
        if (ret)
        {
            found++;
        }
        else
        {
            INFO("not found key#%s", sk.mem);
        }

		// Change from 10000 to 1000000
		if ((i % 1000000) == 0) {
			fprintf(stderr,"random read finished %d ops%30s\r", 
					i, 
					"");

			fflush(stderr);
		}
    }

	// Print the number of keys found for each thread, along with the thread number and id in bold yellow color
	printf("\033[1;33m #%d Thread with id %lu finished and found %ld keys\033[0m\n\n\n", thread_num, pthread_self(), found);


	// return the number of keys found for each thread
    return (void*)found;
}

/* New function, this is the function that creates and calls the threads, opens and closes the database */
void _threaded_read_test(int count, int r,int num_of_threads_user_given)
{
	// For every thread number
    int i;

    //int found = 0;
	long found = 0; // we change it to long, in order to escape warning: "cast to pointer from integer of different size

    double cost;
    long long start, end;
    pthread_t threads[num_of_threads_user_given];
    // DB *db;



    db_for_threads = db_open(DATAS); // we use the global variable for db handling
    start = get_ustime_sec();

    for (i = 0; i < num_of_threads_user_given; i++)
    {
		
		// INITIALIZING-SETTING VALUES
		// Initializing targ struct for each thread
		// We use malloc to allocate memory for a thread_arg struct on the heap. 
		// We then set the r field to the r argument passed to _threaded_read_test from the main func , from the user , 
		// and the thread_num field to i, the current thread number in the loop.

		/* we need to allocate memory dynamically for the thread_arg struct because we will be passing a pointer to this struct as the argument 
		to each thread created with pthread_create. Each thread will have its own copy of the thread_arg struct,
		and therefore each copy must be located in separate memory.*/

        thread_arg *targ = malloc(sizeof(thread_arg));
        targ->r = r;															
        targ->thread_num = i;
		targ->count = count;
		targ->num_of_threads_user_given = num_of_threads_user_given;
		
		/* In contrast when I initialized the targ struct statically like this, had problems when calculating odd number of read and writes, maybe
		cause some threads would receive a pointer to that stack memory from the main thread that called it. 
		Generally to be sure, it's recommended to allocate it ALWAYS dynamically 
		thread_arg targ;
		targ.r = r;
		targ.thread_num = i; */

		// We pass the above thread_arg struct as an argument to pthread_create, 
		// which will be passed to the threaded_read_worker function when the thread is created
        pthread_create(&threads[i], NULL, threaded_read_worker,(void *)targ);

    }

    for (i = 0; i < num_of_threads_user_given; i++)
    {
        void *result;
        pthread_join(threads[i], &result);
        //found += (int)result;
		found += (long)result; // we change it to long, in order to escape warning: "cast to pointer from integer of different size"
    }

    db_close(db_for_threads); // We use the global variable for db handling
    end = get_ustime_sec();
    cost = end - start;
    printf("\033[0;40m");
    printf(LINE);
	// Changed "found:%d", to "found:%ld" , for the same reason as above
    printf("|Threaded-Read (done:%d, found:%ld): %.6f sec/op; %.1f reads/sec(estimated); cost:%.3f(sec)\n",
            count, found,
            (double)(cost / count),
            (double)(count / cost),
            (double)cost);
    printf("\033[0m");
}


/* New function */
void _readwrite(int count, int r, int num_of_threads_user_given, int percentage)
{

    //long int found;

    int i;
    double cost;
    long long start, end;
    pthread_t threads_w[100];
    pthread_t threads_r[100];

    /* Getting the number of write and read threads based on the percentage that the user gave */

    // Get the percentage of write threads
    int num_of_write_threads = num_of_threads_user_given * percentage / 100;

    // Get the percentage of read threads, it is the rest of the threads
    int num_of_read_threads = num_of_threads_user_given - num_of_write_threads;

    db_for_threads = db_open(DATAS);
    start = get_ustime_sec();

    for (i = 0; i < num_of_write_threads; i++)
    {
        thread_arg *targ = malloc(sizeof(thread_arg));
        targ->r = r;
        targ->thread_num = i;
        targ->count = ((count * percentage) / 100);
        targ->num_of_threads_user_given = num_of_write_threads;

        
        pthread_create(&threads_w[i], NULL, threaded_write_worker, (void *)targ);
    }

    for (i = 0; i < num_of_read_threads; i++)
    {
        thread_arg *targ = malloc(sizeof(thread_arg));
        targ->r = r;
        targ->thread_num = i;
        targ->count = ((count * (100 - percentage)) / 100); // Use (100 - percentage) for read threads
        targ->num_of_threads_user_given = num_of_read_threads; // Use num_of_read_threads for read threads

        
        pthread_create(&threads_r[i], NULL, threaded_read_worker, (void *)targ);
    }

    for (i = 0; i < num_of_write_threads; i++)
    {
        pthread_join(threads_w[i], NULL);
    }

    //void *result;
    for (i = 0; i < num_of_read_threads; i++)
    {
        void *result;
        pthread_join(threads_r[i], &result);
        //found += (int)result;
		//found += (long)result; // we change it to long, in order to escape warning: "cast to pointer from integer of different size"
    }

   

   


    db_close(db_for_threads);

    end = get_ustime_sec();
    cost = end - start;
    printf("\033[0;40m");
    printf(LINE);
    printf("|Threaded-ReadWrite (done:%d): %.6f sec/op; %.1f ops/sec(estimated); cost:%.3f(sec);\n"
        , count, (double)(cost / count * num_of_threads_user_given)
        , (double)(count * num_of_threads_user_given / cost), (double)cost);
}
