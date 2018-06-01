#ifndef SYNCH_CONSOLE_HH
#define SYNCH_CONSOLE_HH

#include "threads/synch.hh"
#include "machine/console.hh"

class SynchConsole {
    public:
        SynchConsole(const char *readFile, const char *writeFile);

        ~SynchConsole();

        char GetChar();

        void PutChar(char c);

        void PutDone();

        void GetDone();

    private:
        Console   *console;
        Semaphore *readAvail;
        Semaphore *writeDone;
        Lock *readLock;
        Lock *writeLock;
};

#endif
