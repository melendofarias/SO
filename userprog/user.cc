
#include "user.h"

void
readStrFromUsr(int usrAddr, char *outStr){
    
    int value = 1;
    int size = 1;
    int i= 0;

	machine->ReadMem( usrAddr, size, &value);
    while((char)value != '\0'){
		outStr[i] = (char)value;    DEBUG('q', "leiiii %c\n",  (char)value);

		i++; 
		machine->ReadMem( usrAddr+i, size, &value);
    }
    
}

void
readBuffFromUsr(int usrAddr, char *outBuff, int byteCount){
    
    int value = 1;
    int size = 1;
    int i= 0;
    
    while( i < byteCount){
		machine->ReadMem( usrAddr +i, size, &value);
		outBuff[i] = (char)value;
		i++; 
    }
    
}

void
writeStrToUsr(char *str, int usrAddr){
    
    int value = 1;
    int size = 1;
    int i= 0;
    
    while(str[i] != '\0'){
		value = (int) str[i];
		machine->WriteMem( usrAddr + i, size, value);
		i++; 
    }
    
}

void
writeBuffToUsr(char *buff, int usrAddr, int byteCount){
    
    int value = 1;
    int size = 1;
    int i= 0;
    
    while( i < byteCount){
		value = (int) buff[i];
		machine->WriteMem( usrAddr + i, size, value);
		i++; 
    }
    
}
