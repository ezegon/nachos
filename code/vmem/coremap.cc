#include "coremap.hh"

Coremap::Coremap(int n) : BitMap(n)
{
    nItems = n;
    nextVictim = -1;
    lastVictim = 0;
}

int
Coremap::Find(AddressSpace *own, unsigned virtualPageN)
{
    int free = BitMap::Find();
    if(free == -1) {
        int victim = SelectVictim();
        ASSERT((0 <= victim) && (victim < nItems));
        free = victim;
        owner[free]->SaveToSwap(ppnToVpn[free]);
    }
    owner[free] = own;
    ppnToVpn[free] = vpn;
    return free;
}

/*
int
Coremap::SelectVictim()
{
    int i, index, lastOne = (lastVictim + NUM_PHYS_PAGES);
    TranslationEntry auxEntry;
    for(index = lastVictim + 1; index < lastOne; index++) {
        auxEntry = owner[i]->BringPage(ppnToVpn[i]);
        if(auxEntry.use)
            auxEntry.use = false;
        else if (auxEntry.dirty)
            ;
        else {
            lastVictim = i;
            return i;
        }
    }
    lastVictim = (lastVictim + 1) % NUM_PHYS_PAGES;
    ASSERT(ppnToVpn[lastVictim] >= 0);
    return lastVictim;
}*/

int
Coremap::SelectVictim()
{
    nextVictim = ++nextVictim % nItems;
    return nextVictim;
}
