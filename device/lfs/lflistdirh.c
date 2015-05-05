#include <xinu.h>

int lflistdirh(did32 diskdev, char *directorypath) {
    
    //get numbers of paths and store each path
    
    int bound = numofpaths(directorypath);
    
    char ds [100][50];
    
    if (bound == -1) {
        
        ds [0][0] = '/';
        ds [0][1] = '\0';
    }
    
    else {
        
        extractPath(directorypath, ds);
    }
    
	int r = OK;
	int * result = &r;

	
    findDestination (Lf_data.lf_dskdev, 0, 0, bound, ds, 0, result);

	if (*result == SYSERR) {
		return SYSERR;	
	}

	return OK;
}

void findDestination (did32 diskdev, ibid32 nextibp, int depth, int bound, char ds [100][50], int mode, int * result) {

	if (depth > bound && bound != -1) {
		
		return;
	}

	//cant make root
			
	if (bound == -1 && mode == 1) {
				
		*result = SYSERR;
		return;			
	}


	//get root iblock
    
    struct lfiblk ib;
    struct lfiblk * ibp = &ib;
    lfibget(diskdev, nextibp, ibp);


	//if anything in path is a file, return SYSERR

	if (ibp -> isDirectory != 1) {

		*result = SYSERR;
	
		return;
	}


    //list all files in directory

	ibid32 foundNext = SYSERR;	

    char dbuff[LF_BLKSIZ];

    int i = 0;
    
    for (i = 0; i < ibp -> countfiles; i ++) {

		
		//extract data block
        
        read(diskdev, dbuff, ibp -> ib_dba[i]);

		int j = 0;

		struct lfdbfree db;
		struct lfdbfree * dbp = &db;

		char * to = (char*) dbp;

		for (j = 0; j < sizeof(struct lfdbfree); j ++) {

				*to = dbuff[j];
				to ++;
		}
        
		//extract ldentry

		struct ldentry ld;
		struct ldentry * ldp = &ld;

		to = (char *) ldp; 

		for (j = 0; j < sizeof(struct ldentry); j ++) {

			*to = dbp -> lf_unused[j];
			to ++;
		}

		//compare path and name of file

		char * filename = ldp -> ld_name;

		//ls

		if (mode == 0) {
			
			//list root directory
			if (bound == -1 || depth == bound) {

				foundNext = OK;
				kprintf("%s\t", filename);

				if (i == ibp -> countfiles - 1) {
			
					kprintf("\r\n");				
				}
			}
		
			else if (strcmp (filename, ds[depth])) {

				foundNext = ldp -> ld_ilist;
				findDestination (Lf_data.lf_dskdev, foundNext, depth + 1, bound, ds, mode, result);
			}
		}
	
		//mkdir				
		
		else {
			
			if (depth == bound - 1) {
				
				//directory already exists

				struct lfiblk nbk;
				struct lfiblk * nbkp = &nbk;

				lfibget(diskdev, ldp -> ld_ilist, nbkp);
						
				if (strcmp(filename, ds[depth]) && nbkp -> isDirectory == 1) {
				
					*result = SYSERR;
					break;
				}		
			
				foundNext = OK;
			}

			else if (strcmp (filename, ds[depth])) {

				foundNext = ldp -> ld_ilist;
				findDestination (Lf_data.lf_dskdev, foundNext, depth + 1, bound, ds, mode, result);	
			}
		}
    }

	if (ibp -> countfiles == 0) {
		
		foundNext = OK;
	}

	//if no file match with next path return SYSERR

	if (foundNext == SYSERR) {
		
		*result = SYSERR;		
		return;
	}

	if (*result == SYSERR) {	
		return;
	}
	
	if (mode == 1 && depth == bound - 1) {
		//allocate iblock for the new directory 
		
		ibid32 ibnum = lfiballoc();
		struct lfiblk newib;
   		struct lfiblk * newibp = &newib;
    	lfibget(diskdev, ibnum, newibp);
		newibp -> isDirectory = 1;
		newibp -> countfiles = 0;

		lfibput (diskdev, ibnum, newibp);

		//create ldentry and store file name

		struct ldentry newld;
		struct ldentry * newldp = &newld;
		memcpy (newld.ld_name, ds[depth], strlen(ds[depth]));

		newld.ld_name[strlen(ds[depth])] = '\0';
		newld.ld_size = 0;
		newld.ld_ilist = ibnum;

		//allocate data block to store ldentry
		
		struct lfdbfree dbuff;
		struct lfdbfree * dbuffp = &dbuff;
		dbid32 dnum = lfdballoc(&dbuff);
		memcpy(dbuff.lf_unused, (char *)newldp, sizeof(newld));
		write (diskdev, (char *)dbuffp, dnum);

		//update parent directory
		ibp -> ib_dba [ibp -> countfiles] = dnum; 
		ibp -> countfiles ++;
		lfibput(diskdev, nextibp, ibp);
		
	}
}

