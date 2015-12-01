#ifndef PROGRAM_EXECUTOR_INCLUDED
#define PROGRAM_EXECUTOR_INCLUDED

#include "structs.h"
#include <stdint.h>

/*
* Extracts the opcode and register values from each instruction word using the 
* bitwise operations of Bitpack; calls the function chooser using the newly 
* extracted values
* Arguments: The Resource struct 'res', a uint32_t instruction word that
* contains the values to be extracted
* Returns: nothing
*/
extern void decode_instruction(Resource res, uint32_t word);

/*
* Takes the opcode and assigns it to the appropriate instruction
* Arguments: The Resource struct 'res' to pass to the instruction function, the
* Instruction struct 'instr' with the opcode and the registers, and an 
* extracted value for when the opcode is 13
* Returns: nothing
*/
extern void function_chooser(Resource res, Instruction instr, uint32_t val);

extern void cond_move(Resource res, Instruction instr);

extern void seg_load(Resource res, Instruction instr);

extern void seg_store(Resource res, Instruction instr);

extern void add(Resource res, Instruction instr);

extern void multiply(Resource res, Instruction instr);

extern void division(Resource res, Instruction instr);

extern void bitwise_nand(Resource res, Instruction instr);

extern void halt(Resource res);

extern void map_seg(Resource res, Instruction instr);

extern void unmap_seg(Resource res, Instruction instr);

extern void output(Resource res, Instruction instr);

extern void input(Resource res, Instruction instr);

extern void load_program(Resource res, Instruction instr);

extern void load_val(Resource res, Instruction instr, uint32_t val);

#endif
