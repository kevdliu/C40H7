/*
* program_executor.c
* by Alyssa Fusillo and Kevin Liu, 11/23
* Homework 6
* Summary: This implementation contains functions that decode opcode and 
* register values from instruction words, assign opcodes to their corresponding
* functions, and execute these functions
*/

#include "program_executor.h"
#include "bitpack.h"
#include "mem.h"
#include "mem_manager.h"
#include "assert.h"

#include <stdlib.h>
#include <stdio.h>

/*
* Extracts the opcode and register values from each instruction word using the 
* bitwise operations of Bitpack; calls the function chooser using the newly 
* extracted values
* Arguments: The Resource struct 'res', a uint32_t instruction word that
* contains the values to be extracted
* Returns: nothing
*/
void decode_instruction(Resource res, uint32_t word)
{
        Instruction instr;
        NEW(instr);

        int opcode = (int) Bitpack_getu(word, 4, 32 - 4);

        instr->opcode = opcode;
        if (opcode != 13) {
                instr->reg_A = (int) Bitpack_getu(word, 3, 6);
                instr->reg_B = (int) Bitpack_getu(word, 3, 3);
                instr->reg_C = (int) Bitpack_getu(word, 3, 0);

                function_chooser(res, instr, 0);
        } else if (opcode == 13) {
                instr->reg_A = (int) Bitpack_getu(word, 3, 32 - 7);
                uint32_t val = Bitpack_getu(word, 25, 0);

                function_chooser(res, instr, val);
        }
}

/*
* Takes the opcode and assigns it to the appropriate instruction
* Arguments: The Resource struct 'res' to pass to the instruction function, the
* Instruction struct 'instr' with the opcode and the registers, and an 
* extracted value for when the opcode is 13
* Returns: nothing
*/
void function_chooser(Resource res, Instruction instr, uint32_t val)
{
        int opcode = instr->opcode;
        if (opcode == 0) {
                cond_move(res, instr);
        } else if (opcode == 1) {
                seg_load(res, instr);
        } else if (opcode == 2) {
                seg_store(res, instr);
        } else if (opcode == 3) {
                add(res, instr);
        } else if (opcode == 4) {
                multiply(res, instr);
        } else if (opcode == 5) {
                division(res, instr);
        } else if (opcode == 6) {
                bitwise_nand(res, instr);
        } else if (opcode == 7) {
                halt(res);
        } else if (opcode == 8) {
                map_seg(res, instr);
        } else if (opcode == 9) {
                unmap_seg(res, instr);
        } else if (opcode == 10) {
                output(res, instr);
        } else if (opcode == 11) {
                input(res, instr);
        } else if (opcode == 12) {
                load_program(res, instr);
        } else if (opcode == 13) {
                load_val(res, instr, val);
        } else {
                assert(0);
        }

        FREE(instr);
}

/*
* Copies the value in register B to register A if the value in register C is 
* not 0
* Arguments: The Resource struct res, the instruction struct 'instr' with the
* opcode and the register numbers to be utilized
* returns: nothing
*/
void cond_move(Resource res, Instruction instr)
{
        if (res->registers[instr->reg_C] != 0) {
                res->registers[instr->reg_A] = res->registers[instr->reg_B];
        }
}

/*
* Loads the segment's value at m[$r[B]][$r[C]] to register A
* Arguments: The Resource struct res, the instruction struct 'instr' 
* with the opcode and the register numbers to be utilized
* Returns: nothing
*/
void seg_load(Resource res, Instruction instr)
{
        /* Get memory segment */
        Seq_T seg = Table_get(res->segments, 
                        get_atom(res->registers[instr->reg_B]));
        assert(seg != NULL);

        assert(res->registers[instr->reg_C] < (unsigned) Seq_length(seg));
        /* Get value from memory segment and put in register */
        uint32_t word = (uint32_t)(uintptr_t) Seq_get(seg, 
                        res->registers[instr->reg_C]);
        res->registers[instr->reg_A] = word; 
}

/*
* Stores the  value in [$r[C] inside m[$r[B]][$r[C]]
* Arguments: The Resource struct 'res' to pass to the instruction function, the
* Instruction struct 'instr' with the opcode and the registers
* Returns: nothing
*/
void seg_store(Resource res, Instruction instr)
{
        /* Get memory segment */
        Seq_T seg = Table_get(res->segments, 
                        get_atom(res->registers[instr->reg_A]));
        assert(seg != NULL);

        assert(res->registers[instr->reg_B] < (unsigned) Seq_length(seg));
        /* Put value in memory segment */
        Seq_put(seg, res->registers[instr->reg_B],
                        (void *)(uintptr_t) res->registers[instr->reg_C]);
}

/*
* Adds the values in registers B and C, mods the result by 2^32 to make sure 
* the value stays within the confines of a 32 bit word, and stores the result 
* in register A
* Arguments: The Resource struct 'res' to pass to the instruction 
* function, the
* Instruction struct 'instr' with the opcode and the registers
* Returns: nothing
*/
void add(Resource res, Instruction instr)
{
        uint32_t b = res->registers[instr->reg_B];
        uint32_t c = res->registers[instr->reg_C];
        uint64_t max_int = 1;
        max_int = max_int << 32;
        uint32_t a = (b + c) % max_int;

        res->registers[instr->reg_A] = a; 
}

/*
* 
* Multiplies the values in registers B and C, mods the result by 2^32 to make 
* sure the value stays within the confines of a 32 bit word, and stores the 
* result in register A
* Arguments: The Resource struct 'res' to pass to the instruction function, the
* Instruction struct 'instr' with the opcode and the registers
* Returns: nothing
*/
void multiply(Resource res, Instruction instr)
{
        uint32_t b = res->registers[instr->reg_B];
        uint32_t c = res->registers[instr->reg_C];
        uint64_t max_int = 1;
        max_int = max_int << 32;
        uint32_t a = (b * c) % max_int;

        res->registers[instr->reg_A] = a;
}

/*
* Divides the value in register B by that in C and stores the result in
* register A
* Arguments: The Resource struct 'res' to pass to the instruction function, the
* Instruction struct 'instr' with the opcode and the registers
* Returns: nothing
*/
void division(Resource res, Instruction instr)
{
        uint32_t b = res->registers[instr->reg_B];
        uint32_t c = res->registers[instr->reg_C];
        assert(c != 0);

        uint32_t a = (b / c);

        res->registers[instr->reg_A] = a;
}

/*
* Performs a bitwise 'nand' on the values in registers B and C; loads the 
* result into register A
* Arguments: The Resource struct 'res' to pass to the instruction function, the
* Instruction struct 'instr' with the opcode and the registers
* Returns: nothing
*/
void bitwise_nand(Resource res, Instruction instr)
{
        uint32_t b = res->registers[instr->reg_B];
        uint32_t c = res->registers[instr->reg_C];
        uint32_t a = ~(b & c);
        res->registers[instr->reg_A] = a;
}

/*
* Stops computation by setting the run flag to 0
* Arguments: The Resource struct 'res' to pass to the instruction function, the
* Instruction struct 'instr' with the opcode and the registers
* Returns: nothing
*/
void halt(Resource res)
{
        res->run = 0;
}

/*
* Creates and maps a new segment; sets the value in register B to the new id
* Arguments: The Resource struct res, the instruction struct 'instr' with the
* opcode and the register numbers to be utilized
* Returns: nothing
*/
void map_seg(Resource res, Instruction instr)
{
        uint32_t id = mem_create_segment(res, res->registers[instr->reg_C]);
        res->registers[instr->reg_B] = id;
}

/*
* Unmaps and deletes a segment 
* Arguments: The Resource struct res, the instruction struct 'instr' with 
* the opcode and the register numbers to be utilized
* Returns: nothing
*/
void unmap_seg(Resource res, Instruction instr)
{
        assert(res->registers[instr->reg_C] != 0);

        mem_delete_segment(res, res->registers[instr->reg_C]);
}

/*
* Outputs the value (between 0 and 255) in register C
* Arguments: The Resource struct 'res' to pass to the instruction function, the
* Instruction struct 'instr' with the opcode and the registers
* Returns: nothing
*/
void output(Resource res, Instruction instr)
{
        uint32_t reg_C = res->registers[instr->reg_C];
        assert(reg_C <= 255);

        putchar((int) reg_C);
}

/*
* Loads input into register C; if at the end of the file, register C is loaded
* with a 32-bit word full of '1' bits
* Arguments: The Resource struct 'res'; the instruction struct 'instr' 
* with the coding opcode and the three registers to be utilized
* Returns: nothing
*/
void input(Resource res, Instruction instr)
{
        char input;
        input = getchar();

        if (input == EOF){
                res->registers[instr->reg_C] = ~0;
        } else {
                res->registers[instr->reg_C] = (uint32_t) input;
        }
}

/*
* Duplicates segment $m[$r[B]] and replaces $m[0] with the duplicate; updates
* the program counter to the value in register C
* Arguments: The Resource struct 'res' the
* Instruction struct 'instr' with the opcode and the registers
* Returns: nothing
*/
void load_program(Resource res, Instruction instr)
{
        if (res->registers[instr->reg_B] == 0) {
                /*Decrements the new counter by one since it will be
                        incremented right after in main() */
                res->program_counter = res->registers[instr->reg_C] - 1;
                return;
        }

        Seq_T seg = Table_remove(res->segments, get_atom(0));
        Seq_free(&seg);

        /* Copy each word from src_seg to dest_seg */
        Seq_T src_seg = Table_get(res->segments,
                        get_atom(res->registers[instr->reg_B]));
        assert(src_seg != NULL);

        Seq_T dest_seg = Seq_new(Seq_length(src_seg));
        for (int i = 0; i < Seq_length(src_seg); i++) {
                Seq_addhi(dest_seg, Seq_get(src_seg, i));
        }

        Table_put(res->segments, get_atom(0), dest_seg);
        res->program_counter = res->registers[instr->reg_C] - 1;
}

/*
* Loads a value, or the value of the lower 25 bits of the instruction word, 
* into register A
* Arguments: The Resource struct 'res', the instruction struct 'instr' with 
* the opcode and the three register numbers to be utilized
* Returns: nothing
*/
void load_val(Resource res, Instruction instr, uint32_t val)
{
        res->registers[instr->reg_A] = val;
}
