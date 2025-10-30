#include "s3.h"

int main(int argc, char *argv[])
{

    /// Stores the command line input
    char line[MAX_LINE];

    char lwd[MAX_PROMPT_LEN - 6];
    init_lwd(lwd); /// Implement this function: initializes lwd with the cwd (using getcwd)

    /// Stores pointers to command arguments.
    /// The first element of the array is the command name.
    char *args[MAX_ARGS];

    /// Stores the number of arguments
    int argsc;

    char *commands_pipe[MAX_LINE];
    int num_command_pipe;

    char *command_array[MAX_LINE];
    int num_commands;

    while (1)
    {
        read_command_line(line);
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
    }

    return 0;
}
