#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>

#include "colors.h"
#include "commands.h"
#include "utils.h"


#define MAX_DIR_LEN 256

const char *EXIT_CMD = "exit";
const char *CD_CMD = "cd";


/**
 * Change directory in the shell, implements  cd command.
 * If no arguments are provided to cd then this changes directory
 * to $HOME. All the arguments after the frist arguments are ignored.
 */
void change_dir_cmd(char **cmd) {
    char *new_dir;

    if(cmd[1] == NULL ||
        strcmp(cmd[1], "~") == 0 ||
        strcmp(cmd[1], "~/") == 0
    ) {
        const char *HOME;

        if ((HOME = getenv("HOME")) == NULL) {
            HOME = getpwuid(getuid())->pw_dir;
        }

        new_dir = strdup(HOME);
    } else if(cmd[1][0] == '/') {
        new_dir = strdup(cmd[1]);
    } else {
        char cwd[MAX_DIR_LEN];
        if (getcwd(cwd, MAX_DIR_LEN) == NULL) {
            perror(ANSI_COLOR_RED "getcwd() error : " ANSI_COLOR_RESET);
            return;
        }

        new_dir = malloc(strlen(cwd) + strlen(cmd[1]) + 2);
        strcpy(new_dir, cwd);
        strcat(new_dir, "/");
        strcat(new_dir, cmd[1]);
    }

    if(chdir(new_dir)) {
        perror(ANSI_COLOR_RED "Error while chdir" ANSI_COLOR_RESET);
    }

    free(new_dir);
}


/**
 * Run a shell command from cmd if it exist. It returns
 * 0 otherwise.
 */
int shell_cmd(char **cmd) {
    char *command = *cmd;

    if(!strcmp(command, EXIT_CMD)) {
        printf(ANSI_COLOR_YELLOW
            "Exitting...\n\n"
            ANSI_COLOR_RESET);
        exit(0);
    } else if(!strcmp(command, CD_CMD)) {
        change_dir_cmd(cmd);
    } else {
        return 0;
    }

    return 1;
}
