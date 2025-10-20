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
    execvp(args[0],args);
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
        printf("\n");
    }
}

/*
TO IMPLEMENT - Rishabh
Return:
    0 if no redirection
    1 if <
    2 if >
    3 if >>     
*/
int command_with_redirection(char *args[], int argsc)
{
    for(int i =0; i<argsc;i++){
        if(strcmp(args[i],"<") == 0){
            return INPUT_REDIRECTION;
        }
        if(strcmp(args[i],">") == 0){
            return OUTPUT_REDIRECTION_WRITE;
        }
        if(strcmp(args[i],">>") == 0){
            return OUTPUT_REDIRECTION_APPEND;
        }
    }
    return NO_REDIRECTION;
}

/*
Launch program function with redirection parameter
*/
void launch_program_with_redirection(char *args[], int argsc, int redirection)
{
    int rc = fork();
    
    if(rc<0){
        fprintf(stderr, "An error creating a child fork occured\n");
        exit(1);
    }
    else if (rc==0){
        switch (redirection)
        {
            case INPUT_REDIRECTION: 
                child_with_input_redirected(args, argsc);
                break;
            case OUTPUT_REDIRECTION_WRITE:
                child_with_output_redirected_write(args,argsc);
                break;
            case OUTPUT_REDIRECTION_APPEND:
                child_with_output_redirected_append(args,argsc);
                break;
        }
    }
    else{
        reap();
        printf("\n");
    }
}

/*
A function which extracts the filepath of a specifc redirection, and then removes both redirection operator and filepath from the args array
*/
void extract_redirection_file(char *args[], int *argsc, int redirection, char* filepath)
{
    int redirection_index = 0;

    for(int index = 0; index < *argsc; index++)
    {
        if(strcmp(args[index],">")==0 && redirection == OUTPUT_REDIRECTION_WRITE 
        || strcmp(args[index],">>")==0 && redirection == OUTPUT_REDIRECTION_APPEND 
        || strcmp(args[index],"<")==0 && redirection == INPUT_REDIRECTION)
        {
            redirection_index = index;
            strcpy(filepath,args[index+1]);
            if (DEBUG_PRINT) printf("Filepath: %s, Redirection operator: %i\n", filepath, redirection_index);
            break;
        }
    }

    args[redirection_index] = NULL;
    args[redirection_index + 1] = NULL;

    for(int index = redirection_index; index + 2 < *argsc; index++)
    {
        args[index] = args[index+2];
        args[index+2] = NULL;
    }

    *argsc -= 2;
}

/*
Handles children with input redirected
*/
void child_with_input_redirected(char *args[], int argsc)
{
    char filepath[MAX_LINE];
    extract_redirection_file(args,&argsc, INPUT_REDIRECTION, filepath);
    
    int fd = open(filepath, O_RDONLY);
    if(fd >= 0)
    {
        dup2(fd, STDIN_FILENO);
        close(fd);
        execvp(args[0],args); 
    }
    else if(DEBUG_PRINT)
        printf("An error occured opening %s\n", filepath);
} 

/*
TO IMPLEMENT - Rishabh
*/
void child_with_output_redirected_write(char *args[], int argsc)
{
    char filepath[MAX_LINE];
    extract_redirection_file(args,&argsc, OUTPUT_REDIRECTION_WRITE, filepath);
    int fd = open(filepath, O_WRONLY|O_CREAT,0644); 
    if(fd >= 0)
    {
        dup2(fd, STDOUT_FILENO);
        close(fd);
        execvp(args[0],args);         
    }
    else if(DEBUG_PRINT)
        printf("An error occured opening %s\n", filepath);
} 

/*
TO IMPLEMENT - Ryan
*/
void child_with_output_redirected_append(char *args[], int argsc)
{
    char filepath[MAX_LINE];
    extract_redirection_file(args,&argsc, OUTPUT_REDIRECTION_APPEND, filepath);
    
    int fd = open(filepath, O_WRONLY|O_CREAT|O_APPEND,0644);
    if(fd >= 0)
    {
        dup2(fd,STDOUT_FILENO);
        close(fd);
        execvp(args[0],args); 
    }
    else if(DEBUG_PRINT)
        printf("An error occured opening %s\n", filepath);
} 