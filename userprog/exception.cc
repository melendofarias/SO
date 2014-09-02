// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "user.h"
#include <stdio.h>
//#include "SynchConsole.h"

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------
void increaseProgramCounter() {
	/*
	machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg)); // Previous program counter (for debugging)
	machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg) );  //Current program counter
	machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg) +4); //Next program counter (for branch delay) 
	*/
	int pc = machine->ReadRegister(PCReg);
	machine->WriteRegister(PrevPCReg, pc);
	pc = machine->ReadRegister(NextPCReg);
	machine->WriteRegister(PCReg, pc);
	pc+=4;
	machine->WriteRegister(NextPCReg, pc);

}

/* AssingId toma un puntero a un archivo abierto OpenFile
 * y retorna un id unico por thread para el mismo (openfileId)
 */
OpenFileId
AssignId(OpenFile* file){
	int i =	 3;									//se reservan 0,1 y 2 
												//para consoleInput,Output y errores
	while (currentThread->descriptores[i])		//mientras haya algo
		i++;
	currentThread->descriptores[i]= file;
	
	return i;
}


void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

    if (which == SyscallException){
		switch (type) {
			case SC_Halt: {
				DEBUG('a', "Shutdown, initiated by user program.\n");
				interrupt->Halt();
				break;
			}
			case SC_Create: {
				int addr_name = machine->ReadRegister(4);
				char name[64];
				readStrFromUsr(addr_name, name);
				if (fileSystem->Create(name, 1024)){
					DEBUG('o', "Create, initiated by user program.\n");
				}
				else {
					DEBUG('o', "Create, an error ocurred.\n");
				} 
				machine->WriteRegister(2,0);
				
				break;
			}
			case SC_Read: {
				int addr_buff = machine->ReadRegister(4);
				int lon = machine->ReadRegister(5);
				OpenFileId id = machine->ReadRegister(6);
				int bytes=0;
				char *buffer;
				buffer = (char *)malloc(lon * sizeof (char));
						
				int i = 0;
				if(id == ConsoleInput)
				{
					DEBUG('o', "READING, initiated by user program.\n");
					for(i=0; i < lon; i++)
					{
						buffer[i] = sconsole->readConsole();
						
						if(buffer[i] == '\n')	
							break;
						bytes++;
					}
					DEBUG('o', "Bytes leidos: %d\n", bytes);
					writeBuffToUsr(buffer,addr_buff, bytes);
					
				}
				else{break;}
				machine->WriteRegister(2,bytes);				
				delete buffer;
				break;
			}
			case SC_Write: {
					int addr_buff = machine->ReadRegister(4);
					int lon = machine->ReadRegister(5);
					OpenFileId id = machine->ReadRegister(6);
					
					int bytes=0;
					char *buffer;
					buffer = (char *)malloc(lon * sizeof (char));						
					int i = 0;
					if(id == ConsoleOutput)	{
						DEBUG('o', "Writing, initiated by user program.\n");
						readBuffFromUsr(addr_buff, buffer, lon);
						
						for(i=0; i < lon; i++)
						{
							sconsole->writeConsole(buffer[i]);							
							bytes++;
						}
						DEBUG('o', "Bytes escritos: %d\n", bytes);					
					}
					else{break;}
					machine->WriteRegister(2,bytes);				
					delete buffer;
					break;
			}
			
			case SC_Open: {
				int addr_name = machine->ReadRegister(4);
				DEBUG('o', "addr_name %d\n", addr_name);
				char name[64];
				readStrFromUsr(addr_name, name);
				OpenFileId id ;
				OpenFile* file = fileSystem->Open(name); 
				if (file){
					id = AssignId(file); 
					DEBUG('o', "Open, initiated by user program.\n");
				}
				else {
					DEBUG('o', "Open, an error ocurred.\n");
				} 
				DEBUG('o', "Open, OpenFileId: %d\n", id);
				machine->WriteRegister(2,id);
				
				break;
			}
			
			case SC_Close: {
				OpenFileId id = machine->ReadRegister(4);
				DEBUG('o', "Close id------------> %d\n", id);
				
				OpenFile* file;
				file = currentThread->descriptores[id];
				if (file) {	 //si hay algo
					currentThread->descriptores[id] = NULL;
					file->~OpenFile();
					DEBUG('o', "Close, Id %d initiated by user program.\n", id);
				}
				else {
					DEBUG('o', "Close, an error ocurred (OpenFileId %d) \n", id);
				}  
				machine->WriteRegister(2,0);
				
				break;
			}
			case SC_Exec: {
				int addr_name = machine->ReadRegister(4);
				char name[128];
				readStrFromUsr(addr_name, name);
				
				Thread* execThread = new thread(name);
				
				
				int pid = system->AddProc(execThread);
				//startProcess ya esta definido en progtest.cc
				
				
				OpenFile* file = fileSystem->Open(name); 
				if (file){
					id = AssignId(file); 
					DEBUG('o', "Open, initiated by user program.\n");
				}
				else {
					DEBUG('o', "Open, an error ocurred.\n");
				} 
				DEBUG('o', "Open, OpenFileId: %d\n", id);
				machine->WriteRegister(2,id);
				
				break;
			}
		}	
	}
    else {
		printf("Unexpected user mode exception %d %d\n", which, type);
		ASSERT(false);
    }
    increaseProgramCounter();
}


