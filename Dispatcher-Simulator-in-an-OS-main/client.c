#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <sys/un.h>
#include <stddef.h>
#include <string.h>
#define PortNumber 8000
#define MessageBufferSize 2048


// Printing the log message and terminating the program if required
void log_msg(const char *msg, bool should_terminate){

    printf("%s\n", msg) ;
    if (should_terminate){
    	exit(-1) ;
    }

}


// Function to create a socket from the client's side to enable communication
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
		log_msg("Failed to create the client socket. Terminating the program.", true) ;
	}

	// To bind the client's local address in the memory
	struct sockaddr_in client_addr ;
	memset(&client_addr, 0, sizeof(client_addr)) ;
	client_addr.sin_family = AF_INET ;
	client_addr.sin_addr.s_addr = inet_addr("127.0.0.1") ;
	client_addr.sin_port = htons(PortNumber) ;

	// To bind a name to the socket
	if (connect(file_descriptor, (struct sockaddr *) &client_addr, sizeof(client_addr)) < 0) 
        log_msg("Bind for the socket failed. Terminating the program.", 1) ;
    
    return file_descriptor ;

}


// Function to send data to the server via the created socket
void send_msg_to_server_via_socket(int argc, char *argv[], char *socket_name) {
    
	// Creating a socket to communicate (both send and receive data) with the server
	int socket_fd = create_socket(socket_name) ;

    log_msg("Client attempting to connect and communicate with the server", false) ;
     
    // Just a message from the server to confirm that the connection has been established correctly
    char message_buffer[MessageBufferSize] ;
    memset(message_buffer, '\0', sizeof(message_buffer)) ;
    if (read(socket_fd, message_buffer, sizeof(message_buffer)) > 0) {
            printf("The message received from the server: %s\n", message_buffer) ;
    }

    // Passing the number of arguments (discrete types of data) being sent sequentially to the server
    uint32_t num_args = htonl(argc-2) ;
    write(socket_fd, &num_args, sizeof(num_args)) ;

 	// Passing all the individual data (number of such data specified and sent earlier) one by one to the server
    for(int i=2; i<argc; i++){
        memset(message_buffer, '\0', sizeof(message_buffer)) ;
        strcpy(message_buffer, argv[i]) ;
        int sending = send(socket_fd, message_buffer, MessageBufferSize, 0) ;  //
        printf("Sending the data [%s] to the server\n", message_buffer) ;
    }

    // Receiving a messge from the server and printing it (usually for displaying the notification of a successful enqueue of the client request)
    memset(message_buffer, '\0', sizeof(message_buffer)) ;
    if (read(socket_fd, message_buffer, sizeof(message_buffer)) > 0){
            printf("Message received from the server: %s\n", message_buffer) ;
    }
    
    // Receiving a messge from the server and printing it (usually for displaying the final message from the server, ie it's response to the client request)
    memset(message_buffer, '\0', sizeof(message_buffer)) ;
    if (read(socket_fd, message_buffer, sizeof(message_buffer)) > 0){
            printf("Message received from the server: %s\n", message_buffer) ;
    }

    log_msg("Client request successfully handled. Closing the connection", false) ;

    // Closing the socket connection after successful operation and handling of the client request
    close(socket_fd) ;

}


int main(int argc, char *argv[]) {

    if (argc < 4) {
        printf("Incorrect passage of arguments. Terminating the Program. Try again with correct argument passing as specified below.\n");
        printf("Correct argument structure: %s  [LSFP] [DLL_NAME] [FUNC_NAME] [ARG_NAMES]\n", argv[0]) ;
        printf("LSFP: Local Socket File Path - for executing the file descriptor\n") ;
        printf("DLL_NAME: Link to the library to use\n") ;
        printf("FUNC_NAME: Name of the function to execute (should be a part of the DLL called)\n") ;
        printf("ARG_NAMES: An array of the arguments to pass the requested function, specified in a string format\n") ;
        exit(-1);
    }

    // Send the message (as specified by the user in the command line) via a socket to the server
    send_msg_to_server_via_socket(argc, argv, argv[1]) ;

    return 0;

}