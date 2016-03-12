/**
 * A simple example of using of ports
 * @author Alexander Titov
 * Copyright 2011 MDSP team
 */

#include <iostream>
#include <cstdlib>
#include <string>

#include <perf_sim.h>

int main( int argc, char* argv[])
{
    if ( argc < 3 || argc > 4 )
    {
        std::cout << "2 or 3 arguments required: mips_exe filename, amount of instrs to run and silent mode key[-d](optional)" << endl;
        std::exit(EXIT_FAILURE);
    }

    PerfMIPS* perf_mips = new PerfMIPS();

    if ( argc == 3 )
    {
        perf_mips->run( std::string( argv[1] ), atoi( argv[2] ), true );
    }
    else
    {
        size_t index = 0;
        string arg = ( std::string ) argv[3];
        while ( index = arg.find(' ') != std::string::npos )
            arg.erase( index, 1 );

        if ( arg == "-d" )
            perf_mips->run(std::string(argv[1]), atoi(argv[2]), false );
        else
            std::exit(EXIT_FAILURE);
    }
    delete perf_mips;

    return 0;
}
