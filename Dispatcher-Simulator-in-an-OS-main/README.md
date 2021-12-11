# Dispatcher-Simulator-in-an-OS
A program to mimic how processes are managed by the dispatcher in an OS, developed in C on a Linux OS.<br>
<h4>A Brief Description of the Program</h4>
The program mimics the functioning of a dispatcher in an OS. After receiving requests from the client (in this specific program, to invoke a function call with client specified arguments from the DLL), the request listener adds them to a request queue, managed by one thread pool (with the number of threads specified by the user). Further, each request is dequeued from the request queue one after another by the dispatcher to execute the client requests, and is handled by another thread pool (with the number of threads executing once again being user specified). A limit on the amount of shared memory and files would also be taken as a user specified quantity and would be enforced throughout execution.
<img src="/img/program_working.png">
<br>
<h4>Working of the Program</h4>
Initially, on executing either the server or client programs, a similar function by the name ‘create_socket’ is run, to create a network socket and enable bidirectional communication between the server and client.<br><br>
On executing the server initially, after creating the socket, the request queue and two thread pools are initialised (one thread pool to enable multiple client request enqueue into the request queue, and the other to execute the dequeued requests simultaneously).<br><br>
On executing the client, the function ‘send_msg_to_server_via_socket’ is called, which sends the total number of discrete data elements being sent, along with the data (command line arguments specified by the user) to the server with the server, one after another run in a loop.<br><br>
Once a client sends a request, the ‘get_client_request’ function in the server program receives and processes the data sent as a part of the request, and then the ‘process_client_request’ function further enables the enqueue of that request into the queue. Simultaneously, the ‘thread_function’ takes care of the logic to dequeue requests from the queue into different threads from the thread pool and execute them via the ‘handle_requests’ function.<br><br>
Further, a limit (both hard and soft, definitions specified via API’s)is set on the total memory that the program is using, as well as the number of files it is accessing.<br><br>
<b>A more detailed description of each of the major functions looks like:</b><br><br>
<i>create_socket</i> (from <i>server.c</i> & <i>client.c</i>): This function creates the socket for bidirectional communication between the server and client, and can also log messages and terminate the program in case of bind errors.<br><br>
<i>get_client_request</i> (from <i>server.c</i>): This function receives the number of discrete data elements a particular client is sending (in case of multiple arguments) followed by the data itself, after which it is stored in the <i>request</i> structure.<br><br>
<i>process_client_request</i> (from <i>server.c</i>): This function takes each request returned by the previous function and enqueue’s them into the request queue (unless it is full, in which case the exception is called).<br><br>
<i>thread_function</i> (from <i>server.c</i>): This function is responsible for the dequeue process after which the <i>handle_requests</i> function is called to process the client request and then write back to the client.<br><br>
<i>handle_requests</i> (from <i>server.c</i>): This function invokes the function <i>get_dll_answer</i> from <i>dll.h</i> which returns the answer to the client request, and then subsequently writes this answer to the client.<br><br>
<i>send_msg_to_server_via_socket</i> (from <i>client.c</i>): This function takes the client request parameters (given via command line) and sends them sequentially to the server for processing. (the number of parameters being sent are sent before the actual parameters themselves though).<br><br>
<i>get_dll_answer</i> (from <i>dll.h</i>): This function returns the string of the answer for the client request, after invoking the DLL, and constructing the handle.<br><br>
The DLL <i>/lib/x86_64-linux-gnu/libm.so.6</i> is compatible to run in this program with the following functions:<br>
<img src="/img/functions_supported.png">
To optimise the code since the number of arguments across all functions chosen is uniform and equal to 1, the structure for the request has slightly been modified from the assignment sheet (since creative liberty has been mentioned to be taken whenever appropriate), visible in the <i>server.c</i> file.<br><br>
The <i>unit_testing.c</i> file contains a large number of test cases to confirm the working of many functions but is not exhaustive of all the different functions and capabilities of the code.<br><br>
Mutex has been added to the code as a standard practice to prevent interference.<br><br>
<h4>Compiling and Executing the Program</h4>
This code can be complied and run only on linux based systems (in my case, an Ubuntu 20.04 LTS version).<br>
To compile the server, a command of the following structure can be run:

    gcc server.c -o server.out -pthread -ldl
And then to run the server, run the command:

    ./server.out [LSFP] [SB] [NoTE] [NOTD] [SotQ] [PML] [PFL]
<i>LSFP</i>: Local Socket File Path - for executing the file descriptor (can be anything arbitrary like <i>./server_loc</i> for example).<br><br>
<i>SB</i>: Server Backlog - ie the maximum requests the server can handle at once.<br><br>
<i>NoTE</i>: Number of Threads in Enqueue - User dependent input, can vary the number of threads to affect concurrency of queue functioning of the enqueue as well as each client's request.<br><br>
<i>NoTD</i>: Number of Threads in Dispatcher - User dependent input, can vary the number of threads to affect concurrency of queue functioning of the dispatcher as well as each client's request.<br><br>
<i>SotQ</i>: Size of the Queue - ie the size of the queue used by the dispatcher to run each of the clients' requests.<br><br>
<i>PML</i>: Program Memory Limit (in MB) - Maximum amount of memory the entire program can occupy in the RAM.<br><br>
<i>PFL</i>: Program File Limit - Maximum number of files the program can open simultaneously at once.<br><br>
An example of a command to run the server with arbitrary arguments would look like:

    ./server.out ./server_loc 30 8 8 10 10 10
To compile the client, a command of the following structure can be run:

    gcc client.c -o client.out
And then to run the client, run the command:

    ./client.out [LSFP] [DLL_NAME] [FUNC_NAME] [ARG_NAMES]
<i>LSFP</i>: Local Socket File Path - for executing the file descriptor<br><br>
<i>DLL_NAME</i>: Link to the library to use<br><br>
<i>FUNC_NAME</i>: Name of the function to execute (should be a part of the DLL called)<br><br>
<i>ARG_NAMES</i>: An array of the arguments to pass the requested function, specified in a string format. However, since this program supports only functions from the <i>math.h</i> API’s DLL which take only single arguments, and hence only one should be specified.<br><br>
An example of a command to run the client with arbitrary arguments would look like:

    ./client.out ./client_loc /lib/x86_64-linux-gnu/libm.so.6 cos 3.14
To compile and run the unit test file, run the following commands sequentially:

    gcc unit_testing.c -o unit_testing.out -ldl
    ./unit_testing.out
To run the bash file to enable testing of multiple clients simultaeously (to be run after startiing the server), sequentially run:

    chmod u+x ./multi_client.bash
    ./multi_client.bash
The first <i>chmod</i> command is necessary to give permission to execute the <i>bash</i> file.

