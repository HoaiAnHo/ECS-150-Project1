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

// searching for special chars
// int special_char(char* command)
// {
//         // 
//         return 0;
// }

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
                command1 = cmd;
                nl = strchr(command1, ' ');
                *nl = '\0';
                nl++;

                //more args
                arg1 = strchr(cmd, '\0');

                printf("command1: %s \n", command1);
                printf("arg1:%s \n", arg1);

                char *args[] = {command1, arg1, NULL};



                /* Regular command */
                // retval = system(cmd);
                // fprintf(stdout, "Return status value for '%s': %d\n",
                //         cmd, retval);

                /* Add "/bin/" to command */
                //char bin[CMDLINE_MAX] = "/bin/";
                //strcat(bin, cmd);
                //printf("final command: '%s'", src);


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
