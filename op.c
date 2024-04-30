/**************************************************************
 *
 *                     op.c
 *
 *     Assignment: HW6 um
 *     Authors: Jason Singer, Anna Zou
 *     Date: April 10, 2024
 *
 *     This file contains the functions for the specific operations the UM can
 *     carry out. Morever, the op module contains an array of function pointers
 *     to be used by the UM module as it carries out the instructions
 *
 **************************************************************/


#include <stdint.h>
#include <stdio.h>
#include "assert.h"
#include "bitpack.h"
#include <stdbool.h>
#include "um.h"
#include "seg.h"
#include "op.h"

/*three register function declarations. Note that some of these functions have
uneccesary parameters, but this syntax allows for the use of an array of 
function pointers*/
void conditional_move(Um universe, unsigned rA, unsigned rB, unsigned rC);
void segment_load(Um universe, unsigned rA, unsigned rB, unsigned rC);
void segment_store(Um universe, unsigned rA, unsigned rB, unsigned rC);
void add(Um universe, unsigned rA, unsigned rB, unsigned rC);
void multiply(Um universe, unsigned rA, unsigned rB, unsigned rC);
void divide(Um universe, unsigned rA, unsigned rB, unsigned rC);
void bitNAND(Um universe, unsigned rA, unsigned rB, unsigned rC);
void map_segment(Um universe, unsigned rA, unsigned rB, unsigned rC);
void unmap_segment(Um universe, unsigned rA, unsigned rB, unsigned rC);
void output(Um universe, unsigned rA, unsigned rB, unsigned rC);
void input(Um universe, unsigned rA, unsigned rB, unsigned rC);
void load_program(Um universe, unsigned rA, unsigned rB, unsigned rC);

/*array of function pointers for 3 register functions, indexed by opcode*/
func_ptr operations[] = {
        conditional_move,
        segment_load,
        segment_store,
        add,
        multiply,
        divide,
        bitNAND,
        NULL,
        map_segment,
        unmap_segment,
        output,
        input,
        load_program
};



/************ output ************
*
* Description: Function that outputs the character stored in register C
*
* Parameters: Um universe: a pointer to an initilized UM struct
*             rA: voided
*             rB: voided
*             rC: ID of the register storing the character to be output
*           
* Returns: void
*
* Expects: universe != NULL
*      
* Notes: undefined behavior if the value stored in rC > 255
*      
**********************************/
void output(Um universe, unsigned rA, unsigned rB, unsigned rC) {
        assert(universe);
        (void) rA;
        (void) rB;
        char val = get_register(universe, rC);
        putchar(val);
}

/************ input ************
*
* Description: Function that inputs a character from input and stores it in
*               a specified register
*
* Parameters: Um universe: a pointer to an initilized UM struct
*             rA: voided
*             rB: voided
*             rC: ID of the register storing the inputted char
* Returns: void
*
* Expects: universe != NULL
*      
* Notes: N/A
*      
**********************************/
void input(Um universe, unsigned rA, unsigned rB, unsigned rC) {
        assert(universe);
        (void) rA;
        (void) rB;
        int input = fgetc(stdin);
        set_register(universe, rC, (uint32_t)input);
        if (input == EOF) {
                set_register(universe, rC, (uint32_t)~0);
        }
}

/************ conditional_move ************
*
* Description: Function that stores the value in register B into register A
* if register C stores 0
*
* Parameters: Um universe: a pointer to an initilized UM struct
*             rA: index of the register that the value in rB will be stored
*             rB: index of the register that the value that will be put into rA
*                 is stored
*             rC: ID of the register whose value will be checked if it's 0
*           
* Returns: void
*
* Expects: universe != NULL
*      
* Notes: N/A
*      
**********************************/
void conditional_move(Um universe, unsigned rA, unsigned rB, unsigned rC)
{
        assert(universe);
        if(get_register(universe, rC) != 0) {
            uint32_t val = get_register(universe, rB);
            set_register(universe, rA, val);
        }
        return;
}

/************ segment_load************
*
* Description: Function that loads the value in m[rB][rC] into rA
*
* Parameters: Um universe: a pointer to an initilized UM struct
*             rA: index of the register that will store the word from the 
*                 loaded segment at the specified
*             rB: index of the register that whose value is segment ID of the 
*                 segment being loaded
*             rC: index of the word in the segment that is being loaded
*           
* Returns: void
*
* Expects: universe != NULL
*      
* Notes: if a segment ID that has not been mapped, or a word index greater 
* than the amount of words in a segnent, is in rB or rC respectively, the UM
* will fail
**********************************/
void segment_load(Um universe, unsigned rA, unsigned rB, unsigned rC)
{
        assert(universe);
        allSegments segSeqs = get_seg_sequences(universe);
        uint32_t segmentIndex = get_register(universe, rB);
        segment seg = get_segment(segSeqs, segmentIndex);
        uint32_t memIndex =  get_register(universe, rC);
        set_register(universe, rA, get_mem(seg)[memIndex]);
        return;
}

/************ segment_store************
*
* Description: Function that stores the value in rC into m[rA][rB]
*
* Parameters: Um universe: a pointer to an initilized UM struct
*             rA: index of the register whose value is the segment ID
*                 the value in rC will be stored into
*             rB: index of the register that whose value is the index of the 
*                 word in the segment the value in rC will be stored into
*             rC: index of the register whose value is being stored
*           
* Returns: void
*
* Expects: universe != NULL
*      
* Notes: if a segment ID that has not been mapped, or a word index greater 
* than the amount of words in a segnent, is in rA or rB respectively, the UM
* will fail
**********************************/
void segment_store(Um universe, unsigned rA, unsigned rB, unsigned rC)
{
        assert(universe);
        allSegments segs = get_seg_sequences(universe);
        uint32_t segmentIndex = get_register(universe, rA);
        segment segA = get_segment(segs, segmentIndex);
        get_mem(segA)[get_register(universe, rB)] = get_register(universe, rC);
        return;
}

/************add*****************************************
*
* Description: Function that adds the values in rB and rC and stores the sum
* in rC
*
* Parameters: Um universe: a pointer to an initilized UM struct
*             rA: index of the register the sum will be stored
*             rB: index of one of the register's whose values are added
*             rC: index of one of the register's whose values are added
*           
* Returns: void
*
* Expects: universe != NULL
*      
* Notes: addition is done in mod 2^32
*********************************************************/
void add(Um universe, unsigned rA, unsigned rB, unsigned rC)
{
        assert(universe);
        uint32_t val = get_register(universe, rB) + get_register(universe, rC);
        set_register(universe, rA, val);
}

/************multiply****************************8**
*
* Description: Function that multiplies the values in rB and rC and stores the 
* product in rC
*
* Parameters: Um universe: a pointer to an initilized UM struct
*             rA: index of the register the product will be stored
*             rB: index of one of the register's whose values are multiplied
*             rC: index of one of the register's whose values are multiplied
*           
* Returns: void
*
* Expects: universe != NULL
*      
* Notes: multiplication is done in mod 2^32
****************************************************/
void multiply(Um universe, unsigned rA, unsigned rB, unsigned rC)
{
        assert(universe);
        uint32_t valB = get_register(universe, rB);
        uint32_t valC = get_register(universe, rC);
        set_register(universe, rA, valC * valB);
}

/************divide****************************************
*
* Description: Function that divide the values in rB and rC and stores the 
* quotient in rC
*
* Parameters: Um universe: a pointer to an initilized UM struct
*             rA: index of the register the quotient will be stored
*             rB: index of the register whose value is the dividend
*             rC: index of the register whose value is the divisor
*           
* Returns: void
*
* Expects: universe != NULL
*      
* Notes: divisoin is done on integers, all quotients are equal to the floor of 
* the result
****************************************************/
void divide(Um universe, unsigned rA, unsigned rB, unsigned rC)
{
        assert(universe);
        uint32_t val = get_register(universe, rB) / get_register(universe, rC);
        set_register(universe, rA, val);
}

/************bitNAND****************************8**
*
* Description: Function that gets the bitwise NAND of the values stored in 
* registers B and C, and stores the result in register A
*
* Parameters: Um universe: a pointer to an initilized UM struct
*             rA: index of the register the result will be stored
*             rB: index of one of the registers whose value is NAND'ed
*             rB: index of one of the registers whose value is NAND'ed
*           
* Returns: void
*
* Expects: universe != NULL
*      
* Notes: N/A
****************************************************/
void bitNAND (Um universe, unsigned rA, unsigned rB, unsigned rC)
{
        assert(universe);
        uint32_t valB = get_register(universe, rB);
        uint32_t valC = get_register(universe, rC);
        set_register(universe, rA, ~(valB & valC));
        return;
}
 
/************map_segment****************************8**
*
* Description: Function that maps a segment with a number of words equal
* to the value in rC, and stores the ID of the new segment in rB
*
* Parameters: Um universe: a pointer to an initilized UM struct
*             rA: voided
*             rB: index of one of the registers whose value is set to the ID
*                 of the mapped segment
*             rC: index of one of the registers whose value is the number of 
*                 words in the new segment
*           
* Returns: void
*
* Expects: universe != NULL
*      
* Notes: allocates memory for a new segment struct and all needed memory space
****************************************************/
void map_segment (Um universe, unsigned rA, unsigned rB, unsigned rC) {
        assert(universe);
        (void) rA;
        allSegments segs = get_seg_sequences(universe);
        uint32_t val = get_register(universe, rC);
        uint32_t segId = init_segment(val, segs);
        set_register(universe, rB, segId);
        return;
}

/************unmap_segment******************************
*
* Description: Function that unmaps a the segment whose ID is = to the value 
* stored in rC
*
* Parameters: Um universe: a pointer to an initilized UM struct
*             rA: voided
*             rB: voided
*             rC: index of one of the registers whose value is unampped ID
*           
* Returns: void
*
* Expects: universe != NULL
*      
* Notes: frees all memory of the unampped segemnt. UM will fail if function  
* attempts to unmap an ID that does not correlate to a mapped segment 
****************************************************/
void unmap_segment (Um universe, unsigned rA, unsigned rB, unsigned rC) 
{
       // printf("in unmap segment\n");
        assert(universe);
        (void) rA;
        (void) rB;
        allSegments segs = get_seg_sequences(universe);
        uint32_t val = get_register(universe, rC);
        segment segC = get_segment(segs, val);
        free_segment(segC);
        unmap_id(val, segs);
        return;
}


/************unmap_segment******************************
*
* Description: Function that duplicates segment r[b] and sets the program
* counter equal to the value in r[c], and places the duplicated segemnt in m[0]
* Parameters: Um universe: a pointer to an  initilized UM struct
*             rA: voided
*             rB: index of the register whose value is equal to the duplicated
*                 segement's ID
*             rC: index of one of the registers whose value is unampped ID
*           
* Returns: void
*
* Expects: universe != NULL
*      
* Notes: frees all memory associated with the old segment 0, and allocates 
* memory for the duplicated segment. This particular operation is not 
* is equal to 0
**************************************************/
void load_program(Um universe, unsigned rA, unsigned rB, unsigned rC)
{
        assert(universe);
        (void)rA;
        uint32_t wordIndex = get_register(universe, rC);
        allSegments segs = get_seg_sequences(universe);
        if (get_register(universe, rB) == 0){
                set_pc(universe, wordIndex);
                return;
        }
        segment segB = copy_and_replace(segs, get_register(universe, rB));
        (void) segB;
        set_pc(universe, wordIndex);
        return;
}

/************load_value******************************
*
* Description: Function that sets a specific register equal to a value
*             rA: the ID pf the register whose value is set
*             val: 32 bit integer value the register is set equal to 
*           
* Returns: void
*
* Expects: universe != NULL
*      
* Notes: no pointer to this function in the operation array
**************************************************/
void load_value(Um universe, unsigned rA, uint32_t val) 
{
        assert(universe);
        set_register(universe, rA, val);        
}