/// Entry point into the Nachos kernel from user programs.
///
/// There are two kinds of things that can cause control to transfer back to
/// here from user code:
///
/// * System calls: the user code explicitly requests to call a procedure in
///   the Nachos kernel.  Right now, the only function we support is `Halt`.
///
/// * Exceptions: the user code does something that the CPU cannot handle.
///   For instance, accessing memory that does not exist, arithmetic errors,
///   etc.
///
/// Interrupts (which can also cause control to transfer from user code into
/// the Nachos kernel) are handled elsewhere.
///
/// For now, this only handles the `Halt` system call.  Everything else core
/// dumps.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#include <string.h>
#include "syscall.h"
#include "args.cc"
#include "threads/system.hh"

void IncreasePC();
void InitUserProc(void *);


/// Entry point into the Nachos kernel.  Called when a user program is
/// executing, and either does a syscall, or generates an addressing or
/// arithmetic exception.
///
/// For system calls, the following is the calling convention:
///
/// * system call code in `r2`;
/// * 1st argument in `r4`;
/// * 2nd argument in `r5`;
/// * 3rd argument in `r6`;
/// * 4th argument in `r7`;
/// * the result of the system call, if any, must be put back into `r2`.
///
/// And do not forget to increment the pc before returning. (Or else you will
/// loop making the same system call forever!)
///
/// * `which` is the kind of exception.  The list of possible exceptions is
///   in `machine.hh`.
void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

    if (which == SYSCALL_EXCEPTION) {
        switch (type)
        {
            case SC_Halt:
                DEBUG('e', "Shutdown, initiated by user program.\n");
                interrupt->Halt();
                break;

            case SC_Create:
                {
                    int fname = machine->ReadRegister(4);
                    char name[256];
                    ASSERT(fname != NULL);
                    ReadStringFromUser(fname, name, 256);
                    DEBUG('e',"Creating file with name: %s\n",name);
                    fileSystem->Create(name,0);
                    IncreasePC();
                    break;
                }

            case SC_Read:
                {
                    int from = machine->ReadRegister(4);
                    int size = machine->ReadRegister(5);
                    OpenFileId id = machine->ReadRegister(6);
                    char *buffer = new char[size];
                    int readLength = 0;
                    DEBUG('e',"Reading file with id %d\n", id);
                    if(id == ConsoleInput) {
                        unsigned i;
                        for(i = 0; i < size; i++)
                        {
                            buffer[i] = synchConsole->GetChar();
                        }
                        WriteBufferToUser((const char*)buffer, from, size);
                        readLength = i;
                    } else {
                        OpenFile *f = currentThread->GetFile(id);
                        if(f != NULL) {
                            readLength = f->Read(buffer, size);
                            WriteBufferToUser((const char*)buffer, from, size);
                        }
                    }

                    machine->WriteRegister(2, readLength);
                    IncreasePC();
                    break;
                }

            case SC_Write:
                {
                    int from = machine->ReadRegister(4);
                    int size = machine->ReadRegister(5);
                    OpenFileId id = machine->ReadRegister(6);
                    int writeLength = 0;
                    char buffer[size];
                    DEBUG('e',"Writing file with id: %d\n",id);
                    if(id == ConsoleOutput) {
                        unsigned i;
                        ReadBufferFromUser(from, buffer, size);
                        for(i = 0; i < size; i++)
                        {
                            synchConsole->PutChar(buffer[i]);
                        }
                        writeLength = i;
                    } else {
                        OpenFile *f = currentThread->GetFile(id);
                        if(f != NULL) {
                            ReadBufferFromUser(from, buffer, size);
                            writeLength = f->Write((const char*)buffer, size);
                        }
                    }
                    machine->WriteRegister(2, writeLength);
                    IncreasePC();
                    break;
                }

            case SC_Open:
                {
                    int pname = machine->ReadRegister(4);
                    char name[256];
                    ReadStringFromUser(pname, name, 256);
                    OpenFile *file = fileSystem->Open(name);
                    OpenFileId fid = -1;
                    DEBUG('e', "Opening file %s\n", name);
                    if(file != NULL) {
                        fid = currentThread->AddFile(file);
                        if(fid < 0)
                            delete file;
                    }
                    machine->WriteRegister(2, fid);
                    IncreasePC();
                    break;
                }

            case SC_Close:
                {
                    int fid = machine->ReadRegister(4);
                    currentThread->RemoveFile(fid);
                    IncreasePC();
                    break;
                }

             case SC_Exit:
                {
                    int status = machine->ReadRegister(4);
                    //currentThread->RemoveAllFiles();
                    currentThread->Finish(status);
                    IncreasePC();
                    break;
                }

            case SC_Join:
                {
                    SpaceId pid = machine->ReadRegister(4);
                    Thread *thread = pTable->Get(pid);
                    thread->Join();
                    machine->WriteRegister(2, 0);
                    IncreasePC();
                    break;
                }

            case SC_Exec:
                {
                    int pname = machine->ReadRegister(4);
                    int pargs = machine->ReadRegister(5);
                    int joinable = machine->ReadRegister(6);
                    char name[256];
                    ASSERT(name != NULL);
                    ReadStringFromUser(pname, name, 256);
                    DEBUG('e',"Excuting program: %s\n", name);
                    OpenFile *file = fileSystem->Open(name);
                    SpaceId sid = 0;
                    ASSERT(file != NULL);
                    if(joinable)
                        Thread *t = new Thread(strdup(name),true);
                    else
                        Thread *t = new Thread(strdup(name));
                    AddressSpace *as = new AddressSpace(file);
                    t->space = as;
                    sid = t->GetSid();
                    t->Fork(InitUserProc, SaveArgs(pargs));
                    machine->WriteRegister(2, sid);
                    delete file;
                    IncreasePC();
                    break;
                }

            default:
                {
                    printf("Unkown syscall %d\n",type);
                    ASSERT(false);
                }
        }
    } else {
        printf("Unexpected user mode exception %d %d\n", which, type);
        printf("%d\n", SYSCALL_EXCEPTION);
        ASSERT(false);
    }
}

void IncreasePC()
{
    int pc = machine->ReadRegister(PC_REG);
    machine->WriteRegister(PREV_PC_REG, pc);
    pc = machine->ReadRegister(NEXT_PC_REG);
    machine->WriteRegister(PC_REG, pc);
    pc += 4;
    machine->WriteRegister(NEXT_PC_REG, pc);
}

void InitUserProc(void *args)
{
    currentThread->space->InitRegisters();
    currentThread->space->RestoreState();

    int argc = WriteArgs((char **) args);
    int argv = (machine->ReadRegister(STACK_REG)) + 16;
    machine->WriteRegister(4, argc);
    char buffer[512];
    int vAddr;
    machine->ReadMem(argv, 4, &vAddr);
    ReadStringFromUser(vAddr, buffer, 512);

    machine->WriteRegister(5, argv);
    machine->Run();
}
