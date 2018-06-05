#include "syscall.h"
//#include <stdio.h>
// #include <stdlib.h>

int main(int argc, char **args){
    OpenFileId new_file;
    char read_char;
    new_file = Open(args[1]);

    while(Read(&read_char, 1, new_file) != 0){
        Write(&read_char, 1, ConsoleOutput);
    }
//    else {
//	read_char = "Empty file";
//        Write(&read_char, sizeof("Empty file"), ConsoleOutput);
//    }
    //Close(new_file);
    Exit(0);
}
