#include "s3.h"

/// Simple for now, but will be expanded in a following section
void construct_shell_prompt(char shell_prompt[])
{
    char cwd[MAX_PROMPT_LEN - 6];
    getcwd(cwd, 100);
    sprintf(shell_prompt, "[%s s3]$", cwd);
}


/// Prints a shell prompt and reads input from the user
void read_command_line(char line[])
{
    char shell_prompt[MAX_PROMPT_LEN];
    construct_shell_prompt(shell_prompt);
    printf("%s", shell_prompt);

    /// See man page of fgets(...)
    if (fgets(line, MAX_LINE, stdin) == NULL)
    {
        perror("fgets failed");
        exit(1);
    }
    /// Remove newline (enter)
    line[strlen(line) - 1] = '\0';
}

void parse_command(char line[], char *args[], int *argsc)
{
    /// Implements simple tokenization (space delimited)
    /// Note: strtok puts '\0' (null) characters within the existing storage,
    /// to split it into logical cstrings.
    /// There is no dynamic allocation.

    /// See the man page of strtok(...)
    char *token = strtok(line, " ");
    *argsc = 0;

    if (DEBUG_PRINT)
        printf("Parsed input: \n");

    while (token != NULL && *argsc < MAX_ARGS - 1)
    {
        if (DEBUG_PRINT)
            printf("    [%i] %s\n", *argsc, token);
        args[(*argsc)++] = token;
        token = strtok(NULL, " ");
    }

    args[*argsc] = NULL; /// args must be null terminated
}

/// Launch related functions
void child(char *args[], int argsc)
{
    execvp(args[0], args);
}

/*
Creates a child process and calls child() from it to execute the command
*/
void launch_program(char *args[], int argsc)
{
    if (strcmp(args[0], "exit") == 0)
        exit(0);

    int rc = fork();

    if (rc < 0)
    {
        fprintf(stderr, "An error creating a child fork occured\n");
        exit(1);
    }
    else if (rc == 0)
    {
        child(args, argsc);
    }
    else
    {
        reap();
        printf("\n");
    }
}

/*
Determines which redirection a command has
*/
int command_with_redirection(char *args[], int argsc)
{
    for (int i = 0; i < argsc; i++)
    {
        if (strcmp(args[i], "<") == 0)
        {
            return INPUT_REDIRECTION;
        }
        else if (strcmp(args[i], ">") == 0)
        {
            return OUTPUT_REDIRECTION_WRITE;
        }
        else if (strcmp(args[i], ">>") == 0)
        {
            return OUTPUT_REDIRECTION_APPEND;
        }
    }
    return NO_REDIRECTION;
}


/*
Returns which type of CD process it is - Rishabh
*/
int command_with_cd(char *args[], int argsc)
{
    if (strcmp(args[ARG_PROGNAME], "cd") == 0)
    {
        if (args[ARG_1] == NULL)
        {
            return CD_HOME;
        }
        else if (strcmp(args[ARG_1], "-") == 0)
        {
            return CD_MINUS;
        }
        else
        {
            return CD_DIR;
        }
    }
    return NO_CD;
}

/*
tokenise the commands by '|'
*/
void parse_pipes(char line[], char *commands[], int num_commands)
{

}

/*
Check whether the command has pipes
*/
void command_with_pipes(char *args[], int argsc)
{

}




/*
Launch program function with redirection parameter
*/
void launch_program_with_redirection(char *args[], int argsc, int redirection)
{
    int rc = fork();

    if (rc < 0)
    {
        fprintf(stderr, "An error creating a child fork occured\n");
        exit(1);
    }
    else if (rc == 0)
    {
        switch (redirection)
        {
        case INPUT_REDIRECTION:
            child_with_input_redirected(args, argsc);
            break;
        case OUTPUT_REDIRECTION_WRITE:
            child_with_output_redirected_write(args, argsc);
            break;
        case OUTPUT_REDIRECTION_APPEND:
            child_with_output_redirected_append(args, argsc);
            break;
        }
    }
    else
    {
        reap();
        printf("\n");
    }
}

/*
A function which extracts the filepath of a specifc redirection, and then removes both redirection operator and filepath from the args array
*/
void extract_redirection_file(char *args[], int *argsc, int redirection, char *filepath)
{
    int redirection_index = 0;

    for (int index = 0; index < *argsc; index++)
    {
        if (strcmp(args[index], ">") == 0 && redirection == OUTPUT_REDIRECTION_WRITE || strcmp(args[index], ">>") == 0 && redirection == OUTPUT_REDIRECTION_APPEND || strcmp(args[index], "<") == 0 && redirection == INPUT_REDIRECTION)
        {
            redirection_index = index;
            strcpy(filepath, args[index + 1]);
            if (DEBUG_PRINT)
                printf("Filepath: %s, Redirection operator: %i\n", filepath, redirection_index);
            break;
        }
    }

    args[redirection_index] = NULL;
    args[redirection_index + 1] = NULL;

    for (int index = redirection_index; index + 2 < *argsc; index++)
    {
        args[index] = args[index + 2];
        args[index + 2] = NULL;
    }

    *argsc -= 2;
}

/*
Handles children with input redirected
*/
void child_with_input_redirected(char *args[], int argsc)
{
    char filepath[MAX_LINE];
    extract_redirection_file(args, &argsc, INPUT_REDIRECTION, filepath);

    int fd = open(filepath, O_RDONLY);
    if (fd >= 0)
    {
        dup2(fd, STDIN_FILENO);
        close(fd);
        execvp(args[0], args);
    }
    else if (DEBUG_PRINT)
        printf("An error occured opening %s\n", filepath);
}

/*
Handles children with output redirect to write to a file
*/
void child_with_output_redirected_write(char *args[], int argsc)
{
    char filepath[MAX_LINE];
    extract_redirection_file(args, &argsc, OUTPUT_REDIRECTION_WRITE, filepath);
    int fd = open(filepath, O_WRONLY | O_CREAT, 0644);
    if (fd >= 0)
    {
        dup2(fd, STDOUT_FILENO);
        close(fd);
        execvp(args[0], args);
    }
    else if (DEBUG_PRINT)
        printf("An error occured opening %s\n", filepath);
}

/*
Handles children with output redirected to append a file
*/
void child_with_output_redirected_append(char *args[], int argsc)
{
    char filepath[MAX_LINE];
    extract_redirection_file(args, &argsc, OUTPUT_REDIRECTION_APPEND, filepath);

    int fd = open(filepath, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd >= 0)
    {
        dup2(fd, STDOUT_FILENO);
        close(fd);
        execvp(args[0], args);
    }
    else if (DEBUG_PRINT)
        printf("An error occured opening %s\n", filepath);
}

/*
Updates the directory to the cwd
*/
void init_lwd(char *lwd)
{
    getcwd(lwd, MAX_PROMPT_LEN - 6);
}

/*
Runs cd based on the type of CD process it is determined earlier
*/
void run_cd(char *args[], int argsc, char *lwd, int cd)
{
    int ret;
    if (cd == CD_DIR)
    {
        init_lwd(lwd);
        ret = chdir(args[ARG_1]);
    }
    else if (cd == CD_HOME)
    {
        init_lwd(lwd);
        ret = chdir(getenv("HOME"));
    }
    else if (cd == CD_MINUS)
    {
        ret = chdir(lwd);
        init_lwd(lwd);
    }

    if (ret != 0 && DEBUG_PRINT)
        printf("An error occured changing directory\n");
}



/*
Loop for piping together 
*/

void launch_program_with_piping(char *commands[], int num_commands)
{





}








