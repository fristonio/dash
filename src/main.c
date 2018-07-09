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
void run(char *cmd, char **args, int *status){
     int pid;

     if((pid = fork()) == -1){
         perror(ANSI_COLOR_RED "Error while forking : \n" ANSI_COLOR_RESET);
         return;
     }


    if(pid == 0) {
        // Child process
        // If we launch non-existing commands we end the process
        if (execvp(cmd, args) == -1){
            printf(ANSI_COLOR_RED "Command not found\n" ANSI_COLOR_RESET);
            exit(1);
        }
     }

     // PID now contains the Process ID of the child.
     // We are using wait here, since we have the PID and we know which
     // child process to wait for we are using waitpid.
     waitpid(pid, status, 0);
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
        char **cmd = str_split(input_cmd, ' ', &count);
        char *command = strdup(cmd[0]);

        if(shell_cmd(cmd)) {
            free(cmd);
            free(command);
            continue;
        }

        int status = 0;
        run(command, cmd, &status);

        char status_env[10];
        sprintf(status_env, "?=%d", status);
        putenv(status_env);

        free(command);
        free(cmd);
    }

    return 0;
}
