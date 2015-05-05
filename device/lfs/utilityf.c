/* lfscreate.c  -  lfscreate */

#include <xinu.h>

int numofpaths (char * path) {
    
    int len = strlen(path);
    
    if (len == 1 && path[0] == '/') {
        
        return -1;
    }
    
    int i = 0;
    int count = 0;
    
    for (i = 0; i < len; i ++) {
        
        if (path[i] == '/') {
            
            count ++;
        }
    }
    
    if (path[len - 1] != '/') {
        
        count ++;
        
    }

	if (path[0] == '/') {
		count --;
	}
    
    return count;
}

int extractPath (char * path, char paths [100][50]) {
    
    
    int i = 0;
    int j = 0;
    int k = 0;
    
    for (i = 0; i  < strlen(path); i ++) {
        
        if (path[i] == '/') {
            
            if (i != 0 && i != (strlen(path) - 1)) {
            
                paths[j][k] = '\0';
                j ++;
                k = 0;
            }
        }
        else {
            
            paths[j][k] = path[i];
			k ++;
        }
    }

	paths [j][k] = '\0';
    
    return j;
}

int strlen(char * path) {
	
	int i = 0;

	while (path[i] != '\0') {

		i ++;
	}

	return i;
}

int strcmp (char * a, char * b) {

	int i = 0;

	while (a[i] != '\0') {
	
		if ((b[i] != a[i]) || (a[i + 1] != b [i + 1])) {
				return 0;	
		}

		i ++;
	}

	return 1;
}


