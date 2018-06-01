/// Simple test case for the threads assignment.
///
/// Create several threads, and have them context switch back and forth
/// between themselves by calling `Thread::Yield`, to illustrate the inner
/// workings of the thread system.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2007-2009 Universidad de Las Palmas de Gran Canaria.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "system.hh"

#ifndef SEMAPHORE_TEST
    #include "synch.hh"
    Semaphore *sem;
#endif


/// Loop 10 times, yielding the CPU to another ready thread each iteration.
///
/// * `name` points to a string with a thread name, just for debugging
///   purposes.
void
SimpleThread(void *name_)
{
    // Reinterpret arg `name` as a string.
    char *name = (char *) name_;


    #ifndef SEMAPHORE_TEST
        sem->P();
    #endif
    // If the lines dealing with interrupts are commented, the code will
    // behave incorrectly, because printf execution may cause race
    // conditions.
    for (unsigned num = 0; num < 10; num++) {
        //IntStatus oldLevel = interrupt->SetLevel(IntOff);
        printf("*** Thread `%s` is running: iteration %u\n", name, num);
        //interrupt->SetLevel(oldLevel);
        currentThread->Yield();
    }
    
    #ifndef SEMAPHORE_TEST
        sem->V();
    #endif
    
    //IntStatus oldLevel = interrupt->SetLevel(IntOff);
    printf("!!! Thread `%s` has finished\n", name);
    //interrupt->SetLevel(oldLevel);
}

/// Set up a ping-pong between several threads.
///
/// Do it by launching ten threads which call `SimpleThread`, and finally
/// calling `SimpleThread` ourselves.
void
ThreadTest()
{
    DEBUG('t', "Entering thread test\n");    
    
    #ifndef SEMAPHORE_TEST
        sem = new Semaphore("semTest", 3);
    #endif
    
    Thread *threadList[5];
    for(int i = 0; i < 5; i++)
    {
        char *name = new char[64];
        sprintf(name, "No %d", i+1);
        threadList[i] = new Thread(name);
        threadList[i]->Fork(SimpleThread, (void *) name);
    }
}
