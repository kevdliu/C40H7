/*
* mem_manager.c
* by Alyssa Fusillo and Kevin Liu, 11/23
* Homework 6
* Summary: This implementation contains the functions to manage memory
* throughout the program. It can load the program and initially store it in the
* 0 segment, it can create and delete memory segments, and it can manage 
* segment identification tags.
*/

#include <mem_manager.h>
#include <stdlib.h>
#include <stdio.h>
#include <bitpack.h>
#include <inttypes.h>
#include <string.h>

#include "atom.h"
#include "assert.h"

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
void mem_load_program(uint32_t *words, char* path, int num_words)
{
        FILE* program = fopen(path, "rb");
        for (int i = 0; i < num_words; i++) {
                uint32_t word = 0;
                /* Read the word 8 bits at a time to not overflow char */
                for (int x = 1; x <= 4; x++) {
                        int c = getc(program);
                        word = Bitpack_newu(word, 8, 32 - (x * 8), c);
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
uint32_t mem_create_segment(Resource res, int num_words)
{
        uint32_t *seg = malloc(sizeof(uint32_t) * num_words);
        for (int i = 0; i < num_words; i++) {
                seg[i] = 0;
        }

        uint32_t id = mem_create_id(res);
        if (id >= (unsigned) Seq_length(res->segments)) {
                Seq_addhi(res->segments, seg);
        } else {
                Seq_put(res->segments, id, seg);
        }

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
void mem_delete_segment(Resource res, uint32_t old_id)
{
        uint32_t *seg = Seq_get(res->segments, old_id);
        assert(seg != NULL);

        free(seg);
        seg = NULL;

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
uint32_t mem_create_id(Resource res)
{
        /* If the sequence of free id's is empty, create a new id */
        if (Seq_length(res->free_ids) == 0) {
                /* Make sure the id don't exceed the max value for a 32-bit 
                unsigned integer */
                assert(res->top_id <= 4294967294);

                uint32_t new_id = res->top_id;
                res->top_id++;
                return new_id;       
        }

        uint32_t id = (uint32_t)(uintptr_t) Seq_remhi(res->free_ids);
        return id;
}

