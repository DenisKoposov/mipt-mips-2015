/**
 * func_sim.cpp - mips single-cycle simulator
 * @author Pavel Kryukov pavel.kryukov@phystech.edu
 * Copyright 2015 MIPT-MIPS
 */

#ifndef PERF_SIM_H
#define PERF_SIM_H

#include <func_instr.h>
#include <func_memory.h>
#include <ports.h>
#include <rf.h>
#include <log.h>

class PerfMIPS
{
    private:

        RF* rf;
        uint32 PC;
        bool PC_validity_bit;
        FuncMemory* mem;
        size_t cycle;
        size_t executed_instrs;
        size_t increment;
        //----------Ports-----------------
        //------- Fetch <-> Decode--------
        //--------- Data Ports------------
        WritePort<uint32>* from_fetch;
        ReadPort<uint32>*  to_decode;
        //----------Stall ports-----------
        WritePort<bool>* from_decode_stall;
        ReadPort<bool>*  to_fetch_stall;
        //-------Decode <-> Execute-------
        //----------Data Ports------------
        WritePort<FuncInstr>* from_decode;
        ReadPort<FuncInstr>* to_execute;
        //---------Stall ports------------
        WritePort<bool>* from_execute_stall;
        ReadPort<bool>*  to_decode_stall;
        //---------Execute <-> Memory-----
        // Data Ports
        WritePort<FuncInstr>* from_execute;
        ReadPort<FuncInstr>*  to_memory;
        // Stall ports
        WritePort<bool>* from_memory_stall;
        ReadPort<bool>*  to_execute_stall;
        // Memory <-> Writeback
        // Data Ports
        WritePort<FuncInstr>* from_memory;
        ReadPort<FuncInstr>*  to_writeback;
        // Stall ports
        WritePort<bool>* from_writeback_stall;
        ReadPort<bool>*  to_memory_stall;
        //----------Clocks----------------
        void clock_fetch( size_t cycle );
        void clock_decode( size_t cycle );
        void clock_execute( size_t cycle );
        void clock_memory( size_t cycle );
        void clock_writeback( size_t cycle );
        //---------Internal Data----------
        uint32 fetch_raw;
        uint32 decode_raw;
        FuncInstr decode_instr;
        bool got_data;
        bool ready;
        FuncInstr execute_instr;
        FuncInstr memory_instr;
        FuncInstr writeback_instr;
        //--------------------------------
        uint32 fetch() const
        {
            return mem->read( PC );
        }

        void read_src( FuncInstr& instr ) const
        {
            rf->read_src1( instr );
            rf->read_src2( instr );
	    }

        void load( FuncInstr& instr ) const
        {
            instr.set_v_dst( mem->read( instr.get_mem_addr(), instr.get_mem_size() ) );
        }

        void store( const FuncInstr& instr )
        {
            mem->write( instr.get_v_src2(), instr.get_mem_addr(), instr.get_mem_size() );
        }

	    void load_store( FuncInstr& instr )
	    {
            if ( instr.is_load() )
                load( instr );
            else if ( instr.is_store() )
                store( instr );
        }

        void wb( const FuncInstr& instr )
        {
            rf->write_dst( instr );
        }

    public:
        PerfMIPS();
        void run( const std::string& tr, uint32 instrs_to_run, bool silent );
        ~PerfMIPS();
};

#endif //PERF_SIM_H

