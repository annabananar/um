/*
 * umtests.c
 *
 * Functions to generate UM unit tests. Once complete, this module
 * should be augmented and then linked against umlabwrite.c to produce
 * a unit test writing program.
 *  
 * A unit test is a stream of UM instructions, represented as a Hanson
 * Seq_T of 32-bit words adhering to the UM's instruction format.  
 * 
 * Any additional functions and unit tests written for the lab go
 * here. 
 *  
 */


#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <seq.h>
#include <bitpack.h>


typedef uint32_t Um_instruction;
typedef enum Um_opcode {
        CMOV = 0, SLOAD, SSTORE, ADD, MUL, DIV,
        NAND, HALT, ACTIVATE, INACTIVATE, OUT, IN, LOADP, LV
} Um_opcode;


/* Functions that return the two instruction types */


Um_instruction three_register(Um_opcode op, int ra, int rb, int rc)
{
       uint32_t word = 0;
       word = Bitpack_newu(word, 4, 28, op);
       word = Bitpack_newu(word, 3, 0, rc);
       word = Bitpack_newu(word, 3, 3, rb);
       word = Bitpack_newu(word, 3, 6, ra);
       return word;
       
}

Um_instruction loadval(unsigned ra, unsigned val) {
        Um_instruction word = 0;
        word = Bitpack_newu(word, 4, 28, LV);
        word = Bitpack_newu(word, 3, 25, ra);
        word = Bitpack_newu(word, 25, 0, val);
        return word;
}


/* Wrapper functions for each of the instructions */

static inline Um_instruction halt(void) 
{
        return three_register(HALT, 0, 0, 0);
}

typedef enum Um_register { r0 = 0, r1, r2, r3, r4, r5, r6, r7 } Um_register;

static inline Um_instruction cmov(Um_register a, Um_register b, Um_register c) 
{
        return three_register(CMOV, a, b, c);
}

static inline Um_instruction add(Um_register a, Um_register b, Um_register c) 
{
        return three_register(ADD, a, b, c);
}

static inline Um_instruction multiply
(Um_register a, Um_register b, Um_register c)
{
        return three_register(MUL, a, b, c);
}

static inline Um_instruction divide(Um_register a, Um_register b, Um_register c)
{
        return three_register(DIV, a, b, c);
}

static inline Um_instruction nand(Um_register a, Um_register b, Um_register c)
{
        return three_register(NAND, a, b, c);
}

Um_instruction input(Um_register c) {
        return three_register(IN, 0, 0, c);
}

Um_instruction output(Um_register c){
        return three_register(OUT, 0, 0, c);
}

Um_instruction load_segment(Um_register a, Um_register b, Um_register c) 
{
        return three_register(SLOAD, a, b, c);
}

Um_instruction store_segment(Um_register a, Um_register b, Um_register c) 
{
        return three_register(SSTORE, a, b, c);
}

Um_instruction map_segment(Um_register b, Um_register c) 
{
        return three_register(ACTIVATE, 0, b, c);
}

Um_instruction unmap_segment(Um_register c) 
{
        return three_register(INACTIVATE, 0, 0, c);
}

Um_instruction load_program(Um_register b, Um_register c)
{
        return three_register(LOADP, 0, b, c);
}

/* Functions for working with streams */

static inline void append(Seq_T stream, Um_instruction inst)
{
        assert(sizeof(inst) <= sizeof(uintptr_t));
        Seq_addhi(stream, (void *)(uintptr_t)inst);
}

const uint32_t Um_word_width = 32;

void Um_write_sequence(FILE *output, Seq_T stream)
{
        assert(output != NULL && stream != NULL);
        int stream_length = Seq_length(stream);
        for (int i = 0; i < stream_length; i++) {
                Um_instruction inst = (uintptr_t)Seq_remlo(stream);
                for (int lsb = Um_word_width - 8; lsb >= 0; lsb -= 8) {
                        fputc(Bitpack_getu(inst, 8, lsb), output);
                }
        }
}


/* Unit tests for the UM */

void build_halt_test(Seq_T stream)
{
        append(stream, halt());
}

void build_LV_test(Seq_T stream)
{
        append(stream, loadval(r1, 6));
        append(stream, halt());
}

void build_verbose_halt_test(Seq_T stream)
{
        append(stream, loadval(r1, 'B'));
        append(stream, output(r1));
        append(stream, loadval(r1, 'a'));
        append(stream, output(r1));
        append(stream, loadval(r1, 'd'));
        append(stream, output(r1));
        append(stream, loadval(r1, '!'));
        append(stream, output(r1));
        append(stream, loadval(r1, '\n'));
        append(stream, output(r1));
        append(stream, halt());
}

void build_add(Seq_T stream)
{
        append(stream, add(r1, r2, r3));
        append(stream, halt());
}

void build_add_print(Seq_T stream)
{
        append(stream, loadval(r1, 48));
        append(stream, loadval(r2, 6));
        append(stream, add(r3, r1, r2));
        append(stream, output(r3));
        append(stream, halt());
}

void build_inNout(Seq_T stream) 
{
        append(stream, input(r1));
        append(stream, output(r1));
        append(stream, halt());
}

void build_multiply(Seq_T stream)
{
        append(stream, loadval(r1, 2));
        append(stream, loadval(r2, 102));
        append(stream, multiply(r3, r1, r2));
        append(stream, halt());
}

void build_multiply_big(Seq_T stream)
{
        append(stream, loadval(r1, 33554431));
        append(stream, loadval(r2, 33554431));
        append(stream, multiply(r3, r1, r2));
        append(stream, halt());
}

void build_divide(Seq_T stream)
{
        append(stream, loadval(r1, 130));
        append(stream, loadval(r2, 2));
        append(stream, loadval(r4, 131));
        append(stream, divide(r3, r1, r2));
        append(stream, output(r3));
        append(stream, divide(r3, r4, r2));
        append(stream, output(r3));
        append(stream, halt());
}

void build_bitNAND(Seq_T stream) 
{
        append(stream, loadval(r1, 2));
        append(stream, loadval(r2, 10));
        append(stream, nand(r3, r1, r2));
        append(stream, halt());
}


void build_bitNAND_same(Seq_T stream) 
{
        append(stream, loadval(r1, 1));
        append(stream, loadval(r2, 1));
        append(stream, nand(r3, r1, r2));
        append(stream, halt());
}

void build_store_segment(Seq_T stream)
{
        append(stream, loadval(r1, 1));
        append(stream, loadval(r2, 2));
        append(stream, map_segment(r1, r2));
        append(stream, loadval(r3, 97));
        append(stream, store_segment(r1, r2, r3));
        append(stream, load_segment(r3, r1, r2));
        append(stream, output(r3));
        append(stream, halt());
}

void build_load_program(Seq_T stream) 
{
        append(stream, loadval(r1, 6));
        append(stream, loadval(r2, 0));
        append(stream, loadval(r3, 5));
        append(stream, load_program(r2, r1));
        append(stream, divide(r1, r3, r2));
        append(stream, unmap_segment(r1));
        append(stream, loadval(r1, 100));
        append(stream, output(r1));
        append(stream, halt());
}

void build_map_and_unmap(Seq_T stream) 
{
        append(stream, loadval(r1, 2));
        append(stream, map_segment(r2, r1));
        append(stream, unmap_segment(r2));
        append(stream, halt());
}

void test_everything(Seq_T stream) 
{
        append(stream, loadval(r1, 3));
        append(stream, load_program(r2, r1));
        append(stream, halt());
        append(stream, loadval(r4, 96));
        append(stream, map_segment(r5, r1));
        append(stream, loadval(r6, 1));
        append(stream, store_segment(r5, r6, r4));
        append(stream, loadval(r4, 10));
        append(stream, load_segment(r4, r5, r6));
        append(stream, unmap_segment(r6));
        append(stream, add(r2, r4, r6));
        append(stream, output(r2));
        append(stream, loadval(r2, 96));
        append(stream, loadval(r3, 2));
        append(stream, loadval(r7, 6));
        append(stream, multiply(r1, r2, r3));
        append(stream, divide(r0, r1, r7));
        append(stream, output(r0));
        append(stream, halt());
}

void print_alphabet(Seq_T stream)
{
        append(stream, loadval(r1, 97));
        append(stream, loadval(r2, 1));

        for (int i = 0; i < 26; i++){
                append(stream, output(r1));
                append(stream, add(r1, r1, r2));
        }
}

void build_cmov(Seq_T stream) 
{
        append(stream, loadval(r1, 97));
        append(stream, loadval(r2, 103));
        append(stream, cmov(r3, r1, r2));
        append(stream, output(r3));
        append(stream, cmov(r3, r2, r4));
        append(stream, output(r3));
        append(stream, halt());
}

void map_unmap_remap(Seq_T stream)
{
        append(stream, loadval(r1, 3));
        append(stream, map_segment(r2, r1));
        append(stream, output(r2));
        append(stream, map_segment(r4, r1));
        append(stream, unmap_segment(r2));
        append(stream, map_segment(r6, r1));
        append(stream, loadval(r7, 90));
        append(stream, add(r6, r6, r7)); 
        append(stream, output(r6));
        append(stream, halt());
}