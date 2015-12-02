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
void mem_load_program(Seq_T words, char* path, int num_words)
{
        FILE* program = fopen(path, "rb");
        for (int i = 0; i < num_words; i++) {
                uint32_t word = 0;
                /* Read the word 8 bits at a time to not overflow char */
                for (int x = 1; x <= 4; x++) {
                        int c = getc(program);
                        word = Bitpack_newu(word, 8, 32 - (x * 8), c);
                }

                Seq_addhi(words, (void *)(uintptr_t) word);
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
{               //TODO: USE QUEUES INSTEAD OF SEQUENCES
        Seq_T seg = Seq_new(num_words);
        /* Fill new sequence with appropriate number of 0's */
        for (int i = 0; i < num_words; i++) {
                uint32_t word = 0;
                Seq_addhi(seg, (void *)(uintptr_t) word);
        }
        uint32_t id = mem_create_id(res);
        Table_put(res->segments, get_atom(id), seg);

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
        Seq_T seg = Table_remove(res->segments, get_atom(old_id));
        assert(seg != NULL);

        Seq_free(&seg);

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

/*
* Converts a uint32_t to an atom to be utilized as a key in the Hanson table
* Arguments: the uint32_t id to be converted
* Returns: a character pointer
*/
const char* get_atom(uint32_t id)
{
        char str_id[32];
        sprintf(str_id, "%u", id);
        const char *atom_key = Atom_new(str_id, strlen(str_id));
        return atom_key;
}
