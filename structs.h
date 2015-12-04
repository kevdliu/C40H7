#ifndef STRUCTS_INCLUDED
#define STRUCTS_INCLUDED

#include <stdint.h>

/* Struct that contains all of the resources available to the UM 
(registers, memory segments, program counter etc.) */
typedef struct {
        uint32_t** segments;
        unsigned num_segments;
        uint32_t top_id;
        Seq_T free_ids;
        uint32_t *registers;
        uint32_t program_counter;        
        int run;
} *Resource;

#endif
