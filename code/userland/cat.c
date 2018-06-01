#include "syscall.h"

int main(int argc, char **args){
    OpenFileId new_file;
    char read_char;
    new_file = Open(args[1]);
    if (Read(&read_char, 1, new_file)){
        puts("");
        while(read_char != "\0"){
            printf("%c", read_char);
            if (!Read(&read_char, 1, new_file)){
                puts("Error while reading");
                return 0;
            }
        }
        puts("");
    }
    return 0;
}
