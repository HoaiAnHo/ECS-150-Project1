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
int redirect_out(char *command);
int redirect_in(char *command);

int main(void)
{
        char cmd[CMDLINE_MAX];

        while (1) {
                char *nl;
                //int retval;
                int built_in = 0; //flag for built in function
                char *command_copy;
                //char *parse_arg = NULL;
                //char *full_cmd;
                pid_t pid;
                char *dir;
                int complete;
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
                // if not a pipeline command, put whole command into the 1st array element
                if (pipe_spot == 0) {
                        pipe_cmds[pipe_spot].raw_cmd = cmd;
                        pipe_spot++;
                }
                pipe_spot--;
                
                // if pipeline, initiate forks
                if (pipe_spot == 1) {
                        int filedesc[2];
                        pipe(filedesc);
                        pipe_id = fork();
                        fork();
                        //pipe_id = fork();
                        //parent
                        if (pipe_id > 0) {
                                close(filedesc[0]);
                                dup2(filedesc[1], STDOUT_FILENO);
                                close(filedesc[1]);
                                pipe_spot = 0;
                        }
                        //child 1?
                        else{
                                close(filedesc[1]);
                                dup2(filedesc[0], STDIN_FILENO);
                                close(filedesc[0]);
                                pipe_spot = 1;
                        }
                }

                // current raw command for the rest of operations
                char* cur_cmd = pipe_cmds[pipe_spot].raw_cmd;
                printf("%s\n", cur_cmd);

                /* Builtin command */
                /* Exit */
                if (!strcmp(cur_cmd, "exit")) {
                        fprintf(stderr, "Bye...\n");
                        fprintf(stderr, "+ completed '%s' [%d]\n", cur_cmd, EXIT_SUCCESS);
                        break;
                }

                /* cd */
                if (!strncmp(cur_cmd, "cd", 2)) {
                        built_in = 1;
                        dir = strchr(cur_cmd, ' ');
                        if (dir[0] == ' ') {
                                dir++;
                        }
                        complete = chdir(dir);
                        fprintf(stderr, "+ completed '%s' [ %d ]\n", cur_cmd, WEXITSTATUS(complete));
                }

                /* pwd */
                if (!strcmp(cur_cmd, "pwd")){
                        getcwd(cwd_buffer, 256);
                }



                struct cmd_line c1;

                /* Check for Output Redirection */
                c1.meta_char_out = redirect_out(cur_cmd);

                /* Check for Input Redirection */
                c1.meta_char_in = redirect_in(cur_cmd);

                /* Parse for arguments */
                /* first argument */
                command_copy = strdup(cur_cmd);
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
                c1.command1 = strtok(cur_cmd, " ");
                //printf("command: %s", c1.command1);
                

                /* Get FileName */
                if (c1.meta_char_out) {
                        c1.o_filename = strchr(command_copy, '>');
                        //printf("file name: %s", c1.o_filename);
                        while(c1.o_filename[0] == '>' || c1.o_filename[0] == ' ') {
                                c1.o_filename++;
                        }
                        //printf("filename: %s", c1.o_filename);
                        fd = open(c1.o_filename,O_WRONLY | O_CREAT | O_TRUNC, 0644);
                }

                /* Clean up Argument */
                //printf("arg: %s\n", c1.arg1);
                if (c1.meta_char_out) {
                        if(!c1.arg1){
                                //still doesn't really work
                                c1.command1 = strtok(cur_cmd, ">");
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

                /* Regular command */
                // retval = system(cmd);
                // fprintf(stdout, "Return status value for '%s': %d\n",
                //         cmd, retval);

                /* fork() + exec() + wait() */
                if (built_in == 0) {
                        pid = fork();
                        if (pid == 0) {
                                /* Child */
                                /* Setup for Output Redirection */
                                if (c1.meta_char_out) {
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
                                fprintf(stderr, "+ completed '%s' [%d]\n", command_copy, WEXITSTATUS(status));
                        } else {
                                perror("fork");
                                exit(1);
                        }
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
int redirect_in(char *command){
        char* meta_char;
        meta_char = strchr(command, '<');
        if (meta_char) {
                return 1;
        }
        return 0;
}