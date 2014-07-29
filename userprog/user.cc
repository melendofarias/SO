
#include "user.h"

void
readStrFromUsr(int usrAddr, char *outStr){
    
    int *value = 1;
    int size = 1;
    int i= 0;
    
	machine->ReadMem( usrAddr, size, value);
    while((char)value != '\0'){
		outStr[i] = (char)value;
		machine->ReadMem( usrAddr, size, value);
		i++; 
    }
    
}
