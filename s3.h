#ifndef _S3_H_
#define _S3_H_

/// See reference for what these libraries provide
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

/// Constants for array sizes, defined for clarity and code readability
#define MAX_LINE 1024
#define MAX_ARGS 128
#define MAX_PROMPT_LEN 256

#define DEBUG_PRINT 1

/// Enum for readable argument indices (use where required)
enum ArgIndex
{
    ARG_PROGNAME,
    ARG_1,
    ARG_2,
    ARG_3,
};

enum RedirectionIndex
{
    NO_REDIRECTION,
    OUTPUT_REDIRECTION_WRITE,
    OUTPUT_REDIRECTION_APPEND,
    INPUT_REDIRECTION
};

enum CDIndex
{
    NO_CD,
    CD_DIR,
    CD_HOME,
    CD_MINUS
};

/// With inline functions, the compiler replaces the function call
/// with the actual function code;
/// inline improves speed and readability; meant for short functions (a few lines).
/// the static here avoids linker errors from multiple definitions (needed with inline).
static inline void reap()
{
    wait(NULL);
}

/// Shell I/O and related functions (add more as appropriate)
void read_command_line(char line[]);
void construct_shell_prompt(char shell_prompt[]);
void parse_command(char line[], char *args[], int *argsc);
int command_with_redirection(char *args[], int argsc);
int command_with_cd(char *args[], int argsc);
void init_lwd(char lwd[]);

/// Child functions (add more as appropriate)
void child(char *args[], int argsc);
void child_with_input_redirected(char *args[], int argsc);
void child_with_output_redirected_write(char *args[], int argsc);
void child_with_output_redirected_append(char *args[], int argsc);
void extract_redirection_file(char *args[], int *argsc, int redirection, char *filepath);

/// Program launching functions (add more as appropriate)
void launch_program(char *args[], int argsc);
void launch_program_with_redirection(char *args[], int argsc, int redirection);
void run_cd(char *args[], int argsc, char lwd[], int redirection);

#endif