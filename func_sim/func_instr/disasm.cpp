#include <func_instr.h>
#include <elf_parser.h>
#include <func_memory.h>

#include <cstring>
#include <cerrno>
#include <cassert>
#include <cstdlib>

using namespace std;

int main (int argc, char** argv)
{
//argv[1] -- elf filename
//argv[2] -- section name
    if (argc > 3 || argc < 3)
    {
        cerr << "argc != 3" << endl;
        exit(EXIT_FAILURE);
    }

    uint32 start = 0;
    uint32 section_size = 0;

    vector<ElfSection> sections_array;
    ElfSection::getAllElfSections( argv[1], sections_array);

    int flag = 0;

    for ( vector<ElfSection>::iterator it = sections_array.begin(); it != sections_array.end(); ++it)
    {
        if ( !strcmp( argv[2], it->name))
        {
            flag = 1;
            start = it->start_addr;
            section_size = it->size;
            break;
        }
    }

    if (flag == 0)
    {
        cerr << "THERE IS NO SUCH SECTION" << endl;
        exit(EXIT_FAILURE);
    }

    FuncMemory func_mem( argv[1], 32, 10, 12);

    section_size += start;

    for ( int i = start; i < section_size; i += 4) {
       FuncInstr command( ( uint32) func_mem.read( i, sizeof(uint32)));
       cout << command.Dump("    ") << endl;;
    }

    return 0;
}
