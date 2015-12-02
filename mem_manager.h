#ifndef MEM_MANAGER_INCLUDED
#define MEM_MANAGER_INCLUDED

#include <stdint.h>
#include "structs.h"

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
extern void mem_load_program(uint32_t *words, char* path, int num_words);

/*
* Creates a new segment, which is represented as a sequence; inserts
* it into the table of segments with its new corresponding identification
* number
* Arguments: the Resource res struct; an integer which is
* the number of uint32_t words that the new segment will have
* Returns: a uint32_t of the segment's id
*/
extern uint32_t mem_create_segment(Resource res, int num_words);

/*
* Removes and frees the segment, represented as a sequence, at the given id
* number in the table. This id number is then added to the sequence of free id
* numbers
* Arguments: The Resource struct 'res', a uint32_t of id of the old segment
* to be deleted
* Returns: nothing
*/
extern void mem_delete_segment(Resource res, uint32_t id);

/*
* Checks the sequence of free id numbers. If the sequence is empty, this
* function returns the value of the top_idea counter and then increments it.
* Otherwise, the last free id in the sequence is returned
* Arguments: the Resource res struct
* Returns: a uint32_t id
*/
extern uint32_t mem_create_id(Resource res);

/*
* Converts a uint32_t to an atom to be utilized as a key in the Hanson table
* Arguments: the uint32_t id to be converted
* Returns: a character pointer
*/
extern const char* get_atom(uint32_t id);

#endif
