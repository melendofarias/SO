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

//defino la tabla de procesos
Thread* processIdTable[maxCantProcess];

//defino una variable que controle la 
//cantidad de procesos actuales en el sistema
int cantProcesses=1;		//siempre está corriendo un main principal

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
StartProcess(void* name)
{
	DEBUG('o', "StartProcess..currentThread %d pid %d\n", currentThread, currentThread->pid);
	
	currentThread->space->InitRegisters();		// set the initial register values
    
    currentThread->space->RestoreState();		// load page table register
	currentThread->CheckOverflow();
    
    machine->Run();			// jump to the user progam
    ASSERT(false);			// machine->Run never returns;
					// the address space exits
					// by doing the syscall "exit"
}

void
do_Exec(int pid, OpenFile* executable, char* filename){
	Thread* execThread = new Thread(filename);
	execThread->pid = pid;
	processIdTable[pid] = execThread;
	DEBUG('o', "Exec ---------> pid: %d Current %d filename: %s \n", pid, currentThread->pid, filename);
		
	//aloco espacio	
	AddrSpace* space = new AddrSpace(executable);    
	execThread->space = space;
	delete executable;

	cantProcesses++;

	//realizo el fork para dejarlo listo a ejecutar
	execThread->Fork(StartProcess,(void *)executable, 0,0);
	
}

void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);
//DEBUG('o',"entro al exceptionHandler %d\n", currentThread);
    if (which == SyscallException)
    {
		switch (type) {
			case SC_Halt: {
				DEBUG('a', "Shutdown, initiated by user program.\n");
				int i, j;
				for(i=0; i<maxCantProcess; i++){				
					for(j=0; j<MAX_OPEN_FILES; j++)
						delete processIdTable[i];
					
					if (processIdTable[i] != currentThread and processIdTable[i] != NULL)
						delete processIdTable[i];	//borro el thread
						
					processIdTable[i] = NULL;	//limpio la tabla
				}		
				if(currentThread->space != NULL){
					delete currentThread->space;
					currentThread->space = NULL;
				}
				
				interrupt->Halt();
				break;
			}
			case SC_Create: {
				int addr_name = machine->ReadRegister(4);
				char name[64];
				readStrFromUsr(addr_name, name);
				if (fileSystem->Create(name, 1024)){
					DEBUG('o', "Create, initiated by user program: %s.\n", name);
					DEBUG('o', "------------------------\n SISCALL CREATE currentThread %d \n", currentThread);
				}
				else {
					DEBUG('o', "Create, an error ocurred: %s.\n", name);
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
				else{
						ASSERT(id>=3 and id <= MAX_OPEN_FILES);
					
					if (currentThread->descriptores[id]){				
					  
						DEBUG('o', "Bytes leidos: %d %s\n", bytes, buffer);	
						bytes=(currentThread->descriptores[id])->Read(buffer,lon);
								
						DEBUG('o', "Bytes leidos: %d %s\n", bytes, buffer);					
					}
					else{
						printf("ID de openfile inexistente");
						ASSERT(false);
						}
				
					}
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
					
				readBuffFromUsr(addr_buff, buffer, lon);
				if(id == ConsoleOutput)	{
						
					DEBUG('o', "Writing, initiated by user program.\n");
											
					for(i=0; i < lon; i++)
					{
						sconsole->writeConsole(buffer[i]);							
						bytes++;
					}
					DEBUG('o', "Bytes escritos: %d\n", bytes);					
				}
				else{
					ASSERT(id>=3 and id <= MAX_OPEN_FILES);
					
					if (currentThread->descriptores[id]){				
							
						bytes=(currentThread->descriptores[id])->Write(buffer,lon);
								
						DEBUG('o', "Bytes escritos a archivo!: %d\n", bytes);					
					}
					else{
						printf("ID de openfile inexistente");
						ASSERT(false);
					}
				}
				
					
				machine->WriteRegister(2,bytes);				
					
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
					DEBUG('o', "Open, initiated by user program: %s.\n", name);
				}
				else {
					DEBUG('o', "Open, an error ocurred: %s.\n", name);
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
				DEBUG('o', "CURRENTTHREAD %d\n", currentThread);
				machine->WriteRegister(2,0);
				
				break;
			}
			case SC_Exec: {
				
				DEBUG('o', "------------------------\n SISCALL EXEC currentThread %d \n", currentThread);
				int addr_name = machine->ReadRegister(4);
				int argc = machine->ReadRegister(5);
				int addr_argv = machine->ReadRegister(6);
				
				char filename[128];
				readStrFromUsr(addr_name, filename);
				
				char argumento[argc][128];
				int i;
				int value = 1;
				int tam = 0;
				for (i=0; i<argc ; i++){
					//machine->ReadMem( addr_argv +i*4, 4, &value);

					readStrFromUsr(addr_argv + tam*4, argumento[i]);
					tam = tam + strlen(argumento[i]);
					DEBUG('q', "argc %d argumento %s value %d  tam %d\n", argc, argumento[i], value, tam);
				}
				
				
				
				//abro el archivo a ejecutar
				OpenFile *executable = fileSystem->Open(filename);

				if (executable == NULL) {
				printf("Exec: Unable to open file %s\n", filename);
				ASSERT(false);
				}

				processIdTable[currentThread->pid]=currentThread;
				//guardo el nuevo threads en la tabla de procesos
				//asigno un pid
				int pid=1;
				while(processIdTable[pid] != NULL)
					pid++;
						
				if (pid < maxCantProcess){
					machine->WriteRegister(2,pid);
					do_Exec(pid, executable, filename);
				
				}
				else{
					printf("Exec: Unable to get an Identifier of process(PID): %s\n", filename);
					ASSERT(false);
				}
				
				
				DEBUG('o', "LASTExec ---------> pid: %d Current %d \n", pid, currentThread->pid);
				
				
				break;
			}
			case SC_Join:{
				int pid = machine->ReadRegister(4);
				
				processIdTable[pid]->Join();
				processIdTable[pid]=NULL;
				machine->WriteRegister(2, 0);
				
				break;
			}			
			case SC_Exit: {
				int estado = machine->ReadRegister(4);
				cantProcesses--;
				DEBUG('o', "Exit status: %d ...currentThread %d QUEDAN %d\n", estado, currentThread, cantProcesses);
				
				if (estado==0 ) {
					printf("Exit: el proceso finalizó con normalidad\n");
				}else{
					printf("Exit: el proceso finalizó con ERRORES\n");
				}
				
				
				if (cantProcesses<=0){
						interrupt->Halt();
					}
				if((currentThread->space!=NULL)){
					delete currentThread->space;
					currentThread->space = NULL;
					processIdTable[currentThread->pid]=NULL;														
				}
				DEBUG('o', "Exit FIN\n");
								
				currentThread->Finish();
				break;
			}
		}	
	}
    else {
		printf("Unexpected user mode exception %d %d (SyscallException: %d)\n", which, type, SyscallException);
		ASSERT(false);
    }
    increaseProgramCounter();
}


