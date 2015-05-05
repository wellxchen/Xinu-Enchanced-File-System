#include<xinu.h>

/* lfsControl.c - lfsControl */


/*------------------------------------------------------------------------
 * lfsControl - Provide control functions for a local file system
 *------------------------------------------------------------------------
 */
devcall	lfsControl (
                    struct dentry	*devptr,
                    int32	func,
                    int32	arg1,
                    int32	arg2
)
{
    int retval;
  
    if (func == LF_CTL_MKDIR) {
        retval = mkdir((char*)arg1);
        return retval;
    }
    
	
    return SYSERR;
}
