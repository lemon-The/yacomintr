#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include "yastring.h"
#include "yalist.h"
enum {
    true = 1,
    false = 0,
    increment = 6,                  /*to increase string*/
    max_separator_len = 2,          /*max number of chars in separator 
                                    *string*/   
    separator_chars_amount = 7,     /*size of separator array*/
    separator_syntax_correct = 0,   /*take_separator_from_string()
                                    *output*/
    separator_syntax_error = 1,
    string_ended = 0,               /*word ending flags*/
    separator_occured = 1,
    gap_occured = 2,
    unmathced_quotes = 3,
    new_line_occured = 0,           /*string ending flags*/
    eof_occured = 1
};

const char *separators[] = {">>",   "&&",   /*available separators*/
                            "||",   ">",
                            "<",    "|",
                            ";",    "(",
                            ")",    "&"};

const char separator_chars[separator_chars_amount] = {'>', '<', '|', ';',
    '(', ')', '&'}; /*characters that are contained in separators*/

/*puts together program's name and paramaters into array*/
char **assemble_commandline(commandline *cmdl)
{
    char **cmdline;
    char **cmdtmp;
    int cmdlength = 0;
    commandline *tmp;
    for(tmp = cmdl; tmp; tmp=tmp->next)
        cmdlength++;
    cmdline = malloc(sizeof(char*)*(cmdlength+1));
    cmdtmp = cmdline;
    for(tmp = cmdl; tmp; tmp = tmp->next){
        *cmdtmp = tmp->word;
        cmdtmp++;
    }
    *cmdtmp = NULL;
    return cmdline;
}

/*starts new process in another process group*/
int exec_prog(pipeline *ppl, int pgid)
{
    char **cmdline;
    int pid;
    if(!pgid)
        pgid = getpid();
    cmdline = assemble_commandline(ppl->args);
    pid = fork();
    if(pid == -1) {
        perror("fork");
        exit(1);
    } else if(pid == 0) {
        setpgid(getpid(), pgid);    /*init new proccess*/
        tcsetpgrp(0, pgid);
        signal(SIGTTIN, SIG_DFL);
        signal(SIGTTOU, SIG_DFL);
        signal(SIGINT, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);
        if(ppl->i_stream != 0){     /*replace i/o streams if they are*/
            dup2(ppl->i_stream, 0); /*not standart*/
            close(ppl->i_stream);
        }
        if(ppl->o_stream != 1){
            dup2(ppl->o_stream, 1);
            close(ppl->o_stream);
        }
        execvp(*cmdline, cmdline);
        perror("execv");
        exit(2);
    }
    setpgid(pid, pgid);
    tcsetpgrp(0, pgid);
    free(cmdline);
    return pid;
}

/*waits for particular process*/
void wait_prog(int p)
{
    int status;
    int pid;
    while((pid = wait(&status)) != -1){
        if(WIFEXITED(status))
            printf("\n[%d] exited with code: %d\n", pid,
                WEXITSTATUS(status));
        else
            printf("\n[%d] killed by code: %d\n", pid, WTERMSIG(status));
        if(pid == p)
            break;
    }
}

/*checks for finished processes*/
void check_zombies()
{
    int status;
    int pid;
    while((pid = wait4(-1, &status, WNOHANG, NULL)) != -1 && pid != 0){
        if(WIFEXITED(status))
            printf("[%d] exited with code: %d\n", pid,
                WEXITSTATUS(status));
        else
            printf("[%d] killed by code: %d\n", pid, WTERMSIG(status));
    }
}

/*Is chararacter contained in a separator_chars array*/
int is_it_separator_char(char c)
{
    int i;
    for(i = 0; i < separator_chars_amount; i++)
        if(separator_chars[i] == c)
            return true;
    return false;
}

/*checks for word ending characters*/
int is_word_ended(char c, int ignr_sprtr)
{
    if(c == 0 || ((c == '\t' || c == ' ' || is_it_separator_char(c))
        && !ignr_sprtr))
        return true;
    return false;
}

/*gets string from standart input stream*/
int read_string_from_stdin(char **str)
{
    char c;
    int i = 0;
    char *string = NULL;
    init_string(&string, increment);
    while((c = getchar()) != EOF){
        if(c == '\n')
            break;
        if(i >= get_string_len(string))
            increase_string(&string, increment);
        string[i] = c;
        i++;
    }
    string[i] = 0;
    *str = string;
    if(c == EOF)
        return eof_occured;
    else
        return new_line_occured;
}

/*retrieves word from string from specific position*/
int take_word_from_string(const char *str, int *pos, char **wrd)
{
    char c;
    int i = *pos;
    int j = 0;
    int separator_as_char = false;  /*include any chars to the word*/
    char *word = NULL;
    if(i >= get_string_len(str))
        return string_ended;
    init_string(&word, increment);
    for(; !is_word_ended(c = str[i], separator_as_char); i++){
        if(c == '"'){
            separator_as_char = !separator_as_char;
        } else{
            if(j >= get_string_len(word))
                increase_string(&word, increment);
            if(c =='\\')            /*include one any char to the word*/
                word[j] = str[++i];
            else                    /*include usual char to the word*/
                word[j] = c;
            j++;
        }
    }
    *pos = i;
    if(separator_as_char){          /*if there is not closing quote*/
        delete_string(&word);
        return unmathced_quotes;
    }
    word[j] = 0;
    if(!*word)
        delete_string(&word);       /*free word memory if word is empty*/
    *wrd = word;
    if(c == 0)
        return string_ended;
    else if(c == '\t' || c == ' ')
        return gap_occured;
    else
        return separator_occured;
}

/*put together words related to one program*/
int assemble_cmdline(const char *str, commandline **cmdl, int *pos)
{
    int i = *pos;
    char *word = NULL;
    int wrd_end_code;
    commandline *cmdline = NULL;
    do {    /*retrieve words until end of string or separator char*/
        wrd_end_code = take_word_from_string(str, &i, &word);
        if(word)
            add_to_commandline_end(&cmdline, word);
        delete_string(&word);
        if(wrd_end_code == gap_occured)
            i++;
    } while(wrd_end_code != string_ended &&
        wrd_end_code != separator_occured);
    *cmdl = cmdline;
    *pos = i;
    if(wrd_end_code == separator_occured){
        return separator_occured;
    } else if(wrd_end_code == string_ended){
        return string_ended;
    } else{
        delete_commandline(&cmdline);
        return unmathced_quotes;
    }
}

/*retrieves separator from string from specific position*/
int take_separator_from_string(const char *str, char **sprtr, int *pos)
{
    int i = *pos;
    int j = 0;
    char *separator = NULL;
    const char **tmp = separators;
    init_string(&separator, max_separator_len);
    for(; is_it_separator_char(str[i]) && separator[j]; i++, j++)
        separator[j] = str[i];
    separator[j] = 0;
    *pos = i;
    for(; *tmp; tmp++){
        if(check_string_equality(separator, *tmp)){
            *sprtr = separator;
            return separator_syntax_correct;
        }
    }
    delete_string(&separator);
    *sprtr = NULL;
    return separator_syntax_error;
}

/*closes inuput/output streams if they are not standart streams*/
void close_io_streams(int in, int out)
{
    if(in != 0) 
        close(in);
    if(out != 1)
        close(out);
}

/*retrieves file name from specific position*/
int get_file_name(const char *str, int *pos, char **fname)
{
    char *file_name = NULL;
    int wrd_end_code;
    do {
        wrd_end_code = take_word_from_string(str, pos, &file_name);
        if(!file_name && wrd_end_code == gap_occured){
            (*pos)++;
            delete_string(&file_name);
        } else if(wrd_end_code == unmathced_quotes){
            fprintf(stderr, "Error: unmathced quotes\n");
            delete_string(&file_name);
            return 1;
        } else if(!file_name && wrd_end_code == string_ended){
            fprintf(stderr, "Error: file name didn't entered\n");
            delete_string(&file_name);
            return 1;
        }
    } while(!file_name);
    *fname = file_name;
    return 0;
}

/*changes input or output stream for the process*/
int change_io_stream(pipeline *ppl, const char *str, const char *sprtr,
    int *pos)
{
    char *file_name = NULL;
    int fd;
    get_file_name(str, pos, &file_name);
    if(check_string_equality(sprtr, ">")){
        fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, 0666); 
    } else if(check_string_equality(sprtr, "<")){
        fd = open(file_name, O_RDONLY);
    } else if(check_string_equality(sprtr, ">>")){
        fd = open(file_name, O_WRONLY | O_APPEND);
    } 
    delete_string(&file_name);
    if(fd == -1){
        perror("open");
        fprintf(stderr, "Opening file failed, stopped\n");
        return 1;
    }
    //fflush(stdout);
    if(check_string_equality(sprtr, ">") || check_string_equality(sprtr, ">>")){
        ppl->o_stream = fd;
    } else if(check_string_equality(sprtr, "<")){
        ppl->i_stream = fd;
    } 
    return 0;
}

/*executes programs contained in a pipeline*/
int start_pipeline_prog(pipeline **ppl, int pgid)
{
    if(!*ppl)
        return 0;
    int fd[2];
    pipe(fd);
    if(!(*ppl)->i_stream)
        (*ppl)->i_stream = start_pipeline_prog(&((*ppl)->next), pgid);
    (*ppl)->o_stream = fd[1];
    exec_prog(*ppl, pgid);
    close(fd[1]);
    close_io_streams((*ppl)->i_stream, (*ppl)->o_stream);
    return fd[0];
}

/* retrieves next program from string from specific position and adds it
 * to pipeline */
void make_pipeline(pipeline **ppl, const char *str, int *pos)
{
    int cmdline_st;
    int spr_status;
    commandline *tmp = NULL;
    char *sprtr = NULL;
    /*retrieve next program until sprtr != "|"*/
    do{
        cmdline_st = assemble_cmdline(str, &tmp, pos);
        if(tmp){
            add_program(ppl, tmp);
            delete_commandline(&tmp);
        }
        if(cmdline_st == separator_occured){
            spr_status = take_separator_from_string(str, &sprtr, pos);
            if(spr_status != separator_syntax_correct ||
                !check_string_equality(sprtr, "|"))
            {
                delete_string(&sprtr);
                (*pos)--;
                break;
            }
        }
        delete_string(&sprtr);
    } while(cmdline_st == separator_occured);
    if(!(*ppl)->next){
        delete_pipeline(ppl);
        return;
    }
}

/*starts program*/
int start_prog(pipeline **ppl, const char *str, int *pos)
{
    int pid;
    int pgid = 0;
    /*if it is not pipeline*/
    if(!(*ppl)->next){
        pid = exec_prog(*ppl, pgid);
        return pid;
    }
    /*start pipeline*/
    pid = fork();
    if(pid == 0){
        setpgid(getpid(), getpid());
        tcsetpgrp(0, getpid());
        signal(SIGTTIN, SIG_DFL);
        signal(SIGTTOU, SIG_DFL);
        signal(SIGINT, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);
        (*ppl)->i_stream = start_pipeline_prog(&((*ppl)->next), getpid());
        pid = exec_prog(*ppl, pgid);
        if((*ppl)->i_stream != 0)
            close((*ppl)->i_stream);
        while(wait(NULL) != -1)
            {}
        exit(0);
    }
    setpgid(pid, pid);
    tcsetpgrp(0, pid);
    return pid;
}

/*perfoms operations depending on separator*/
int handle_separator(pipeline **ppl, const char *sprtr, const char *str,
    int *pos) 
{
    check_zombies();
    if(!*ppl || !(*ppl)->args){
        fprintf(stderr, "error\n");
        delete_pipeline(ppl);
        return 0;
    } else if(check_string_equality((*ppl)->args->word, "cd")){
        /*if there is only one argument*/
        if((*ppl)->args->next && !(*ppl)->args->next->next){ 
            chdir((*ppl)->args->next->word);
        } else if(!(*ppl)->next){
            chdir(getenv("HOME"));
        } else{
            fprintf(stderr, "cd: to many arguments\n");
        }
        delete_pipeline(ppl);
        return 0;
    } else if(check_string_equality(sprtr, "&")){
        start_prog(ppl, str, pos);
        delete_pipeline(ppl);
        return 0;
    } else if(check_string_equality(sprtr, "\0")){
        int pid = start_prog(ppl, str, pos);
        wait_prog(pid);
        delete_pipeline(ppl);
        return 0;
    } else if(check_string_equality(sprtr, ">") ||
            check_string_equality(sprtr, "<") ||
            check_string_equality(sprtr, ">>"))
    {
        change_io_stream(*ppl, str, sprtr, pos);
        return 0;
    } else if(check_string_equality(sprtr, "|")) {
        make_pipeline(ppl, str, pos);
        return 0;
    } else{
        fprintf(stderr, "Feature not implemented yet\n");
        delete_pipeline(ppl);
        return 0;
    }
}

void process_string(char *string)
{
    pipeline *progs = NULL;
    commandline *cmdline = NULL;
    char *separator = NULL;
    int i = 0;
    int spr_status;
    int cmdline_end_code;
    init_pipeline(&progs);
    do {
        cmdline_end_code = assemble_cmdline(string, &cmdline, &i);
        if(cmdline){
            add_program(&progs, cmdline);
            delete_commandline(&cmdline);
        }
        if(cmdline_end_code == separator_occured){
            spr_status = take_separator_from_string(string, &separator, 
                &i);
            if(spr_status == separator_syntax_correct){
                handle_separator(&progs, separator, string, &i);
            } else{
                delete_pipeline(&progs);
                fprintf(stderr, "Incorrect separator syntax\n");
            }
            delete_string(&separator);
        } else if(cmdline_end_code == unmathced_quotes){
            fprintf(stderr, "Unmatched quotes\n");
            delete_pipeline(&progs);
        }
    } while(cmdline_end_code != string_ended);
    if(progs && progs->args)
        handle_separator(&progs, "\0", NULL, NULL);
    else
        check_zombies();
    delete_pipeline(&progs);
}

void init_shell()
{
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    if(tcgetpgrp(0) != getpid()){
        if(getpgid(getpid()) != getpid())
            setpgid(getpid(), getpid());
        tcsetpgrp(0, getpid());
    }
}

int main()
{
    int eof_flag = false;
    char *string = NULL;
    init_shell();
    while(!eof_flag){
        tcsetpgrp(0, getpid());
        printf("> ");
        eof_flag = read_string_from_stdin(&string);
        process_string(string);
        delete_string(&string);
    }
    printf("\n");
    check_zombies();
    return 0;
}
