#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <time.h>


// To maintain the time out interval
time_t begin, end ;
struct timespec now ;


// Initializing the characteristics of the program as global variables
int p, q, n, m, t, a, N, T, num_processes = 0, num_allocated_processes = 0 ;
double avg_turnaround_time = 0.0 ;


// A structure storing the information about a sinle ,contiguous block in the simulated physical memory.
struct memory_node{

	int empty ;
	int thread ;
	int size ;
	int start ;
	int end ;
	struct memory_node *prev ;
	struct memory_node *next ;

} ;


// A structure storing pointers to the beginning and end of the simulated physical memory
struct memory{

	struct memory_node *front ;
	struct memory_node *back ;

} ;


// A structure that stores all the relevant information about a newly spawned process
struct thread_info{

	int thread_number ;
	int size ;
	int duration ;
	struct timespec start_time, finish_allocation_time ;
	struct memory_node *allocated_block ;

} ;


// A structure to store the information of a single process when in the 'pending' or 'allocated' lists
struct ll_node{

	struct thread_info *process ;
	struct ll_node *prev ;
	struct ll_node *next ;

} ;


// Global pointers to the 'front' or 'head' of the pending and allocated process lists respectively
struct ll_node *pending_processes = NULL ;
struct ll_node *allocated_processes = NULL ;


// Global pointers to the 'front' or 'head' and the last allocated block in the physical memory
struct memory_node *front = NULL ;
struct memory_node *last_allocation = NULL ;


// A random number seed generator
void random_init() {

    srand(time(NULL)) ;

}


// A function to generate a random Process Arrival Rate (PAR) within the specified limits
int generate_PAR(){

	return (int)((rand()%((int)(1.2*n - 0.1*n + 1.0))) + 0.1*n) ;

}


// A function to to generate a random process size request within specified limits and being a multiple of 10 MB
int generate_process_size(){

	int size = (int)((rand()%((int)(3.0*m - 0.5*m + 1.0))) + 0.5*m) ;
	return size + 10 - (size%10) ;

}


// A function to generate a random process running time request within specified limits and being a multiple of 5 minutes
int generate_process_duration(){

	int duration = (int)((rand()%((int)(6.0*n - 0.5*n + 1.0))) + 0.5*n) ;
	return duration + 5 - (duration%5) ;

}


int length_of_ll(struct ll_node *head){

	struct ll_node *current = head ;
	int answer = 0 ;
	while(current != NULL){
		current = current->next ;
		answer++ ;
	}
	return answer ;

}


// A function to calculate the total physical memory utilization (in %). Called at the end of execution of the program
double calculate_memory_utilization(){

	struct memory_node *current = front ;
	int memory_utilized = 0 ;
	while(current != NULL){
		if(current->empty != 1){
			memory_utilized += current->size ;
		}
		current = current->next ;
	}
	double memory_utilization = (((float)memory_utilized)/p)*100 ;
	return memory_utilization ;

}


// A function to create a process of random sizes and running durations (within specified limits)
struct thread_info* create_process(){

	struct thread_info *process = (struct thread_info *)malloc(sizeof(struct thread_info)) ;
	process->thread_number = num_processes ;
	num_processes++ ;
	process->size = generate_process_size() ;
	process->duration = generate_process_duration() ;
	process->allocated_block = NULL ;
	clock_gettime(CLOCK_MONOTONIC,&(process->start_time));

	return process ;

}


// A function to merge to consecutive, empty/unallocated blocks in the physical memory together, called after every process deallocation
struct memory_node* merge_empty_nodes(){

	struct memory_node *current = front->next ;

	while(current != NULL){

		if(current->empty == 1 && (current->prev)->empty == 1){
			(current->prev)->size += current->size ;
			(current->prev)->end = current->end ;
			(current->prev)->next = current->next ;
			if(current->next != NULL){
				(current->next)->prev = current->prev ;
			}
			struct memory_node *temp = current ;
			current = current->next ;
			temp->prev = NULL ;
			temp->next = NULL ;
			free(temp) ;
			continue ;
		}

		current = current->next ;

	}

	return front ;

}


// A function to simulate the best-fit dynamic memory allocation algorithm
struct memory_node* best_fit(struct thread_info *info, int *changed){

	// Initializing all the required variables and structures beforehand
	int thread_number = info->thread_number ;
	int size = info->size ;
	int min_acceptable_size = INT_MAX ;
	struct memory_node *current = front->next ;
	struct memory_node *ideal_node = NULL ;
	*changed = 1 ;

	// Looping till the block having the 'best fit' is found
	while(current != NULL){

		if(current->empty == 1 && (current->size > size && current->size < min_acceptable_size)){
			min_acceptable_size = current->size ;
			ideal_node = current ;
		}

		current = current->next ;

	}

	// In case there is no block big enough to accomodate the process, it won't be allocated
	if(ideal_node == NULL){
		*changed = 0 ;
		return front ;
	}

	// Adjusting the physical memory to make space for the new block for the process
	struct memory_node *prev_node = ideal_node->prev ;
	struct memory_node *new_node = (struct memory_node *)malloc(sizeof(struct memory_node)) ;
	new_node->empty = 0 ;
	new_node->thread = thread_number ;
	new_node->size = size ;
	new_node->start = ideal_node->start ;
	new_node->end = ideal_node->start + size ;
	ideal_node->size -= size ;
	ideal_node->start = ideal_node->start + size ;
	new_node->prev = prev_node ;
	prev_node->next = new_node ;
	ideal_node->prev = new_node ;
	new_node->next = ideal_node ;
	last_allocation = new_node ;

	// Keeping track of the time once the process has been successfully allocated
	clock_gettime(CLOCK_MONOTONIC,&(info->finish_allocation_time));
	double time_taken;
    time_taken = (info->finish_allocation_time.tv_sec - info->start_time.tv_sec) * 1e9;
    time_taken = (time_taken + (info->finish_allocation_time.tv_nsec - info->start_time.tv_nsec)) * 1e-9;
	avg_turnaround_time = ((avg_turnaround_time*num_allocated_processes) + time_taken)/(num_allocated_processes + 1) ;
	num_allocated_processes++ ;

	info->allocated_block = new_node ;

	return front ;

}


// A function to simulate the first-fit dynamic memory allocation algorithm
struct memory_node* first_fit(struct thread_info *info, int *changed){

	// Initializing all the required variables and structures beforehand
	int thread_number = info->thread_number ;
	int size = info->size ;
	struct memory_node *current = front->next ;
	*changed = 1 ;

	// Looping till the block having the 'first fit' is found
	while(current != NULL){

		if(current->empty == 1 && current->size > size){
			break ;
		}

		current = current->next ;

	}

	// In case there is no block big enough to accomodate the process, it won't be allocated
	if(current == NULL){
		*changed = 0 ;
		return front ;
	}

	// Adjusting the physical memory to make space for the new block for the process
	struct memory_node *prev_node = current->prev ;
	struct memory_node *new_node = (struct memory_node *)malloc(sizeof(struct memory_node)) ;
	new_node->empty = 0 ;
	new_node->thread = thread_number ;
	new_node->size = size ;
	new_node->start = current->start ;
	new_node->end = current->start + size ;
	current->size -= size ;
	current->start = current->start + size ;
	new_node->prev = prev_node ;
	prev_node->next = new_node ;
	current->prev = new_node ;
	new_node->next = current ;
	last_allocation = new_node ;

	// Keeping track of the time once the process has been successfully allocated
	clock_gettime(CLOCK_MONOTONIC,&(info->finish_allocation_time));
	double time_taken;
    time_taken = (info->finish_allocation_time.tv_sec - info->start_time.tv_sec) * 1e9;
    time_taken = (time_taken + (info->finish_allocation_time.tv_nsec - info->start_time.tv_nsec)) * 1e-9;
	avg_turnaround_time = ((avg_turnaround_time*num_allocated_processes) + time_taken)/(num_allocated_processes + 1) ;
	num_allocated_processes++ ;

	info->allocated_block = new_node ;

	return front ;

}


// A function to simulate the next-fit dynamic memory allocation algorithm
struct memory_node* next_fit(struct thread_info *info, int *changed){

	// Initializing all the required variables and structures beforehand
	int thread_number = info->thread_number ;
	int size = info->size ;
	struct memory_node *current = last_allocation->next ;
	*changed = 1 ;

	// Looping till the block having the 'next fit' is found
	while(current != NULL){

		if(current->empty == 1 && current->size > size){
			break ;
		}

		current = current->next ;

	}

	// In case there is no block big enough to accomodate the process, it won't be allocated
	if(current == NULL){
		*changed = 0 ;
		return front ;
	}

	// Adjusting the physical memory to make space for the new block for the process
	struct memory_node *prev_node = current->prev ;
	struct memory_node *new_node = (struct memory_node *)malloc(sizeof(struct memory_node)) ;
	new_node->empty = 0 ;
	new_node->thread = thread_number ;
	new_node->size = size ;
	new_node->start = current->start ;
	new_node->end = current->start + size ;
	current->size -= size ;
	current->start = current->start + size ;
	new_node->prev = prev_node ;
	prev_node->next = new_node ;
	current->prev = new_node ;
	new_node->next = current ;
	last_allocation = new_node ;

	// Keeping track of the time once the process has been successfully allocated
	clock_gettime(CLOCK_MONOTONIC,&(info->finish_allocation_time));
	double time_taken;
    time_taken = (info->finish_allocation_time.tv_sec - info->start_time.tv_sec) * 1e9;
    time_taken = (time_taken + (info->finish_allocation_time.tv_nsec - info->start_time.tv_nsec)) * 1e-9;
	avg_turnaround_time = ((avg_turnaround_time*num_allocated_processes) + time_taken)/(num_allocated_processes + 1) ;
	num_allocated_processes++ ;

	info->allocated_block = new_node ;

	return front ;

}


// A function to handle deallocation of a process (memory block) from the physical memory
struct memory_node* deallocate_thread_memory(struct memory_node *node){

	printf("Process %d Removed from the Physical Memory after Running its Duration\n",node->thread) ;
	node->empty = 1 ;
	node->thread = -1 ;
	front = merge_empty_nodes() ;
	return front ;

}


// Deallocating a process after it's duration to run has lapsed
struct ll_node* deallocate_process(struct ll_node *node){

	front = deallocate_thread_memory((node->process)->allocated_block) ;
	if(node->prev == NULL){
		if(node->next == NULL){
			allocated_processes = NULL ;
			free(node) ;
			return allocated_processes ;
		}
		allocated_processes = node->next ;
		(node->next)->prev = NULL ;
		node->next = NULL ;
		free(node) ;
		return allocated_processes ;
	}
	if(node->next == NULL){
		(node->prev)->next = NULL ;
		node->prev = NULL ;
		free(node) ;
		return allocated_processes ;
	}
	(node->prev)->next = node->next ;
	(node->next)->prev = node->prev ;
	node->prev = NULL ;
	node->next = NULL ;
	free(node) ;
	return allocated_processes ;

}


// A function to remove a block from the 'pending' list in case it has been allocated a block in the physical memory
struct ll_node* remove_pending_block(struct ll_node *node){

	if(node->prev == NULL){
		if(node->next == NULL){
			pending_processes = NULL ;
			free(node) ;
			return pending_processes;
		}
		pending_processes = node->next ;
		(node->next)->prev = NULL ;
		node->next = NULL ;
		free(node) ;
		return pending_processes;
	}
	if(node->next == NULL){
		(node->prev)->next = NULL ;
		node->prev = NULL ;
		free(node) ;
		return pending_processes;
	}
	(node->prev)->next = node->next ;
	(node->next)->prev = node->prev ;
	node->prev = NULL ;
	node->next = NULL ;
	free(node) ;
	return pending_processes;	

}


// A function to add a newly spawned process to the 'pending' list
struct ll_node* add_to_pending(struct thread_info *process){

	struct ll_node *new_node = (struct ll_node *)malloc(sizeof(struct ll_node)) ;
	new_node->process = process ;
	new_node->prev = NULL ;
	new_node->next = NULL ;

	if(pending_processes == NULL){
		return new_node ;
	}
	else{
		struct ll_node *current = pending_processes ;
		while(current->next != NULL){
			current = current->next ;
		}
		current->next = new_node ;
		new_node->prev = current ;
		return pending_processes ;
	}

}


// A function to add a process to the 'allocated' list in case it has been allocated a block in the physical memory
struct ll_node* add_to_allocated(struct thread_info *process){

	struct ll_node *new_node = (struct ll_node *)malloc(sizeof(struct ll_node)) ;
	new_node->process = process ;
	new_node->prev = NULL ;
	new_node->next = NULL ;

	if(allocated_processes == NULL){
		return new_node ;
	}
	else{
		struct ll_node *current = allocated_processes ;
		while(current->next != NULL){
			current = current->next ;
		}
		current->next = new_node ;
		new_node->prev = current ;
		return allocated_processes ;
	}

}


// A scan over the 'pending' list to check if any processes can be allocated, and if so, removing them from 'pending' and adding them to 'allocated'
struct ll_node* free_from_pending(){

	struct ll_node *current = pending_processes ;
	int var = 0 ;
	int *changed = &var ;
	while(current != NULL){

		if(a == 0){
			front = best_fit(current->process,changed) ;
		}
		else if(a == 1){
			front = first_fit(current->process,changed) ;
		}
		else{
			front = next_fit(current->process,changed) ;
		}

		if((*changed) == 0){
			current = current->next ;
		}
		else{
			allocated_processes = add_to_allocated(current->process) ;
			printf("Process %d Successfully Allocated in the Physical Memory\n",(current->process)->thread_number) ;
 			struct ll_node *temp = current ;
			current = current->next ;
			pending_processes = remove_pending_block(temp) ;
		}

	}

	return pending_processes ;

}


// A scan over the 'allocated' list and checking for processes whose duration has lapsed and if there are any found, deallocating them
struct ll_node* free_from_allocated(){

	struct ll_node *current = allocated_processes ;
	while(current != NULL){
		struct ll_node *temp = current ;
		current = current->next ;
		clock_gettime(CLOCK_MONOTONIC,&now);
		double time_taken;
	    time_taken = (now.tv_sec - (temp->process)->finish_allocation_time.tv_sec) * 1e9;
	    time_taken = (time_taken + (now.tv_nsec - (temp->process)->finish_allocation_time.tv_nsec)) * 1e-9;
		if(time_taken > (temp->process)->duration){
			allocated_processes = deallocate_process(temp) ;
		}
	}

	return allocated_processes ;

}


int main(int argc, char *argv[]){

	// If the user enters the command line arguments in an incorrect format
    if (argc != 9){
    	printf("Incorrect passage of arguments. Terminating the Program. Try again with correct argument passing as specified below.\n") ;
        printf("Correct argument structure: [p] [q] [n] [m] [n] [t] [a] [N] [T]\n") ;
        printf("p: Capacity of Physical Memory (in MB)\n") ;
        printf("q: Space Occupied by the OS in the Physical Memory in MB (under the condition q < 0.2*p)\n") ;
        printf("n: Used to Characterize the Process Arrival Rate (0.1*n < PAR < 1.2*n)\n") ;
        printf("m: Used to Characterize the Size of the Processes (0.5*m < SoP < 3*m, in multiples of 10 MB)\n") ;
        printf("t: Used to Characterize the Duration of the Processes (0.5*t < DoP < 6*t, in multiples of 5 minutes)\n") ;
        printf("a: Used to Characterize the Algorithm to be Used (0 -> first-fit, 1 -> best-fit, 2 -> next-fit)\n") ;
        printf("N: Max Size of the Pending Queue\n") ;
        printf("T: Overall time the program will run (in seconds). Note: Should be long enough to achieve steady state\n") ;
        exit(-1) ;
    }

    // Initializing the random number seed generator
    random_init() ;

    // Initializing the program variables from the user specified command line arguments
    p = atoi(argv[1]), q = atoi(argv[2]), n = atoi(argv[3]), m = atoi(argv[4]), t = atoi(argv[5]), a = atoi(argv[6]), N = atoi(argv[7]), T = atoi(argv[8]) ;

    // Considering the case when the physical memory set aside for the OS is more than the specified limit
    if(q > 0.2*p){
    	printf("The value of OS Memory Entered (q) is greater than 0.2 times the Overall Physical Memory (p). Terminating the Program.") ;
    	exit(-1) ;
    }
   
   	// Initializing the process arrival rate and duration from the random generator functions
   	int process_arrival_rate = generate_PAR() ;
   	double process_arrival_interval = (1/((float)process_arrival_rate))*1000000 ;
    
    // Initializing the memory nodes for the OS memory, as well for whatever is left over
    struct memory_node *os_node = (struct memory_node *)malloc(sizeof(struct memory_node)) ;
    struct memory_node *initial_node = (struct memory_node *)malloc(sizeof(struct memory_node)) ;

    // Initializing the block for the OS in the physical memory
    os_node->empty = 0 ;
    os_node->thread = -2 ;
    os_node->size = q ;
    os_node->start = 0 ;
    os_node->end = q ;
    os_node->prev = NULL ;
    os_node->next = initial_node ;

    // Initializing the block for the rest of the empty memory space (initially) in the physical memory
    initial_node->empty = 1 ;
    initial_node->thread = -1 ;
    initial_node->size = p - q ;
    initial_node->start = q ;
    initial_node->end = p ;
    initial_node->prev = os_node ;
    initial_node->next = NULL ;


    // Initializing the 'front' or 'head' and the last allocated block of the physical memory from the previously declared global variables
    front = os_node ;
    last_allocation = os_node ;

    // Initializing the time to start running the program (once processes start spawning)
    time(&begin) ;

    // Indefinately spawning processes, allocating and deallocating memory for them (under randomly generated process sizes and durations)
    while(1){

    	// Exiting if the time to run the overall program has lapsed
    	time(&end) ;
    	double diff = difftime(end,begin) ;
		if(diff > T){
			break ;
		}

		// If too many processes are laready waiting to be allocated, not spawning any new process.
		if(length_of_ll(pending_processes) > N){
			printf("Pending queue is full at the moment. Not spawning any new processes till some space frees up\n") ;
		}
		// Otherwise, spawning a new process and adding it to the pending queue
		else{
			struct thread_info *process = create_process() ;
    		printf("Process %d Spawned with duration %d\n",process->thread_number,process->duration) ;
    		pending_processes = add_to_pending(process) ;
		}

    	// Deallocting completed processes from the 'allocated' list
    	allocated_processes = free_from_allocated() ;
    	// Allocating blocks in the physical memory to processes in the 'pending' list 
    	pending_processes = free_from_pending() ;

    	// Sleeping as per the process arrival rate generated earlier
    	usleep(process_arrival_interval) ;

    }

    // Showing statistics related to the average memory utilization and turnaround time during process allocation
    printf("Average Memory Untilization in Steady State: %lf%%\n",calculate_memory_utilization()) ;
    printf("Average Turnaround Time in Steady State: %lf seconds\n",avg_turnaround_time) ;

    return 0 ;

}