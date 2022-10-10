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
int main(void)
{
        char cmd[CMDLINE_MAX];

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
                }

                /* command */
                command1 = strtok(cmd, " ");

                /* Exclude Output Redirection from Arguments */
                if (c1.meta_char) {
                        c1.o_filename = c1.meta_char;
                        while(c1.o_filename[0] == '>') {
                                c1.o_filename++;
                        }
                        printf("filename: %s", c1.o_filename);
                        fd = open(c1.o_filename,O_WRONLY | O_CREAT, 0644);
                        printf("arg1: %s ", c1.arg1);
                        c1.arg1 = strtok(c1.arg1,">");
                        printf("arg1: %s ", c1.arg1);
                        c1.arg1 = strtok(c1.arg1," ");
                        // if (strcmp(c1.command1, "echo")) {
                        //         c1.arg1 = strtok(c1.arg1," ");
                        // }
                        //c1.arg1 = strtok(c1.arg1," ");
                        // if (c1.arg1[0] == ' ') {
                        //         c1.arg1 = NULL;
                        // } else {
                        //         c1.arg1 = strtok(c1.arg1," ");
                        // }
                        // printf("arg2: %s ", c1.arg1);
                }

                char *args[] = {c1.command1, c1.arg1, NULL};


                /* Builtin command */
                /* Exit */
                if (!strcmp(cmd, "exit")) {
                        fprintf(stderr, "Bye...\n");
                        break;
                }

                /* cd */
                if (!strcmp(c1.command1, "cd")) {
                        chdir(c1.arg1);
                }

                /* pwd */
                if (!strcmp(cmd, "pwd")){
                        getcwd(cwd_buffer, 256);
                }

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
