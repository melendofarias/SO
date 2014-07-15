// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create several threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustrate the inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.
//
// Parts from Copyright (c) 2007-2009 Universidad de Las Palmas de Gran Canaria
//

#include "copyright.h"
#include "system.h"
#include "synch.h"


//----------------------------------------------------------------------
// SimpleThread
// 	Loop 10 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"name" points to a string with a thread name, just for
//      debugging purposes.
//----------------------------------------------------------------------


Puerto *puerto  = new Puerto("puerto"); 
Semaphore *semap = new Semaphore("semaforo_test",1);

void
SimpleThread(void* name)
{
    // Reinterpret arg "name" as a string
    char* threadName = (char*)name;
    
    // If the lines dealing with interrupts are commented,
    // the code will behave incorrectly, because
    // printf execution may cause race conditions.
    for (int num = 0; num < 10; num++) {
        //IntStatus oldLevel = interrupt->SetLevel(IntOff);
	printf("*** thread %s looped %d times\n", threadName, num);
	//interrupt->SetLevel(oldLevel);
        currentThread->Yield();
    }
    //IntStatus oldLevel = interrupt->SetLevel(IntOff);
    printf(">>> Thread %s has finished\n", threadName);
    //interrupt->SetLevel(oldLevel);
}

//----------------------------------------------------------------------
// ThreadTest
// 	Set up a ping-pong between several threads, by launching
//	ten threads which call SimpleThread, and finally calling 
//	SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest()
{
    DEBUG('t', "Entering SimpleTest");

    for ( int k=1; k<=10; k++) {
      char* threadname = new char[100];
      sprintf(threadname, "Hilo %d", k);
      Thread* newThread = new Thread (threadname);
      newThread->Fork (SimpleThread, (void*)threadname, 1, 0 );
    }
    
    SimpleThread( (void*)"Hilo 0");
}

//----------------------------------------------------------------------
// Funciones utilizadas para TestPuerto.
//----------------------------------------------------------------------
void
Productor(int n)
{
	DEBUG('p',"enviando el numero: %d\n",n);
	puerto->Send(n);
	DEBUG('p',"SALI Escritor: %d\n",n);
}
void
Consumidor()
{	DEBUG('p',"recibiendo \n");
	puerto->Receive();
	DEBUG('p',"SALI Lector\n");
	
}
void
Test_1(void* n)
{
	DEBUG('p', "Escritor, %d\n",n);	
	//int k = 2 ;
	//Productor((int)(n+1));
	Productor((int )n);
	
}
void 
Test_2(void* name)
{
	DEBUG('p', "Lector, %d\n",name);
	Consumidor();
}

void
PuertoTest()
{
		
	
	DEBUG('p', "Prueba consumidor-Productor\n");
	for(int k=0; k<3; k++)
	{ 
	 
	  Thread* newThread = new Thread ("Escritores");
      newThread->Fork (Test_1, (void *)k, 1,0);	
	}	
	for(int k=0; k<4; k++)
	{ 
	  Thread* newThread = new Thread ("Lectores");
      newThread->Fork (Test_2, (void *)k,1, 1);	
	}
	Thread* newThread = new Thread ("Escritores");
      newThread->Fork (Test_1, (void *)4, 1,2);	
}


void
Caso1(void *n){
	DEBUG('w',"Caso 1 en V() \n");
	semap->V();
	
}
void
Caso2(void *n){
	DEBUG('w',"Caso 2 en P() \n");
	semap->P();

}

void
JoinTest(){
	Thread* newThread_1 = new Thread ("P");
      newThread_1->Fork (Caso1, (void *)3, 1,2);	
	Thread* newThread = new Thread ("p");
      newThread->Fork (Caso2, (void *)4, 1,0);	
}
//--------------------------------------------------------
//-----------Test Main------------------------------------
//--------------------------------------------------------

void 
Test()
{


	printf("Test: t --> Thread\n");
	printf("Test: p --> Puerto\n");
	printf("Test: j --> Join\n");
	printf("Ingrese test a realizar : ");
	char test = getchar();
	if (test == 't' )
		ThreadTest();
	else if (test == 'p')
		PuertoTest();
	else if (test == 'j')
		JoinTest();

}



