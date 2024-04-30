/**************************************************************
 *
 *                     um.h
 *
 *      Assignment: HW6 um
 *      Authors: Jason Singer, Anna Zou
 *      Date: April 10, 2024
 *
 *      Interface for the UM module, including getters and setters
 *      for the registers and segment sequences, as well as initilzation,
 *      running, and freeing of the program to be used by a main driver
 *
 **************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "seg.h"


struct Um;
typedef struct Um *Um;

/*functions used by main*/
Um init_um(FILE *instructions);
void run_um(Um universe);

/*functions used by other modules*/
uint32_t get_register(Um universe, unsigned reg);
void set_register(Um universe, unsigned reg, uint32_t val);
allSegments get_seg_sequences(Um universe);
void free_um(Um universe);
void set_pc(Um universe, uint32_t val);