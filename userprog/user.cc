
#include "user.h"

void
readStrFromUsr(int usrAddr, char *outStr){
    
    int value = 1;
    int size = 1;
    int i= 0;

	machine->ReadMem( usrAddr, size, &value);
    while((char)value != '\0'){
		outStr[i] = (char)value;    
		DEBUG('q', "leiiii %c\n",  (char)value);

		i++; 
		machine->ReadMem( usrAddr+i, size, &value);
    }
    outStr[i]='\0';
}


int
readStrFromUsrSpecial(int usrAddr, char *outStr, char divide){
    
    int value = 1;
    int size = 1;
    int i=0, nextAddr;

	
	machine->ReadMem( usrAddr, size, &value);
	nextAddr = usrAddr+1;
	//limpio los caracteres separadores
    while((char)value == divide){			
			machine->ReadMem( nextAddr, size, &value);
			nextAddr++;
	}
	
    while((char)value != '\0' and (char)value != divide){
		outStr[i] = (char)value;    
		DEBUG('q', "leiiii %c\n",  (char)value);

		
		machine->ReadMem( nextAddr, size, &value);		
		i++;
		nextAddr++;
    }
    outStr[i]='\0';
    
    return nextAddr;
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
