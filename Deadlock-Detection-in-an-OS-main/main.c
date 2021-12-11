#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>
#include <limits.h>
#include <time.h>


// To enable multi-threading capabilities using a thread pool of a fixed size
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER ;


// To maintain the time out interval
time_t start, end ;


// A structure to maintain information about each resource overall
struct thread_info{

	int threadID ;
	int *release_wait_time_bounds ;
	int *requested ;
	int *present ;
	int *required ;

} ;


// A structure to maintain information of resources specific to a particular thread
struct resource_info{

	int *maximum ;
	int *available ;

} ;


// A structure containing the information to be used by the working thread function
struct working_thread_data{

	int NUM_RESOURCES ;
	int TIME_OUT ;
	int *terminated_threads ;
	struct resource_info *global_resources ;
	struct thread_info *current_thread ;

} ;


// A structure having all the data needed by the deadlock detection thread to detect deadlocks
struct dd_thread{

	int sleep_time ;
	int NUM_RESOURCES ;
	int NUM_THREADS ;
	int HEURISTIC_NUMBER ;
	int TIME_OUT ;
	int num_deadlocks ;
	int *terminated_threads ;
	struct resource_info *global_resources ;
	struct thread_info **ind_threads ;

} ;


// A random number seed generator
void random_init() {

    srand(time(NULL)) ;

}


// A function to generate and store random numbers within a specific range into an array
void generate_random_int(int lower_limit, int upper_limit[], int array[], int array_size){

        for(int i=0; i<array_size; i++){
        	array[i] = (rand()%((upper_limit[i]*8)/10 - lower_limit + 1)) + lower_limit ;
        }

}


// A useful auxilliary function to copy the contents of one array into another
void copy_array(int len, int array_source[], int array_destination[]){

	for(int i=0; i<len; i++){
		array_destination[i] = array_source[i] ;
	}

}


// A useful auxilliary function to copy the contents of one array into another
void set_zero_array(int len, int array[]){

	for(int i=0; i<len; i++){
		array[i] = 0 ;
	}

}


// A print function for an array mostly to be used for debugging
void print_array(int len, int array[]){

	for(int i=0; i<len; i++){
		printf("%d  ",array[i]) ;
	}

}


// A function to confirm whether all elements in an array are 1 or not
int check_all_1(int len, int array[]){


	for(int i=0; i<len; i++){
		if(array[i] == 0){
			return 0 ;
		}
	}
	return 1 ;

}


// A function to check Banker's Condition in Banker's Algorithm
int bankers_condition(int num_resources, int array_required[], int array_work[], int terminated, int finished){

	if(terminated==1 || finished == 1){
		return 0 ;
	}
	for(int i=0; i<num_resources; i++){
		if(array_required[i] > array_work[i]){
			return 0 ;
		}
	}
	return 1 ;

}


// A function to mark a thread for termination by tbe deadlock detection algorithm
void terminate_thread(struct dd_thread *info, int thread_id){

	printf("Terminating Thread %d since it is involved in a deadlock\n",thread_id) ;

	for(int i=0; i<info->NUM_RESOURCES; i++){
		((info->global_resources)->available)[i] += (((info->ind_threads)[thread_id])->present)[i] ;
		(((info->ind_threads)[thread_id])->requested)[i] = 0 ;
		(((info->ind_threads)[thread_id])->present)[i] = 0 ;
		(((info->ind_threads)[thread_id])->required)[i] = 0 ;
	}

	(info->terminated_threads)[thread_id] = 1 ;

}


// In the zero'th heuristic, all the threads marked 'unfinshed' at the end of Banker's Algorithm are terminated
int heuristic0(struct dd_thread *info, int finished[]){

	// Initialize variables which determine whether the system is deadlocked or not
	int deadlocked = 0, num_deadlocked_threads = 0 ;
	int deadlocked_threads[info->NUM_THREADS] ;

	// Tracjing all of the threads marked 'unfinished' and terminating them
	for(int i=0; i<info->NUM_THREADS; i++){
		if((info->terminated_threads)[i] == 1){
			continue ;
		}
		if(finished[i] == 0){
			deadlocked = 1 ;
			deadlocked_threads[num_deadlocked_threads] = i ;
			num_deadlocked_threads++ ;
			terminate_thread(info,i) ;
			finished[i] = 1 ;
			continue ;
		}
	}

	// Showing the user which threads were deadlocked
	if(deadlocked == 1){
		printf("Deadlock detected! Heuristic 0: Thus, all the threads invloved in deadlocks (and have been terminated) are: ") ;
		for(int i=0; i<num_deadlocked_threads; i++){
			printf("Thread %d  ",deadlocked_threads[i]) ;
		}
		printf("\n") ;
		return 1 ;
	}

	return 0 ;

}


// In the first heuristic, the thread found to be 'unfinished' along with having maximum number of resources allocated overall is terminated, after which the 'work' array is updated and Banker's Algorithm re-run
int heuristic1(struct dd_thread *info, int work[], int finished[]){

	// Initialize variables which determine whether the system is deadlocked or not
	int deadlocked = 0, max_resources = 0 ;
	int deadlocked_thread ; 

	// Finding the 'unfinished' thread having minimum resources overall and terminating it
	for(int i=0; i<info->NUM_THREADS; i++){
		if((info->terminated_threads)[i] == 1){
			continue ;
		}
		if(finished[i] == 0){
			deadlocked = 1 ;
			int num_resources = 0 ;
			for(int j=0; j<info->NUM_RESOURCES; j++){
				num_resources += (((info->ind_threads)[i])->present)[j] ;
			}
			if(num_resources > max_resources){
				deadlocked_thread = i ;
				max_resources = num_resources ;
			}
		}
	}

	// Updating the 'work' array and showing the user the terminated thread
	if(deadlocked == 1){
		printf("Deadlock detected! Heuristic 2: One thread found to be invloved in the deadlock (and has been terminated) is: Thread %d\n",deadlocked_thread) ;
		for(int i=0; i<info->NUM_RESOURCES; i++){
			work[i] += (((info->ind_threads)[deadlocked_thread])->present)[i] ;
		}
		terminate_thread(info,deadlocked_thread) ;
		finished[deadlocked_thread] = 1 ;
		return 1 ;
	}

	return 0 ;

}


// In the second heuristic, the 'unfinished' thread having minimum resources overall is terminated and then Banker's Algorithm is re-run after updating the 'work' array
int heuristic2(struct dd_thread *info, int work[], int finished[]){

	// Initialize variables which determine whether the system is deadlocked or not
	int deadlocked = 0, min_resources = INT_MAX ;
	int deadlocked_thread ; 

	// Finding the 'unfinished' thread having minimum resources overall and terminating it
	for(int i=0; i<info->NUM_THREADS; i++){
		if((info->terminated_threads)[i] == 1){
			continue ;
		}
		if(finished[i] == 0){
			deadlocked = 1 ;
			int num_resources = 0 ;
			for(int j=0; j<info->NUM_RESOURCES; j++){
				num_resources += (((info->ind_threads)[i])->present)[j] ;
			}
			if(num_resources < min_resources){
				deadlocked_thread = i ;
				min_resources = num_resources ;
			}
		}
	}

	// Updating the 'work' array and showing the user the terminated thread
	if(deadlocked == 1){
		printf("Deadlock detected! Heuristic 2: One thread found to be invloved in the deadlock (and has been terminated) is: Thread %d\n",deadlocked_thread) ;
		for(int i=0; i<info->NUM_RESOURCES; i++){
			work[i] += (((info->ind_threads)[deadlocked_thread])->present)[i] ;
		}
		terminate_thread(info,deadlocked_thread) ;
		finished[deadlocked_thread] = 1 ;
		return 1 ;
	}

	return 0 ;

}


// The thread function for each of the working threads (active in resource competition)
void* working_thread_function(void *thread_data){

	// All the information needed by the thread to acquire and release resources and the waiting time/random pauses
	struct working_thread_data *all_info = (struct working_thread_data *)thread_data ;
	int thread_waiting_lower = ((all_info->current_thread)->release_wait_time_bounds)[0], thread_waiting_upper = ((all_info->current_thread)->release_wait_time_bounds)[1] ;

	// Loop indefinately
	while(1){

		// Terminating the thread if the time out interval has passed
		time(&end) ;
		double diff = difftime(end,start) ;
		if(diff > all_info->TIME_OUT){
			pthread_exit(NULL) ;
		}

		// Terminating and Restarting a thread if it is marked to be terminated by the deadlock detection procedure
		int thread_id = (all_info->current_thread)->threadID ;
		if((all_info->terminated_threads)[thread_id] == 1){
			pthread_mutex_lock(&mutex) ;
			printf("Respawning a new Thread %d in place of it's old one\n",thread_id) ;
			generate_random_int(0,(all_info->global_resources)->maximum,(all_info->current_thread)->requested,all_info->NUM_RESOURCES) ;
			set_zero_array(all_info->NUM_RESOURCES,(all_info->current_thread)->present) ;
			copy_array(all_info->NUM_RESOURCES,(all_info->current_thread)->requested,(all_info->current_thread)->required) ;
			(all_info->terminated_threads)[thread_id] = 0 ;
			pthread_mutex_unlock(&mutex) ;
			int wait_time = (rand()%(thread_waiting_upper - thread_waiting_lower + 1)) + thread_waiting_lower ;
			usleep(wait_time) ;
			continue ;
		}

		printf("Thread %d Running Currently\n", (all_info->current_thread)->threadID) ;

		// Since these operations invloves modifying shared resources, a mutex lock is used to prevent interference from other threads when one thread has access
		pthread_mutex_lock(&mutex) ;

		// Checking if the thread in execution already has it's resource requirements satisfied
		int all_zero = 1 ;
		for(int i=0; i<all_info->NUM_RESOURCES; i++){
			if(((all_info->current_thread)->required)[i] != 0){
				all_zero = 0 ;
				break ;
			}
		}

		// In case the thread already has all of it's required resources, it will release all of it's resources, set new requirements, sleep, and then execute again
		if(all_zero == 1){
			printf("Thread %d's requirement fulfilled. Holding on to the resources for a while and then releasing them\n",(all_info->current_thread)->threadID) ;
			int thread_waiting_lower = ((all_info->current_thread)->release_wait_time_bounds)[0], thread_waiting_upper = ((all_info->current_thread)->release_wait_time_bounds)[1] ;
			int wait_time = (rand()%(thread_waiting_upper - thread_waiting_lower + 1)) + thread_waiting_lower ;
			usleep(wait_time) ;
			for(int i=0; i<all_info->NUM_RESOURCES; i++){
				((all_info->global_resources)->available)[i] += ((all_info->current_thread)->present)[i] ;
			}
			generate_random_int(0,(all_info->global_resources)->maximum,(all_info->current_thread)->requested,all_info->NUM_RESOURCES) ;
			set_zero_array(all_info->NUM_RESOURCES,(all_info->current_thread)->present) ;
			copy_array(all_info->NUM_RESOURCES,(all_info->current_thread)->requested,(all_info->current_thread)->required) ;
			pthread_mutex_unlock(&mutex) ;
			continue ;
		}

		// If resource requirements are pending, the thread randomly selects a resource who's requirement is not fullfilled, and acquires how much ever it can from what is available
		int max_loop_runs = 1000, current_run = 0 ;
		while(current_run < max_loop_runs){
			int random_resource = (rand()%(all_info->NUM_RESOURCES)) ;
			if(((all_info->current_thread)->required)[random_resource] == 0){
				current_run++ ;
				continue ;
			} 
			if(((all_info->global_resources)->available)[random_resource] >=  ((all_info->current_thread)->required)[random_resource]){
				((all_info->global_resources)->available)[random_resource] -= ((all_info->current_thread)->required)[random_resource] ;
				((all_info->current_thread)->present)[random_resource] += ((all_info->current_thread)->required)[random_resource] ;
				((all_info->current_thread)->required)[random_resource] = 0 ;
			}
			else{
				((all_info->current_thread)->required)[random_resource] -= ((all_info->global_resources)->available)[random_resource] ;
				((all_info->current_thread)->present)[random_resource] += ((all_info->global_resources)->available)[random_resource] ;
				((all_info->global_resources)->available)[random_resource] = 0 ;
			}
			current_run++ ;
			break ;
		}


		// After all shared resource operations are done, unlock the mutex (so other threads can access them), and wait for the specified period
		pthread_mutex_unlock(&mutex) ;
		int wait_time = (rand()%(thread_waiting_upper*1000 - thread_waiting_lower*1000 + 1)) + thread_waiting_lower*1000 ;
		usleep(wait_time) ;

	}  

}


// The thread function for the deadlock detection thread using Banker's Algorithm (checks after a specified thread interval by the user)
void* deadlock_detection_thread_function(void *dd_thread_info){

	// All the information required by Banker's Algorithm to detect deadlocks
	struct dd_thread *info = (struct dd_thread *)dd_thread_info ;

	// Loop indefinately
    while(1){

    	// Terminating the thread if the time out intreval has passed
    	time(&end) ;
	double diff = difftime(end,start) ;
	if(diff > info->TIME_OUT){
		pthread_exit(NULL) ;
	}

    	// Sleep for a user specified period between each subsequent deadlock check
    	// Mutex to prevent working threads from acquiring/releasing resources while deadlock detection is in progress
    	sleep(info->sleep_time) ;
    	pthread_mutex_lock(&mutex) ;

    	// Check if all working threads have terminated. If yes, exit the program
    	int all_terminated = 1 ;
    	for(int i=0; i<info->NUM_THREADS; i++){
    		if(info->terminated_threads[i] == 0){
    			all_terminated = 0 ;
    			break ;
    		}
    	}
    	if(all_terminated == 1){
    		printf("All threads have terminated, exiting the program\n") ;
    		exit(-1) ;
    	}

    	// Initialize variables which determine whether the system is deadlocked or not
    	int deadlocked = 0 ;
    	int deadlocked_threads[info->NUM_THREADS] ;
    	int num_deadlocked_threads = 0 ;
    	int heuristic_1_2_marked = 0 ;
		
		// Initializing the work & finished arrays as per Banker's Algorithm
    	int work[info->NUM_RESOURCES], finished[info->NUM_THREADS] ;
    	copy_array(info->NUM_RESOURCES,(info->global_resources)->available,work) ;
    	set_zero_array(info->NUM_RESOURCES,finished) ;

    	// Checking threads with resource requirements already fulfilled, marking them as finished, and then updating the work array accordingly
    	for(int i=0; i<info->NUM_THREADS; i++){
    		if((info->terminated_threads)[i] == 1){
    			continue ;
    		}
    		int fulfilled = 1 ;
    		for(int j=0; j<info->NUM_RESOURCES; j++){
    			if((((info->ind_threads)[i])->required)[j] != 0){
    				fulfilled = 0 ;
    				break ;
    			}
    		}
    		if(fulfilled == 1){
    			finished[i] = 1 ;
    			for(int j=0; j<info->NUM_RESOURCES; j++){
    				work[j] += (((info->ind_threads)[i])->present)[j] ;
    			}
    		}
    	}

    	// Implementing Banker's Algorithm
    	while(1){

    		// Initializing terminating condition variables of Banker's Algorithm
    		int termination_condition = 1 ;
    		int bankers_index = 0 ;

    		// Checking Banker's condition for each thread
    		for(int i=0; i<info->NUM_THREADS; i++){
    			if(bankers_condition(info->NUM_RESOURCES,((info->ind_threads)[i])->required,work,(info->terminated_threads)[i],finished[i]) == 1){
    				termination_condition = 0 ;
    				bankers_index = i ;
    				break ;
    			}
    		}

    		// Updating the work and finished arrays for threads satisfying Banker's Condition
    		if(termination_condition == 0){
    			finished[bankers_index] = 1 ;
    			for(int i=0; i<info->NUM_RESOURCES; i++){
    				work[i] += (((info->ind_threads)[bankers_index])->present)[i] ;
    			}
    			continue ;
    		}

    		// Once no thread satisfies Banker's Condition, checking the finished array to see if the system is deadlocked or not based on the various heuristics defined
    		else{
    			if(info->HEURISTIC_NUMBER == 0){
    				int deadlocked = heuristic0(info,finished) ;
    				if(deadlocked == 0){
    					printf("Checking for a deadlock: No deadlock detected\n") ;
    				}
    				else{
    					(info->num_deadlocks)++ ;
    				}
    			}
    			else{
    				if(info->HEURISTIC_NUMBER == 1){
    					int deadlocked = heuristic1(info,work,finished) ;
    					if(deadlocked == 0){
    						printf("Checking for a deadlock: No deadlock detected\n") ;
    					}
    					else{
    						if(heuristic_1_2_marked == 0){
    							(info->num_deadlocks)++ ;
    							heuristic_1_2_marked = 1 ;
    						}
    						continue ;
    					}
    				}
    				else{
    					int deadlocked = heuristic2(info,work,finished) ;
    					if(deadlocked == 0){
    						printf("Checking for a deadlock: No deadlock detected\n") ;
    					}
    					else{
    						if(heuristic_1_2_marked == 0){
    							(info->num_deadlocks)++ ;
    							heuristic_1_2_marked = 1 ;
    						}
    						continue ;
    					}
    				}
    			}
    		}

    		break ;

    	}

    	printf("Total Number of deadlocks detected so far: %d\n",info->num_deadlocks) ;

    	// Unlocking the mutex so the threads not involved in the deadlock can acquire and release resources normally
		pthread_mutex_unlock(&mutex) ;

    }

}


int main(int argc, char *argv[]) {

    // If the user enters the command line arguments in an incorrect format
    if (argc < 9 || argc%2 == 0){
    	printf("Incorrect passage of arguments. Terminating the Program. Try again with correct argument passing as specified below.\n") ;
        printf("Correct argument structure: [NoT] [DCT] [HNo] [MaxT] [Resource A] [Amount A] [Resource B] [Amount B] ...\n") ;
        printf("NoT: Number of Threads to be used in the Program\n") ;
        printf("DCT: Deadlock Checking Time ie the time in between each subsequent check for a deadlock (in seconds)\n") ;
        printf("HNo: The Heuristic to be used (further explanation on this in the README)\n") ;
        printf("MaxT: The maximum time (in seconds) the program can run, after which it will terminate\n") ;
        printf("Resource X: Name of the Resource\n") ;
        printf("Amount X: Number of Initial Instances of 'X' Available\n") ;
        printf("... Means that multiple such pairs of resources and their amounts can be specified\n") ;
        exit(-1) ;
    }

    // Initializing the random seed generator
    random_init() ;

    // Initializing some useful & common variables
    int NUM_THREADS = atoi(argv[1]) ;
    int DEADLOCK_CHECK_TIME = atoi(argv[2]) ;
    int HEURISTIC_NUMBER = atoi(argv[3]) ;
    int TIME_OUT = atoi(argv[4]) ;
    int NUM_RESOURCES = (argc-5)/2 ;

    // Initialising the overall resource information based on what the user has passed
    struct resource_info *resources = (struct resource_info *)malloc(sizeof(struct resource_info)) ;
    resources->maximum = (int *)malloc(NUM_RESOURCES*sizeof(int)) ;
    resources->available = (int *)malloc(NUM_RESOURCES*sizeof(int)) ;
    int resource_number = 6, i = 0 ;
    while(resource_number <= argc-1){
    	(resources->maximum)[i] = atoi(argv[resource_number]) ;
    	resource_number += 2 ;
    	i++ ;
    }
    copy_array(NUM_RESOURCES,resources->maximum,resources->available) ;

    printf("Resources Initially: ") ;
    print_array(NUM_RESOURCES, resources->maximum) ;
    printf("\n") ;

    // The time a thread can hold on to it's resources (once all of it's requirements are met) spans between 0.7 and 1.5 times the deadlock check interval
    int *resource_release_time_bounds = (int *)malloc(2*sizeof(int)) ;
    resource_release_time_bounds[0] = (1000*DEADLOCK_CHECK_TIME)*0.7 ;
    resource_release_time_bounds[1] = (1000*DEADLOCK_CHECK_TIME)*1.5 ;
    
    // Initialising the thread specific resource requirements as per a bounded random metric
    struct thread_info *threads[NUM_THREADS] ;
    for(int i=0; i<NUM_THREADS; i++){

    	threads[i] = (struct thread_info *)malloc(sizeof(struct thread_info)) ;

    	(threads[i])->threadID = i ;

    	(threads[i])->release_wait_time_bounds = resource_release_time_bounds ;

    	(threads[i])->requested = (int *)malloc(NUM_RESOURCES*sizeof(int)) ;
    	generate_random_int(0,resources->maximum,(threads[i])->requested,NUM_RESOURCES) ;

    	(threads[i])->present = (int *)malloc(NUM_RESOURCES*sizeof(int)) ;
    	set_zero_array(NUM_RESOURCES,(threads[i])->present) ;

    	(threads[i])->required = (int *)malloc(NUM_RESOURCES*sizeof(int)) ;
    	copy_array(NUM_RESOURCES,(threads[i])->requested,(threads[i])->required) ;

    }

    printf("Thread Requirements Set Randomly:\n") ;
    for(int i=0; i<NUM_THREADS; i++){
    	printf("Thread %d: ",i) ;
	    print_array(NUM_RESOURCES,(threads[i])->requested) ;
	    printf("\n") ;
    }

    // Initializing an array to keep track of terminated threads ;
    int *terminated_threads = (int *)malloc(NUM_THREADS*sizeof(int)) ;
    set_zero_array(NUM_THREADS,terminated_threads) ;

    // Creating the required data structures for passong to the working thread function code for execution
    struct working_thread_data *thread_args[NUM_THREADS] ;
    for(int i=0; i<NUM_THREADS; i++){
    	thread_args[i] = (struct working_thread_data *)malloc(sizeof(struct working_thread_data)) ;
    	(thread_args[i])->NUM_RESOURCES = NUM_RESOURCES ;
    	(thread_args[i])->TIME_OUT = TIME_OUT ;
    	(thread_args[i])->global_resources = resources ;
    	(thread_args[i])->current_thread = threads[i] ;
    	(thread_args[i])->terminated_threads = terminated_threads ;
    }

    // Creating the data structure having all the information needed by the deadlock detection function
    struct dd_thread *dd_thread_info = (struct dd_thread *)malloc(sizeof(struct dd_thread)) ;
    dd_thread_info->sleep_time = DEADLOCK_CHECK_TIME ;
    dd_thread_info->NUM_RESOURCES = NUM_RESOURCES ;
    dd_thread_info->NUM_THREADS = NUM_THREADS ;
    dd_thread_info->HEURISTIC_NUMBER = HEURISTIC_NUMBER ;
    dd_thread_info->TIME_OUT = TIME_OUT ;
    dd_thread_info->num_deadlocks = 0 ;
    dd_thread_info->global_resources = resources ;
    dd_thread_info->ind_threads = threads ;
    dd_thread_info->terminated_threads = terminated_threads ;

    // Measuring the time of execution from the moment the worker threads start running
    time(&start) ;

    // Creating and executing the working threads
    pthread_t working_threads[NUM_THREADS] ;
    for(int i=0; i<NUM_THREADS; i++){
    	pthread_create(&working_threads[i], NULL, working_thread_function, thread_args[i]) ;
    }

    // Creating and executing the deadlock detection threads
    pthread_t deadlock_detection_thread ;
    pthread_create(&deadlock_detection_thread, NULL, deadlock_detection_thread_function, dd_thread_info) ;
    pthread_join(deadlock_detection_thread, NULL) ;

    // In case the user does not issue a keyboard interrupt, the average time between deadlocks throughout the code run is displayed after the time out interval
    double avg_time_between_deadlocks = ((float)TIME_OUT)/(dd_thread_info->num_deadlocks) ;
    printf("Average time between all deadlocks in this program: %lf\n",avg_time_between_deadlocks) ;

}



