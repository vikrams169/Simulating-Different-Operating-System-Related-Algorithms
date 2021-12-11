#ifndef dynamic_library_loader
#include <dlfcn.h>
#include <string.h>


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

    char *answer = malloc(256) ;
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


#define dll
#endif