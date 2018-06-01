#include "synch_console.hh"

static void
ReadAvail(void *arg)
{
    ((SynchConsole *)arg)->GetDone();
}

static void
WriteDone(void *arg)
{
    ((SynchConsole *)arg)->PutDone();
}

SynchConsole::SynchConsole(const char *readFile, const char *writeFile)
{
    readAvail = new Semaphore("Console readAvail", 0);
    writeDone = new Semaphore("Console writeDone", 0);
    readLock = new Lock("Console read");
    writeLock = new Lock("Console write");
    console = new Console(readFile, writeFile, ReadAvail, WriteDone, this);
}

SynchConsole::~SynchConsole()
{
    delete console;
    delete writeLock;
    delete readLock;
    delete writeDone;
    delete readAvail;
}

char
SynchConsole::GetChar()
{
    readLock->Acquire();
    readAvail->P();
    char ret = console->GetChar();
    readLock->Release();
    return ret;
}

void
SynchConsole::PutChar(char c)
{
    writeLock->Acquire();
    console->PutChar(c);
    writeDone->P();
    writeLock->Release();
}

void
SynchConsole::PutDone()
{
    writeDone->V();
}

void
SynchConsole::GetDone()
{
    readAvail->V();
}
