#include<xinu.h>


status mkdir(char *path)
{
    int bound = numofpaths(path);

	//can't make root directory
    
    if (bound == -1) {
   
        return SYSERR;
    }

	//extract path

    char ds [100][50];
    
    extractPath(path, ds);

	int r = OK;
	int * result = & r;
     
	
    findDestination (Lf_data.lf_dskdev, 0,0, bound, ds, 1, result);


	if (*result == SYSERR) {
	
		return SYSERR;	
	}
    
    return OK;
}
