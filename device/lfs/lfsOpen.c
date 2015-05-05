/* lfsOpen.c  -  lfsOpen */

#include <xinu.h>

/*------------------------------------------------------------------------
 * lfsOpen - open a file and allocate a local file pseudo-device
 *------------------------------------------------------------------------
 */
devcall	lfsOpen (
	 struct	dentry	*devptr,	/* entry in device switch table	*/
	 char	*name,			/* name of file to open		*/
	 char	*mode			/* mode chars: 'r' 'w' 'o' 'n'	*/
	)
{
	struct	lfdir	*dirptr;	/* ptr to in-memory directory	*/
	char		*from;	/* ptrs used during copy	*/
	char		*nam, *cmp;	/* ptrs used during comparison	*/
	int32		i;		/* general loop index		*/
	did32		lfnext;		/* minor number of an unused	*/
					/*    file pseudo-device	*/
	struct	lflcblk	*lfptr;		/* ptr to open file table entry	*/
	//bool8		found;		/* was the name found?		*/
	int32	retval;			/* value returned from function	*/
	int32	mbits;			/* mode bits			*/


	//extract paths

	int bound = numofpaths(name);

	if (bound == -1) {
		return SYSERR;
	}

	char ds [100][50];
	extractPath(name, ds);

	/* Check length of name file (leaving space for NULLCH */

	from = name;
	for (i=0; i< LF_NAME_LEN; i++) {
		if (*from++ == NULLCH) {
			break;
		}
	}
	if (i >= LF_NAME_LEN) {		/* name is too long */
		return SYSERR;
	}

	/* Parse mode argument and convert to binary */

	mbits = lfgetmode(mode);
	if (mbits == SYSERR) {
		return SYSERR;
	}

	/* If named file is already open, return SYSERR */

	lfnext = SYSERR;
	for (i=0; i<Nlfl; i++) {	/* search file pseudo-devices	*/
		lfptr = &lfltab[i];
		if (lfptr->lfstate == LF_FREE) {
			if (lfnext == SYSERR) {
				lfnext = i; /* record index */
			}
			continue;
		}

		/* Compare requested name to name of open file */

		nam = name;
		cmp = lfptr->lfname;
		while(*nam != NULLCH) {
			if (*nam != *cmp) {
				break;
			}
			nam++;
			cmp++;
		}

		/* See if comparison succeeded */

		if ( (*nam==NULLCH) && (*cmp == NULLCH) ) {
			return SYSERR;
		}
	}
	if (lfnext == SYSERR) {	/* no slave file devices are available	*/
		return SYSERR;
	}

	/* Obtain copy of directory if not already present in memory	*/

	dirptr = &Lf_data.lf_dir;
	wait(Lf_data.lf_mutex);
	if (! Lf_data.lf_dirpresent) {
	    retval = read(Lf_data.lf_dskdev, (char *)dirptr,LF_AREA_ROOT);
	    if (retval == SYSERR ) {
		signal(Lf_data.lf_mutex);
		return SYSERR;
	    }
	    Lf_data.lf_dirpresent = TRUE;
	}

	lfptr = &lfltab[lfnext];

	int r = OK;
	int * result = &r;
	
    findFD (Lf_data.lf_dskdev, 0, 0, bound, ds, result, lfptr, mbits, name);

	if (r == SYSERR) {
	
		signal(Lf_data.lf_mutex);
		return r;
	}

	signal(Lf_data.lf_mutex);

	return lfptr->lfdev;
}

void findFD (did32 diskdev, ibid32 nextibp,
             int depth, int bound,
             char ds [100][50], int * result,
             struct	lflcblk	* lfptr, int mbits, char * name) {

	if (depth > bound && bound != -1) {
		
		return;
	}

	//get root iblock
    
    struct lfiblk ib;
    struct lfiblk * ibp = &ib;
    lfibget(diskdev, nextibp, ibp);


    //find destination

    dbid32 dnum;
	ibid32 ibnum;
	int found = SYSERR;
	int foundNext = SYSERR;
	struct ldentry * temp;
    struct lfdbfree * dbuffp;

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

		//get iblock of the file being compared
		struct lfiblk nbk;
		struct lfiblk * nbkp = &nbk;

		lfibget(diskdev, ldp -> ld_ilist, nbkp);

		//compare path and name of file

		char * filename = ldp -> ld_name;

		if (depth == bound - 1) {
				
			//file already exists

			if (strcmp(filename, ds[depth]) && nbkp -> isDirectory != 1) {
				
				
				foundNext = OK;
				found = OK;
				ibnum = ldp -> ld_ilist;
                dnum = ibp -> ib_dba [i];
				temp = ldp;
                dbuffp = dbp;
				break;
			}		
		}

		else if (strcmp (filename, ds[depth])) {

			if (nbkp -> isDirectory == 1) {

				foundNext = OK;
				findFD (Lf_data.lf_dskdev, ldp -> ld_ilist, depth + 1, bound, ds, result, lfptr, mbits, name);
			}
		}
    }

	//before searching for the file, if there is no match, return SYSERR

	if (foundNext == SYSERR && depth < bound - 1) {
	
		*result = SYSERR;
	}

	if (*result == SYSERR) {	

		return;
	}
	
	//open the file

	if (depth == bound - 1) {
		
        
		//if file does not exist

		if (found == SYSERR) {
		
			// if file must exist, return SYSERR
			if (mbits & LF_MODE_O) {
				
				*result = SYSERR;
				return;
			}
			
			//create a ldentry for the file
			struct ldentry newld;
			struct ldentry * newldp = &newld;
			memcpy (newld.ld_name, ds[depth], strlen(ds[depth]));

			newld.ld_name[strlen(ds[depth])] = '\0';
			newld.ld_size = 0;
			newld.ld_ilist = LF_INULL;
			temp = newldp;

			//create a data block for parent to store file information
			struct lfdbfree dbuff;
			dbuffp = &dbuff;
			dnum = lfdballoc(&dbuff);
			memcpy(dbuff.lf_unused, (char *)newldp, sizeof(newld));
			write (diskdev, (char *)dbuffp, dnum);

			//update parent directory
			ibp -> ib_dba [ibp -> countfiles] = dnum; 
			ibp -> countfiles ++;
			lfibput(diskdev, nextibp, ibp);

		}

		//if file exists and file must not exist

		else if (mbits & LF_MODE_N) { 

				*result = SYSERR;
				return;
		}

		//set up psudo device
		
		lfptr->lfstate = LF_USED;
		lfptr->lfdirptr = temp;
		lfptr->lfmode = mbits & LF_MODE_RW;
		lfptr->lfinum    = LF_INULL;
		lfptr->lfdnum    = LF_DNULL;
		lfptr->lfpos     = 0;
		memcpy(lfptr->lfname, name, strlen(name));
		lfptr -> lfname[strlen(name)] = '\0'; 
		lfptr->lfbyte = &lfptr->lfdblock[LF_BLKSIZ];
		lfptr->lfibdirty = FALSE;
		lfptr->lfdbdirty = FALSE;
        lfptr->parent = dbuffp;
        lfptr->parentd = dnum;
	}
}

