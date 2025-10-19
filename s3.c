#include "s3.h"

///Simple for now, but will be expanded in a following section
void construct_shell_prompt(char shell_prompt[])
{
    strcpy(shell_prompt, "[s3]$ ");
}

///Prints a shell prompt and reads input from the user
void read_command_line(char line[])
{
    char shell_prompt[MAX_PROMPT_LEN];
    construct_shell_prompt(shell_prompt);
    printf("%s", shell_prompt);

    ///See man page of fgets(...)
    if (fgets(line, MAX_LINE, stdin) == NULL)
    {
        perror("fgets failed");
        exit(1);
    }
    ///Remove newline (enter)
    line[strlen(line) - 1] = '\0';
}

void parse_command(char line[], char *args[], int *argsc)
{
    ///Implements simple tokenization (space delimited)
    ///Note: strtok puts '\0' (null) characters within the existing storage, 
    ///to split it into logical cstrings.
    ///There is no dynamic allocation.

    ///See the man page of strtok(...)
    char *token = strtok(line, " ");
    *argsc = 0;

    if(DEBUG_PRINT) printf("Parsed input: \n");

    while (token != NULL && *argsc < MAX_ARGS - 1)
    {
        if(DEBUG_PRINT) printf("    [%i] %s\n", *argsc, token);
        args[(*argsc)++] = token;
        token = strtok(NULL, " ");
    }
    
    args[*argsc] = NULL; ///args must be null terminated
}

///Launch related functions
void child(char *args[], int argsc)
{
    ///Implement this function:

    ///Use execvp to load the binary 
    ///of the command specified in args[ARG_PROGNAME].
    ///For reference, see the code in lecture 3.
}

/*
Creates a child process and calls child() from it to execute the command
*/
void launch_program(char *args[], int argsc)
{
    if(strcmp(args[0],"exit") == 0)
        exit(0);

    int rc = fork();
    
    if(rc<0){
        fprintf(stderr, "An error creating a child fork occured\n");
        exit(1);
    }
    else if (rc==0){
        child(args, argsc);
    }
    else{
        reap();
    }

    
}