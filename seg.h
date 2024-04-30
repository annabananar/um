/**************************************************************
 *
 *                     seg.h
 *
 *     Assignment: HW6 um
 *     Authors: Jason Singer, Anna Zou
 *     Date: April 10, 2024
 *
 *     Interface for the segments module; contains declarations of 
       functions that work with segments (refer to the seg.c header for more 
       details on what the segment module does), as well as two privately 
       defined structs of type allSegments and segment.
 *
 **************************************************************/

#include <stdlib.h>
#include <seq.h>
#include <except.h>
#include <stdint.h>
#include <stdio.h>

#ifndef SEG_H_
#define SEG_H_
struct allSegments;
typedef struct allSegments *allSegments;

struct segment;
typedef struct segment *segment;

allSegments init_allSegs(FILE *instructions);
uint32_t init_segment(uint32_t numWords, allSegments umSegs);
void free_allSegments(allSegments umSegs);
void free_segment(segment seg);
void unmap_id(uint32_t id, allSegments umSegs);
segment copy_and_replace(allSegments umSegs, uint32_t id);

/*Functionf for other modules to interact with segments*/
segment get_segment(allSegments umSegs, uint32_t id);
uint32_t *get_mem(segment seg);

#endif