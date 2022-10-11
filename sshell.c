#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define CMDLINE_MAX 512

struct cmd_line {
        char *command1;
        char *arg1;
        int meta_char;
        char *o_filename;
};
int redirect_out(char *command);

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
                char *dir;
                pid_t pid;
                char cwd_buffer[256];
                int fd;


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

                struct cmd_line c1;

                /* Check for Output Redirection */
                c1.meta_char = redirect_out(cmd);

                /* Parse for arguments */
                /* first argument */
                command_copy = strdup(cmd);
                c1.arg1 = strchr(command_copy,' ');

                /* extracts the space from the argument */
                if (c1.arg1) {
                        c1.arg1++;
                        while (c1.arg1[0] == ' ' && c1.arg1 != NULL) {
                                c1.arg1++;
                        }
                }
                //printf("meta_char: %s ", c1.meta_char);

                /* Command */
                c1.command1 = strtok(cmd, " ");
                //printf("command: %s", c1.command1);
                

                /* Get FileName */
                if (c1.meta_char) {
                        c1.o_filename = strchr(command_copy, '>');
                        //printf("file name: %s", c1.o_filename);
                        while(c1.o_filename[0] == '>' || c1.o_filename[0] == ' ') {
                                c1.o_filename++;
                        }
                        //printf("filename: %s", c1.o_filename);
                        fd = open(c1.o_filename,O_WRONLY | O_CREAT | O_TRUNC, 0644);
                }

                /* Clean up Argument */
                printf("arg: %s\n", c1.arg1);
                if (c1.meta_char) {
                        if(!c1.arg1){
                                //still doesn't really work
                                c1.command1 = strtok(cmd, ">");
                                printf("parsed command: %s", c1.command1);
                                c1.arg1 = NULL;
                        } else {
                                c1.arg1 = strtok(c1.arg1, ">");
                                //printf("new arg: %s\n", c1.arg1);
                                if (c1.arg1 == c1.o_filename) {
                                        c1.arg1 = NULL;
                                        //printf("arg1: %s\n", c1.arg1);
                                }
                        }
                }
                
                /* special case */
                        if (strcmp(c1.command1, "echo") != 0) {
                                c1.arg1 = strtok(c1.arg1," ");
                        }

                char *args[] = {c1.command1, c1.arg1, NULL};


                /* Builtin command */
                /* Exit */
                if (!strcmp(cmd, "exit")) {
                        fprintf(stderr, "Bye...\n");
                        //fprintf(stderr, "+ completed '%s' [ %d ]\n", command_copy, WEXITSTATUS(status));
                        break;
                }

                /* cd */
                if (!strcmp(c1.command1, "cd")) {
                        printf("directory: %s \n", c1.arg1);
                        dir = strtok(c1.arg1,"\n");
                        chdir(dir);
                }

                /* pwd */
                if (!strcmp(cmd, "pwd")){
                        getcwd(cwd_buffer, 256);
                }

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

int redirect_out(char *command){
        char* meta_char;
        meta_char = strchr(command, '>');
        if (meta_char) {
                return 1;
        }
        return 0;
}