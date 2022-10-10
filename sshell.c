#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

// at the start of the main, check for '|' char
// if found, we're making multiple cmd_arg structures
struct cmd_arg {
        char full_cmd[CMDLINE_MAX];
        char* command;
        char* args[];
        char* filename = NULL;
}

//this wasn't tested
// int redirect_check(char* given_cmd) {
//         int final_fd;
//         char* cmd_ptr = strchr(given_cmd, '>');
//         if(cmd_ptr){
//                 while (cmd_ptr != ' ' || cmd_ptr != NULL){
//                         cmd_ptr++;
//                 }
//                 final_fd = open(cmd_ptr, O_WRONLY | O_CREAT, 0644);
//                 dup2(final_fd, STDOUT_FILENO);
//                 close(final_fd);
//                 // i'm assuming all the executed stuff will now go to the file
//                 return final_fd;
//         }
//         return 0;
// }

int main(void)
{
        char cmd[CMDLINE_MAX];
        // struct cmd_arg pipe_commands[3];
        // char* pipe_check = strchr(cmd, '|');
        // if (pipe_check){
        //         char *parse_ptr = *cmd;
        //         int command_index = 0;
        //         while (parse_ptr != NULL){
        //                 pipe_commands[command_index] = parse_ptr;
        //         }
        // }

        while (1) {
                char *nl;
                //int retval;
                char *arg1;
                //char *arg2 = "";
                char *command1;
                char *command2;
                //char *full_cmd;
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
                /* Exit */
                if (!strcmp(cmd, "exit")) {
                        fprintf(stderr, "Bye...\n");
                        break;
                }

                /* cd */
                // if (!strcmp(cmd, "cd")) {

                // }

                /* Parse for arguments */
                /* first argument */
                command2 = strdup(cmd);
                //printf("command1: %s ", command2);
                arg1 = strchr(command2,' ');
                //printf("arg1: %s \n", arg1);
                /* extracts the space from the argument */
                if (arg1) {
                        arg1++;
                        while (arg1[0] == ' ' && arg1 != NULL)
                        {
                                arg1++;
                        }
                }
                //printf("%c", arg1[0]);

                /* command */
                command1 = strtok(cmd, " ");

                //printf("command: %s \n", command1);
                //printf("arg1: %s \n", arg1);



                char *args[] = {command1, arg1, NULL};



                /* Regular command */
                // retval = system(cmd);
                // fprintf(stdout, "Return status value for '%s': %d\n",
                //         cmd, retval);

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
                        fprintf(stderr, "+ completed '%s' [ %d ]\n", command2, WEXITSTATUS(status));
                } else {
                        perror("fork");
                        exit(1);
                }
        }

        return EXIT_SUCCESS;
}
