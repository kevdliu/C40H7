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
#include "seq.h"
#include "mem.h"
#include "table.h"
#include "mem_manager.h"
#include "structs.h"
#include "assert.h"

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
        Seq_T words = Seq_new(num_words);
        mem_load_program(words, file_name, num_words);

        assert(Seq_length(words) != 0);

        /* Initialize Resource struct */
        Resource res;
        NEW(res);

        uint32_t registers[] = {0, 0, 0, 0, 0, 0, 0, 0}; 
        res->registers = registers;
        res->run = 1;
        res->program_counter = 0;
        res->free_ids = Seq_new(64);
        res->top_id = 1;
        res->segments = Table_new(64, NULL, NULL);

        /* Put the sequence as the first memory segment */
        Table_put(res->segments, get_atom(0), words);

        /* Execution loop that gets the first memory segment and runs the 
        instruction word according to the program counter */
        while (res->run == 1) {
                Seq_T run_seg = Table_get(res->segments, get_atom(0));

                uint32_t word = (uint32_t)(uintptr_t) Seq_get(run_seg, 
                                res->program_counter);
                decode_instruction(res, word);
                res->program_counter++;
        }

        free_res(res);

        return 0;
}

/*
* Frees the main Resource struct
* Arguments: the Resource struct to be freed
* Returns: nothing
*/
void free_res(Resource res)
{
        Table_T segments = res->segments;
        for (int i = 0; i < Table_length(segments); i++) {
                Seq_T seg = Table_get(segments, get_atom(i));
                if (seg != NULL) {
                        Seq_free(&seg);
                }
        }
        Table_free(&segments);

        Seq_T free_ids = res->free_ids;
        Seq_free(&free_ids);

        FREE(res);
}
