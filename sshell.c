#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define CMDLINE_MAX 512
#define SIZE 500


struct cmd_line {
        char *command1;
        char *arg1;
        int meta_char_out;
        int meta_char_in;
        char *o_filename;
        char *in_filename;
};
// struct stack {
//         char *directory[100];
//         int size;
//         //int top;
// }; 
// typedef struct stack s1;

//int top = -1;
char *stack_dir[SIZE];
int top = -1;

/* Function Prototypes */
// int redirect_out(char *command);
// int redirect_in(char *command);
int checkRedirect(char *command, char direction);
void removeSpace(char *command_line);
//void get_filename(char* filename, char *command, char delimiter);
//void newStack(s1 stackDir);
// void push(char* dir_stack[], char* dir, int *index);
// int pop(char* dir_stack[], int *index);
// int isEmpty(int *index);
// int print(char* dir_stack[], int *index);
void push(char* directory);
int pop();
int isEmpty();
int print();


int main(void)
{
        char cmd[CMDLINE_MAX];
        // s1 *dir_stack = (s1*)malloc(sizeof(s1));
        // dir_stack->size = -1;
        // //dir_stack->top = -1;
        // int top = -1;

        while (1) {
                char *nl;
                int built_in = 0; //flag for built in function
                //char *check_pipe;
                //char *arg1;
                //char *arg2 = "";
                //int index = 0;
                //char *command1 = NULL;
                //char *command2;
                //char *pipe_cmd;
                char *command_copy;
                char *working_dir;
                char *dir;
                int complete;
                pid_t pid;
                char cwd_buffer[256];
                int fd_out;
                int fd_in;


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
                        fprintf(stderr, "+ completed '%s' [%d]\n", cmd, EXIT_SUCCESS);
                        break;
                }
                /* cd */
                if (!strncmp(cmd, "cd", 2)) {
                        built_in = 1;
                        dir = strchr(cmd, ' ');
                        if (dir[0] == ' ') {
                                dir++;
                        }
                        complete = chdir(dir);
                        fprintf(stderr, "+ completed '%s' [ %d ]\n", cmd, WEXITSTATUS(complete));
                }
                /* pwd */
                if (!strcmp(cmd, "pwd")){
                        built_in = 1;
                        working_dir = getcwd(cwd_buffer, 256);
                        if (working_dir) {
                                printf("%s\n", working_dir);
                                complete = 0;
                                fprintf(stderr, "+ completed '%s' [ %d ]\n", cmd, WEXITSTATUS(complete));
                        }
                }

                /* Directory Stack */
                /* dirs */
                if (!strcmp(cmd, "dirs")) {
                        built_in = 1;
                        if (isEmpty()){
                                getcwd(cwd_buffer, 256);
                                //push(stack_dir, cwd_buffer, &top);
                                push(cwd_buffer);
                        }
                        complete = print();
                        fprintf(stderr, "+ completed '%s' [ %d ]\n", cmd, WEXITSTATUS(complete));
                }

                /* pushd */
                if (!strncmp(cmd, "pushd", 5)) {
                        built_in = 1;
                        dir = strchr(cmd, ' ');
                        if (dir[0] == ' ') {
                                dir++;
                        }
                        printf("directory: %s\n", dir);
                        complete = chdir(dir);
                        if (!complete) {
                                getcwd(cwd_buffer, 256);
                                push(cwd_buffer);
                        } else {
                                fprintf(stderr, "Error: no such directory\n");
                        }
                        fprintf(stderr, "+ completed '%s' [ %d ]\n", cmd, WEXITSTATUS(complete));
                }

                /* popd */
                if (!strcmp(cmd, "popd")) {
                        built_in = 1;
                        if (isEmpty()) {
                                fprintf(stderr, "Error: directory stack empty\n");
                                complete = 1;
                        } else {
                                complete = pop();
                        }
                        fprintf(stderr, "+ completed '%s' [ %d ]\n", cmd, WEXITSTATUS(complete));
                }


                struct cmd_line c1;

                /* Check for Output Redirection */
                //printf("%d", c1.meta_char_out);
                c1.meta_char_out = checkRedirect(cmd, '>');

                /* Check for Input Redirection */
                c1.meta_char_in = checkRedirect(cmd, '<');

                /* Check for Pipeline */
                // check_pipe = strchr(cmd, '|');
                // if (check_pipe) {
                //         pipe_cmd = strdup(cmd);
                //         command2 = strchr(cmd, '|');
                //         command1 = strtok(pipe_cmd, "|");
                //         printf("pipe cmd 1 : %s \n", command1);
                //         printf("pipe cmd 2 : %s \n", command2);
                // }

                /* extract space from arg */
                // if (command1) {
                // }


                /* Parse for arguments */
                /* first argument */
                command_copy = strdup(cmd);
                c1.arg1 = strchr(command_copy,' ');

                /* extracts the space from the argument */
                if (c1.arg1) {
                        removeSpace(c1.arg1);
                }

                /* Command */
                c1.command1 = strtok(cmd, " ");

                /* Get Output FileName */
                if (c1.meta_char_out) {
                        c1.o_filename = strchr(command_copy, '>');
                        c1.o_filename++;
                        removeSpace(c1.o_filename);
                        fd_out = open(c1.o_filename,O_WRONLY | O_CREAT | O_TRUNC, 0644);
                }

                /* Get Input FileName */
                if (c1.meta_char_in) {
                        c1.in_filename = strrchr(command_copy, '<');
                        c1.in_filename++;
                        removeSpace(c1.in_filename);
                        if (c1.in_filename == NULL) {
                                fprintf(stderr, "Error: no input file\n");
                        }
                        fd_in = open(c1.in_filename, O_RDONLY, 0644);
                        if (fd_in == -1) {
                                fprintf(stderr, "Error: cannot open input file\n");
                                built_in = 1;
                        }
                }

                /* Clean up Argument */
                if (c1.meta_char_out) {
                        if(!c1.arg1){
                                //still doesn't really work ??
                                c1.command1 = strtok(cmd, ">");
                                printf("parsed command: %s", c1.command1);
                                c1.arg1 = NULL;
                        } else {
                                c1.arg1 = strtok(c1.arg1, ">");
                                removeSpace(c1.arg1);
                                if (c1.arg1 == c1.o_filename) {
                                        c1.arg1 = NULL;
                                        //printf("arg1: %s\n", c1.arg1);
                                }
                        }
                }
                //clean up argument for file input
                if (c1.meta_char_in) {
                        c1.arg1 = strtok(c1.arg1, "<");
                        removeSpace (c1.arg1);
                }

                        /* special case */
                if (strcmp(c1.command1, "echo") != 0) {
                        c1.arg1 = strtok(c1.arg1," ");
                }

                char *args[] = {c1.command1, c1.arg1, NULL};


                /* fork() + exec() + wait() */
                if (built_in == 0) {
                        pid = fork();
                        if (pid == 0) {
                                /* Child */
                                /* Setup for Output Redirection */
                                if (c1.meta_char_out) {
                                        dup2(fd_out, STDOUT_FILENO);
                                        execvp(c1.command1, args);
                                        close(fd_out);
                                } else if (c1.meta_char_in) {
                                        dup2(fd_in, STDIN_FILENO);
                                        execvp(c1.command1, args);
                                        close(fd_in);
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
        }

        return EXIT_SUCCESS;
}

/* Function Definitions */
int checkRedirect(char *command, char direction) {
        char* meta_char;
        meta_char = strchr(command, direction);
        if (meta_char) {
                return 1;
        }
        return 0;
}
void removeSpace(char *command_line) {
        /* Removes spaces in the front */
        while(command_line[0] == ' ' && command_line != NULL) {
                command_line++;
        }
        /* Removes spaces in the end */
        size_t index;
        index = strlen(command_line) - 1;
        while(command_line[index] == ' ' && command_line != NULL) {
                command_line[index] = '\0';
                index--;
        }
}
// void get_filename(char* filename, char *command, char delimiter) {
//                 filename = strchr(command, delimiter);
//                 //printf("file name: %s", c1.o_filename);
//                 if (filename[0] == '>'){
//                         filename++;
//                 }
//                 //remove_space(filename);
//                 //printf("file name: %s", filename);
// }
// void push(char* dir_stack[], char* dir, int *index) {
//         // printf("top: %d\n", index);
//         // //stackDir->top++;
//         // index++;
//         // //printf("incremented top: %d\n", stackDir->top);
//         // stackDir->directory[index] = dir;
//         // printf("directory: %s\n", stackDir->directory[index]);
//         // //printf("directory: %s\n", stackDir->directory[stackDir->top--]);
//         // stackDir->size++;
//         *index++;
//         dir_stack[*index] = dir;
//         printf("directory: %s \n", dir_stack[*index]);
// }
// int pop(char* dir_stack[], int *index) {
//         // stackDir->directory[index] = NULL;
//         // //stackDir->top--;
//         // index--;
//         // stackDir->size--;
//         // return 0;
//         dir_stack[*index] = NULL;
//         //index--;
//         return 0;
// }
// int isEmpty(int *index) {
//         if (*index == -1) {
//                 return 1;
//         }
//         return 0;
// }
// int print(char* dir_stack[], int *index) {
//         //printf("%s",stackDir->directory[stackDir->top]);
//         //int i = stackDir->top;
//         // while(i >= 0) {
//         //         printf("%s\n", stackDir->directory[i]);
//         //         i--;
//         // }
//         // for (int i = index; i >= 0; i--) {
//                 printf("%s\n", dir_stack[*index]);
//         //         printf("i: %d\n", i);
//         // }
//         return 0;
// }
void push(char* directory) {
        top++;
        stack_dir[top] = directory;
}
int pop() {
        stack_dir[top] = NULL;
        top--;
        return 0;
}
int isEmpty() {
        if (top == -1) {
                return 1;
        }
        return 0;
}
int print() {
        int i = top;
        for (i = top; i > -1; i--) {
                printf("%s\n", stack_dir[i]);
                printf("i: %d\n", i);
        }
        return 0;
}