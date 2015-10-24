/**
 * func_memory.cpp - the module implementing the concept of
 * programer-visible memory space accesing via memory address.
 * @author Alexander Titov <alexander.igorevich.titov@gmail.com>
 * Copyright 2012 uArchSim iLab project
 */

// Generic C
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <cassert>

// Generic C++
#include <iostream>
#include <string>
#include <sstream>

// uArchSim modules
#include <func_memory.h>

#define BITS_IN_BYTE 8
#define WORD_LENG 4

FuncMemory::FuncMemory( const char* executable_file_name,
                        uint64 addr_bits,
                        uint64 page_bits,
                        uint64 offset_bits)
{
    this->addr_size = addr_bits;
    this->page_num_size = page_bits;
    this->offset_size = offset_bits;
    this->max_sets = (uint64) 1 << ( addr_bits - page_bits
                                                 - offset_bits);
    this->max_pages = ( uint64) 1 << page_bits;
    this->max_bytes = ( uint64) 1 << offset_size;
    /*mem_size = Maximal volume of available memory*/
    this->mem_size = ( uint64) 1 << addr_bits;

    vector<ElfSection> sections_array;
    ElfSection:: getAllElfSections( executable_file_name, sections_array);

    vector<ElfSection>::const_iterator it;
    vector<ElfSection>::const_iterator ending = sections_array.end();

    array_of_sets = ( uint8***) calloc ( max_sets, sizeof(uint8**));

    if ( array_of_sets == NULL)
        {
        cerr << "ERROR_INIT: Memory allocation fault" << endl;
        exit( EXIT_FAILURE);
        }

    for ( it = sections_array.begin(); it != ending; ++it)
        {
        /*Saving of the address to the first instruction of .text section*/
        if ( strcmp( it->name, ".text") == 0)
            {
                this->start_text_addr = it->start_addr;
            }
        /*Writing content to functional memory*/
        uint64 addr = it->start_addr;

        uint64 bytes_copied = 0;

        while ( bytes_copied < ( it->size))
            {

            uint64 cur_set = getSet( addr + bytes_copied);       //
            uint64 cur_page = getPage( addr + bytes_copied);     // current parameters of set, page, offset
            uint64 cur_offset = getOffset( addr + bytes_copied); //

            if ( cur_set < 0 || cur_set >= max_sets)
                {
                cerr << "ERROR_INIT: cur_set(set pointer) is out of bounds"
                     << endl;
                exit( EXIT_FAILURE);
                }

            if ( array_of_sets[ cur_set] == NULL)
                {
                array_of_sets[ cur_set] = ( uint8**) calloc ( max_pages, sizeof(uint8*));

                if ( array_of_sets[ cur_set] == NULL)
                    {
                    cerr << "ERROR_INIT: Memory allocation fault" << endl;
                    exit( EXIT_FAILURE);
                    }
                }

            if ( array_of_sets[ cur_set][ cur_page] == NULL)
                {
                array_of_sets[ cur_set][ cur_page] = ( uint8*) calloc ( max_bytes, sizeof( uint8));

                if ( array_of_sets[ cur_set][ cur_page] == NULL)
                    {
                    cerr << "ERROR_INIT: Memory allocation fault" << endl;
                    exit( EXIT_FAILURE);
                    }
                }

            if ( ( max_bytes - cur_offset) < ( it->size - bytes_copied))
                {
                memcpy( array_of_sets[ cur_set][ cur_page] + cur_offset,
                      ( it->content) + bytes_copied, max_bytes - cur_offset);

                bytes_copied += max_bytes - cur_offset;
                } else
                {
                memcpy( array_of_sets[ cur_set][ cur_page] + cur_offset,
                      ( it->content) + bytes_copied, it->size - bytes_copied);

                bytes_copied = it->size;
                }
            }
        }
}

FuncMemory::~FuncMemory()
{
    for ( int i = 0; i < max_sets; ++i)
        {
        for ( int j = 0; j < max_pages; ++j)
            {
            free(   array_of_sets[i][j]);
            array_of_sets[i][j] = 0;
            }

        free( array_of_sets[i]);
        array_of_sets[i] = 0;
        }

   free( array_of_sets);
   array_of_sets = 0;
}

uint64 FuncMemory::getSet( uint64 addr) const
{
    uint64 set = addr >> ( this->page_num_size + this->offset_size);
    return set;
}

uint64 FuncMemory::getPage( uint64 addr) const
{
    uint64 mask = ( ( uint64) 1 << this->page_num_size) - 1;
    uint64 page = ( addr >> this->offset_size) & mask;
    return page;
}

uint64 FuncMemory::getOffset( uint64 addr) const
{
    uint64 mask = ( ( uint64) 1 << this->offset_size) - 1;
    uint64 offset = addr & mask;
    return offset;
}

uint64 FuncMemory::startPC() const
{
    return this->start_text_addr;
}

uint64 FuncMemory::read( uint64 addr, unsigned short num_of_bytes) const
{
   if ( num_of_bytes == 0)
       {
       cerr << "ERROR_READ: Invalid num_of_bytes parameter" << endl;
       abort();
       }

    uint64 value = 0;
    uint64 bytes_read = 0;

    while ( bytes_read < num_of_bytes)
        {
        uint64 cur_set = getSet( addr + bytes_read);
        uint64 cur_page = getPage( addr + bytes_read);
        uint64 cur_offset = getOffset( addr + bytes_read);

        if ( cur_offset == max_bytes)
            {
            cur_offset = 0;
            cur_page++;
            }

        if ( cur_page == max_pages)
            {
            cur_page = 0;
            cur_set++;
            }

        if ( cur_set < 0 || cur_set >= max_sets)
            {
            cerr << "ERROR_SET_READ: "
                 << "Index of the set is out of bound" << endl;
            abort();
            }

        if ( array_of_sets == NULL || array_of_sets[ cur_set] == NULL)
            {
            cerr << "ERROR_SET_READ: "
                 << "Attempt to read from uninitialized memory" << endl;
            abort();
            }

        if ( array_of_sets[ cur_set][ cur_page] == NULL)
            {
            cerr << "ERROR_PAGE_READ: "
                 << "Attempt to read from uninitialized memory" << endl;
            abort();
            }
        /*The same bit in every next byte is (2^BITS_IN_BYTE) times more sufficient*/
        value = value
                + ( (uint64) array_of_sets[ cur_set][ cur_page][ cur_offset]
                << ( BITS_IN_BYTE * ( bytes_read)));
        bytes_read++;
        }

    return value;
}

void FuncMemory::write( uint64 value, uint64 addr, unsigned short num_of_bytes)
{
    if ( num_of_bytes == 0)
       {
       cerr << "ERROR_WRITE: Invalid num_of_bytes parameter" << endl;
       abort();
       }

    uint8* temp = ( uint8*) calloc ( sizeof( uint64), sizeof( uint8**));
    memcpy( temp, &value, sizeof( uint64)); // Converts uint64 value
                                          // to temporary byte array
    uint64 bytes_wrote = 0;

    while ( bytes_wrote < num_of_bytes)
        {
        uint64 cur_set = getSet( addr + bytes_wrote);
        uint64 cur_page = getPage( addr + bytes_wrote);
        uint64 cur_offset = getOffset( addr + bytes_wrote);

        if ( cur_offset == max_bytes)
            {
            cur_offset = 0;
            cur_page++;
            }

        if ( cur_page == max_pages)
            {
            cur_page = 0;
            cur_set++;
            }

        if ( array_of_sets[ cur_set] == NULL)
            {
            array_of_sets[ cur_set] = ( uint8**) calloc ( max_pages, sizeof( uint8*));

            if ( array_of_sets[ cur_set] == NULL)
                {
                cerr << "ERROR_SET_WRITE: Memory allocation fault" << endl;
                exit( EXIT_FAILURE);
                }
            }

        if ( array_of_sets[ cur_set][ cur_page] == NULL)
            {
            array_of_sets[ cur_set][ cur_page] = ( uint8*) calloc ( max_bytes, sizeof( uint8));

            if ( array_of_sets[ cur_set][ cur_page] == NULL)
                {
                cerr << "ERROR_PAGE_WRITE: Memory allocation fault" << endl;
                exit( EXIT_FAILURE);
                }
            }

        array_of_sets[ cur_set][ cur_page][ cur_offset] = temp[ bytes_wrote];
        bytes_wrote++;
        }

    free( temp);
    temp = 0;
}

string FuncMemory::dump( string indent) const
{
    ostringstream oss;

    oss << indent << "Dump functional memory"                           << endl
        << indent << "  Parameters:"                                    << endl
        << indent << "    Address size = " << this->addr_size << " bits"<< endl
        << indent << "    Set bits = " << ( this->addr_size
                                          - this->page_num_size
                                          - this->offset_size)
        << " bits" << endl
        << indent  << "    Page bits = "    << this->page_num_size
        << " bits" << endl
        << indent  << "    Offset bits = "  << this->offset_size
        << " bits" << endl
        << indent  << "    Start .text addr = 0x" << hex
        << this->start_text_addr
        << dec     << endl
        << indent  << "    Size of memory = " << this->mem_size
        << " bytes" << endl
        << indent  << "  Content:" << endl;

    uint64 cur_offset = 0;
    uint64 cur_set = 0;
    uint64 cur_page = 0;
    bool skip_was_printed = false; //if some "00000000" have been skipped
                                   //this variable is true

    while ( cur_set < this->max_sets)
    {
        if ( !skip_was_printed)
        {/*if the set is't initialized, it is empty and hasn't to be printed*/
            if ( this->array_of_sets[ cur_set] == NULL)
            {
                cur_set++;
                cur_page = 0;
                cur_offset = 0;
                oss << indent << "  ....  " << endl;
                skip_was_printed = true;
            } else
            { /*the same rule is for pages*/
                if ( this->array_of_sets[ cur_set][ cur_page] == NULL)
                {
                    cur_page++;
                    cur_offset = 0;
                    oss << indent << "  ....  " << endl;
                    skip_was_printed = true;
                } else
                { /*and null words*/
                    string word = getWord( cur_set, cur_page, cur_offset, WORD_LENG);
                    if ( word.compare( "00000000") == 0)
                    {
                        oss << indent << "  ....  " << endl;
                        skip_was_printed = true;
                    } else
                    {
                        oss << indent << "    0x" << hex << getAddress( cur_set,
                                                                        cur_page,
                                                                        cur_offset,
                                                                        this->addr_size,
                                                                        this->page_num_size,
                                                                        this->offset_size)
                            << indent << ":    " << word << endl;

                        skip_was_printed = false;
                    }
                    cur_offset += sizeof( uint32);
                }
            }
        } else
        {
            if ( this->array_of_sets[ cur_set] != NULL &&
                 this->array_of_sets[ cur_set][ cur_page] != NULL)
            {
                string word = getWord(cur_set, cur_page, cur_offset, sizeof(uint32));
                if ( word.compare( "00000000") != 0)
                {
                    oss << indent << "    0x" << hex << getAddress( cur_set,
                                                                    cur_page,
                                                                    cur_offset,
                                                                    this->addr_size,
                                                                    this->page_num_size,
                                                                    this->offset_size)
                        << indent << ":    " << word << endl;


                    skip_was_printed = false;
                }
                cur_offset += sizeof( uint32);
            } else
            {
                if ( this->array_of_sets[ cur_set] == NULL)
                {
                    cur_set++;
                    cur_page = 0;
                    cur_offset = 0;
                } else
                {
                    if ( this->array_of_sets[ cur_set][ cur_page] == NULL)
                    {
                        cur_page++;
                        cur_offset = 0;
                    }
                }
            }
        }

        if ( cur_offset >= this->max_bytes)
        {
            cur_page += cur_offset / this->max_bytes;
            cur_offset = cur_offset % this->max_bytes;
        }
        if ( cur_page >= this->max_pages)
        {
            cur_set += cur_page / this->max_pages;
            cur_page = cur_page % this->max_pages;
        }
    }

    return oss.str();
}
/* Gets 4-byte word from pointed set, page and offset*/
string FuncMemory::getWord(uint64 set, uint64 page, uint64 offset, size_t word_leng) const
{
    ostringstream oss;
    oss << hex;

    uint64 cur_leng = 0;
    /* convert each byte into 2 hex digits*/
    while ( set < this->max_sets && cur_leng < word_leng)
    {
        oss.width( 2);
        oss.fill( '0');

        if ( this->array_of_sets[ set] != NULL &&
             this->array_of_sets[ set][ page] != NULL)
        {
            oss << (uint16) this->array_of_sets[ set][ page][ offset]; // need conversion to uint16
            offset++;
        } else
        {
            oss << "00";
        }

        cur_leng++;

        if ( offset == this->max_bytes)
            {
            offset = 0;
            page++;
            }
        if ( page == this->max_pages)
            {
            page = 0;
            set++;
            }
    }

    return oss.str();
}
/* Gets address from its components*/
uint64 FuncMemory::getAddress( uint64 set,
                               uint64 page,
                               uint64 offset,
                               uint64 address_bits,
                               uint64 page_bits,
                               uint64 offset_bits) const
{
    uint64 addr = ( set << ( page_bits + offset_bits))
                + ( page << offset_bits) + offset;
    return addr;
}
