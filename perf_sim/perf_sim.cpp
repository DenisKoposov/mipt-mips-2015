#include <iostream>

#include <perf_sim.h>

#define PORT_LATENCY 1
#define PORT_FANOUT 1
#define PORT_BW 1
#define INSTR_SIZE 4

PerfMIPS::PerfMIPS() : decode_instr( 0),
                       execute_instr( 0),
                       memory_instr( 0),
                       writeback_instr( 0)
{
    rf = new RF();
    //-----------------------------------Initialization of Data Ports-----------------------
    //--------Fetch <-> Decode--------
    //--------- Data Ports------------
    from_fetch = new WritePort<uint32>( "FETCH_DECODE", PORT_BW, PORT_FANOUT );
    to_decode   = new ReadPort<uint32>( "FETCH_DECODE", PORT_LATENCY );
    //----------Stall ports-----------
    from_decode_stall = new WritePort<bool>( "FETCH_DECODE_STALL", PORT_BW, PORT_FANOUT );
    to_fetch_stall     = new ReadPort<bool>( "FETCH_DECODE_STALL", PORT_LATENCY );
    //-------Decode <-> Execute-------
    //----------Data Ports------------
    from_decode = new WritePort<FuncInstr>( "DECODE_EXECUTE", PORT_BW, PORT_FANOUT );
    to_execute   = new ReadPort<FuncInstr>( "DECODE_EXECUTE", PORT_LATENCY );
    //---------Stall ports------------
    from_execute_stall = new WritePort<bool>( "DECODE_EXECUTE_STALL", PORT_BW, PORT_FANOUT );
    to_decode_stall     = new ReadPort<bool>( "DECODE_EXECUTE_STALL", PORT_LATENCY );
    //---------Execute <-> Memory-----
    // Data Ports
    from_execute = new WritePort<FuncInstr>( "EXECUTE_MEMORY", PORT_BW, PORT_FANOUT );
    to_memory     = new ReadPort<FuncInstr>( "EXECUTE_MEMORY", PORT_LATENCY );
    // Stall ports
    from_memory_stall = new WritePort<bool>( "EXECUTE_MEMORY_STALL", PORT_BW, PORT_FANOUT );
    to_execute_stall   = new ReadPort<bool>( "EXECUTE_MEMORY_STALL", PORT_LATENCY );
    // Memory <-> Writeback
    // Data Ports
    from_memory  = new WritePort<FuncInstr>( "MEMORY_WRITEBACK", PORT_BW, PORT_FANOUT );
    to_writeback  = new ReadPort<FuncInstr>( "MEMORY_WRITEBACK", PORT_LATENCY );
    // Stall ports
    from_writeback_stall = new WritePort<bool>( "MEMORY_WRITEBACK_STALL", PORT_BW, PORT_FANOUT );
    to_memory_stall       = new ReadPort<bool>( "MEMORY_WRITEBACK_STALL", PORT_LATENCY );

    Port<FuncInstr>::init();
    Port<bool>::init();
    Port<uint32>::init();
    //--------------------------------------------------------------------------------------
}
//============FETCH=======================ok
void PerfMIPS::clock_fetch( size_t cycle )
{
    bool is_stall = false;
    to_fetch_stall->read( &is_stall, cycle );
    to_fetch_stall->lost( cycle );

    if ( is_stall || !PC_validity_bit )
    {
        fetch_raw = 0;
        return;
    }

    fetch_raw = fetch();
    from_fetch->write( fetch_raw, cycle );
}
//============DECODE======================
void PerfMIPS::clock_decode( size_t cycle )
{
    bool is_stall = false;
    to_decode_stall->read( &is_stall, cycle );
    to_decode_stall->lost( cycle );

    if ( is_stall )
    {
        from_decode_stall->write( true, cycle );
        decode_instr = FuncInstr( 0 );
        return;
    }

    to_decode->lost( cycle );
    got_data = to_decode->read( &decode_raw, cycle );
    decode_instr = FuncInstr( decode_raw, PC );

    if ( !got_data && !ready )
    {
        decode_instr = FuncInstr( 0 );
        return;
    }

    if ( rf->check( decode_instr.get_src1_num() ) == false ||
         rf->check( decode_instr.get_src2_num() ) == false )
    {
        from_decode_stall->write( true, cycle );
        decode_instr = FuncInstr( 0 );
        ready = true;
        return;
    }

    ready = false;
    rf->invalidate( decode_instr.get_dst_num() );
    read_src( decode_instr );
    from_decode->write( decode_instr, cycle );
    decode_raw = 0;

    if ( decode_instr.is_jump() )
    {
        PC_validity_bit = false;
        return;
    }

    PC += increment;
}
//=============EXECUTE====================
void PerfMIPS::clock_execute( size_t cycle )
{
    bool is_stall = false;
    to_execute_stall->read( &is_stall, cycle );
    to_execute_stall->lost( cycle );

    if ( is_stall )
    {
        from_execute_stall->write( true, cycle );
        execute_instr = FuncInstr( 0 );
        return;
    }

    to_execute->lost( cycle );

    if ( !to_execute->read( &execute_instr, cycle )  )
    {
        execute_instr = FuncInstr( 0 );
        return;
    }

    execute_instr.execute();
    from_execute->write( execute_instr, cycle );
}
//=============MEMORY=====================
void PerfMIPS::clock_memory( size_t cycle )
{
    bool is_stall = false;
    to_memory_stall->read( &is_stall, cycle );
    to_memory_stall->lost( cycle );

    if ( is_stall )
    {
        from_memory_stall->write( true, cycle );
        memory_instr = FuncInstr( 0 );
        return;
    }

    to_memory->lost( cycle );

    if ( !to_memory->read( &memory_instr, cycle ) )
    {
        memory_instr = FuncInstr( 0 );
        return;
    }

    load_store( memory_instr );
    from_memory->write( memory_instr, cycle );
}
//=============Writeback===================
void PerfMIPS::clock_writeback( size_t cycle )
{
    to_writeback->lost( cycle );
    if ( !to_writeback->read( &writeback_instr, cycle ) )
    {
        writeback_instr = FuncInstr( 0 );
        return;
    }

    wb( writeback_instr );

    if ( writeback_instr.is_jump() )
    {
        PC = writeback_instr.get_new_PC();
        PC_validity_bit = true;
    }

    executed_instrs++;
}
//========================================
void PerfMIPS::run( const std::string& tr, uint32 instrs_to_run, bool silent )
{
    mem = new FuncMemory( tr.c_str() );
    PC = mem->startPC();
    executed_instrs = 0;
    cycle = 1;
    fetch_raw = 0;
    decode_raw = 0;
    increment = 4;
    PC_validity_bit = true;
    ready = false;
    got_data = false;

    while ( executed_instrs < instrs_to_run )
    {
        clock_writeback( cycle ); // each instruction writeback increases executed_instrs variable
        clock_decode( cycle );
        clock_fetch( cycle );
        clock_execute( cycle );
        clock_memory( cycle );

        if ( !silent )
            std::cout << "fetch\t\tcycle "<< std::dec << cycle << ": 0x" << std::hex << fetch_raw << endl
            << "decode\t\tcycle "<< std::dec << cycle << ": " << std::hex << decode_instr << endl
            << "execute\t\tcycle "<< std::dec << cycle << ": " << std::hex << execute_instr << endl
            << "memory\t\tcycle "<< std::dec << cycle << ": " << std::hex << memory_instr << endl
            << "writeback\tcycle "<< std::dec << cycle << ": " << std::hex << writeback_instr<< endl << endl;
        else
            if ( ( writeback_instr.is_jump() ) ||
                 ( writeback_instr.get_src1_num() != REG_NUM_ZERO ) ||
                 ( writeback_instr.get_src2_num() != REG_NUM_ZERO ) ||
                 ( writeback_instr.get_dst_num()  != REG_NUM_ZERO ) )
            std::cout << writeback_instr << endl;

        ++cycle;
    }
    delete mem;
}

PerfMIPS::~PerfMIPS() {
    delete rf;
}
