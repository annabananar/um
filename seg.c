/**************************************************************
 *
 *                     seg.c
 *
 *     Assignment: HW6 um
 *     Authors: Jason Singer, Anna Zou
 *     Date: April 10, 2024
 *
 *     A module that handles the segments and the memory for each segment; 
       is responsible for initializing, removing, retrieving, and duplicating 
       memories/segments.
 *
 **************************************************************/

#include "seg.h"
#include <stdio.h>
#include <bitpack.h>
#include <assert.h>
#include <string.h>

#define SEQ_HINT 100
#define WORDSIZE 4
#define CHARBITS 8
#define ONE 1 

/*the allSegements struct contains a sequence storing pointers to all the 
mapped segments, with the index of the sequence corresponding to the segement
ID. It also contians a sequence of all unmapped IDs, so that unmapped IDs can
be stored and reused*/
struct allSegments
{
        Seq_T mapped;
        Seq_T unmapped;
};

/*the segments struct represents a single segment of the UM. Its member 
variables represent the number of words an instance of a segment can store, 
and a pointer to an array of 32 bit integers representing the memory itself */
struct segment
{
      uint32_t numWords;
      uint32_t *memory;  
};

/*helper functions */
segment init_seg0(FILE *instructions);
void fill_seg0(FILE* instructions, segment seg0);
segment copy(segment seg);




/************init_allSegs****************************************
*
* Description: Function that inilizes an instance of the allSegments struct
*
* Parameters: FILE *instructions: pointer to a file with bitpacked UM 
*                                 instructions
* Returns: void
*
* Expects: instructions != NULL
*      
* Notes: N/A
**************************************************************/
allSegments init_allSegs(FILE *instructions)
{
        assert(instructions);
        allSegments umSegments = malloc(sizeof(struct allSegments));
        Seq_T mapped = Seq_new(SEQ_HINT);
        Seq_T unmapped = Seq_new(SEQ_HINT);
        umSegments->mapped = mapped;
        umSegments->unmapped = unmapped;

        segment seg0 = init_seg0(instructions);
        Seq_addhi(umSegments->mapped, seg0);
        return umSegments;
}

/************init_seg0****************************************
*
* Description: Function that inilizes segment 0 of the UM
*
* Parameters: FILE *instructions: pointer to a file with bitpacked UM 
*                                 instructions
*
* Returns: a pointer to a segement struct representing segment 0
*
* Expects: instructions != NULL
*      
* Notes: N/A
**************************************************************/
segment init_seg0(FILE *instructions)
{
        assert(instructions != NULL);
        uint32_t numInstructions = 0;
        int i = 0;
        while (!(feof(instructions))){
                fgetc(instructions);
                i++;
                if (i == 4){
                        i = 0;
                        numInstructions++;
                }
        }
        rewind(instructions);

        segment seg0 =  malloc(sizeof(struct segment));
        seg0->numWords = numInstructions;
        uint32_t *mem = (uint32_t *)calloc(numInstructions, 
                        sizeof(uint32_t));
        assert(mem != NULL);
        seg0->memory = mem;
        fill_seg0(instructions, seg0);    
        return seg0;
}


/************fill_seg0****************************************
*
* Description: Function that fills the memory array of segment 0 with 
*              instructions from a file
*
* Parameters: FILE *instructions: pointer to a file with bitpacked UM 
*                                 instructions
*             segment seg0: an intilized segment 0
*
* Returns: void
*
* Expects: instructions != NULL, seg0 != NULL
*      
* Notes: alters the memory array in seg0
**************************************************************/
void fill_seg0(FILE* instructions, segment seg0)
{
        assert(instructions);
        assert(seg0);
        for (uint32_t i = 0; i < seg0->numWords; i++){
                uint32_t word = 0;
                for (int j = 1; j <= WORDSIZE; j++){
                        word = Bitpack_newu(word, CHARBITS, 
                                (CHARBITS * WORDSIZE) - (CHARBITS * j), 
                                fgetc(instructions));
                }
                seg0->memory[i] = word;
        }
}

/************init_segment****************************************
*
* Description: Function that initilzes a segment with a given number
* of words and assigns that segment an identifier
*
* Parameters: uint32_t numWords: the number of words in the new segment
*             allSegments umSegs: an inilized allSegments struct
*
* Returns: the ID of the new segment as a 32bit unsigned integer
*
* Expects: umSegs != NULL, and memory allocation suceeds
*      
* Notes: The ID the new segment is assigned will always be an unmapped
*       segment's ID unless there are no unmapped IDs to be used
**************************************************************/
uint32_t init_segment(uint32_t numWords, allSegments umSegs)
{
        assert(umSegs);
        segment newSeg = malloc(sizeof(struct segment));
        assert(newSeg);
        newSeg->numWords = numWords;
        uint32_t *memory = (uint32_t *)calloc(numWords, sizeof(uint32_t));
        assert(memory);
        newSeg->memory = memory;

        /*conditional checking if there are no unmapped IDs to use*/
        if (Seq_length(umSegs->unmapped) == 0){
                Seq_addhi(umSegs->mapped, newSeg);
                return Seq_length(umSegs->mapped) - ONE;
        }
        uint32_t id = (uint32_t )(uintptr_t)Seq_remhi(umSegs->unmapped);
        Seq_put(umSegs->mapped, id, newSeg);
        return id;
}


/************copy_and_replace****************************************

* Description: Function that copies a specified segment and replaces segment
*              0 with the specified segment
*
* Parameters: allSegments umSegs: an intilized allSegments struct
*             uint32_t id: the id of the segment to be copied
*
* Returns: a pointer to the newly copied segment
*
* Expects: umSegs != NULL
*      
* Notes: UM will fail if the ID does not correspond to a mapped segment
**************************************************************/
segment copy_and_replace(allSegments umSegs, uint32_t id)
{
        assert(umSegs);
        segment toCopy = Seq_get(umSegs->mapped, id);
        segment copied = copy(toCopy);

        segment oldSegment = Seq_put(umSegs->mapped, 0, copied);
        free_segment(oldSegment);

        return copied;
}

/************copy****************************************
*
* Description: Function that copies a segment
*
* Parameters: segment seg: the segment being copied
*
* Returns: a pointer to the newly copied segment
*
* Expects: segment != NULL, and memory allocation suceeds
*      
* Notes: The ID the new segment is assigned will always be an unmapped
*       segment's ID unless there are no unmapped IDs to be used
**************************************************************/
segment copy(segment seg)
{
        assert(seg);
        segment newSeg = malloc(sizeof(struct segment));
        assert(newSeg);
        newSeg->numWords = seg->numWords;
        uint32_t *memory = malloc(sizeof(uint32_t) * seg->numWords);
        memcpy(memory, seg->memory, sizeof(uint32_t) * seg->numWords);
        newSeg->memory = memory;
        
        return newSeg;
}


/************free_allSegments****************************************
*
* Description: Function that frees all memory associated with segments
*
* Parameters: allSegemnts umSegs: an inilized allSegments struct
*
* Returns: void
*
* Expects: umSegs != NULL
*      
* Notes: N/A
**************************************************************/
void free_allSegments(allSegments umSegs) 
{
        assert(umSegs);
        for (int i = 0; i < Seq_length(umSegs->mapped); i++) {
                if (Seq_get(umSegs->mapped, i) != NULL){
                        segment thisSegment = 
                                (segment)Seq_get(umSegs->mapped, i);
                        free_segment(thisSegment);
                }
        }
        Seq_free(&(umSegs->mapped));
        Seq_free(&(umSegs->unmapped));
        free(umSegs);
}

/************free_segment****************************************
*
* Description: Function that frees the memory associated with a single segment
*
* Parameters: segment seg: a pointer to an inilized segment struct
*
* Returns: void
*
* Expects: seg != NULL
*      
* Notes: N/A
**************************************************************/
void free_segment(segment seg) 
{
        assert(seg);
        free(seg->memory);
        free(seg);
}

/************unmap_id****************************************
*
* Description: Function that changes the mapped and unmapped sequences when
* unmapping a segment
*
* Parameters: uint32_t id: the id of the segment being unmapped
*             allSegments umSegs: a pointer to an inilized allSegments struct
*
* Returns: void
*
* Expects: umSegs != NULL
*      
* Notes: N/A
**************************************************************/
void unmap_id(uint32_t id, allSegments umSegs) 
{
       
        assert(umSegs);
        Seq_put(umSegs->mapped, id, NULL);
        Seq_addhi(umSegs->unmapped, (void *)(uintptr_t)id);

}

/************get_segment****************************************
*
* Description: Function that serves as a getter for other modules in order to
*              get a segment
*              
* Parameters: allSegments umSegs: a pointer to an inilized allSegments struct
*             uint32_t id: the id of the segment being unmapped
*x 
* Returns: the segment with the given ID
*
* Expects: umSegs != NULL and id < the number of mapped segments
*      
* Notes: if the provided ID is of an unmapped segment, function will return 
*        NULL
**************************************************************/
segment get_segment(allSegments umSegs, uint32_t id)
{
        assert(umSegs);
        assert(id < (uint32_t)Seq_length(umSegs->mapped));
        segment thisSeg = (segment)Seq_get(umSegs->mapped, id);
        return thisSeg;
}

/************get_segment****************************************
*
* Description: Function that serves as a getter for other modules in order to
* directly interact with the memory 
*              
* Parameters: sgment: a pointer to an inilized segment struct
*
* Returns: the memory pointer of the segment
*
* Expects: seg != NULL
*      
* Notes: if the provided ID is of an unmapped segment, function will return 
*        NULL
**************************************************************/
uint32_t *get_mem(segment seg)
{
        assert(seg);
        return seg->memory;
}