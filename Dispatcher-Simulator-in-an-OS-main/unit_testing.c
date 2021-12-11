/*

This file has the same functions and structures used in other files repeated.
Though unnecessary, it is just for the case of testing the validity of these functions.

*/


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <dlfcn.h>


struct request{

	char *dll_name ;
	char *func_name ;
	char *func_args ;

} ;


struct Node{

	int *client_soc ;
	struct request *client_req ;
	struct Node *next ;

} ;


struct client_request_info{

	int *client_soc ;
	struct request *client_req ;

} ;


struct queue_info{

	struct Node *front ;
	struct Node *back ;
	int max_size ;
	int current_size ;

} ;


struct queue_thread{

	struct queue_info *request_queue ;
	int *client_fd ;

} ;


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

	return client_info ;

}


// A function to check whether a string (an argument in the list of arguments passed by the client) is a numeric value or not, to get clarity on it's data type
int check_if_number(char *msg){

	int i=0; 
	while(msg[i]){
		if (((int)msg[i] >= 58 || (int)msg[i] <= 47) && (int)msg[i] != 46){
			return 0 ;
		}
		i++ ;
	}
	return 1 ;

}


// A function to get the answer from the DLL, using the client provided link, function name, and arguments
char* get_dll_answer(char* dll_name, char* function_name, char* args){

    // Since only one DLL is supported in this code
    char supported_dll[100] = "/lib/x86_64-linux-gnu/libm.so.6" ;
    if(strcmp(supported_dll,dll_name)!=0){
        return "The DLL requested by you to process is currently not available for use\0" ;
    }

    // Since we use the math.h DLL, all arguments must be numerical values
    if(check_if_number(args) == 0){
    	return "The argument provided by you is not a number and thus cannot be used in this DLL\0" ;
    }

    // Creating the handle
    void *handle = dlopen(dll_name,RTLD_LAZY);
    if (!handle) {
        return "Some error occurred while tring to load the required DLL. Please consider trying again\0";
    }

    char *answer = malloc(64) ;
    char *error_handling ;

    // One by one checking all the different function names in the DLL and handling them separately
   
    if(strcmp(function_name,"sin")==0){
        double (*sin_val)(double) ;
        sin_val = dlsym(handle,"sin") ;
        if ((error_handling = dlerror()) != NULL)  {
            strcat(answer, "Some error occurred while tring to load the required function from the DLL. Please consider trying again\0") ;
            dlclose(handle) ;
		    free(dll_name) ;
		    free(function_name) ;
		    free(args) ;
            return answer ;
        }
        else{
        	gcvt((*sin_val)(atof(args)),3,answer) ;
        	dlclose(handle) ;
		    free(dll_name) ;
		    free(function_name) ;
		    free(args) ;
        	return answer ;
        }
    }

    else if(strcmp(function_name,"sinh")==0){
        double (*sinh_val)(double) ;
        sinh_val = dlsym(handle,"sinh") ;
        if ((error_handling = dlerror()) != NULL)  {
            strcat(answer, "Some error occurred while tring to load the required function from the DLL. Please consider trying again\0") ;
            dlclose(handle) ;
		    free(dll_name) ;
		    free(function_name) ;
		    free(args) ;
            return answer ;
        }
        else{
        	gcvt((*sinh_val)(atof(args)),3,answer) ;
        	dlclose(handle) ;
		    free(dll_name) ;
		    free(function_name) ;
		    free(args) ;
        	return answer ;
        }
    }

    else if(strcmp(function_name,"asin")==0){
        double (*asin_val)(double) ;
        asin_val = dlsym(handle,"asin") ;
        if ((error_handling = dlerror()) != NULL)  {
            strcat(answer, "Some error occurred while tring to load the required function from the DLL. Please consider trying again\0") ;
            dlclose(handle) ;
		    free(dll_name) ;
		    free(function_name) ;
		    free(args) ;
            return answer ;
        }
        else{
        	gcvt((*asin_val)(atof(args)),3,answer) ;
        	dlclose(handle) ;
		    free(dll_name) ;
		    free(function_name) ;
		    free(args) ;
        	return answer ;
        }
    }

    else if(strcmp(function_name,"cos")==0){
        double (*cos_val)(double) ;
        cos_val = dlsym(handle,"cos") ;
        if ((error_handling = dlerror()) != NULL)  {
            strcat(answer, "Some error occurred while tring to load the required function from the DLL. Please consider trying again\0") ;
            dlclose(handle) ;
		    free(dll_name) ;
		    free(function_name) ;
		    free(args) ;
            return answer ;
        }
        else{
        	gcvt((*cos_val)(atof(args)),3,answer) ;
        	dlclose(handle) ;
		    free(dll_name) ;
		    free(function_name) ;
		    free(args) ;
        	return answer ;
        }
    }

    else if(strcmp(function_name,"cosh")==0){
        double (*cosh_val)(double) ;
        cosh_val = dlsym(handle,"cosh") ;
        if ((error_handling = dlerror()) != NULL)  {
            strcat(answer, "Some error occurred while tring to load the required function from the DLL. Please consider trying again\0") ;
            dlclose(handle) ;
		    free(dll_name) ;
		    free(function_name) ;
		    free(args) ;
            return answer ;
        }
        else{
        	gcvt((*cosh_val)(atof(args)),3,answer) ;
        	dlclose(handle) ;
		    free(dll_name) ;
		    free(function_name) ;
		    free(args) ;
        	return answer ;
        }
    }

    else if(strcmp(function_name,"acos")==0){
        double (*acos_val)(double) ;
        acos_val = dlsym(handle,"acos") ;
        if ((error_handling = dlerror()) != NULL)  {
            strcat(answer, "Some error occurred while tring to load the required function from the DLL. Please consider trying again\0") ;
            dlclose(handle) ;
		    free(dll_name) ;
		    free(function_name) ;
		    free(args) ;
            return answer ;
        }
        else{
        	gcvt((*acos_val)(atof(args)),3,answer) ;
        	dlclose(handle) ;
		    free(dll_name) ;
		    free(function_name) ;
		    free(args) ;
        	return answer ;
        }
    }

    else if(strcmp(function_name,"tan")==0){
        double (*tan_val)(double) ;
        tan_val = dlsym(handle,"tan") ;
        if ((error_handling = dlerror()) != NULL)  {
            strcat(answer, "Some error occurred while tring to load the required function from the DLL. Please consider trying again\0") ;
            dlclose(handle) ;
		    free(dll_name) ;
		    free(function_name) ;
		    free(args) ;
            return answer ;
        }
        else{
        	gcvt((*tan_val)(atof(args)),3,answer) ;
        	dlclose(handle) ;
		    free(dll_name) ;
		    free(function_name) ;
		    free(args) ;
        	return answer ;
        }
    }

    else if(strcmp(function_name,"tanh")==0){
        double (*tanh_val)(double) ;
        tanh_val = dlsym(handle,"tanh") ;
        if ((error_handling = dlerror()) != NULL)  {
            strcat(answer, "Some error occurred while tring to load the required function from the DLL. Please consider trying again\0") ;
            dlclose(handle) ;
		    free(dll_name) ;
		    free(function_name) ;
		    free(args) ;
            return answer ;
        }
        else{
        	gcvt((*tanh_val)(atof(args)),3,answer) ;
        	dlclose(handle) ;
		    free(dll_name) ;
		    free(function_name) ;
		    free(args) ;
        	return answer ;
        }
    }

    else if(strcmp(function_name,"atan")==0){
        double (*atan_val)(double) ;
        atan_val = dlsym(handle,"atan") ;
        if ((error_handling = dlerror()) != NULL)  {
            strcat(answer, "Some error occurred while tring to load the required function from the DLL. Please consider trying again\0") ;
            dlclose(handle) ;
		    free(dll_name) ;
		    free(function_name) ;
		    free(args) ;
            return answer ;
        }
        else{
        	gcvt((*atan_val)(atof(args)),3,answer) ;
        	dlclose(handle) ;
		    free(dll_name) ;
		    free(function_name) ;
		    free(args) ;
        	return answer ;
        }
    }

    else if(strcmp(function_name,"exp")==0){
        double (*exp_val)(double) ;
        exp_val = dlsym(handle,"exp") ;
        if ((error_handling = dlerror()) != NULL)  {
            strcat(answer, "Some error occurred while tring to load the required function from the DLL. Please consider trying again\0") ;
            dlclose(handle) ;
		    free(dll_name) ;
		    free(function_name) ;
		    free(args) ;
            return answer ;
        }
        else{
        	gcvt((*exp_val)(atof(args)),3,answer) ;
        	dlclose(handle) ;
		    free(dll_name) ;
		    free(function_name) ;
		    free(args) ;
        	return answer ;
        }
    }

    else if(strcmp(function_name,"sqrt")==0){
        double (*sqrt_val)(double) ;
        sqrt_val = dlsym(handle,"sqrt") ;
        if ((error_handling = dlerror()) != NULL)  {
            strcat(answer, "Some error occurred while tring to load the required function from the DLL. Please consider trying again\0") ;
            dlclose(handle) ;
		    free(dll_name) ;
		    free(function_name) ;
		    free(args) ;
            return answer ;
        }
        else{
        	gcvt((*sqrt_val)(atof(args)),3,answer) ;
        	dlclose(handle) ;
		    free(dll_name) ;
		    free(function_name) ;
		    free(args) ;
        	return answer ;
        }
    }

    else if(strcmp(function_name,"log")==0){
        double (*log_val)(double) ;
        log_val = dlsym(handle,"log") ;
        if ((error_handling = dlerror()) != NULL)  {
            strcat(answer, "Some error occurred while tring to load the required function from the DLL. Please consider trying again\0") ;
            dlclose(handle) ;
		    free(dll_name) ;
		    free(function_name) ;
		    free(args) ;
            return answer ;
        }
        else{
        	gcvt((*log_val)(atof(args)),3,answer) ;
        	dlclose(handle) ;
		    free(dll_name) ;
		    free(function_name) ;
		    free(args) ;
        	return answer ;
        }
    }

    else if(strcmp(function_name,"log10")==0){
        double (*log10_val)(double) ;
        log10_val = dlsym(handle,"log10") ;
        if ((error_handling = dlerror()) != NULL)  {
            strcat(answer, "Some error occurred while tring to load the required function from the DLL. Please consider trying again\0") ;
            dlclose(handle) ;
		    free(dll_name) ;
		    free(function_name) ;
		    free(args) ;
            return answer ;
        }
        else{
        	gcvt((*log10_val)(atof(args)),3,answer) ;
        	dlclose(handle) ;
		    free(dll_name) ;
		    free(function_name) ;
		    free(args) ;
        	return answer ;
        }
    }

    else if(strcmp(function_name,"ceil")==0){
        double (*ceil_val)(double) ;
        ceil_val = dlsym(handle,"ceil") ;
        if ((error_handling = dlerror()) != NULL)  {
            strcat(answer, "Some error occurred while tring to load the required function from the DLL. Please consider trying again\0") ;
            dlclose(handle) ;
		    free(dll_name) ;
		    free(function_name) ;
		    free(args) ;
            return answer ;
        }
        else{
        	gcvt((*ceil_val)(atof(args)),3,answer) ;
        	dlclose(handle) ;
		    free(dll_name) ;
		    free(function_name) ;
		    free(args) ;
        	return answer ;
        }
    }

    else if(strcmp(function_name,"floor")==0){
        double (*floor_val)(double) ;
        floor_val = dlsym(handle,"floor") ;
        if ((error_handling = dlerror()) != NULL)  {
            strcat(answer, "Some error occurred while tring to load the required function from the DLL. Please consider trying again\0") ;
            dlclose(handle) ;
		    free(dll_name) ;
		    free(function_name) ;
		    free(args) ;
            return answer ;
        }
        else{
        	gcvt((*floor_val)(atof(args)),3,answer) ;
        	dlclose(handle) ;
		    free(dll_name) ;
		    free(function_name) ;
		    free(args) ;
        	return answer ;
        }
    }

    else{
    	memset(answer,0,sizeof(answer)) ;
	    strcat(answer,"The name of the function passed for the given DLL is incorrect\0") ;
	    return answer ;
    }

}


int main(){

	struct request *client_req = (struct request *)malloc(sizeof(struct request)) ;
	char *dll_name = malloc(256) ;
	strcpy(dll_name,"/lib/x86_64-linux-gnu/libm.so.6") ;
	char *func_name = malloc(256) ;
	strcpy(dll_name,"cos") ;
	char *args = malloc(256) ;
	strcpy(dll_name,"3.14") ;

	client_req->dll_name = dll_name ;
	client_req->func_name = func_name ;
	client_req->func_args = args ;

	struct Node *front = NULL, *back = NULL ;
	int p = 1 ;
	int *client_soc = &p ;

	back = enqueue(&front,&back,client_soc,client_req) ;

	if((front != NULL && back != NULL) && strcmp(front->client_req->dll_name,dll_name) == 0 && strcmp(front->client_req->func_name,func_name) == 0 && strcmp(front->client_req->func_args,args) == 0){
		printf("Function 'enqueue' working correctly\n") ;
	}
	else{
		printf("Function 'enqueue' not functioning\n") ;
	}

	struct client_request_info *client = dequeue(&front,&back) ;

	if((front == NULL && back == NULL) && (strcmp(client->client_req->dll_name,dll_name) == 0 && strcmp(client->client_req->func_name,func_name) == 0 && strcmp(client->client_req->func_args,args) == 0)){
		printf("Function 'dequeue' working correctly\n") ;
	}
	else{
		printf("Function 'dequeue' not functioning\n") ;
	}


	memset(dll_name,0,sizeof(dll_name)) ;
	strcpy(dll_name,"/lib/x86_64-linux-gnu/libm.so.6") ;
	memset(func_name,0,sizeof(func_name)) ;
	strcpy(func_name,"cos") ;
	memset(args,0,sizeof(args)) ;
	strcpy(args,"3.14") ;
	char *answer1 = get_dll_answer(dll_name,func_name,args) ;
	if(strcmp(answer1,"-1")==0){
		printf("Function 'cos' in the DLL working correctly\n") ;
	}
	else{
		printf("Function 'cos' in the DLL not functioning\n") ;
	}


	memset(dll_name,0,256) ;
	strcpy(dll_name,"/lib/x86_64-linux-gnu/libm.so.6") ;
	memset(func_name,0,256) ;
	strcpy(func_name,"sinh") ;
	memset(args,0,256) ;
	strcpy(args,"10") ;
	char *answer2 = get_dll_answer(dll_name,func_name,args) ;
	if(strcmp(answer2,"1.1e+04")==0){
		printf("Function 'sinh' in the DLL working correctly\n") ;
	}
	else{
		printf("Function 'sinh' in the DLL not functioning\n") ;
	}


	memset(dll_name,0,256) ;
	strcpy(dll_name,"/lib/x86_64-linux-gnu/libm.so.6") ;
	memset(func_name,0,256) ;
	strcpy(func_name,"atan") ;
	memset(args,0,256) ;
	strcpy(args,"765.876") ;
	char *answer3 = get_dll_answer(dll_name,func_name,args) ;
	if(strcmp(answer3,"1.57")==0){
		printf("Function 'atan' in the DLL working correctly\n") ;
	}
	else{
		printf("Function 'atan' in the DLL not functioning\n") ;
	}

}
