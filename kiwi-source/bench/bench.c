/*Theodoros Goltsios 
        AM 1991          */

#include "bench.h"

void _random_key(char *key,int length) {
	int i;
	char salt[36]= "abcdefghijklmnopqrstuvwxyz0123456789";

	for (i = 0; i < length; i++)
		key[i] = salt[rand() % 36];
}

void _print_header(int count)
{
	double index_size = (double)((double)(KSIZE + 8 + 1) * count) / 1048576.0;
	double data_size = (double)((double)(VSIZE + 4) * count) / 1048576.0;

	printf("Keys:\t\t%d bytes each\n", 
			KSIZE);
	printf("Values: \t%d bytes each\n", 
			VSIZE);
	printf("Entries:\t%d\n", 
			count);
	printf("IndexSize:\t%.1f MB (estimated)\n",
			index_size);
	printf("DataSize:\t%.1f MB (estimated)\n",
			data_size);

	printf(LINE1);
}

void _print_environment()
{
	time_t now = time(NULL);

	printf("Date:\t\t%s", 
			(char*)ctime(&now));

	int num_cpus = 0;
	char cpu_type[256] = {0};
	char cache_size[256] = {0};

	FILE* cpuinfo = fopen("/proc/cpuinfo", "r");
	if (cpuinfo) {
		char line[1024] = {0};
		while (fgets(line, sizeof(line), cpuinfo) != NULL) {
			const char* sep = strchr(line, ':');
			if (sep == NULL || strlen(sep) < 10)
				continue;

			char key[1024] = {0};
			char val[1024] = {0};
			strncpy(key, line, sep-1-line);
			strncpy(val, sep+1, strlen(sep)-1);
			if (strcmp("model name", key) == 0) {
				num_cpus++;
				strcpy(cpu_type, val);
			}
			else if (strcmp("cache size", key) == 0)
				strncpy(cache_size, val + 1, strlen(val) - 1);	
		}

		fclose(cpuinfo);
		printf("CPU:\t\t%d * %s", 
				num_cpus, 
				cpu_type);

		printf("CPUCache:\t%s\n", 
				cache_size);
	}
}

int main(int argc,char** argv)
{
	long int count;

	srand(time(NULL));
	if (argc < 3) {
		fprintf(stderr,"Usage: db-bench <write | read | thread_read | thread_write | readwrite> <count>\n");
		exit(1);
	}
	
	if (strcmp(argv[1], "write") == 0) {
		int r = 0;

		count = atoi(argv[2]);
		_print_header(count);
		_print_environment();
		if (argc == 4)
			r = 1;
		_write_test(count, r);
	} else if (strcmp(argv[1], "read") == 0) {
		int r = 0;

		count = atoi(argv[2]);
		_print_header(count);
		_print_environment();
		if (argc == 4)
			r = 1;
		
		_read_test(count, r);

	/* New added code, for threaded read use */	
	} else if (strcmp(argv[1], "thread_read") == 0) {
		int r = 0;

		count = atoi(argv[2]);
		//_print_header(count);
		//_print_environment();

		// We changed the value from 4 to 5, because we need to pass the number of threads as an argument
		if (argc == 5)
			r = 1;

		// Print error message if user doesn't pass the number of threads and teriminate the program
		if(argc < 4)
		{
			fprintf(stderr,"Usage: db-bench <thread_read> <count> <number of threads given by user>\n");
			return 1;
		}
		
		// Get the number of threads given by the user
		int num_of_threads_user_given = atoi(argv[3]);

		/* We call the new function here, that creates threads and calls the hreaded_read_worker function, that each thread will execute */
		_threaded_read_test(count, r, num_of_threads_user_given); 

	/* New added code, for threaded write use */
	} else if (strcmp(argv[1], "thread_write") == 0) {
		int r = 0;

		count = atoi(argv[2]);
		//_print_header(count);
		//_print_environment();

		// We changed the value from 4 to 5, because we need to pass the number of threads as an argument
		if (argc == 5)
			r = 1;

		// Print error message if user doesn't pass the number of threads and teriminate the program
		if(argc < 4)
		{
			fprintf(stderr,"Usage: db-bench <thread_write> <count> <number of threads given by user>\n");
			return 1;
		}
		
		// Get the number of threads given by the user
		int num_of_threads_user_given = atoi(argv[3]);

		/* We call the new function here, that creates threads and calls the threaded_write_worker function, that each thread will execute */
		_threaded_write_test(count, r, num_of_threads_user_given); 

	} else if (strcmp(argv[1], "readwrite") == 0) {
		int r = 0;

		count = atoi(argv[2]);

		// We changed the value from 4 to 6, because we need to pass the number of threads as an argument and the percentage of operations
		if (argc == 6)
			r = 1;

		// Print error message if user doesn't pass the percentage and teriminate the program
		if(argc < 5)
		{
			fprintf(stderr,"Usage: db-bench <readwrite> <count> <number of threads given by user> <percentage>\n");
			return 1;
		}

		
		// Get the percentage of operations given by the user
		int percentage = atoi(argv[4]);

		// Get the number of threads given by the user
		int num_of_threads_user_given = atoi(argv[3]);


		/* We call the new read_write function here */
		_readwrite(count, r, num_of_threads_user_given, percentage);


	} else {
		fprintf(stderr,"Usage: db-bench <write | read | thread_read | thread_write | readwrite> <count> <random>\n");
		exit(1);
	}

	return 1;
}
