#ifndef YALIST_H_SENTINEL
#define YALIST_H_SENTINEL
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "yastring.h"
struct word_item{
    char *word;
    struct word_item *next;
} typedef commandline;

/*struct num_item{
    int num;
    struct num_item *next;
};*/

struct prog_item{
    struct word_item *args;
    int i_stream;
    int o_stream;
    struct prog_item *next;
} typedef pipeline;

void init_commandline(commandline **cmdl);

void delete_commandline(commandline **cmdl);

void add_to_commandline_end(commandline **cmdl, char *word);

void print_commandline(commandline *cmdl);

//////////////////////////////////////////////////////////////////////

void init_pipeline(pipeline **ppl);

void delete_pipeline(pipeline **ppl);

void add_to_pipeline_end(pipeline **ppl, commandline *cmdl);

void add_program(pipeline **ppl, commandline *cmdl);

void print_pipeline(pipeline *ppl);

//void delete_first_program(pipeline **ppl);

//void add_word(pipeline **ppl, char *word);

void change_io_streams(pipeline *ppl, int i, int o);

//////////////////////////////////////////////////////////////////////

/*void init_ilist(struct num_item **lst);

void delete_ilist(struct num_item **lst);

void add_to_ilist_end(struct num_item **lst, int num);

void print_ilist(struct num_item *lst);

int get_ilist_length(struct num_item *lst);*/

#endif
