#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assert.h"
#include "fmt.h"
#include "seq.h"

extern void Um_write_sequence(FILE *output, Seq_T instructions);

extern void build_halt_test(Seq_T instructions);
extern void build_verbose_halt_test(Seq_T instructions);
extern void build_add(Seq_T instructions);
extern void build_add_print(Seq_T instructions);
extern void build_inNout(Seq_T stream);
extern void build_multiply(Seq_T stream);
extern void build_multiply_big(Seq_T stream);
extern void build_divide(Seq_T stream);
extern void build_bitNAND(Seq_T stream);
extern void build_bitNAND_same(Seq_T stream);
extern void build_store_segment(Seq_T stream);
extern void build_load_program(Seq_T stream);
extern void build_map_and_unmap(Seq_T stream);
extern void test_everything(Seq_T stream);
extern void print_alphabet(Seq_T stream);
extern void build_cmov(Seq_T stream);
extern void map_unmap_remap(Seq_T stream);

/* The array `tests` contains all unit tests for the lab. */

static struct test_info {
        const char *name;
        const char *test_input;          /* NULL means no input needed */
        const char *expected_output;
        /* writes instructions into sequence */
        void (*build_test)(Seq_T stream);
} tests[] = {
        { "halt",         NULL, "", build_halt_test },
        { "halt-verbose", NULL, "", build_verbose_halt_test },
        { "add", NULL, "", build_add},
        {"print-six", NULL, "", build_add_print},
        { "inNout", NULL, "", build_inNout},
        { "multiply", NULL, "", build_multiply },
        { "multiply_big", NULL, "", build_multiply_big }, 
        { "build_divide", NULL, "", build_divide },
        { "NAND", NULL, "", build_bitNAND },
        { "NAND_same", NULL, "", build_bitNAND_same },
        { "seg_store", NULL, "", build_store_segment },
        { "load_p", NULL, "", build_load_program },
        { "map_and_unmap", NULL, "", build_map_and_unmap },
        { "test_all", NULL, "", test_everything },
        { "print_abc", NULL, "", print_alphabet },
        { "build_cmov", NULL, "", build_cmov },
        { "map_unmap_remap", NULL, "", map_unmap_remap }
};

  
#define NTESTS (sizeof(tests)/sizeof(tests[0]))

/*
 * open file 'path' for writing, then free the pathname;
 * if anything fails, checked runtime error
 */
static FILE *open_and_free_pathname(char *path);

/*
 * if contents is NULL or empty, remove the given 'path', 
 * otherwise write 'contents' into 'path'.  Either way, free 'path'.
 */
static void write_or_remove_file(char *path, const char *contents);

static void write_test_files(struct test_info *test);


int main (int argc, char *argv[])
{
        bool failed = false;
        if (argc == 1)
                for (unsigned i = 0; i < NTESTS; i++) {
                        printf("***** Writing test '%s'.\n", tests[i].name);
                        write_test_files(&tests[i]);
                }
        else
                for (int j = 1; j < argc; j++) {
                        bool tested = false;
                        for (unsigned i = 0; i < NTESTS; i++)
                                if (!strcmp(tests[i].name, argv[j])) {
                                        tested = true;
                                        write_test_files(&tests[i]);
                                }
                        if (!tested) {
                                failed = true;
                                fprintf(stderr,
                                        "***** No test named %s *****\n",
                                        argv[j]);
                        }
                }
        return failed; /* failed nonzero == exit nonzero == failure */
}


static void write_test_files(struct test_info *test)
{
        FILE *binary = open_and_free_pathname(Fmt_string("%s.um", test->name));
        Seq_T instructions = Seq_new(0);
        test->build_test(instructions);
        Um_write_sequence(binary, instructions);
        Seq_free(&instructions);
        fclose(binary);

        write_or_remove_file(Fmt_string("%s.0", test->name),
                             test->test_input);
        write_or_remove_file(Fmt_string("%s.1", test->name),
                             test->expected_output);
}


static void write_or_remove_file(char *path, const char *contents)
{
        if (contents == NULL || *contents == '\0') {
                remove(path);
        } else {
                FILE *input = fopen(path, "wb");
                assert(input != NULL);

                fputs(contents, input);
                fclose(input);
        }
        free(path);
}


static FILE *open_and_free_pathname(char *path)
{
        FILE *fp = fopen(path, "wb");
        assert(fp != NULL);

        free(path);
        return fp;
}
