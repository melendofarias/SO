// synch.cc 
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks 
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(const char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List<Thread*>;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    
    while (value == 0) { 			// semaphore not available
	queue->Append(currentThread);		// so go to sleep
	currentThread->Sleep();
    } 
    value--; 					// semaphore available, 
						// consume its value
    
    interrupt->SetLevel(oldLevel);		// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = queue->Remove();
    if (thread != NULL)	   // make thread ready, consuming the V immediately
	scheduler->ReadyToRun(thread);
    value++;
    interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments 
// Note -- without a correct implementation of Condition::Wait(), 
// the test case in the network assignment won't work!
Lock::Lock(const char* debugName) {
	name = debugName;
	sem = new Semaphore(name, 1);	
	semInvP = new Semaphore("AccCambioPrio",1);
	thname = NULL;
	

}

Lock::~Lock() {
	delete sem; 
}

void Lock::Acquire() {
	//control->P();
	int pcurrent;
	if(!isHeldByCurrentThread())
	{	semInvP -> P();
		if(thname != NULL)
		{
			Prioritymax = thname->getPriority();
			pcurrent = currentThread->getPriority();
			DEBUG('p', "Prioridades ,%d, %d \n", pcurrent, Prioritymax);
			if(Prioritymax < pcurrent) 
			{
				DEBUG('p', "Prioridades ,%s, %d \n",thname->getName(), pcurrent);
				
				scheduler->ChangeQueuePriority(thname,pcurrent);
				thname->setPriority(pcurrent);
				Prioritymax = pcurrent;
			}
		}	
		//thname = currentThread;
		semInvP -> V();
		
		DEBUG('p', "Prioridades  \n");
		sem -> P();
		DEBUG('p', "Sale de P \n");
		thname = currentThread;
	}
	else
	DEBUG('p', "No pude hacer Acquire  \n");
}
bool Lock::isHeldByCurrentThread(){
	return (thname == currentThread);
}	
	
void Lock::Release() {
	ASSERT(thname == currentThread);
	if (isHeldByCurrentThread()){
		thname = NULL;
		sem -> V();
	}
}
//Condition//
Condition::Condition(const char* debugName, Lock* conditionLock) { 
		name = debugName;
		lock = conditionLock;
		semList = new List<Semaphore*> ;
}

Condition::~Condition() { 
		delete lock;
		delete semList;
}

void Condition::Wait() { 
	ASSERT(lock->isHeldByCurrentThread());
	Semaphore * sem = new Semaphore(name, 1) ;
	semList -> Append(sem);
	lock -> Release();
	sem -> P();
	lock -> Acquire();	
}

void Condition::Signal() { 
	ASSERT(lock->isHeldByCurrentThread());
	Semaphore * sem;
	if ( !(semList -> IsEmpty())){
		sem = semList -> Remove(); 
		sem -> V();
	}
	//lock -> Release();
}

void Condition::Broadcast() {
	ASSERT(lock->isHeldByCurrentThread());
	while ( !(semList -> IsEmpty()))
		Signal();
}

//Puertos//
Puerto::Puerto(const char * debugName){
	access = true;
	
	pname = debugName;
	plock = new Lock(pname);
	pcondS = new Condition(pname, plock);
	pcondR = new Condition(pname, plock);
	DEBUG('p', "Se crea el puerto: \"%s\"\n", pname);
}

Puerto::~Puerto(){
	delete plock;
	delete pcondS;
	delete pcondR;
}  
		
void Puerto::Send(int msg){
	plock -> Acquire();
	while (!access){
		DEBUG('p', "esperando buffer vacio: \"%s\"\n", pname);
		pcondS-> Wait();
	}
	DEBUG('p', "grabando buffer con: \"%i\"\n", msg);
	buffer = msg;
	access = false;
	pcondR -> Signal();
	plock -> Release();
	
}
	
void Puerto::Receive(){
	plock -> Acquire();
	int buf;
	while (access){
		DEBUG('p', "esperando buffer con datos: \"%s\"\n", pname);
		pcondR-> Wait();
	}
	buf = buffer;
	DEBUG('p', "recibi el mensaje: \"%i\"\n", buf);
	
	access = true;
	pcondS-> Signal();
	plock -> Release();
}












