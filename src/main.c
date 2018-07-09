#include "colors.h"
#include "main.h"
#include "utils.h"
#include "commands.h"


/**
 * Prints the shell prompt to the stdout.
 */
void show_prompt() {
    char user[MAX_BUF_LEN];
    char host[MAX_BUF_LEN];
    char cwd[MAX_BUF_LEN];

    getlogin_r(user, MAX_BUF_LEN - 1);
    gethostname(host, MAX_BUF_LEN - 1);
    if (getcwd(cwd, MAX_BUF_LEN) == NULL)
       perror("getcwd() error");

    printf(ANSI_COLOR_BLUE "%s"
        ANSI_COLOR_RED "@"
        ANSI_COLOR_CYAN "%s"
        ANSI_COLOR_RESET " $ "
        ANSI_COLOR_GREEN "%s\n>> "
        ANSI_COLOR_RESET, user, host, cwd);
}

/**
 * Run a command given in cmd with the arguments provided
 * in args. It waits for the child process created to be finished
 * and returns modifiying the integer `status` with the exit status
 * of the child process.
 */
void run(char *cmd, char **args, int *status, int detach){
     int pid;

     if((pid = fork()) == -1){
         perror(ANSI_COLOR_RED "Error while forking : \n" ANSI_COLOR_RESET);
         return;
     }


    if(pid == 0) {
        // Child process
        // Check if the process is to be run in the background.
        // If yes then redirect stdout and stderr to /dev/null
        // So there is no pinrts on the current stdout of the parent
        // process
        if(detach) {
            int dev_null = open("/dev/null",0);
            if(dev_null == -1){
                perror("Error in open(/dev/null)");
                exit(EXIT_FAILURE);
            }

            if(dup2(dev_null, STDOUT_FILENO) == -1 ||
                dup2(dev_null, STDERR_FILENO)
            )
                perror("Error while changing stdout for child");
        }

        // If we launch non-existing commands we end the process
        if (execvp(cmd, args) == -1){
            printf(ANSI_COLOR_RED "Command not found\n" ANSI_COLOR_RESET);
            exit(1);
        }
    }

    if(detach) {
        printf(ANSI_COLOR_GREEN
            "Process started in background with PID : %d\n\n"
            ANSI_COLOR_RESET,
            pid
        );
    } else {
        // PID now contains the Process ID of the child.
        // We are using wait here, since we have the PID and we know which
        // child process to wait for we are using waitpid.
        waitpid(pid, status, 0);
    }
}


/**
 * Process a command as a string, this function returns
 * the status of the running command.
 */
int process_command(char *cmd) {
    int detach = 0;
    char *trimmed_cmd = trim(cmd);

    if(!trimmed_cmd) {
        return 0;
    }

    int count;
    char **cmd_list = str_split(trimmed_cmd, ' ', &count);
    char *command = strdup(cmd_list[0]);

    if(!strcmp(cmd_list[count - 1], "&"))
        detach = 1;

    if(shell_cmd(cmd_list)) {
        free(cmd_list);
        free(command);
        return 0;
    }

    int status = 0;
    run(command, cmd_list, &status, detach);

    char status_env[10];
    sprintf(status_env, "?=%d", status);
    putenv(status_env);

    free(cmd_list);
    free(command);
    return status;
}


int main() {
    const char prompt[] = "$ ";
    char input[INPUT_MAXLEN];
/*  const wchar_t welcome_text[] =
        "   ██████╗  █████╗ ███████╗██╗  ██╗"
        "   ██╔══██╗██╔══██╗██╔════╝██║  ██║"
        "   ██║  ██║███████║███████╗███████║"
        "   ██║  ██║██╔══██║╚════██║██╔══██║"
        "   ██████╔╝██║  ██║███████║██║  ██║"
        "   ╚═════╝ ╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝";*/


    putenv("SHELL=dash");

    // Main command loop of the shell.
    while(1) {
        show_prompt();

        if(!fgets(input, INPUT_MAXLEN, stdin)) {
            printf("Cannot read from standard input\n");
            continue;
        }

        char *input_cmd = trim(input);
        if(!input_cmd) {
            continue;
        }

        int count;
        char **cmd_list = str_split(input_cmd, ';', &count);
        while(*cmd_list) {
            process_command(*cmd_list);
            cmd_list++;
        }
    }

    return 0;
}
