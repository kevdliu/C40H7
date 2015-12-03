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

#include "program_executor.h"
#include "mem.h"
#include "mem_manager.h"
#include "structs.h"

/* Frees all resources when done */
void free_res(Resource res);

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

        // free_res(res);

        return 0;
}

/*
* Frees the main Resource struct
* Arguments: the Resource struct to be freed
* Returns: nothing
*/

void free_res(Resource res)
{
        uint32_t **segments = res->segments;
        unsigned seg_length = res->num_segments;
        for (unsigned i = 0; i < seg_length; i++) {
                uint32_t *seg = segments[i];
                if (seg != NULL) {
                        free(seg);
                }
        }
        free(*segments);

        Seq_T free_ids = res->free_ids;
        Seq_free(&free_ids);

        FREE(res);
}
