#ifndef COREMAP_HH
#define COREMAP_HH

#include "userprog/address_space.hh";
#include "machine/machine.hh";
#include "lib/table.hh";
#include "lib/bitmap.hh";

class Coremap: public BitMap
{
public:
    Coremap(int n);

    int Find(AddressSpace *addr, unsigned i);

    void SetDirty(int page);

    void SetUsed(int page);

private:
    int SelectVictim();

    Table<AddressSpace*> *owner;

    int nextVictim;

    int lastVictim;

    int nItems;
}

#endif
