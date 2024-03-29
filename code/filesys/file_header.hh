/// Data structures for managing a disk file header.
///
/// A file header describes where on disk to find the data in a file, along
/// with other information about the file (for instance, its length, owner,
/// etc.)
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2018 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#ifndef NACHOS_FILESYS_FILEHDR__HH
#define NACHOS_FILESYS_FILEHDR__HH


#include "machine/disk.hh"
#include "lib/bitmap.hh"


#define NUM_DIRECT     ((SECTOR_SIZE - 2 * sizeof (int)) / sizeof (int))
#define MAX_FILE_SIZE  (NUM_DIRECT * SECTOR_SIZE)

/// The following class defines the Nachos "file header" (in UNIX terms, the
/// “i-node”), describing where on disk to find all of the data in the file.
/// The file header is organized as a simple table of pointers to data
/// blocks.
///
/// The file header data structure can be stored in memory or on disk.  When
/// it is on disk, it is stored in a single sector -- this means that we
/// assume the size of this data structure to be the same as one disk sector.
/// Without indirect addressing, this limits the maximum file length to just
/// under 4K bytes.
///
/// There is no constructor; rather the file header can be initialized
/// by allocating blocks for the file (if it is a new file), or by
/// reading it from disk.
class FileHeader {
public:

    /// Initialize a file header, including allocating space on disk for the
    /// file data.
    bool Allocate(BitMap *bitMap, unsigned fileSize);

    /// De-allocate this file's data blocks.
    void Deallocate(BitMap *bitMap);

    /// Initialize file header from disk.
    void FetchFrom(unsigned sectorNumber);

    /// Write modifications to file header back to disk.
    void WriteBack(unsigned sectorNumber);

    /// Convert a byte offset into the file to the disk sector containing the
    /// byte.
    unsigned ByteToSector(unsigned offset);

    /// Return the length of the file in bytes
    unsigned FileLength() const;

    /// Print the contents of the file.
    void Print();

  private:
    unsigned numBytes;  ///< Number of bytes in the file
    unsigned numSectors;  ///< Number of data sectors in the file
    unsigned dataSectors[NUM_DIRECT];  ///< Disk sector numbers for each data
                                       ///< block in the file.
};


#endif
