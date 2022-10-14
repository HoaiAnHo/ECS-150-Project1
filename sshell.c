#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


#define CMDLINE_MAX 512
#define SIZE 500


struct cmd_line {
        char *command1;
        char *arg1;
        int metaCharOut;
        int metaCharIn;
        char *oFilename;
        char *inFilename;
};
struct stack {
        char *directory[SIZE];
        int size;
        int top;
}; 
typedef struct stack s1;

/* Function Prototypes */
int checkRedirect(char *command, char direction);
char *removeSpace(char *commandLine);
void cd(char *command);
void pwd(char *command);
void newStack(s1 *dirStack);
void push(s1 *dirStack, char* directory);
int pop(s1 *dirStack);
int isEmpty(s1 *dirStack);
int print(s1 *dirStack);


int main(void)
{
        char cmd[CMDLINE_MAX];
        s1 stackDir;
        newStack(&stackDir);

        while (1) {
                char *nl;
                int builtIn = 0;
                char *commandCopy;
                char *dir;
                int complete;
                pid_t pid;
                char cwdBuffer[256];
                int fdOut;
                int fdIn;

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
                        builtIn = 1;
                        cd(cmd);
                }

                /* pwd */
                if (!strcmp(cmd, "pwd")){
                        builtIn = 1;
                        pwd(cmd);
                }

                /* Directory Stack */
                /* dirs */
                if (!strcmp(cmd, "dirs")) {
                        builtIn = 1;
                        if (isEmpty(&stackDir)){
                                getcwd(cwdBuffer, 256);
                                push(&stackDir, cwdBuffer);
                        }
                        complete = print(&stackDir);
                        fprintf(stderr, "+ completed '%s' [%d]\n", cmd, WEXITSTATUS(complete));
                }

                /* pushd */
                if (!strncmp(cmd, "pushd", 5)) {
                        builtIn = 1;
                        dir = strchr(cmd, ' ');
                        if (dir[0] == ' ') {
                                dir++;
                        }
                        complete = chdir(dir);
                        if (!complete) {
                                getcwd(cwdBuffer, 256);
                                push(&stackDir, cwdBuffer);
                        } else {
                                fprintf(stderr, "Error: no such directory\n");
                        }
                        fprintf(stderr, "+ completed '%s' [%d]\n", cmd, WEXITSTATUS(complete));
                }

                /* popd */
                if (!strcmp(cmd, "popd")) {
                        builtIn = 1;
                        if (isEmpty(&stackDir)) {
                                fprintf(stderr, "Error: directory stack empty\n");
                                complete = 1;
                        } else {
                                complete = pop(&stackDir);
                                chdir("..");
                        }
                        fprintf(stderr, "+ completed '%s' [%d]\n", cmd, WEXITSTATUS(complete));
                }

                struct cmd_line c1;

                /* Check for Output Redirection */
                c1.metaCharOut = checkRedirect(cmd, '>');

                /* Check for Input Redirection */
                c1.metaCharIn = checkRedirect(cmd, '<');

                /* Parse for Arguments */
                /* first argument */
                commandCopy = strdup(cmd);
                c1.arg1 = strchr(commandCopy,' ');

                /* extracts the space from the argument */
                if (c1.arg1) {
                        c1.arg1 = removeSpace(c1.arg1);
                }

                /* Command */
                c1.command1 = strtok(cmd, " ");

                /* Error Checking */
                if (c1.command1 == NULL) {
                        fprintf(stderr, "Error: missing command\n");
                }

                /* Get Output FileName */
                if (c1.metaCharOut) {
                        c1.oFilename = strchr(commandCopy, '>');
                        c1.oFilename++;
                        c1.oFilename = removeSpace(c1.oFilename);
                        if (c1.oFilename == NULL) {
                                fprintf(stderr, "Error: no output file\n");
                        }
                        //fdOut = open(c1.oFilename,O_WRONLY | O_CREAT | O_TRUNC, 0644);
                }

                /* Get Input FileName */
                if (c1.metaCharIn) {
                        c1.inFilename = strrchr(commandCopy, '<');
                        c1.inFilename++;
                        c1.inFilename = removeSpace(c1.inFilename);
                        if (c1.inFilename == NULL) {
                                fprintf(stderr, "Error: no input file\n");
                        }
                        // fdIn = open(c1.inFilename, O_RDONLY, 0644);
                        // if (fdIn == -1) {
                        //         fprintf(stderr, "Error: cannot open input file\n");
                        //         builtIn = 1;
                        // }
                }

                /* Clean up Argument if File Output */
                if (c1.metaCharOut) {
                        if(!c1.arg1){
                                c1.command1 = strtok(cmd, ">");
                                printf("parsed command: %s", c1.command1);
                                c1.arg1 = NULL;
                        } else {
                                c1.arg1 = strtok(c1.arg1, ">");
                                c1.arg1 = removeSpace(c1.arg1);
                                if (c1.arg1 == c1.oFilename) {
                                        c1.arg1 = NULL;
                                }
                        }
                }

                //* Clean up Argument if File Input */
                if (c1.metaCharIn) {
                        c1.arg1 = strtok(c1.arg1, "<");
                        c1.arg1 = removeSpace (c1.arg1);
                }

                /* Special Case for Echo */
                if (strcmp(c1.command1, "echo") != 0) {
                        c1.arg1 = strtok(c1.arg1," ");
                }

                char *args[] = {c1.command1, c1.arg1, NULL};

                /* fork() + exec() + wait() */
                if (builtIn == 0) {
                        pid = fork();
                        if (pid == 0) {
                                /* Child */
                                /* Setup for Output Redirection */
                                if (c1.metaCharOut) {
                                        fdOut = open(c1.oFilename,O_WRONLY | O_CREAT | O_TRUNC, 0644);
                                        dup2(fdOut, STDOUT_FILENO);
                                        execvp(c1.command1, args);
                                        close(fdOut);
                                        c1.metaCharOut = 0;
                                } else if (c1.metaCharIn) {
                                        fdIn = open(c1.inFilename, O_RDONLY, 0644);
                                        if (fdIn == -1) {
                                                fprintf(stderr, "Error: cannot open input file\n");
                                                break;
                                        }
                                        dup2(fdIn, STDIN_FILENO);
                                        execvp(c1.command1, args);
                                        close(fdIn);
                                        c1.metaCharIn = 0;
                                } else {
                                        execvp(c1.command1, args);
                                }
                                fprintf(stderr, "Error: Command not found");
                                exit(1);
                        } else if (pid > 0) {
                                /* Parent */
                                int status;
                                waitpid(pid, &status, 0);
                                fprintf(stderr, "+ completed '%s' [%d]\n", commandCopy, WEXITSTATUS(status));
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
        char* metaChar;
        metaChar = strchr(command, direction);
        if (metaChar) {
                return 1;
        }
        return 0;
}
char *removeSpace(char *commandLine) {
        /* Removes spaces in the front */
        while(commandLine[0] == ' ' && commandLine != NULL) {
                commandLine++;
        }
        /* Removes spaces in the end */
        size_t index;
        index = strlen(commandLine) - 1;
        while(commandLine[index] == ' ' && commandLine != NULL) {
                commandLine[index] = '\0';
                index--;
        }
        return commandLine;
}
void cd(char *command) {
        char *dir;
        int complete;
        dir = strchr(command, ' ');
        while(dir[0] == ' ') {
                if (dir[0] == ' ') {
                dir++;
                }
        }
        complete = chdir(dir);
        if (complete != 0) {
               fprintf(stderr, "Error: cannot cd into directory\n"); 
        }
        fprintf(stderr, "+ completed '%s' [%d]\n", command, WEXITSTATUS(complete));
}
void pwd(char *command) {
        char cwdBuffer[512];
        int complete;
        getcwd(cwdBuffer, sizeof(cwdBuffer));
        printf("%s\n", cwdBuffer);
        complete = 0;
        fprintf(stderr, "+ completed '%s' [%d]\n", command, WEXITSTATUS(complete));
}
void newStack(s1 *dirStack) {
        dirStack->top = -1;
        dirStack->size = 0;
}
void push(s1 *dirStack, char* directory) {
        ++dirStack->top;
        dirStack->directory[dirStack->top] = directory;
}
int pop(s1 *dirStack) {
        dirStack->directory[dirStack->top] = NULL;
        dirStack->top--;
        return 0;
}
int isEmpty(s1 *dirStack) {
        if (dirStack->top == -1) {
                return 1;
        }
        return 0;
}
int print(s1 *dirStack) {
        for (int i = dirStack->top; i >= 0; --i) {
                printf("%s\n", dirStack->directory[i]);
        }
        return 0;
}