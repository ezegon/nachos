/// Routines for synchronizing threads.
///
/// Three kinds of synchronization routines are defined here: semaphores,
/// locks and condition variables (the implementation of the last two are
/// left to the reader).
///
/// Any implementation of a synchronization routine needs some primitive
/// atomic operation.  We assume Nachos is running on a uniprocessor, and
/// thus atomicity can be provided by turning off interrupts.  While
/// interrupts are disabled, no context switch can occur, and thus the
/// current thread is guaranteed to hold the CPU throughout, until interrupts
/// are reenabled.
///
/// Because some of these routines might be called with interrupts already
/// disabled (`Semaphore::V` for one), instead of turning on interrupts at
/// the end of the atomic operation, we always simply re-set the interrupt
/// state back to its original value (whether that be disabled or enabled).
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2018 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "synch.hh"
#include "system.hh"


/// Initialize a semaphore, so that it can be used for synchronization.
///
/// * `debugName` is an arbitrary name, useful for debugging.
/// * `initialValue` is the initial value of the semaphore.
Semaphore::Semaphore(const char *debugName, int initialValue)
{
    name  = debugName;
    value = initialValue;
    queue = new List<Thread *>;
}

/// De-allocate semaphore, when no longer needed.
///
/// Assume no one is still waiting on the semaphore!
Semaphore::~Semaphore()
{
    delete queue;
}

const char *
Semaphore::GetName() const
{
    return name;
}

/// Wait until semaphore `value > 0`, then decrement.
///
/// Checking the value and decrementing must be done atomically, so we need
/// to disable interrupts before checking the value.
///
/// Note that `Thread::Sleep` assumes that interrupts are disabled when it is
/// called.
void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(INT_OFF);
      // Disable interrupts.

    while (value == 0) {  // Semaphore not available.
        queue->Append(currentThread);  // So go to sleep.
        currentThread->Sleep();
    }
    value--;  // Semaphore available, consume its value.

    interrupt->SetLevel(oldLevel);  // Re-enable interrupts.
}

/// Increment semaphore value, waking up a waiter if necessary.
///
/// As with `P`, this operation must be atomic, so we need to disable
/// interrupts.  `Scheduler::ReadyToRun` assumes that threads are disabled
/// when it is called.
void
Semaphore::V()
{
    Thread   *thread;
    IntStatus oldLevel = interrupt->SetLevel(INT_OFF);

    thread = queue->Pop();
    if (thread != NULL)  // Make thread ready, consuming the `V` immediately.
        scheduler->ReadyToRun(thread);
    value++;
    interrupt->SetLevel(oldLevel);
}

/// Dummy functions -- so we can compile our later assignments.
///
/// Note -- without a correct implementation of `Condition::Wait`, the test
/// case in the network assignment will not work!

Lock::Lock(const char *debugName)
{
    mutex = new Semaphore(debugName, 1);
    mutexOwner = NULL;
}

Lock::~Lock()
{
    delete mutex;
}

const char *
Lock::GetName() const
{
    return name;
}

void
Lock::Acquire()
{
    //~ DEBUG('s',"Lock acquired\n");
    mutex->P();
    mutexOwner = currentThread;
}

void
Lock::Release()
{
    if(IsHeldByCurrentThread()){
        //~ DEBUG('s',"Lock released\n");
        mutex->V();
        mutexOwner = NULL;
    }
}

bool
Lock::IsHeldByCurrentThread() const
{
    return (currentThread == mutexOwner);
}

Condition::Condition(const char *debugName, Lock *conditionLock)
{
    mutex = conditionLock;
    name = debugName;
    queue = new List<Semaphore *>();
}

Condition::~Condition()
{
    delete queue;
}

const char *
Condition::GetName() const
{
    return name;
}

void
Condition::Wait()
{
    ASSERT(mutex->IsHeldByCurrentThread());
    
    Semaphore *sem = new Semaphore("conditionSem",0);
    queue->Append(sem);
    
    mutex->Release();
    //~ DEBUG('s',"Condition '%s' waiting.\n",this->GetName());
    sem->P();
    mutex->Acquire();
    //~ DEBUG('s',"Condition '%s' awoke.\n",this->GetName());
}

void
Condition::Signal()
{
    if(!(queue->IsEmpty()))
        (queue->Pop())->V();
}

void
Condition::Broadcast()
{
    while(!(queue->IsEmpty()))
        (queue->Pop())->V();
}

Port::Port(const char *portName)
{
    name = portName;
    mutex = new Lock("portMutex");
    waiting_send    = new Condition("sendCondition", mutex);
    waiting_receive = new Condition("receiveCondition", mutex);
    isMessage = false;
}

Port::~Port()
{
    delete waiting_send;
    delete waiting_receive;
    delete mutex;
}

void 
Port::Send(int message)
{
    mutex->Acquire();
    while(isMessage)  // Check to see if there is already a message in the Buffer
        waiting_send->Wait(); // If there is, then wait until it doesn't matter
    
    //~ DEBUG('s',"Beginning to send a message.\n");
    
    buffer = message; // Write the message in the buffer
    isMessage = true; // Set the flag
    waiting_receive->Signal(); // Wake up any thread waiting for a message in this port
    
    DEBUG('s',"A message has been sent and is awaiting confirmation.\n");
    
    waiting_send->Wait(); // Wait until the message is received
    DEBUG('s',"Message confirmed by sender.\n");
    mutex->Release();
    //~ DEBUG('s',"Sender released\n");    
}

void 
Port::Receive(int *message)
{
    mutex->Acquire();
    while(!isMessage) // Check to see if there is a message in the Buffer
        waiting_receive->Wait(); // If there are no messages, then wait for one
        
    //~ DEBUG('s',"Beginning to receive a message.\n");
    
    *message = buffer; // Copy the message to the address received
    isMessage = false; // Set the flag
    
    DEBUG('s',"A message has been received.\n");
    
    //~ DEBUG('s',"About to confirm (Signal)\n");
    waiting_send->Signal(); // Tell the sender that the message was received
    mutex->Release();
    //~ DEBUG('s',"Receiver released\n");
}
    
const char *
Port::GetName() const
{
    return name;
}
