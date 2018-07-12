#include "colors.h"
#include "main.h"
#include "utils.h"
#include "commands.h"

/**
 * Prints the shell prompt to the stdout.
 */
void show_prompt()
{
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
	       ANSI_COLOR_GREEN "%s\n>> " ANSI_COLOR_RESET, user, host, cwd);
}

/**
 * Run a command given in cmd with the arguments provided
 * in args. It waits for the child process created to be finished
 * and returns modifiying the integer `status` with the exit status
 * of the child process.
 */
void run(char *cmd, char **args, int *status, int detach)
{
	int pid;

	if ((pid = fork()) == -1) {
		perror(ANSI_COLOR_RED "Error while forking : \n"
		       ANSI_COLOR_RESET);
		return;
	}

	if (pid == 0) {
		// Child process
		// Check if the process is to be run in the background.
		// If yes then redirect stdout and stderr to /dev/null
		// So there is no pinrts on the current stdout of the parent
		// process
		if (detach) {
			int dev_null = open("/dev/null", 0);
			if (dev_null == -1) {
				perror("Error in open(/dev/null)");
				exit(EXIT_FAILURE);
			}

			if (dup2(dev_null, STDOUT_FILENO) == -1 ||
			    dup2(dev_null, STDERR_FILENO)
			    )
				perror("Error while changing stdout for child");
		}
		// If we launch non-existing commands we end the process
		if (execvp(cmd, args) == -1) {
			printf(ANSI_COLOR_RED "Command not found\n"
			       ANSI_COLOR_RESET);
			exit(1);
		}
	}

	if (detach) {
		printf(ANSI_COLOR_GREEN
		       "Process started in background with PID : %d\n\n"
		       ANSI_COLOR_RESET, pid);
		*status = 0;
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
int process_command(char *cmd)
{
	int detach = 0;
	int status = 0;

	char *trimmed_cmd = trim(cmd);

	if (!trimmed_cmd) {
		return 0;
	}

	int count;

	char **cmd_list = str_split(trimmed_cmd, " ", &count);
	char *command = strdup(cmd_list[0]);

	if (!strcmp(cmd_list[count - 1], "&"))
		detach = 1;

	// Try to check if is a shell builtin command if yes
	// run the command and return the status.
	int check_shell_cmd = shell_cmd(cmd_list);
	if (check_shell_cmd <= 0) {
		status = check_shell_cmd < 0 ? 1 : 0;
		goto END;
	}
	// Run the command.
	run(command, cmd_list, &status, detach);
	printf("\n");

 END:
	free(cmd_list);
	free(command);

	char status_env[10];
	sprintf(status_env, "?=%d", status);
	putenv(status_env);

	return status;
}

int main()
{

	char input[INPUT_MAXLEN];
	char *prev_input = NULL;
	int continue_flag = 0;
/*  const wchar_t welcome_text[] =
        "   ██████╗  █████╗ ███████╗██╗  ██╗"
        "   ██╔══██╗██╔══██╗██╔════╝██║  ██║"
        "   ██║  ██║███████║███████╗███████║"
        "   ██║  ██║██╔══██║╚════██║██╔══██║"
        "   ██████╔╝██║  ██║███████║██║  ██║"
        "   ╚═════╝ ╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝";*/

	putenv("SHELL=dash");
	printf(CLEAR_SCREEN_ANSI);

	// Main command loop of the shell.
	while (1) {
		if (continue_flag) {
			printf(" > ");
			fflush(stdout);
			continue_flag = 0;
		} else
			show_prompt();

		if (!fgets(input, INPUT_MAXLEN, stdin)) {
			printf("Cannot read from standard input\n");
			continue;
		}

		char *input_cmd = trim(input);
		if (!input_cmd) {
			continue;
		}
		// Attempt to get multiple lines in this scenario.
		int input_cmd_len = strlen(input_cmd);
		if (input_cmd[input_cmd_len - 1] == '\\' || prev_input != NULL) {
			if (prev_input == NULL) {
				prev_input =
				    (char *)malloc(sizeof(char) *
						   (INPUT_MAXLEN));
				if (prev_input == NULL)
					perror("Malloc falied");
			}

			int prev_input_len = strlen(prev_input);
			int avail_len = INPUT_MAXLEN - (prev_input_len + 1);
			if (avail_len < (input_cmd_len + 1)) {
				prev_input =
				    (char *)realloc(prev_input,
						    sizeof(char) *
						    (INPUT_MAXLEN));
			}

			strcat(prev_input, input_cmd);

			if (input_cmd[input_cmd_len - 1] == '\\') {
				prev_input[strlen(prev_input) - 1] = ' ';
				continue_flag = 1;
				continue;
			} else {
				input_cmd = prev_input;
			}
		}

		input_cmd = trim(input_cmd);
		int count;
		// Individual commands to be run
		char **cmd_list = str_split(input_cmd, ";", &count);
		char **cmd_list_ptr = cmd_list;

		while (*cmd_list) {
			int tmp_count;
			// In each command check dependent commands which depends on their
			// preceedor.
			char **dependent_cmd_list =
			    str_split(*cmd_list, "&&", &tmp_count);
			char **dependent_cmd_list_ptr = dependent_cmd_list;

			while (*dependent_cmd_list) {

				if (process_command(*dependent_cmd_list)) {
					// printf("Error while processing command : %s\n", *dependent_cmd_list);
					break;
				}
				dependent_cmd_list++;
			}

			free(dependent_cmd_list_ptr);
			cmd_list++;
		}

		free(cmd_list_ptr);

		if (prev_input) {
			free(prev_input);
			prev_input = NULL;
		}
	}

	return 0;
}
