#include "syscall.h"

int main(int argc, char **argv){
    char *name = argv[1];
    Create(name);
    Exit(0);
}
