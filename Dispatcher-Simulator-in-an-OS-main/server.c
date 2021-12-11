#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/un.h>
#include <stddef.h>
#include <string.h>
#include <sys/resource.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "dll.h"
#define PortNumber 8000
#define MessageBufferSize 2048


// To enable multi-threading capabilities using a thread pool of a fixed size
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER ;


// The data that the client sends to the server via the socket would be stored in the form of a structure like as follows
struct request{

	char *dll_name ;
	char *func_name ;
	char *func_args ;

} ;


// The client's request and the socket through which communication is happening is maintained in a queue using this structure below
struct Node{

	int *client_soc ;
	struct request *client_req ;
	struct Node *next ;

} ;


// When a client's request is dequeued from the request queue, the following information of the request and socket is handed over for computation
struct client_request_info{

	int *client_soc ;
	struct request *client_req ;

} ;


// Maintaining a simple queue structure to pass as a single argument to the 'thread_function' function since it accepts only a single 'void *' argument
struct queue_info{

	struct Node *front ;
	struct Node *back ;
	int max_size ;
	int current_size ;

} ;


// A structure representing threads which is useful to see how info reagrding different threads can work together to add multiple requests to the queue simultaneously
struct queue_thread{

	struct queue_info *request_queue ;
	int *client_fd ;

} ;


// A function to enable an enqueue to the request queue
struct Node* enqueue(struct Node **front, struct Node **back, int *client_soc, struct request *client_req){

	struct Node *temp = (struct Node *)malloc(sizeof(struct Node)) ;
	temp->client_soc = client_soc ;
	temp->client_req = client_req ;
	temp->next = NULL ;
	if(*front==NULL && *back==NULL){
		*front = temp ;
		*back = temp ;
	}
	else{
		(*back)->next = temp ;
		*back = temp ;
	}
	return *back ;

}


// A function to enable a dequeue from the request queue
struct client_request_info* dequeue(struct Node **front, struct Node **back){

	if(*front == NULL){
		return NULL ;
	}
	
	struct Node *temp = *front ;
	struct client_request_info *client_info = (struct client_request_info *)malloc(sizeof(struct client_request_info)) ;
	client_info->client_req = temp->client_req ;
	client_info->client_soc = temp->client_soc ;
    
    	if(*front == *back){
		*front = NULL ;
		*back = NULL ;
	}
	else{
		*front = (*front)->next ;
		temp->next = NULL ;
	}
	free(temp) ;

	return client_info ;

}


// Printing the log message and terminating the program if required
void log_msg(const char *msg, bool should_terminate){

    printf("%s\n", msg) ;
    if (should_terminate){
    	exit(-1) ;
    }

}


// Function to create a socket from the server's side to enable communication
int create_socket(const char *socket_name){

	// Notifying the creation of a new network socket whenever this function is called
	printf("Creating an AF_IINET (network) socket at the path: %s\n", socket_name) ;

    // In case an old socket with the same name exists, it is replaced with a new one (with the same name)
    if (access(socket_name, F_OK) != -1) {
        log_msg("There is already a socket having the same name. Replacing it with a new one.", false) ;
        if (unlink(socket_name) != 0) {
            log_msg("Something went wrong. Couldn't unlink the old socket connection with the same name. Terminating the program.", true) ;
        }
    }

    // Creating a network (not local), stream (not datagram) socket following TCP protocol
	int file_descriptor = socket(AF_INET, SOCK_STREAM, 0) ;
	if (file_descriptor < 0){
		log_msg("Failed to create the server socket. Terminating the program.", true) ;
	}

	// To bind the server's local address in the memory
	struct sockaddr_in server_addr ;
	memset(&server_addr, 0, sizeof(server_addr)) ;
	server_addr.sin_family = AF_INET ;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY) ;
	server_addr.sin_port = htons(PortNumber) ;

	// To bind a name to the socket
	if (bind(file_descriptor, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) 
        log_msg("Bind for the socket failed. Terminating the program.", 1) ;
    
    return file_descriptor ;

}


// Handling the request of the client by invoking the dll with specified library to be imported, function to be called, and arguments to be passed
void* handle_requests(struct client_request_info *client_info){

    log_msg("Starting a server thread to handle client request", false) ;
    
    char *answer = get_dll_answer((client_info->client_req)->dll_name, (client_info->client_req)->func_name, (client_info->client_req)->func_args) ;
    char result[500] = "Answer to the client request: " ;
    strcat(result,answer) ;
    write(*(client_info->client_soc),result,sizeof(result)) ;
    close(*(client_info->client_soc)) ;
    free(client_info) ;

    log_msg("Client request handled successfully", false) ;

    return NULL ;

}


// A function to receive and store the information sent by a specific client (identified through it's socket file descriptor)
struct request* get_client_request(int socket_fd){

    // Just a message to notify that the server is listening and attemoting to receive the client data
    char receive_message[50] = "Attempting to receive your (client) request" ;
    write(socket_fd, receive_message, sizeof(receive_message)) ;

    // Initially, the client sends the number of discrete data elements it is going to send. This is notedby the sever
    int32_t temp, num_args ;
    read(socket_fd, &temp, sizeof(temp)) ;
    num_args = ntohl(temp) ;

    // Initialising a message buffer to store each piece of information sent by the client
    char message_buffer[MessageBufferSize] ;
    struct request *client_req = (struct request *)malloc(sizeof(struct request)) ;

    // A loop to get exactly the amount of data elements as specified by the client interatively
    for(int32_t i=1; i<=num_args; i++){

        int receive = recv(socket_fd, message_buffer, MessageBufferSize, 0) ;
        if(i==1){
            client_req->dll_name = malloc(strlen(message_buffer)) ;
            strcpy(client_req->dll_name, message_buffer) ;
        }
        else if(i==2){
            client_req->func_name = malloc(strlen(message_buffer)) ;
            strcpy(client_req->func_name, message_buffer) ;
        }
        else{
            client_req->func_args = malloc(strlen(message_buffer)) ;
            strcpy(client_req->func_args, message_buffer) ;
        }
        memset(message_buffer, '\0', sizeof(message_buffer)) ;

    }

    return client_req ;

}



// A function to enqueue new client requests to the request queue (if in fact, the queue is not full yet)
void* process_client_request(void *arg){

	// getting information about the thread in use, as well as the current status of the request queue
    struct queue_thread *thread = (struct queue_thread *)arg ;
    struct queue_info *queue = thread->request_queue ;
    int *client_fd = thread->client_fd ;
    free(arg) ;

    pthread_mutex_lock(&mutex);

    // If the request queue is already full, any further client requests cannot be processed until another dequeue occurs
    if(queue->current_size == queue->max_size){
    	log_msg("Couldn't process the latest request as the request queue was full", false) ;
        char *client_msg = "Couldn't process the latest request as the request queue was full. Please try again later." ;
        write(*client_fd, client_msg, sizeof(client_msg)) ;
        close(*client_fd) ;
    }

    // If the thread isn't full however, just enqueue the client request onto the request queue
    else{
        struct request *client_request = get_client_request(*client_fd) ;
        struct Node **queue_front = &(queue->front) ;
        struct Node **queue_back = &(queue->back) ;
        *queue_back = enqueue(queue_front,queue_back,client_fd,client_request) ;
        (queue->current_size)++ ;
        char success_msg[100] = "Client request added to the queue successfully. Awaiting processing" ;
        write(*client_fd, success_msg, sizeof(success_msg)) ;
    }

    pthread_mutex_unlock(&mutex) ;
    pthread_exit(NULL) ;
    return NULL ;

}


// A thread function to process the dequeue capabilities in order to execute client requests
void* thread_function(void *queue){

    while(1){
        pthread_mutex_lock(&mutex) ;
        struct client_request_info *client_info = dequeue(&(((struct queue_info *)queue)->front),&(((struct queue_info *)queue)->back)) ;
        pthread_mutex_unlock(&mutex) ;
        if(client_info != NULL){
        	(((struct queue_info *)queue)->current_size)-- ;
            handle_requests(client_info) ;
        }
    }

}


// A function to start the server and start listening for, and further processing requests from the client
void start_server_socket(struct queue_info *queue, pthread_t threads[], int num_threads_enqueue, char *socket_name, int server_backlog){

    // Creating a socket to enable communication
    int socket_fd = create_socket(socket_name) ;

    // Start listening for client connections, limited by the server backlog capacity however to prevent extra traffic
    if (listen(socket_fd, server_backlog) < 0) {
        log_msg("Some error occurred. Cannot connect. Terminating the program.", true) ;
    }

    // Listening for connections from potential clients for an unlimited period, until specified to stop
    log_msg("Listening for on the network, awaiting connections with potential clients\n", false) ;
    pthread_t server_threads[8] ;
    int i=0 ;

    // Listen indefinately for connections until the user instructs an interrupt
    while (1) {

    	// Getting a client socket address and attempting to connect
        struct sockaddr_in client_addr ;
        int len = sizeof(client_addr) ;
        int client_fd = accept(socket_fd, (struct sockaddr *) &client_addr, &len) ;

        // If the connection fails, attempts to connect again
        if (client_fd < 0) {
            log_msg("Could not accept the request from client. Please try again.", 0); /* don't terminate, though there's a problem */
            continue;
        }
        else{
        	printf("Client connected successfully!") ;
        }

        // Creating a thread for operation of handling this new connection
        struct queue_thread *thread = (struct queue_thread *)malloc(sizeof(struct queue_thread));
        thread->request_queue = queue ;
        int *client_fd_pointer = malloc(sizeof(int)) ;
        *client_fd_pointer = client_fd ;
        thread->client_fd = client_fd_pointer ;


        // Processing the new connection, and any possible requests that teh client might send over
        pthread_create(&server_threads[i++], NULL, process_client_request, thread);

        // Working the enqueue thread pool
        if(i==num_threads_enqueue){
            i = 0;
            while(i<num_threads_enqueue)
                pthread_join(server_threads[i++],NULL);
            i = 0;
        }
        
    }

}


int main(int argc, char *argv[]) {

    if (argc < 5){
    	printf("Incorrect passage of arguments. Terminating the Program. Try again with correct argument passing as specified below.\n") ;
        printf("Correct argument structure: %s  [LSFP] [SB] [NoTE] [NOTD] [SotQ] [PML] [PFL]\n", argv[0]) ;
        printf("LSFP: Local Socket File Path - for executing the file descriptor\n") ;
        printf("SB: Server Backlog - ie the maximum requests the server can handle at once\n") ;
        printf("NoTE: Number of Threads in Enqueue - User dependent input, can vary the number of threads to effect concurrency of queue functioning of the enqueue as well as each client's request\n") ;
        printf("NoTD: Number of Threads in Dispatcher - User dependent input, can vary the number of threads to effect concurrency of queue functioning of the dispatcher as well as each client's request\n") ;
        printf("SotQ: Size of the Queue - ie the size of the queue used by the dispatcher to run each of the clients' requests\n") ;
        printf("PML: Program Memory Limit (in MB) - Maximum amount of memory the entire program can occupy in the RAM\n") ;
        printf("PFL: Program File Limit - Maximum number of files the program can open simultaneously at once\n") ;
        exit(-1) ;
    }

    // Initialising some user specified arguments
    int NUM_THREADS_ENQUEUE = atoi(argv[3]) ;
    int NUM_THREADS_DISPATCHER = atoi(argv[4]) ;
    int MAX_QUEUE_SIZE = atoi(argv[5]) ;
    int PML = atoi(argv[6]) ;
    int PFL = atoi(argv[7]) ;

    // Setting the memory limit for the entire program
    struct rlimit old_lim, lim, new_lim ;
    lim.rlim_cur = 1024 ;
    lim.rlim_max = 1024*1024*PML ;

    if(setrlimit(RLIMIT_STACK, &lim) == -1){
        fprintf(stderr,"%s\n",strerror(errno));
    }

    // Setting the file limit for the entire program
    lim.rlim_cur = 2048 ;
    lim.rlim_max = 2048*PFL ;

    if(setrlimit(RLIMIT_NOFILE, &lim) == -1){
        fprintf(stderr,"%s\n",strerror(errno));
    }

    // Initialising the pointers required to operate the request queue
    struct Node *front = NULL ;
	struct Node *back = NULL ;

	// Initialising the request queue
	struct queue_info *queue = (struct queue_info *)malloc(sizeof(struct queue_info)) ;
	queue->front = front ;
	queue->back = back ;
	queue->max_size = MAX_QUEUE_SIZE ;
	queue->current_size = 0 ;

    pthread_t working_threads[NUM_THREADS_DISPATCHER] ;

    // Initialisng the working threads
    for(int i=0; i<NUM_THREADS_DISPATCHER; i++){
        pthread_create(&working_threads[i], NULL, thread_function, queue);
    }


    // Staring teh server and listening for, and further processing connections
    start_server_socket(queue, working_threads, NUM_THREADS_ENQUEUE, argv[1], atoi(argv[2]));

}
