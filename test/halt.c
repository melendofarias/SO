/* halt.c
 *	Simple program to test whether running a user program works.
 *	
 *	Just do a "syscall" that shuts down the OS.
 *
 * 	NOTE: for some reason, user programs with global data structures 
 *	sometimes haven't worked in the Nachos environment.  So be careful
 *	out there!  One option is to allocate data structures as 
 * 	automatics within a procedure, but if you do this, you have to
 *	be careful to allocate a big enough stack to hold the automatics!
 */

#include "syscall.h"


int
main()
{
	//char *buff;
	//Read(buff,5, 0);
	
	
	Create("file0");
	int id0 = Open("file0");
	//Write("Exces: \n\0", 8, id0);
	

	/*
	Create("file1");	
	int id1 = Open("file1");
	
	Close(id0);
	

	int idNOexiste = 9;
	Close(idNOexiste); 
	Close(id1);
    */
    //Write("Exces: \n", 8, ConsoleOutput);
    
    int argc = 3;
    char** argv;

    argv[0] ="argv0\0";
    argv[1] ="argv1\0";
    
    char** argv1 = "argv1111 argv2 argv3\0";
	int pid = Exec("/home/marisol/code/test/create", argc, argv1);
	
   	//int pid1 = Exec("/home/marisol/code/test/create");
	
	/*
	Create("file22");
	int id2 = Open("file22");
	Close(id2);
	*/
	
	Exit(0);
	return 0;
	//Halt();
    /* not reached */
}
