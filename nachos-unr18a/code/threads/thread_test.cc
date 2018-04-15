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
#include "synch.hh"

Port main_port("main_port");


/// Loop 10 times, yielding the CPU to another ready thread each iteration.
///
/// * `name` points to a string with a thread name, just for debugging
///   purposes.
void
SimpleThread_receiver(void *name_)
{
    // Reinterpret arg `name` as a string.
    char *name = (char *) name_;

    int message = 0;

    // If the lines dealing with interrupts are commented, the code will
    // behave incorrectly, because printf execution may cause race
    // conditions.
    for (unsigned num = 0; num < 10; num++) {

        //IntStatus oldLevel = interrupt->SetLevel(IntOff);
        DEBUG('s',"Thread '%s' is about to receive a message.\n", name);
        main_port.Receive(&message);

        printf("*** Thread `%s` is running: iteration %u. Received message: '%d'\n", name, num, message);

        //interrupt->SetLevel(oldLevel);
        currentThread->Yield();
    }
    //IntStatus oldLevel = interrupt->SetLevel(IntOff);
    printf("!!! Thread `%s` has finished\n", name);
    //interrupt->SetLevel(oldLevel);
}

void
SimpleThread_emiter(void *name_)
{
    // Reinterpret arg `name` as a string.
    char *name = (char *) name_;

    // If the lines dealing with interrupts are commented, the code will
    // behave incorrectly, because printf execution may cause race
    // conditions.
    for (unsigned num = 0; num < 10; num++) {

        DEBUG('s',"Thread '%s' is about to send a message.\n", name);
        main_port.Send(num);
        
        //IntStatus oldLevel = interrupt->SetLevel(IntOff);
        printf("*** Thread `%s` is running: iteration %u. Sent message: '%d'\n", name, num, num);

        //interrupt->SetLevel(oldLevel);
        currentThread->Yield();
    }
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
    
    char *name = new char[64];
    strncpy(name, "2nd", 64);
    Thread *firstThread = new Thread(name);
    Thread *secondThread = new Thread(name);
    firstThread->Fork(SimpleThread_emiter, (void *) name);
    secondThread->Fork(SimpleThread_receiver, (void *) "1st");
    firstThread->Join();
    puts("I'm happy waiting. :)");
}
