/*
* um.c
* by Alyssa Fusillo and Kevin Liu, 11/23
* Homework 6
* Summary:This is the main implementation that ties the overarching program
* together by calling the functions of the other modules. It uses the 
* program counter to read and execute each instruction word in order. 
* The main data structures and structs are also initialized and freed
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#include "mem.h"
#include "seq.h"
#include "structs.h"

static inline uint32_t pack(uint32_t word, unsigned width, unsigned lsb, uint32_t val);
static inline uint32_t unpack(uint32_t word, unsigned width, unsigned lsb);
static inline void mem_load_program(uint32_t *words, char* path, int num_words);
static inline uint32_t mem_create_segment(Resource res, int num_words);
static inline void mem_delete_segment(Resource res, uint32_t id);
static inline uint32_t mem_create_id(Resource res);

static inline void decode_instruction(Resource res, uint32_t word);
static inline void function_chooser(Resource res, int opcode, int reg_A, int reg_B, int reg_C, uint32_t val);
static inline void cond_move(Resource res, int reg_A, int reg_B, int reg_C);
static inline void seg_load(Resource res, int reg_A, int reg_B, int reg_C);
static inline void seg_store(Resource res, int reg_A, int reg_B, int reg_C);
static inline void add(Resource res, int reg_A, int reg_B, int reg_C);
static inline void multiply(Resource res, int reg_A, int reg_B, int reg_C);
static inline void division(Resource res, int reg_A, int reg_B, int reg_C);
static inline void bitwise_nand(Resource res, int reg_A, int reg_B, int reg_C);
static inline void halt(Resource res);
static inline void map_seg(Resource res, int reg_B, int reg_C);
static inline void unmap_seg(Resource res, int reg_C);
static inline void output(Resource res, int reg_C);
static inline void input(Resource res, int reg_C);
static inline void load_program(Resource res, int reg_B, int reg_C);
static inline void load_val(Resource res, int reg_A, uint32_t val);

static inline void free_res(Resource res);

int main(int argc, char *argv[])
{
        if (argc != 2) {
                fprintf(stderr, "Only one argument is allowed! \n");
                return EXIT_FAILURE;
        }

        char *file_name = argv[1];

        /* get the number of words in the program file */
        struct stat program_stat;
        stat(file_name, &program_stat);
        off_t size = program_stat.st_size;
        int num_words = ((int) size) / 4; 

        /* Load all instruction words from file into the sequence */
        uint32_t *words = malloc(sizeof(uint32_t) * (num_words + 1)); // add 1 b/c first element is the length
        mem_load_program(words, file_name, num_words);

        /* Initialize Resource struct */
        Resource res;
        NEW(res);

        uint32_t registers[] = {0, 0, 0, 0, 0, 0, 0, 0}; 
        res->registers = registers;
        res->run = 1;
        res->program_counter = 1; // add 1 b/c first element is the length
        res->free_ids = Seq_new(64);
        res->top_id = 1;
        res->segments = malloc(sizeof(uint32_t *) * 64);
        res->num_segments = 64;

        /* Put the sequence as the first memory segment */
        res->segments[0] = words;

        /* Execution loop that gets the first memory segment and runs the 
        instruction word according to the program counter */
        while (res->run == 1) {
                uint32_t *run_seg = res->segments[0];

                uint32_t word = run_seg[res->program_counter];
                decode_instruction(res, word);
                res->program_counter++;
        }

        free_res(res);

        return 0;
}

/* _______________________________________________________________________________________________________________________________________________*/

/*
* Extracts the opcode and register values from each instruction word using the 
* bitwise operations of Bitpack; calls the function chooser using the newly 
* extracted values
* Arguments: The Resource struct 'res', a uint32_t instruction word that
* contains the values to be extracted
* Returns: nothing
*/
static inline void decode_instruction(Resource res, uint32_t word)
{
        int opcode = (int) unpack(word, 4, 32 - 4);

        if (opcode != 13) {
                int reg_A = (int) unpack(word, 3, 6);
                int reg_B = (int) unpack(word, 3, 3);
                int reg_C = (int) unpack(word, 3, 0);

                function_chooser(res, opcode, reg_A, reg_B, reg_C, 0);
        } else if (opcode == 13) {
                int reg_A = (int) unpack(word, 3, 32 - 7);
                uint32_t val = unpack(word, 25, 0);

                function_chooser(res, opcode, reg_A, 0, 0, val);
        }
}

/*
* Takes the opcode and assigns it to the appropriate instruction
* Arguments: The Resource struct 'res' to pass to the instruction function, the
* Instruction struct 'instr' with the opcode and the registers, and an 
* extracted value for when the opcode is 13
* Returns: nothing
*/
static inline void function_chooser(Resource res, int opcode, int reg_A, int reg_B, int reg_C, uint32_t val)
{
        if (opcode == 0) {            
                cond_move(res, reg_A, reg_B, reg_C);
        } else if (opcode == 1) {
                seg_load(res, reg_A, reg_B, reg_C);
        } else if (opcode == 2) {
                seg_store(res, reg_A, reg_B, reg_C);
        } else if (opcode == 3) {
                add(res, reg_A, reg_B, reg_C);
        } else if (opcode == 4) {
                multiply(res, reg_A, reg_B, reg_C);
        } else if (opcode == 5) {
                division(res, reg_A, reg_B, reg_C);
        } else if (opcode == 6) {
                bitwise_nand(res, reg_A, reg_B, reg_C);
        } else if (opcode == 7) {
                halt(res);
        } else if (opcode == 8) {
                map_seg(res, reg_B, reg_C);
        } else if (opcode == 9) {
                unmap_seg(res, reg_C);
        } else if (opcode == 10) {
                output(res, reg_C);
        } else if (opcode == 11) {
                input(res, reg_C);
        } else if (opcode == 12) {
                load_program(res, reg_B, reg_C);
        } else if (opcode == 13) {
                load_val(res, reg_A, val);
        }
}

/*
* Copies the value in register B to register A if the value in register C is 
* not 0
* Arguments: The Resource struct res, the instruction struct 'instr' with the
* opcode and the register numbers to be utilized
* returns: nothing
*/
static inline void cond_move(Resource res, int reg_A, int reg_B, int reg_C)
{
        if (res->registers[reg_C] != 0) {
                res->registers[reg_A] = res->registers[reg_B];
        }
}

/*
* Loads the segment's value at m[$r[B]][$r[C]] to register A
* Arguments: The Resource struct res, the instruction struct 'instr' 
* with the opcode and the register numbers to be utilized
* Returns: nothing
*/
static inline void seg_load(Resource res, int reg_A, int reg_B, int reg_C)
{
        /* Get memory segment */
        uint32_t *seg = res->segments[ 
                        res->registers[reg_B]];

        /* Get value from memory segment and put in register */
        uint32_t word = seg[res->registers[reg_C] + 1]; // add 1 b/c first element is the length
        res->registers[reg_A] = word; 
}

/*
* Stores the  value in [$r[C] inside m[$r[B]][$r[C]]
* Arguments: The Resource struct 'res' to pass to the instruction function, the
* Instruction struct 'instr' with the opcode and the registers
* Returns: nothing
*/
static inline void seg_store(Resource res, int reg_A, int reg_B, int reg_C)
{
        /* Get memory segment */
        uint32_t *seg = res->segments[ 
                        res->registers[reg_A]];

        /* Put value in memory segment */
        seg[res->registers[reg_B] + 1] = res->registers[reg_C]; // add 1 b/c first element is the length
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
static inline void add(Resource res, int reg_A, int reg_B, int reg_C)
{
        uint32_t b = res->registers[reg_B];
        uint32_t c = res->registers[reg_C];
        uint64_t max_int = 1;
        max_int = max_int << 32;
        uint32_t a = (b + c) % max_int;

        res->registers[reg_A] = a; 
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
static inline void multiply(Resource res, int reg_A, int reg_B, int reg_C)
{
        uint32_t b = res->registers[reg_B];
        uint32_t c = res->registers[reg_C];
        uint64_t max_int = 1;
        max_int = max_int << 32;
        uint32_t a = (b * c) % max_int;

        res->registers[reg_A] = a;
}

/*
* Divides the value in register B by that in C and stores the result in
* register A
* Arguments: The Resource struct 'res' to pass to the instruction function, the
* Instruction struct 'instr' with the opcode and the registers
* Returns: nothing
*/
static inline void division(Resource res, int reg_A, int reg_B, int reg_C)
{
        uint32_t b = res->registers[reg_B];
        uint32_t c = res->registers[reg_C];

        uint32_t a = (b / c);

        res->registers[reg_A] = a;
}

/*
* Performs a bitwise 'nand' on the values in registers B and C; loads the 
* result into register A
* Arguments: The Resource struct 'res' to pass to the instruction function, the
* Instruction struct 'instr' with the opcode and the registers
* Returns: nothing
*/
static inline void bitwise_nand(Resource res, int reg_A, int reg_B, int reg_C)
{
        uint32_t b = res->registers[reg_B];
        uint32_t c = res->registers[reg_C];
        uint32_t a = ~(b & c);
        res->registers[reg_A] = a;
}

/*
* Stops computation by setting the run flag to 0
* Arguments: The Resource struct 'res' to pass to the instruction function, the
* Instruction struct 'instr' with the opcode and the registers
* Returns: nothing
*/
static inline void halt(Resource res)
{
        res->run = 0;
}

/*
* Creates and maps a new segment; sets the value in register B to the new id
* Arguments: The Resource struct res, the instruction struct 'instr' with the
* opcode and the register numbers to be utilized
* Returns: nothing
*/
static inline void map_seg(Resource res, int reg_B, int reg_C)
{
        uint32_t id = mem_create_segment(res, res->registers[reg_C]);
        res->registers[reg_B] = id;
}

/*
* Unmaps and deletes a segment 
* Arguments: The Resource struct res, the instruction struct 'instr' with 
* the opcode and the register numbers to be utilized
* Returns: nothing
*/
static inline void unmap_seg(Resource res, int reg_C)
{
        mem_delete_segment(res, res->registers[reg_C]);
}

/*
* Outputs the value (between 0 and 255) in register C
* Arguments: The Resource struct 'res' to pass to the instruction function, the
* Instruction struct 'instr' with the opcode and the registers
* Returns: nothing
*/
static inline void output(Resource res, int reg_C)
{
        uint32_t val_reg_C = res->registers[reg_C];

        putchar((int) val_reg_C);
}

/*
* Loads input into register C; if at the end of the file, register C is loaded
* with a 32-bit word full of '1' bits
* Arguments: The Resource struct 'res'; the instruction struct 'instr' 
* with the coding opcode and the three registers to be utilized
* Returns: nothing
*/
static inline void input(Resource res, int reg_C)
{
        char input;
        input = getchar();

        if (input == EOF){
                res->registers[reg_C] = ~0;
        } else {
                res->registers[reg_C] = (uint32_t) input;
        }
}

/*
* Duplicates segment $m[$r[B]] and replaces $m[0] with the duplicate; updates
* the program counter to the value in register C
* Arguments: The Resource struct 'res' the
* Instruction struct 'instr' with the opcode and the registers
* Returns: nothing
*/
static inline void load_program(Resource res, int reg_B, int reg_C)
{
        if (res->registers[reg_B] == 0) {
                /*Decrements the new counter by one since it will be
                        incremented right after in main() */
                res->program_counter = res->registers[reg_C] - 1 + 1; // add 1 b/c first element is the length
                return;
        }

        uint32_t *seg = res->segments[0];
        free(seg);

        // Copy each word from src_seg to dest_seg 
        uint32_t *src_seg = res->segments[
                        res->registers[reg_B]];

        unsigned seg_length = src_seg[0];
        uint32_t *dest_seg = malloc(sizeof(uint32_t) * (seg_length + 1)); // add 1 b/c first element is the length
                
        for (unsigned i = 0; i <= seg_length; i++) {
                dest_seg[i] = src_seg[i];
        }

        res->segments[0] = dest_seg;
        res->program_counter = res->registers[reg_C] - 1 + 1; // add 1 b/c first element is the length   
}

/*
* Loads a value, or the value of the lower 25 bits of the instruction word, 
* into register A
* Arguments: The Resource struct 'res', the instruction struct 'instr' with 
* the opcode and the three register numbers to be utilized
* Returns: nothing
*/
static inline void load_val(Resource res, int reg_A, uint32_t val)
{
        res->registers[reg_A] = val;
}

/* _______________________________________________________________________________________________________________________________________________*/

static inline uint32_t pack(uint32_t word, unsigned width, unsigned lsb, uint32_t val)
{
        unsigned hi = lsb + width;
        unsigned bits = 32 - lsb;
     
        uint32_t new_word = ((word >> hi) << hi | (word << bits) >> bits
                                                               | (val << lsb));   
        return new_word;
}


static inline uint32_t unpack(uint32_t word, unsigned width, unsigned lsb)
{
        uint32_t field = word << (32 - (width + lsb));
        field = field >> (32 - width);
        return field;
}

/*
* Loads a stream of instruction words from a file into a sequence, using
* the bitwise operations of Bitpack. This sequence will be put into the 0 
* segment of memory in um.c
* Arguments: A new Seq_T 'words' which will contain the stream of instruction
* words when they are read in; a character pointer 'path' which is the file 
* name, and an integer which is the number of words that the new sequence will 
* contain
* Returns: nothing
*/
static inline void mem_load_program(uint32_t *words, char* path, int num_words)
{
        FILE* program = fopen(path, "rb");
        words[0] = num_words; // add 1 b/c first element is the length
        for (int i = 1; i <= num_words; i++) {
                uint32_t word = 0;
                /* Read the word 8 bits at a time to not overflow char */
                for (int x = 1; x <= 4; x++) {
                        int c = getc(program);
                        word = pack(word, 8, 32 - (x * 8), c);
                }

                words[i] = word;
        }
        fclose(program);
}

/*
* Creates a new segment, which is represented as a sequence; inserts
* it into the table of segments with its new corresponding identification
* number
* Arguments: the Resource res struct; an integer which is
* the number of uint32_t words that the new segment will have
* Returns: a uint32_t of the segment's id
*/
static inline uint32_t mem_create_segment(Resource res, int num_words)
{
        uint32_t *seg = malloc(sizeof(uint32_t) * (num_words + 1)); // add 1 b/c first element is the length
        seg[0] = num_words;
        for (int i = 1; i <= num_words; i++) {
                seg[i] = 0;
        }

        uint32_t id = mem_create_id(res);

        /* Enlarge array of segments if needed */
        if (id >= res->num_segments) {
                res->segments = realloc(res->segments, sizeof(uint32_t *) * res->num_segments * 2);      
                res->num_segments *= 2;
        }
        res->segments[id] = seg;

        return id;
}

/*
* Removes and frees the segment, represented as a sequence, at the given id
* number in the table. This id number is then added to the sequence of free id
* numbers
* Arguments: The Resource struct 'res', a uint32_t of id of the old segment
* to be deleted
* Returns: nothing
*/
static inline void mem_delete_segment(Resource res, uint32_t old_id)
{
        uint32_t *seg = res->segments[old_id];
        free(seg);
        *seg = 0;

        /* Add now available id to the sequence of free id's */
        Seq_addhi(res->free_ids, (void *)(uintptr_t) old_id);
}

/*
* Checks the sequence of free id numbers. If the sequence is empty, this
* function returns the value of the top_idea counter and then increments it.
* Otherwise, the last free id in the sequence is returned
* Arguments: the Resource res struct
* Returns: a uint32_t id
*/
static inline uint32_t mem_create_id(Resource res)
{
        /* If the sequence of free id's is empty, create a new id */
        if (Seq_length(res->free_ids) == 0) {
                uint32_t new_id = res->top_id;
                res->top_id++;
                return new_id;       
        }

        uint32_t id = (uint32_t)(uintptr_t) Seq_remhi(res->free_ids);
        return id;
}

/* _______________________________________________________________________________________________________________________________________________*/

/*
* Frees the main Resource struct
* Arguments: the Resource struct to be freed
* Returns: nothing
*/
static inline void free_res(Resource res)
{
        uint32_t **segments = res->segments;
        unsigned max_id = res->top_id;
        for (unsigned i = 0; i < max_id; i++) {
                uint32_t *seg = segments[i];
                if (*seg != 0) {
                        free(seg);
                }
        }
        // free(*segments);

        Seq_T free_ids = res->free_ids;
        Seq_free(&free_ids);

        FREE(res);
}
