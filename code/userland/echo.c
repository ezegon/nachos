#include "syscall.h"

int main(int argc, char **argv)
{
    int i;
    char *text = argv[1];
    for(i = 0; text[i] != '\0'; i++)
    {
        Write(text + i, 1, ConsoleOutput);
    }
    Exit(0);
}
