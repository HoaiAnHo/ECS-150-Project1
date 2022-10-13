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
        char *raw_cmd;
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
                char *dir;
                int complete;
                pid_t pid;
                pid_t pid_2;
                char cwdBuffer[256];
                int fdOut;
                int fdIn;
                char* pipe_check;
                int pipe_amount = 0;
                char *pipe_copy;
                char *full_copy;

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

                /* Pipe parsing */
                struct cmd_line pipe_cmds[4];
                pipe_check = strchr(cmd, '|');
                pipe_copy = strdup(cmd);
                if (pipe_check) {
                        char *token = strtok(pipe_copy, "|");
                        while (token != NULL) {
                                while (token[0] == ' ') {
                                        token++;
                                }
                                pipe_cmds[pipe_amount].raw_cmd = token;
                                pipe_amount++;
                                token = strtok(NULL, "|");
                        }
                }
                if (pipe_amount == 0) {
                        pipe_cmds[0].raw_cmd = cmd;
                        pipe_amount = 1;
                }

                for (int i = 0; i < pipe_amount; i++){
                        char *commandCopy;

                        /* Check for Output Redirection */
                        pipe_cmds[i].metaCharOut = checkRedirect(pipe_cmds[i].raw_cmd, '>');

                        /* Check for Input Redirection */
                        pipe_cmds[i].metaCharIn = checkRedirect(pipe_cmds[i].raw_cmd, '<');

                        /* Parse for Arguments */
                        /* first argument */
                        commandCopy = strdup(pipe_cmds[i].raw_cmd);
                        pipe_cmds[i].arg1 = strchr(commandCopy,' ');

                        /* extracts the space from the argument */
                        if (pipe_cmds[i].arg1) {
                                pipe_cmds[i].arg1 = removeSpace(pipe_cmds[i].arg1);
                                //while (pipe_cmds[i].arg1[0] == ' ') pipe_cmds[i].arg1++;
                        }

                        /* Command */
                        pipe_cmds[i].command1 = strtok(pipe_cmds[i].raw_cmd, " ");

                        /* Error Checking */
                        if (pipe_cmds[i].command1 == NULL) {
                                fprintf(stderr, "Error: missing command\n");
                        }

                        /* Get Output FileName */
                        if (pipe_cmds[i].metaCharOut) {
                                pipe_cmds[i].oFilename = strchr(commandCopy, '>');
                                pipe_cmds[i].oFilename++;
                                removeSpace(pipe_cmds[i].oFilename);
                                if (pipe_cmds[i].oFilename == NULL) {
                                        fprintf(stderr, "Error: no output file\n");
                                }
                                fdOut = open(pipe_cmds[i].oFilename,O_WRONLY | O_CREAT | O_TRUNC, 0644);
                        }

                        /* Get Input FileName */
                        if (pipe_cmds[i].metaCharIn) {
                                pipe_cmds[i].inFilename = strrchr(commandCopy, '<');
                                pipe_cmds[i].inFilename++;
                                removeSpace(pipe_cmds[i].inFilename);
                                if (pipe_cmds[i].inFilename == NULL) {
                                        fprintf(stderr, "Error: no input file\n");
                                }
                                fdIn = open(pipe_cmds[i].inFilename, O_RDONLY, 0644);
                                if (fdIn == -1) {
                                        fprintf(stderr, "Error: cannot open input file\n");
                                        builtIn = 1;
                                }
                        }

                        /* Clean up Argument if File Output */
                        if (pipe_cmds[i].metaCharOut) {
                                if(!pipe_cmds[i].arg1){
                                        pipe_cmds[i].command1 = strtok(pipe_cmds[i].raw_cmd, ">");
                                        printf("parsed command: %s", pipe_cmds[i].command1);
                                        pipe_cmds[i].arg1 = NULL;
                                } else {
                                        pipe_cmds[i].arg1 = strtok(pipe_cmds[i].arg1, ">");
                                        pipe_cmds[i].arg1 = removeSpace(pipe_cmds[i].arg1);
                                        if (pipe_cmds[i].arg1 == pipe_cmds[i].oFilename) {
                                                pipe_cmds[i].arg1 = NULL;
                                        }
                                }
                        }

                        //* Clean up Argument if File Input */
                        if (pipe_cmds[i].metaCharIn) {
                                pipe_cmds[i].arg1 = strtok(pipe_cmds[i].arg1, "<");
                                pipe_cmds[i].arg1 = removeSpace(pipe_cmds[i].arg1);
                        }

                        /* Special Case for Echo */
                        if (strcmp(pipe_cmds[i].command1, "echo") != 0) {
                                pipe_cmds[i].arg1 = strtok(pipe_cmds[i].arg1," ");
                        }
                }

                /* fork() + exec() + wait() */
                if (builtIn == 0) {
                        int pipefile[2];
                        if (pipe_amount > 1) {
                                pipe(pipefile);
                        }
                        else {
                                close(pipefile[0]);
                                close(pipefile[1]);
                        }
                        pid = fork();
                        if (pid == 0) {
                                /* Child */
                                /* Setup for piping */
                                if (pipe_amount == 2){
                                        close(pipefile[0]);
                                        dup2(pipefile[1], STDOUT_FILENO);
                                        close(pipefile[1]);
                                }
                                /* Setup for Output Redirection */
                                if (pipe_cmds[0].metaCharOut) {
                                        char *args[] = {pipe_cmds[0].command1, pipe_cmds[0].arg1, NULL};
                                        dup2(fdOut, STDOUT_FILENO);
                                        execvp(pipe_cmds[0].command1, args);
                                        close(fdOut);
                                        pipe_cmds[0].metaCharOut = 0;
                                } else if (pipe_cmds[0].metaCharIn) {
                                        char *args[] = {pipe_cmds[0].command1, pipe_cmds[0].arg1, NULL};
                                        fdIn = open(pipe_cmds[0].inFilename, O_RDONLY, 0644);
                                        if (fdIn == -1) {
                                                fprintf(stderr, "Error: cannot open input file\n");
                                                break;
                                        }
                                        dup2(fdIn, STDIN_FILENO);
                                        execvp(pipe_cmds[0].command1, args);
                                        close(fdIn);
                                        pipe_cmds[0].metaCharIn = 0;
                                } else  {
                                        char *args[] = {pipe_cmds[0].command1, pipe_cmds[0].arg1, NULL};
                                        printf("check0 %i\n", pipe_amount);
                                        printf("%s\n", args[1]);
                                        //segfaults here
                                        execvp(pipe_cmds[0].command1, args);
                                        printf("check1\n");
                                }
                                fprintf(stderr, "Error: Command not found");
                                exit(1);
                        } 
                        else if (pid > 0) {
                                /* Parent */
                                int status;
                                waitpid(pid, &status, 0);
                                if (pipe_amount == 2) pid_2 = fork();
                                if (pid_2 == 0) {
                                        /* Child 2 for piping */
                                        close(pipefile[1]);
                                        dup2(pipefile[0], STDIN_FILENO);
                                        close(pipefile[0]);
                                        char *args2[] = {pipe_cmds[1].command1, pipe_cmds[1].arg1, NULL};
                                        //stalls here
                                        execvp(pipe_cmds[1].command1, args2);
                                        fprintf(stderr, "Error: Command not found");
                                        exit(1);
                                }
                                else if (pid_2 > 0 || pipe_amount == 1) {
                                        printf("check2\n");
                                        if (pipe_amount == 2) {
                                                int new_status;
                                                waitpid(pid_2, &new_status, 0);
                                                fprintf(stderr, "+ completed '%s' [%d][%d]\n", full_copy, WEXITSTATUS(status), WEXITSTATUS(new_status));
                                        }
                                        else if (pipe_amount == 1){
                                                printf("check3\n");
                                                fprintf(stderr, "+ completed '%s' [%d]\n", full_copy, WEXITSTATUS(status));
                                        }
                                }
                                else {
                                        perror("fork");
                                        exit(1);
                                }
                        }
                        else {
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
               fprintf(stderr, "Error: cannot cd into directory"); 
        }
        fprintf(stderr, "+ completed '%s' [%d]\n", command, WEXITSTATUS(complete));
}
void pwd(char *command) {
        char *workingDir;
        char cwdBuffer[512];
        int complete;
        workingDir = getcwd(cwdBuffer, 512);
        //if (workingDir) {
                printf("%s\n", workingDir);
                complete = 0;
                fprintf(stderr, "+ completed '%s' [%d]\n", command, WEXITSTATUS(complete));
        //}
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