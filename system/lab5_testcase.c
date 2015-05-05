/*  main.c  - main */
/* Sample test case for lab 5 */

#include <xinu.h>


int main(int argc, char **argv)
{
	char wbuff[] = {"While disks provide a convenient means of permanent storage, they support only a simple and limited naming mechanism of the data they hold (the block number). File systems are used to overcoming this limitation by providing a more elaborate and useful naming mechanism for the data blocks on a disk. This original Xinu File System is a mini-file system with an elegant design that has a limited numbers of files, short file names, and no hierarchical directory structure; however, it allows files to grow dynamically. \r\nThe objective of this lab is to enhance the existing file system for Xinu. You will extend the functionality of the Xinu File System on top of the remote disk abstraction. You are going to transform the current flat file system into a new file system with hierarchical directory structure. \r\n"};
	int32 wbufflen = sizeof( wbuff );
	char rbuff[ wbufflen * 10 ];
	int32 rbufflen = sizeof( rbuff );
	int32 retval;
	int32 fd[5];
	int32 i, j;

	kprintf("Open remote disk\r\n");
	retval = open(RDISK,"XinuDisk","rw");
	if (retval == SYSERR){
		panic("Error: could not open the remote disk, check the server\n\r");
	}

	kprintf("Initializing file system\r\n");
	retval = lfscreate ( RDISK, 1000, 1024000);
	if( retval == SYSERR )
		panic("Initialize file system failed");

	kprintf("\r\nList root directory:\r\n");
	retval = lflistdirh( LFILESYS, "/" ); //list root dir
	if( retval == SYSERR )
		kprintf("Listing failed\r\n");
		
	// Create 5 directories
	char dirname [] = {"/DIR_1"};
	kprintf("\r\n");
	for( i=0; i<5; i++){
		dirname[5] = '1'+i;
		kprintf("Creating directory %s", dirname );
		retval = control(LFILESYS, LF_CTL_MKDIR, (int)dirname, 0);
		if( retval == SYSERR )
			kprintf("....failed");
		kprintf("\r\n");
	}



	/*
	char temp [] = {"/DIR_1/DIR_11"};
	kprintf("---Directory inside directory\r\n");
	retval = control(LFILESYS, LF_CTL_MKDIR, (int)temp, 0);
	if( retval == SYSERR )
			kprintf("....failed\r\n");
	

	kprintf("---File has same name with existing directory\r\n");
	char temp2 [ ] = {"/DIR_1/DIR_11"};
	fd[0] = open(LFILESYS, temp2, "rwn");
	close(fd[0]);

	kprintf("--open the same file twice\r\n");
	open(LFILESYS, temp2, "rwn");


	retval = lflistdirh( LFILESYS, "/DIR_1" ); //list root dir
		if( retval == SYSERR )
		kprintf("Listing failed\r\n");*/

	//Open a file in each directory
	char filename [] = {"/DIR_1/file"};
	kprintf("\r\n");
	for( i=0; i<5; i++)
	{ 
		filename[5] = '1'+i;
		kprintf("Opening file at %s\r\n", filename);
		fd[i] = open(LFILESYS, filename, "rwn");
		kprintf("  Got the sudo device : %d \r\n", fd[i]);
	}

	
	
	// Write to first file
	kprintf("\r\nWriting %d characters to /DIR_1/file .... ", wbufflen );
	retval = write( fd[0], wbuff, wbufflen );
	if( retval == SYSERR )
		kprintf("Write failed \r\b");
	else
		kprintf("Write returned %d \r\n", retval );

	// Write to last file
	kprintf("\r\nWriting %d characters to /DIR_5/file \r\n", wbufflen*10 );
	int total = 0;
	for( i=0; i<10; i++ ){
		retval = write( fd[4], wbuff, wbufflen );
		if( retval == SYSERR )
			kprintf("Write failed \r\b");
		else {
			kprintf("Write returned %d \r\n", retval );
			total += retval;
		}
	}
	if( total != 0 )
		kprintf("Wrote a total of %d characters\r\n", total );
		
	//Go to the beginning of the file at /DIR_5/file
	seek(fd[4],0);
	
	//Read
	int wrong = 0;
	kprintf("\r\nReading %d characters.... ", rbufflen);
	retval = read( fd[4], rbuff, rbufflen );
	if( retval == SYSERR )
		kprintf("Read failed \r\n");
	else{ 
		kprintf("Read returned %d\r\n", retval );

		//check the words read
		for (j=0,i=0; j<retval; j++,i++)
		{
			i = i % wbufflen;
			if(rbuff[j]!=wbuff[i]){
				wrong ++; 
				kprintf("read wrong at i: %d (expect %c, got %c)\r\n", j, wbuff[i], rbuff[j]);
			}
			if( wrong >= 20 ) break; //stops printing after 20 characters wrong
		} 
		if( wrong == 0)
			kprintf("Congrats. Characters read matches characters wrote.\r\n");
	}
	
	
	//close all files
	kprintf("\r\nClosing all files \r\n");
	for( i=0; i<5; i++ ){
		retval = close( fd[i] );
		if( retval == SYSERR )
			kprintf("Close file /DIR_%d/file failed \r\n", i+1);
	}

	//list files
	kprintf("\r\nList root directory:\r\n");
	retval = lflistdirh( LFILESYS, "/" );
	if( SYSERR == retval ) 
		kprintf("  Listing failed\r\n");
	for( i=0; i<5; i++ ){
		dirname[5] = '1'+i;
		kprintf("\r\nList directory %s:\r\n", dirname);
		retval = lflistdirh( LFILESYS, dirname );
		if( SYSERR == retval ) 
			kprintf("  Listing failed\r\n");
	}

	return OK;
}

