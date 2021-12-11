#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <time.h>


// Defining the Fixed Parameters of the Disk
int NUM_SURFACES = 4, NUM_TRACKS = 25, NUM_SECTORS = 20 ;

// User Input Parameters Related to Disk Rotation Speed (rev/min), Average Seek Time (ms), Number of Surfaces/Platters, Disk Scheduling Algorithm, and Number of Requests
int r, Ts, N, a, NoR ;
double rot_time ;

// Global Variables to keep track of the Number of Tracks Traversed between two Consecutive Jumps
int last_accessed_track, num_tracks_traversed ;

// Global Variables to keep track of Response Time Statistics
double min_request_time = (double)INT_MAX, max_request_time = -1.0, avg_request_time = 0.0, variance_in_request_times = 0.0, throughput = 0.0 ;


// A random number seed generator
void random_init() {

    srand(time(NULL)) ;

}


// A Function to Initially Generate all the Requests on Different Surfaces, Tracks, of Varying Starting Sector and Size
void generate_requests(int requests[NoR][4], int track_info[NUM_TRACKS][NoR+2]){

	for(int i=0; i<NUM_TRACKS; i++){
		track_info[i][0] = 0 ;	// Whether that track has been serviced
		track_info[i][1] = 0 ; 	// Number of requests on this track (including all surfaces)
	}

	for(int i=0; i<NoR; i++){
		requests[i][0] = rand()%NUM_SURFACES ;		// The Surface/Platter Number
		requests[i][1] = rand()%NUM_TRACKS ;		// The Track Number on the Surface
		requests[i][2] = rand()%NUM_SECTORS ;		// The Starting Sector Number on the Trac
		requests[i][3] = rand()%NUM_SECTORS + 1 ;	// The Number of Sectors Occupied
		track_info[requests[i][1]][2+track_info[requests[i][1]][1]] = i ;
		(track_info[requests[i][1]][1])++ ;
	}

	for(int i=0; i<NUM_TRACKS; i++){
		if(track_info[i][1] == 0){
			track_info[i][0] == 1 ;
		}
	}

}


// A Function to Calculate the Access time for a Single Request on a Track based off the Standard Formula
double calculate_access_time(int tracks_traversed, int num_sectors){

	double seek_time = ((3*Ts)/(NUM_TRACKS-1))*tracks_traversed ;
	double rotation_time = rot_time/2 ;
	double transfer_time = num_sectors*rot_time ;

	return seek_time + rotation_time + transfer_time ;

}

// A Function to Calculate all the Resonse Time Statistics
void calculate_statistics(double access_times[NoR]){

	for(int i=0; i<NoR; i++){
		avg_request_time += access_times[i] ;
		if(access_times[i] > max_request_time){
			max_request_time = access_times[i] ;
		}
		if(access_times[i] < min_request_time){
			min_request_time = access_times[i] ;
		}
	}

	avg_request_time = avg_request_time/NoR ;

	throughput = 1000/avg_request_time ;

	for(int i=0; i<NoR; i++){
		variance_in_request_times += pow(access_times[i]-avg_request_time,2) ;
	}

	variance_in_request_times = sqrt(variance_in_request_times/(NoR-1)) ;

}


// A Function to Check whether all the Requests on a Single Track have been Serviced or Not
int all_requests_serviced(int track_info[NUM_TRACKS][NoR+2]){

	for(int i=0; i<NUM_TRACKS; i++){
		if(track_info[i][0] == 0){
			return 0 ;
		}
	}
	return 1 ;

}


// A Function to Service/Handle all the Read/Write/Seek Requests on a Single Track, across all Surfaces/Platters, Updating Response Time Statistics along the way
void service_track(int requests[NoR][4], int track_info[NUM_TRACKS][NoR+2], int track_number, double access_times[NoR], int *access_time_index){

	if(a == 4 && track_number < last_accessed_track){
		num_tracks_traversed = NUM_TRACKS - (last_accessed_track - track_number) ;
	}
	else{
		num_tracks_traversed = abs(last_accessed_track - track_number) ;
	}
	for(int i=0; i<track_info[track_number][1]; i++){
		double access_time ;
		if(i==0){
			access_time = calculate_access_time(num_tracks_traversed,requests[track_info[track_number][2+i]][3]) ;
		}
		else{
			//printf("%d\n",requests[track_info[track_number][2+i]][3]) ;
			access_time = calculate_access_time(0,requests[track_info[track_number][2+i]][3]) ;
		}
		access_times[*access_time_index] = access_time ;
		(*access_time_index)++ ;
	}
	track_info[track_number][0] = 1 ;

}


// A Function to find the Closest Unserviced Track from the Previosuly Serviced Track (used in the SSTF Disk Scheduling Policy)
int closest_unserviced_track(int track_info[NUM_TRACKS][NoR+2], int track_number){

	int pos = NUM_TRACKS, neg = -1 ;
	for(int i=track_number+1; i<NUM_TRACKS; i++){
		if(track_info[i][0] == 0){
			pos = i ;
			break ;
		}
	}
	for(int i=track_number-1; i>=0; i--){
		if(track_info[i][0] == 0){
			neg = i ;
			break ;
		}
	}
	if(pos == NUM_TRACKS && neg == -1){
		return track_number ;
	}
	else if(pos == NUM_TRACKS){
		return neg ;
	}
	else if(neg == -1){
		return pos ;
	}
	else{
		if(track_number - neg > pos - track_number){
			return pos ;
		}
		else{
			return neg ;
		}
	}

}


// A Function to Find the Closest Unserviced Track (in the direction of increasing track number) from the Previously Serviced Track (used in the SCAN Disk Scheduling Policy)
int next_unserviced_track(int track_info[NUM_TRACKS][NoR+2], int track_number){

	int pos = NUM_TRACKS ;
	for(int i=track_number+1; i<NUM_TRACKS; i++){
		if(track_info[i][0] == 0){
			pos = i ;
			break ;
		}
	}
	return pos ;

}


// A Modification of the Previous Function used in the C-SCAN Disk Scheduling Policy
int next_circular_track(int track_info[NUM_TRACKS][NoR+2]){

	int new_track = -1 ;
	for(int i=0; i<NUM_TRACKS; i++){
		if(track_info[i][0] == 0){
			new_track = i ;
			break ;
		}
	}
	return new_track ;
}


// Implementing the Random Disk Scheduling Policy
void random_policy(int requests[NoR][4], int track_info[NUM_TRACKS][NoR+2], double access_times[NoR], int *access_time_index){

	int i = 0 ;
	while(all_requests_serviced(track_info) == 0){
		int track_number = rand()%NUM_TRACKS ;
		if(i == 0){
			last_accessed_track = track_number ;
			i++ ;
		}
		if(track_info[track_number][0] == 0){
			service_track(requests,track_info,track_number,access_times,access_time_index) ;
			last_accessed_track = track_number ;
		}
	}

}


// Implementing the FIFO Disk Scheduling Policy
void fifo_policy(int requests[NoR][4], int track_info[NUM_TRACKS][NoR+2], double access_times[NoR], int *access_time_index){

	for(int i=0; i<NoR; i++){
		int track_number = requests[i][1] ;
		if(track_info[track_number][0] == 1){
			continue ;
		}
		if(i == 0){
			last_accessed_track = track_number ;
			i++ ;
		}
		service_track(requests,track_info,track_number,access_times,access_time_index) ;
		last_accessed_track = track_number ;
	}

}


// Implementing the SSTF Disk Scheduling Policy
void sstf_policy(int requests[NoR][4], int track_info[NUM_TRACKS][NoR+2], double access_times[NoR], int *access_time_index){

	int track_number = rand()%NUM_TRACKS ;
	while(track_info[track_number][0] == 1){
		track_number = rand()%NUM_TRACKS ;
	}
	last_accessed_track = track_number ;
	while(all_requests_serviced(track_info) == 0){
		//printf("Hi\n") ;
		//fflush(stdout) ;
		service_track(requests,track_info,track_number,access_times,access_time_index) ;
		last_accessed_track = track_number ;
		track_number = closest_unserviced_track(track_info,track_number) ;
	}

}


// Implementing the SCAN Disk Scheduling Policy
void scan_policy(int requests[NoR][4], int track_info[NUM_TRACKS][NoR+2], double access_times[NoR], int *access_time_index){

	int track_number = rand()%NUM_TRACKS ;
	while(track_info[track_number][0] == 1){
		track_number = rand()%NUM_TRACKS ;
	}
	last_accessed_track = track_number ;
	while(all_requests_serviced(track_info) == 0){
		service_track(requests,track_info,track_number,access_times,access_time_index) ;

		last_accessed_track = track_number ;
		track_number = next_unserviced_track(track_info,track_number) ;
		if(track_number == NUM_TRACKS){
			track_number = closest_unserviced_track(track_info,track_number) ;
		}
	}
	
}


// Implementing the C-SCANDisk Scheduling Policy
void cscan_policy(int requests[NoR][4], int track_info[NUM_TRACKS][NoR+2], double access_times[NoR], int *access_time_index){

	int track_number = rand()%NUM_TRACKS ;
	while(track_info[track_number][0] == 1){
		track_number = rand()%NUM_TRACKS ;
	}
	last_accessed_track = track_number ;
	while(all_requests_serviced(track_info) == 0){
		last_accessed_track = track_number ;
		service_track(requests,track_info,track_number,access_times,access_time_index) ;
		track_number = next_unserviced_track(track_info,track_number) ;
		if(track_number == NUM_TRACKS){
			track_number = next_circular_track(track_info) ;
		}
	}
	
}


int main(int argc, char *argv[]){

	// If the user enters the command line arguments in an incorrect format
    if (argc != 6){
    	printf("Incorrect passage of arguments. Terminating the Program. Try again with correct argument passing as specified below.\n") ;
        printf("Correct argument structure: [r] [Ts] [N] [NoR]\n") ;
        printf("r: Rotational speed of disk, r revolutions per minute\n") ;
        printf("Ts: Average seek time between consecutive tracks in ms\n") ;
        printf("N: Sector size, N in bytes\n") ;
        printf("a: Algorithm to be used (0: Random ; 1: FIFO ; 2: SSTF ; 3: SCAN ; 4: C-SCAN)\n") ;
        printf("NoR: Number of Requests to be run\n") ;
        exit(-1) ;
    }

    // Declaring the Relevant Variables based off the User Specified Command Line Arguments
    r = atoi(argv[1]), Ts = atoi(argv[2]), N = atoi(argv[3]), a = atoi(argv[4]), NoR = atoi(argv[5]) ;
    rot_time = 60000/(float)r ;

    // Staring the Random Seed Generator
    random_init() ;

    // Initializing the Data Structures to Store all the Relevant Information about the Generated Requests
    int requests[NoR][4] ;					// Data Strcuture to Store Information Specific to each Request
    int track_info[NUM_TRACKS][NoR+2] ;		// Data Strcuture to Store Request Information Specific to each Track

    // Generating Raandom Requests to be Serviced/Handled
    generate_requests(requests,track_info) ;

    // Initializing arrays for statistic generation
    double access_times[NoR] ;
    int index_statistics = 0 ;
    int *access_time_index = &index_statistics ;

    // Servicing/Handling each Request using the Appropriate (user specified) Disk Scheduling Policy
    if(a == 0){
    	random_policy(requests,track_info,access_times,access_time_index) ;
    }
    else if(a == 1){
    	fifo_policy(requests,track_info,access_times,access_time_index) ;
    }
    else if(a == 2){
    	sstf_policy(requests,track_info,access_times,access_time_index) ;
    }
    else if(a == 3){
    	scan_policy(requests,track_info,access_times,access_time_index) ;
    }
    else{
    	cscan_policy(requests,track_info,access_times,access_time_index) ;
    }

    // Updating Response Time Statistics
    calculate_statistics(access_times) ;

    // Printing the Relevant Response Time Statistics
    printf("Statistics on Running the Simulation:\n") ;
    printf("Throughput: %lf respones/second\n",throughput) ;
    printf("Average Response Time: %lf milli seconds\n",avg_request_time) ;
    printf("Variance of the Different Response Times: %lf milli seconds\n",variance_in_request_times) ;
    printf("Minimum Response Time Encountered: %lf milli seconds\n",min_request_time) ;
    printf("Maximum Response Time Encountered: %lf milli seconds\n",max_request_time) ;

	return 0 ;

}


