// scheduler.cc 
//	Routines to choose the next thread to run, and to dispatch to
//	that thread.
//
// 	These routines assume that interrupts are already disabled.
//	If interrupts are disabled, we can assume mutual exclusion
//	(since we are on a uniprocessor).
//
// 	NOTE: We can't use Locks to provide mutual exclusion here, since
// 	if we needed to wait for a lock, and the lock was busy, we would 
//	end up calling FindNextToRun(), and that would put us in an 
//	infinite loop.
//
// 	Very simple implementation -- no priorities, straight FIFO.
//	Might need to be improved in later assignments.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "scheduler.h"
#include "system.h"

//----------------------------------------------------------------------
// Scheduler::Scheduler
// 	Initialize the list of ready but not running threads to empty.
//----------------------------------------------------------------------

Scheduler::Scheduler()
{ 
	int i ;
	MaxPriority = 3;
	//List<Thread*> ArregloreadyListP[3];
	readyListP = new List<Thread*>*[MaxPriority];
	for ( i = 0; i < MaxPriority ; i++){ 								//cambiar 3 por readyListP.size()
		readyListP[i] = new List<Thread*>; 
	}
	//*readyListP = ArregloreadyListP; 
} 

//----------------------------------------------------------------------
// Scheduler::~Scheduler
// 	De-allocate the list of ready threads.
//----------------------------------------------------------------------

Scheduler::~Scheduler()
{ 
    delete *readyListP; 
} 

//----------------------------------------------------------------------
// Scheduler::ReadyToRun
// 	Mark a thread as ready, but not running.
//	Put it on the ready list, for later scheduling onto the CPU.
//
//	"thread" is the thread to be put on the ready list.
//----------------------------------------------------------------------

void
Scheduler::ReadyToRun (Thread *thread)
{
    DEBUG('p', "Putting thread %s on ready list.\n", thread->getName());

    thread->setStatus(READY);
    int pi = thread->getPriority();
    DEBUG('p', "priority,%d\n",pi);
    readyListP[pi]->Append(thread);
}

//----------------------------------------------------------------------
// Scheduler::FindNextToRun
// 	Return the next thread to be scheduled onto the CPU.
//	If there are no ready threads, return NULL.
// Side effect:
//	Thread is removed from the ready list.
//----------------------------------------------------------------------

Thread *
Scheduler::FindNextToRun()
{
	DEBUG('t', "next to run \n");
	int p = MaxPriority-1;
	//Print();
	DEBUG('t', "Priodidad maxima, %d \n", p);
	while (p > 0 && (readyListP[p])->IsEmpty()){		//Cambiar 3 por Length de readyListP
	     p--;
	}
	//printf("Next to run %d\n",p);
/*	if(p == 0)
	{
		DEBUG('p', "p == 0 \n");
		return NULL;
	}
	else*/		
    	return readyListP[p]->Remove();

}

//----------------------------------------------------------------------
// Scheduler::Run
// 	Dispatch the CPU to nextThread.  Save the state of the old thread,
//	and load the state of the new thread, by calling the machine
//	dependent context switch routine, SWITCH.
//
//      Note: we assume the state of the previously running thread has
//	already been changed from running to blocked or ready (depending).
// Side effect:
//	The global variable currentThread becomes nextThread.
//
//	"nextThread" is the thread to be put into the CPU.
//----------------------------------------------------------------------

void
Scheduler::Run (Thread *nextThread)
{
    Thread *oldThread = currentThread;
    
#ifdef USER_PROGRAM			// ignore until running user programs 
    if (currentThread->space != NULL) {	// if this thread is a user program,
        currentThread->SaveUserState(); // save the user's CPU registers
		currentThread->space->SaveState();
    }
#endif
    
    oldThread->CheckOverflow();		    // check if the old thread
					    // had an undetected stack overflow

    currentThread = nextThread;		    // switch to the next thread
    currentThread->setStatus(RUNNING);      // nextThread is now running
    
    DEBUG('t', "Switching from thread \"%s\" to thread \"%s\"\n",
	  oldThread->getName(), nextThread->getName());
    
    // This is a machine-dependent assembly language routine defined 
    // in switch.s.  You may have to think
    // a bit to figure out what happens after this, both from the point
    // of view of the thread and from the perspective of the "outside world".

    SWITCH(oldThread, nextThread);
    
    DEBUG('t', "Now in thread \"%s\"\n", currentThread->getName());

    // If the old thread gave up the processor because it was finishing,
    // we need to delete its carcass.  Note we cannot delete the thread
    // before now (for example, in Thread::Finish()), because up to this
    // point, we were still running on the old thread's stack!
    if (threadToBeDestroyed != NULL) {
        delete threadToBeDestroyed;
	threadToBeDestroyed = NULL;
    }
    
#ifdef USER_PROGRAM
    if (currentThread->space != NULL) {		// if there is an address space
        currentThread->RestoreUserState();     // to restore, do it.
		currentThread->space->RestoreState();
    }
#endif
}

//----------------------------------------------------------------------
// Scheduler::Print
// 	Print the scheduler state -- in other words, the contents of
//	the ready list.  For debugging.
//----------------------------------------------------------------------

static void
ThreadPrint (Thread* t) {
  t->Print();
}

void
Scheduler::Print()
{
	int i ;
	for ( i = 0; i < MaxPriority ; i++){ 				//Cambiar 3 por Length de readyListP
		printf("\nPRIORITY%d - Ready list contents:\n",  i);
		(readyListP[i])->Apply(ThreadPrint);
	}
	
}
void 
Scheduler:: ChangeQueuePriority(Thread *th, int priority)
{	
	DEBUG('p',"Estre a change \n");
 	int i = th->getPriority();
 	DEBUG('p',"Prioridad del thread, %d \n", i);
 	DEBUG('p',"Nueva prioridad del thread, %d \n", priority);
	DEBUG('p',"Antes de CAMBIAR \n");
	Print();
	Thread *thread1;
	Thread *threadh;
	threadh = (readyListP[i])->Remove();
	DEBUG('p',"Nombre: %s \n",threadh->getName());	
	if(threadh == th)
	{
		 DEBUG('p', "Soy la cabeza ,%d, %d \n", i, priority);
		readyListP[priority]->Prepend(th);
	}
	else
		{
			readyListP[i]->Append(threadh);
			while((thread1 = readyListP[i]->Remove())!= threadh)
			{
				if(thread1 == th)
				{	DEBUG('p', "No soy la cabeza ,%d, %d \n", i, priority);
					readyListP[priority]->Prepend(th);
				}
				else
				 	readyListP[i]->Append(thread1);
			}
		
	}
	DEBUG('p',"Despues de CAMBIAR \n");
	Print();

}
