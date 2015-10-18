/**
 * func_memory.h - Header of module implementing the concept of
 * programer-visible memory space accesing via memory address.
 * @author Alexander Titov <alexander.igorevich.titov@gmail.com>
 * Copyright 2012 uArchSim iLab project
 */

// protection from multi-include
#ifndef FUNC_MEMORY__FUNC_MEMORY_H
#define FUNC_MEMORY__FUNC_MEMORY_H

// Generic C++
#include <string>
#include <cassert>

// uArchSim modules
#include <types.h>
#include <elf_parser.h>

using namespace std;

class FuncMemory
{
    // You could not create the object
    // using this default constructor
    FuncMemory(){}

public:
    uint64 start_text_addr; // address of the first instruction
                            // in ".text" section
    uint64 addr_size;       // in bits
    uint64 page_num_size;   // in bits
    uint64 offset_size;     // in bits
    size_t max_sets;        // number of sets
    size_t max_pages;       // pages in set
    size_t max_bytes;       // bytes in page
    size_t mem_size;        // in bytes

    uint8*** array_of_sets;

    FuncMemory ( const char* executable_file_name,
                 uint64 addr_size = 32,
                 uint64 page_num_size = 10,
                 uint64 offset_size = 12);

    virtual ~FuncMemory();

    uint64 getSet( uint64 addr) const;    // These functions extract set, page,
    uint64 getPage( uint64 addr) const;   // or offset, respectively,
    uint64 getOffset( uint64 addr) const; // from the given address.

    uint64 read( uint64 addr, unsigned short num_of_bytes = 4) const;
    void   write( uint64 value, uint64 addr, unsigned short num_of_bytes = 4);

    uint64 startPC() const; // Returns value of the start_text_addr field.

    string dump( string indent = "") const;// Shows a content of the functional
                                           //memory in details

    string getWord( uint64 set,
                    uint64 page,
                    uint64 offset,
                    size_t word_leng) const;

    uint64 getAddress( uint64 set,
                       uint64 page,
                       uint64 offset,
                       uint64 address_bits,
                       uint64 page_bits,
                       uint64 offset_bits) const;
};

#endif // #ifndef FUNC_MEMORY__FUNC_MEMORY_H
