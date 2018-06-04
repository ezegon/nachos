#include "syscall.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **args){
    OpenFileId new_file;
    char read_char;
    new_file = Open(args[1]);
    if (Read(&read_char, 1, new_file)){
        Write("\n",1,ConsoleOutput);
        while(read_char != "\0"){
            Write(&read_char, 1, ConsoleOutput);
            if (!Read(&read_char, 1, new_file)){
                return 0;
            }
        }
    }
    else {
        Write("Empty file", sizeof("Empty file"), ConsoleOutput);
    }
    return 0;
}
