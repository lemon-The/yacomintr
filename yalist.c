#include "yalist.h"
void init_commandline(commandline **cmdl)
{
    *cmdl = NULL;
}

void delete_commandline(commandline **cmdl)
{
    while(*cmdl){
        free((*cmdl)->word);
        struct word_item *tmp = (*cmdl);
        (*cmdl) = (*cmdl)->next;
        free(tmp);
    }
    *cmdl = NULL;
}

void add_to_commandline_end(commandline **cmdl, char *word)
{
    if(!word)
        return;
    struct word_item *new = malloc(sizeof(struct word_item));
    struct word_item *tmp = *cmdl;
    new->word = malloc(sizeof(char)*(get_string_len(word)+1));
    copy_string(new->word, word);
    new->next = NULL;
    if(!tmp){
        *cmdl = new;
    } else{
        for(; tmp->next; tmp = tmp->next)
            {}
        tmp->next = new;
    }
}

void print_commandline(commandline *cmdl)
{
    for(; cmdl; cmdl = cmdl->next)
        printf("[%s]", cmdl->word);
    putchar('\n');
}

////////////////////////////////////////////////////////////

struct word_item *copy_slist(struct word_item *src)
{
    struct word_item *new;
    int str_len;
    if(!src)
        return NULL;
    str_len = get_string_len(src->word);
    new = malloc(sizeof(struct word_item));
    new->word = NULL;
    init_string(&new->word, str_len);
    copy_string(new->word, src->word);
    new->next = copy_slist(src->next);
    return new;
}

void init_pipeline(pipeline **ppl)
{
    /**lst = malloc(sizeof(struct prog_item));
    (*lst)->args = NULL;
    (*lst)->next = NULL;
    (*lst)->i_stream = 0;
    (*lst)->o_stream = 1;*/
    *ppl = NULL;
}

void delete_pipeline(pipeline **ppl)
{
    while(*ppl){
        delete_commandline(&(*ppl)->args);
        if((*ppl)->i_stream != 0)
            close((*ppl)->i_stream);
        if((*ppl)->o_stream != 1)
            close((*ppl)->o_stream);
        struct prog_item *tmp = (*ppl);
        (*ppl) = (*ppl)->next;
        free(tmp);
    }
    *ppl = NULL;
}

void add_to_pipeline_end(pipeline **ppl, commandline *cmdl)
{
    if(!cmdl)
        return;
    struct prog_item *new = malloc(sizeof(struct prog_item));
    struct prog_item *tmp = *ppl;
    new->args = malloc(sizeof(struct word_item));
    new->next = NULL;
    new->i_stream = 0;
    new->o_stream = 1;
    //new->gid = 0;
    if(!tmp){
        *ppl = new;
    } else{
        for(; tmp->next; tmp = tmp->next)
            {}
        tmp->next = new;
    }
}

void print_pipeline(pipeline *ppl)
{
    for(; ppl; ppl = ppl->next){
        struct word_item *tmp = ppl->args;
        for(; tmp; tmp = tmp->next)
            printf("[%s]", tmp->word);
        printf("\nInput stream:     [%d]", ppl->i_stream);
        printf("\nOutput stream:    [%d]\n", ppl->o_stream);
    }
    putchar('\n');
}

void add_program(pipeline **ppl, commandline *cmdl)
{
    if(!cmdl)
        return;
    struct prog_item *new = malloc(sizeof(struct prog_item)); 
    new->args = copy_slist(cmdl);
    //new->args = cmdline;
    new->i_stream = 0;
    new->o_stream = 1;
    //new->gid = 0;
    new->next = *ppl;
    *ppl = new;
}

/*void add_word(pipeline **ppl, char *word)
{
    struct word_item *new = malloc(sizeof(struct word_item));
    struct word_item *tmp = NULL;
    if(!word)
        return;
    if(!*ppl)
        init_pipeline(ppl);
    tmp = (*ppl)->args;
    new->word = malloc(sizeof(char)*(get_string_len(word)+1));
    copy_string(new->word, word);
    new->next = NULL;
    if(!tmp){
        (*ppl)->args = new;
    } else{
        for(; tmp->next; tmp = tmp->next)
            {}
        tmp->next = new;
    }
}*/

void change_io_streams(pipeline *ppl, int i, int o)
{
    ppl->i_stream = i;
    ppl->o_stream = o;
}

////////////////////////////////////////////////////////////

/*void init_ilist(struct num_item **lst){
    *lst = NULL;
}

void delete_ilist(struct num_item **lst){
    while(*lst){
        struct num_item *tmp = (*lst);
        (*lst) = (*lst)->next;
        free(tmp);
    }
    *lst = NULL;
}

void add_to_ilist_end(struct num_item **lst, int num){
    struct num_item *new = malloc(sizeof(struct num_item));
    struct num_item *tmp = *lst;
    new->num = num;
    new->next = NULL;
    if(!tmp){
        *lst = new;
    } else{
        for(; tmp->next; tmp = tmp->next)
            {}
        tmp->next = new;
    }
}

void print_ilist(struct num_item *lst){
    for(; lst; lst = lst->next)
        printf("[%d]", lst->num);
    putchar('\n');
}

int get_ilist_length(struct num_item *lst){
    int res;
    for(res = 0; lst; lst = lst->next, res++)
        {}
    return res;
}*/
