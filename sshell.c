#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#define CMDLINE_MAX 512

// char* parse(char* command) {
//         /* Take in arguments from terminal input */
//         char arg[CMDLINE_MAX] = " ";
//         arg[0] = &strchr(command, ' ');
//         return *arg;
// }
// struct cmd_arg {
//         char command[CMDLINE_MAX];
//         char *args[];
// };

// function to return an array of commands for pipelining

// function to perform output redirection

// function to perform input redirection

// searching for special chars
int special_char(char* command)
{
        // search for pipe |
        // if found, return 1

        // search for output redirection >
        // if found, return 2;

        // search for input redirection <
        // if found, return 3;

        return 0;
}

char* command_parse(char* given_cmd, char* final_cmd){
        int increment = 0;
        while (isalpha(given_cmd[increment])){
                final_cmd[increment] = given_cmd[increment];
                increment++;
        }
        return final_cmd;
}

//assume no piping or redirection
//hopefully you have other functions to deal with that
char* argument_parse(char* given_cmd, char* final_arg){
        //go through to find arg1
        while ((given_cmd != NULL) && (given_cmd[increment] == '\0')){
                if(given_cmd[increment] == ' ') given_cmd++;
                else {
                        *final_arg = *given_cmd;
                }
        }
        return final_arg;
}

int main(void)
{
        char cmd[CMDLINE_MAX];

        while (1) {
                char *nl;
                //int retval;
                char *arg1;
                char *command1;
                pid_t pid;
                
                //char *args[] = {arg1, NULL};


                /* Print prompt */
                printf("sshell$@ucd ");
                fflush(stdout);

                /* Get command line */
                fgets(cmd, CMDLINE_MAX, stdin);

                /* Print command line if stdin is not provided by terminal */
                if (!isatty(STDIN_FILENO)) {
                        printf("%s", cmd);
                        fflush(stdout);
                }

                /* Remove trailing newline from command line */
                nl = strchr(cmd, '\n');
                if (nl){
                        *nl = '\0';
                }

                /* Builtin command */
                if (!strcmp(cmd, "exit")) {
                        fprintf(stderr, "Bye...\n");
                        break;
                }

                /* Parse for arguments */
                // first command
                // command1 = cmd;
                // nl = strchr(command1, ' ');
                // *nl = '\0';
                // nl++;

                //more args
                // arg1 = strchr(cmd, '\0');

                printf("command1: %s \n", command1);
                printf("arg1:%s \n", arg1);

                int cmd_len = strlen(cmd);
                int command1_len = strlen(command1);
                command1 = command_parse(cmd, command1);

                // if cmd_len > command1_len
                // if cmd[command1_len] != '\0'

                char *args[] = {command1, arg1, NULL};

                /* Regular command */
                // retval = system(cmd);
                // fprintf(stdout, "Return status value for '%s': %d\n",
                //         cmd, retval);

                // check if command1 is either pwd or cd
                // chdir()

                /* fork() + exec() + wait() */
                pid = fork();
                if (pid == 0) {
                        /* Child */
                        //printf("cmd: '%s'", cmd);
                        execvp(command1, args);
                        perror("execv");
                        exit(1);
                } else if (pid > 0) {
                        /* Parent */
                        int status;
                        waitpid(pid, &status, 0);
                        //printf( "Return status value for '%s' : %d\n", bin, WEXITSTATUS(status));
                        fprintf(stderr, "+ completed '%s' [ %d ]\n", cmd, WEXITSTATUS(status));
                } else {
                        perror("fork");
                        exit(1);
                }
        }

        return EXIT_SUCCESS;
}
