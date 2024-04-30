/**************************************************************
 *
 *                     um.c
 *
 *     Assignment: HW6 um
 *     Authors: Jason Singer, Anna Zou
 *     Date: April 10, 2024
 *
 *     A module for the um machine itself, responsible for initializing 
       all the architecture related to the machine (e.g. registers, memory) and
       carrying out individual instructions
 *
 **************************************************************/

#include "um.h"
#include "seg.h"
#include <stdio.h>
#include "assert.h"
#include "bitpack.h"
#include <stdbool.h>
#include "op.h"

#define NUM_REGISTERS 8
#define OPBITS 4
#define REGID 3
#define WORDBITS 32
#define VALUE 25
#define REGA 6

typedef enum Um_opcode {
        CMOV = 0, SLOAD, SSTORE, ADD, MUL, DIV,
        NAND, HALT, ACTIVATE, INACTIVATE, OUT, IN, LOADP, LV
} Um_opcode;


/* Representation of our Universal Machine in the program. Member variables
are an array of uint32_t's representing the registers, an allSegments struct
pointer, an integer counting the current instruciton number, and a pointer to
an array of 3 register operations */
struct Um {
        uint32_t registers[NUM_REGISTERS];
        allSegments umSegments;
        uint32_t pc; 
        func_ptr *op_ptr;
};

//helper functions
int compute_instructions(Um universe);
uint32_t get_instruction(Um universe);


//testing function
void print_register(Um universe, unsigned rA);

/************ init_um ************
*
* Description: Function that inilizes an instance of a struct representing
* the universal machine.
*
* Parameters: FILE *instructions: a pointer to a binary file containing UM 
* machine instructions
*           
* Returns: A bool value indicating if value can be represented with width bits
*
* Expects: instructions != NULL
*      
* Notes: allocates memory for UM struct, and calls function to inilize segment
* memory, which will also allocate memory
*      
**********************************/
Um init_um(FILE *instructions)
{
        assert(instructions);
        Um universe = malloc(sizeof(struct Um));
        for (int i = 0; i < NUM_REGISTERS; i++){
                set_register(universe, i, 0);
        }
        universe->pc = 0;
        allSegments umSegs = init_allSegs(instructions);
        universe->umSegments = umSegs;
        universe->op_ptr = operations;
        return universe;
}

/************ run_um ************
*
* Description: Function that begins the process of the UM running instructions,
* and controls when the UM stops running as well
*
* Parameters: Um universe: a pointer to an initilized UM struct
*           
* Returns: void
*
* Expects: universe != NULL
*      
* Notes: N/A
*      
**********************************/
void run_um(Um universe)
{
        assert(universe);
        int op = 0;
        while (op != HALT) {
                op = compute_instructions(universe);
                if (op != LOADP){
                        universe->pc++;
                }
        }
        //print_register(universe, 3);
}

/************ compute_instructions ************
*
* Description: Function that reads in the instruction at segment 0 that
* corresponds to the program counter, and runs the operation corresponding to 
* the opcode of that instruction

* Parameters: Um universe: a pointer to an initilized UM struct
*           
* Returns: the opcode, represented as an integer
*
* Expects: universe != NULL
*      
* Notes: N/A
*      
**********************************/
int compute_instructions(Um universe)
{
        assert(universe);
        uint32_t currInstruction = get_instruction(universe);
        uint32_t opcode = Bitpack_getu(currInstruction, OPBITS, 
                                       WORDBITS - OPBITS);
        if (opcode == HALT){
                return opcode;
        }
        else if (opcode == LV){
                //printf("in loadval \n");
                unsigned regA = Bitpack_getu(currInstruction, REGID, 
                                        WORDBITS - OPBITS - REGID);
                uint32_t val = Bitpack_getu(currInstruction, VALUE, 0);
                //printf("val %d \n", val);
                load_value(universe, regA, val);
                return opcode;
        }
        else {
                unsigned int regA = Bitpack_getu(currInstruction, REGID, REGA);
                unsigned int regB = Bitpack_getu(currInstruction, REGID,  
                        REGID);
                unsigned int regC = Bitpack_getu(currInstruction, REGID, 0);
                universe->op_ptr[opcode](universe, regA, regB, regC);
                return opcode;
        }
}

/************ set_register ************
*
* Description: Function that sets the value in a specified register to
* a value passed in as an argument 

* Parameters: Um universe: a pointer to an initilized UM struct
*             unsigned reg: the ID specifying which register is set
*             uint32_t val: the value the register is being set to
*           
* Returns: void
*
* Expects: universe != NULL and reg < 8
*      
* Notes: used by other modules to alter the register values
*      
**********************************/
void set_register(Um universe, unsigned reg, uint32_t val)
{
        assert(universe);
        assert(reg < NUM_REGISTERS);
        universe->registers[reg] = val;
}

/************ get_register ************
*
* Description: Function that gets the value in a specified register 

* Parameters: Um universe: a pointer to an initilized UM struct
*             unsigned reg: the ID specifying which register to get the value
*                           from
*
*           
* Returns: the uint32_t stored in the specified register
*
* Expects: universe != NULL and reg < 8
*      
* Notes: used by other modules to get the register values
*      
**********************************/
uint32_t get_register(Um universe, unsigned reg)
{
        assert(universe);
        assert(reg < NUM_REGISTERS);
        return universe->registers[reg];
}

/************ free_um ************
*
* Description: Function that frees the memory allocated for the UM and its
* segments 
*
* Parameters: Um universe: a pointer to an initilized UM struct
*            
* Returns: void
*
* Expects: universe != NULL 
*      
* Notes: N/A
*/
void free_um(Um universe){
        assert(universe);
        free_allSegments(universe->umSegments);
        free(universe);
}

/************get_instruction************
*
* Description: Function that gets the 32 bit word that represents
* the instruction that is to be executed next
*
* Parameters: Um universe: a pointer to an initilized UM struct
*
* Returns: a 32 bit word representing a UM instruction
*
* Expects: universe != NULL 
*      
* Notes: N/A
*/
uint32_t get_instruction(Um universe)
{
        assert(universe);
        segment seg0 = get_segment(universe->umSegments, 0);
        uint32_t *instructions = get_mem(seg0);
        return instructions[universe->pc];
}

/************ get_seg_sequences ************
*
* Description: Function that gets the allSegment struct of an initlized UM,
* which contains the mappend and unmapped segment sequences
*
* Parameters: Um universe: a pointer to an initilized UM struct
*
* Returns: a pointer to the UM's allSegments struct
*
* Expects: universe != NULL 
*      
* Notes: used by other modules to get allSegments struct
*/
allSegments get_seg_sequences(Um universe)
{
        assert(universe);
        return universe->umSegments;
}

/************ set_pc ************
*
* Description: Function that sets the program counter to a specified value
*
* Parameters: Um universe: a pointer to an initilized UM struct
*             uint32_t val: the value the program counter is set to
*
* Returns: void
*
* Expects: universe != NULL 
*      
* Notes: used by other modules to change the pc to a specific value
*/
void set_pc(Um universe, uint32_t val)
{
        assert(universe);
        universe->pc = val;
}

/*function for testing only that prints the value in a specified register as 
an integer. Not included in final submission but extremely useful for testing*/
void print_register(Um universe, unsigned rA)
{
        printf("The value in rA is %u", get_register(universe, rA));
}