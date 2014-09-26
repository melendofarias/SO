/* create.c
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
main(char* arg1, char* arg2)
{
    Create("HOLA\0");
	Write("LLLL\n",5,1);
	Write(arg1, 10, 1);
	//Exec("/home/marisol/code/test/create1");
	
	Exit(0);

    return 0;
}
