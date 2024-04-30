/**************************************************************
 *
 *                     op.h
 *
 *     Assignment: HW6 um
 *     Authors: Jason Singer, Anna Zou
 *     Date: April 10, 2024
 *
 *     The interface for the operations module, contains a privately defined 
       array of pointers to instruction-handling functions except for load 
       value.
 *
 **************************************************************/

#include <stdint.h>
#include <stdio.h>
#include "assert.h"
#include "bitpack.h"
#include <stdbool.h>

#ifndef OP_H
#define OP_H
typedef void (*func_ptr)(Um universe, unsigned rA, unsigned rB, unsigned rC);
extern func_ptr operations[];
void load_value(Um universe, unsigned rA, uint32_t val);
#endif