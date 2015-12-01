#ifndef STRUCTS_INCLUDED
#define STRUCTS_INCLUDED

#include <stdint.h>
#include "seq.h"
#include "table.h"

/* Struct that contains all of the resources available to the UM 
(registers, memory segments, program counter etc.) */
typedef struct {
        Table_T segments;
        Seq_T free_ids;
        uint32_t *registers;
        uint32_t program_counter;        
        uint32_t top_id;
        int run;
} *Resource;

/* Struct that contains the information necessary to execute the 
instruction (the operation code and the three registers) */
typedef struct {
    int reg_A;
    int reg_B;
    int reg_C;
    int opcode;
} *Instruction;

#endif
