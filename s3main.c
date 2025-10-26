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

    char *commands[MAX_LINE];
    int num_command;

    while (1)
    {

        read_command_line(line);
        parse_command(line, args, &argsc);

        int pipe = command_with_pipes(args, argsc);
        int redirection = command_with_redirection(args, argsc);
        int cd = command_with_cd(args, argsc);

        if (pipe > 0)
        {
            parse_pipes(line, commands, &num_command);
            launch_program_with_piping(commands, num_command);
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

    return 0;
}
