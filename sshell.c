#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define CMDLINE_MAX 512

struct cmd_line {
        char *command1;
        char *arg1;
        char *meta_char;
        char *o_filename;
        
};
struct full_cmds {
        char *raw_cmd;
};
int main(void)
{
        char cmd[CMDLINE_MAX];

        while (1) {
                char *nl;
                //int retval;
                //char *arg1;
                //char *arg2 = "";
                //char *command1;
                char *command_copy;
                //char *parse_arg = NULL;
                //char *full_cmd;
                pid_t pid;
                pid_t pipe_id;
                char cwd_buffer[256];
                int fd;
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

                // pipe parse test
                struct full_cmds pipe_cmds[3];
                char* pipe_check = strchr(cmd, '|');
                //printf("%s", pi)
                int pipe_spot = 0;
                // if symbol found, split into commands
                char cmd_copy[CMDLINE_MAX];
                strcpy(cmd_copy, cmd);
                if (pipe_check)
                {
                        char* token;
                        token = strtok(cmd_copy, "|");

                        
                        while (token != NULL) {
                                while (token[0] == ' ') {
                                        token++;
                                }
                                pipe_cmds[pipe_spot].raw_cmd = token;
                                pipe_spot++;
                                token = strtok(NULL, "|");
                        }
                }
                if (pipe_spot == 0) {
                        pipe_cmds[pipe_spot].raw_cmd = cmd;
                        pipe_spot++;
                }
                pipe_spot--;
                
                if (pipe_spot == 1) {
                        int filedesc[2];
                        pipe(filedesc);
                        pipe_id = fork();
                        //pipe_id = fork();
                        //parent
                        if (pipe_id > 0) {
                                //printf("#1%i\n", pipe_id);
                                int pipe_status;
                                waitpid(pipe_id, &pipe_status, 0);
                                close(filedesc[0]);
                                dup2(filedesc[1], STDOUT_FILENO);
                                close(filedesc[1]);
                                printf("parent\n");
                                pipe_spot = 1;
                        }
                        //child 1?
                        else if (pipe_id == 0) {
                                close(filedesc[1]);
                                dup2(filedesc[0], STDIN_FILENO);
                                close(filedesc[0]);
                                printf("child\n");
                                pipe_spot = 0;
                        }
                }

                char* cur_cmd = pipe_cmds[pipe_spot].raw_cmd;
                printf("%s\n", cur_cmd);



                struct cmd_line c1;

                /* Parse for arguments */
                /* first argument */
                command_copy = strdup(cur_cmd);
                // with pipe process, check if any leading white spaces for command
                c1.arg1 = strchr(command_copy,' ');


                /* extracts the space from the argument */
                if (c1.arg1) {
                        c1.arg1++;
                        while (c1.arg1[0] == ' ' && c1.arg1 != NULL) {
                                c1.arg1++;
                        }
                        /* Check for Output Redirection */
                        c1.meta_char = strchr(c1.arg1, '>');
                }
                //printf("meta_char: %s ", c1.meta_char);

                /* Command */
                c1.command1 = strtok(cur_cmd, " ");


                /* Exclude Output Redirection from Arguments */
                if (c1.meta_char) {
                        c1.o_filename = c1.meta_char;
                        while(c1.o_filename[0] == '>') {
                                c1.o_filename++;
                        }
                        //printf("filename: %s", c1.o_filename);
                        fd = open(c1.o_filename,O_WRONLY | O_CREAT, 0644);
                        //printf("arg1: %s ", c1.arg1);
                        c1.arg1 = strtok(c1.arg1,">");
                        //printf("arg1: %s ", c1.arg1);
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
                if (!strcmp(cur_cmd, "exit")) {
                        fprintf(stderr, "Bye...\n");
                        break;
                }

                /* cd */
                if (!strcmp(c1.command1, "cd")) {
                        chdir(c1.arg1);
                }

                /* pwd */
                if (!strcmp(cur_cmd, "pwd")){
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
                        /* Setup for Output Redirection */
                        if (c1.meta_char) {
                                dup2(fd, STDOUT_FILENO);
                                execvp(c1.command1, args);
                                close(fd);
                        } else {
                                execvp(c1.command1, args);
                        }
                        perror("execv");
                        exit(1);
                } else if (pid > 0) {
                        /* Parent */
                        int status;
                        waitpid(pid, &status, 0);
                        //printf( "Return status value for '%s' : %d\n", bin, WEXITSTATUS(status));
                        fprintf(stderr, "+ completed '%s' [ %d ]\n", command_copy, WEXITSTATUS(status));
                } else {
                        perror("fork");
                        exit(1);
                }
        }

        return EXIT_SUCCESS;
}