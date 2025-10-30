#include "s3.h"

/// Simple for now, but will be expanded in a following section
void construct_shell_prompt(char shell_prompt[])
{
    char cwd[MAX_PROMPT_LEN - 6];
    getcwd(cwd, 100);
    sprintf(shell_prompt, "\n[%s s3]$", cwd);
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

/*
    Implements simple tokenization (space delimited)
    Note: strtok puts '\0' (null) characters within the existing storage,
    to split it into logical cstrings.
    There is no dynamic allocation.
*/

void parse_command(char line[], char *args[], int *argsc)
{
    char *line_copy = strdup(line);

    /// See the man page of strtok(...)
    char *token = strtok(line_copy, " ");
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
Tokenise the commands by '|'
*/
void parse_pipes(char line[], char *commands[], int *num_commands)
{
    char *token = strtok(line, "|");
    *num_commands = 0;

    if (DEBUG_PRINT)
        printf("Parsed piped input: \n");

    while (token != NULL && *num_commands < MAX_ARGS - 1)
    {
        if (DEBUG_PRINT)
            printf("    Command [%i] %s\n", *num_commands, token);
        commands[(*num_commands)++] = token;
        token = strtok(NULL, "|");
    }

    commands[*num_commands] = NULL; /// args must be null terminated
}

/*
Check whether the command has pipes
*/
int command_with_pipes(char *args[], int argsc)
{
    for (int index = 0; index < argsc; index++)
    {
        if (strcmp(args[index], "|") == 0)
        {
            if (DEBUG_PRINT)
                printf("Pipe operator found\n");
            return PIPE_YES_PLEASE_OUTLOOK;
        }
    }
    return NO_PIPE;
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
    char *args[128];
    int argsc = 0;

    int fd[num_commands - 1][2];
    int rc;

    for (int pipe_index = 0; pipe_index < num_commands - 1; pipe_index++)
    {
        if (pipe(fd[pipe_index]))
        {
            perror("pipe failed");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < num_commands; i++)
    {
        rc = fork();
        if (rc < 0)
        {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }
        else if (rc == 0)
        {
            int shell = command_with_subshell(commands[i]);
            int redirection = NO_REDIRECTION;

            if (shell == NO_SUBSHELL)
            {
                parse_command(commands[i], args, &argsc);
                redirection = command_with_redirection(args, argsc);
            }

            if (i > 0)
            {
                dup2(fd[i - 1][0], STDIN_FILENO);
                close(fd[i - 1][1]);
            }

            if (i < num_commands - 1)
            {
                dup2(fd[i][1], STDOUT_FILENO);
                close(fd[i][0]);
            }

            for (int j = 0; j < num_commands - 1; j++)
            {
                close(fd[j][0]);
                close(fd[j][1]);
            }

            if (shell == SUBSHELL_PRESENT)
            {
                int rc = fork();

                if (rc == 0)
                    execute_subshell(commands[i]);
                else
                    wait(NULL);
            }
            else if (redirection > 0)
            {
                switch (redirection)
                {
                case INPUT_REDIRECTION:
                    child_with_input_redirected(args, argsc);
                case OUTPUT_REDIRECTION_APPEND:
                    child_with_output_redirected_append(args, argsc);
                case OUTPUT_REDIRECTION_WRITE:
                    child_with_output_redirected_write(args, argsc);
                }
            }
            else
            {
                child(args, argsc);
            }

            exit(0);
        }
    }

    for (int i = 0; i < num_commands - 1; i++)
    {
        close(fd[i][0]);
        close(fd[i][1]);
    }

    for (int i = 0; i < num_commands; i++)
        wait(NULL);
}

/*
Tokenise the commands by ';'

TO BE UPDATED:
echo "Start processing..." ; (cd /var/log ; cat syslog |
Algorithm has only tokenise top level ; and ignore any ; in side () or even (())

echo "Start processing..." , (cd /var/log , cat syslog | sort > sorted_syslog.txt) , echo "Finished writing sorted log." , date

*/
void parse_semicolon(char line[], char *commands[], int *num_commands)
{
    *num_commands = 0;
    int num_subshell = 0;
    size_t length = strlen(line);
    int count = 0;
    char cmd[MAX_PROMPT_LEN];

    if (DEBUG_PRINT)
        printf("Parsed batched input: \n");

    for (size_t i = 0; i < length; i++) // manually tokenising here
    {
        if (line[i] == '(')
        {
            num_subshell++;
            cmd[count++] = '(';
        }
        else if (line[i] == ')')
        {
            num_subshell--;
            cmd[count++] = ')';
        }
        else if (line[i] == ';' && num_subshell == 0)
        {
            if (count > 0)
            {
                cmd[count] = '\0'; // ending the string
                if (DEBUG_PRINT)
                    printf("    Command [%i] %s\n", *num_commands, cmd);
                commands[(*num_commands)++] = strdup(cmd); // copying string and adding it to commands
                count = 0;
            }
        }
        else
        {
            cmd[count++] = line[i]; // adding the characters to cmd
        }
    }

    if (count > 0) // handle last line
    {
        cmd[count] = '\0';
        commands[(*num_commands)++] = strdup(cmd);
    }

    commands[*num_commands] = NULL;
}

/*
Executes a subshell
*/
void execute_subshell(char line[])
{
    if (DEBUG_PRINT)
        printf("Entering subshell\n");

    char lwd[MAX_PROMPT_LEN - 6];
    init_lwd(lwd);

    char *args[MAX_ARGS];
    int argsc;

    char *commands_pipe[MAX_LINE];
    int num_command_pipe;

    char *command_array[MAX_LINE];
    int num_commands;

    parse_semicolon(line, command_array, &num_commands);

    for (int command_index = 0; command_index < num_commands; command_index++)
    {
        int shell = command_with_subshell(command_array[command_index]);

        if (shell == SUBSHELL_PRESENT)
        {
            int rc = fork();

            if (rc == 0)
            {
                execute_subshell(command_array[command_index]);
                exit(0);
            }
            else
                wait(NULL);
        }
        else
        {
            parse_command(command_array[command_index], args, &argsc);

            int pipe = command_with_pipes(args, argsc);
            int redirection = command_with_redirection(args, argsc);
            int cd = command_with_cd(args, argsc);

            if (pipe > 0)
            {
                parse_pipes(command_array[command_index], commands_pipe, &num_command_pipe);
                launch_program_with_piping(commands_pipe, num_command_pipe);
            }
            else if (cd > 0)
            {
                run_cd(args, argsc, lwd, cd);
            }
            else if (redirection > 0)
            { /// Command with redirection
                launch_program_with_redirection(args, argsc, redirection);
                reap();
            }
            else /// Basic command
            {
                launch_program(args, argsc);
                reap();
            }
        }
    }

    if (DEBUG_PRINT)
        printf("Exiting subshell\n");
}

/*
Search if there are subshells by looking for brackets
If it does, delete the brackets
If the input or output of the subshell is piped, return NO_SUBSHELL
*/
int command_with_subshell(char line[])
{
    int i = 0;

    while (line[i] != '\0' && line[i] == ' ')
        i++;

    if (line[i] != '(')
    {
        if (DEBUG_PRINT)
            printf("No subshell found\n");
        return NO_SUBSHELL;
    }

    if (DEBUG_PRINT)
        printf("Found potential subshell at index %d\n", i);

    int open_index = i;

    int j = open_index - 1;
    while (j >= 0 && line[j] == ' ')
        j--;

    if (j >= 0 && line[j] == '|')
    {
        if (DEBUG_PRINT)
            printf("Subshell has piped input -> treat as no subshell\n");
        return NO_SUBSHELL;
    }

    int depth = 0;
    int close_index = -1;
    for (int k = open_index; line[k] != '\0'; k++)
    {
        if (line[k] == '(')
            depth++;
        else if (line[k] == ')')
        {
            depth--;
            if (depth == 0)
            {
                close_index = k;
                break;
            }
        }
    }

    if (close_index == -1)
    {
        if (DEBUG_PRINT)
            printf("No matching closing parenthesis for subshell\n");
        return NO_SUBSHELL;
    }

    int p = close_index + 1;
    while (line[p] != '\0' && line[p] == ' ')
        p++;
    if (line[p] == '|')
    {
        if (DEBUG_PRINT)
            printf("Subshell has piped output -> treat as no subshell\n");
        return NO_SUBSHELL;
    }

    line[open_index] = ' ';
    line[close_index] = ' ';

    return SUBSHELL_PRESENT;
}