#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define CMDLINE_MAX 512

struct cmd_line {
        char *raw_cmd;
        char *command1;
        char *arg1;
        char *meta_char;
        char *o_filename;
        char *cpy;
};
int main(void)
{
        char cmd[CMDLINE_MAX];

        while (1) {
                char command_copy[CMDLINE_MAX];
                char cwd_buffer[256];
                int fd;
                char *nl;


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

                // Pipe parsing -----------------------------------------------------
                // We create an array of commands, one for each piped command
                struct cmd_line pipe_cmds[4];
                char* pipe_check = strchr(cmd, '|');
                int pipe_amount = 0;
                // if pipe symbol found, split into raw commands
                char cmd_copy[CMDLINE_MAX];
                strcpy(cmd_copy, cmd);
                strcpy(command_copy, cmd);
                if (pipe_check)
                {
                        char* token;
                        token = strtok(cmd_copy, "|");
                        while (token != NULL) {
                                while (token[0] == ' ') {
                                        token++;
                                }
                                pipe_cmds[pipe_amount].raw_cmd = token;
                                pipe_amount++;
                                token = strtok(NULL, "|");
                        }
                }
                // if no pipe is found, we're dealing with the first cmd only in cmds array -----------
                if (pipe_amount == 0) {
                        pipe_cmds[pipe_amount].raw_cmd = cmd;
                        pipe_amount++;
                }

                // For each raw command, parse and grab elements in a loop
                for (int i = 0; i < pipe_amount; i++){
                        /* Parse for arguments */
                        /* first argument */
                        pipe_cmds[i].cpy = strdup(pipe_cmds[i].raw_cmd);
                        // with pipe process, check if any leading white spaces for command ---------------
                        pipe_cmds[i].arg1 = strchr(pipe_cmds[i].cpy,' ');


                        /* extracts the space from the argument */
                        if (pipe_cmds[i].arg1) {
                                pipe_cmds[i].arg1++;
                                while (pipe_cmds[i].arg1[0] == ' ' && pipe_cmds[i].arg1 != NULL) {
                                        pipe_cmds[i].arg1++;
                                }
                                /* Check for Output Redirection */
                                pipe_cmds[i].meta_char = strchr(pipe_cmds[i].arg1, '>');
                        }

                        /* Command */
                        pipe_cmds[i].command1 = strtok(pipe_cmds[i].raw_cmd, " ");


                        /* Exclude Output Redirection from Arguments */
                        if (pipe_cmds[i].meta_char) {
                                pipe_cmds[i].o_filename = pipe_cmds[i].meta_char;
                                while(pipe_cmds[i].o_filename[0] == '>') {
                                        pipe_cmds[i].o_filename++;
                                }
                                fd = open(pipe_cmds[i].o_filename,O_WRONLY | O_CREAT, 0644);
                                pipe_cmds[i].arg1 = strtok(pipe_cmds[i].arg1,">");
                                pipe_cmds[i].arg1 = strtok(pipe_cmds[i].arg1," ");
                        }
                }


                /* Builtin command */
                /* Exit */
                if (!strcmp(cmd, "exit")) {
                        fprintf(stderr, "Bye...\n");
                        break;
                }

                /* cd */
                if (!strcmp(pipe_cmds[0].command1, "cd")) {
                        chdir(pipe_cmds[0].arg1);
                }

                /* pwd */
                if (!strcmp(cmd, "pwd")){
                        getcwd(cwd_buffer, 256);
                }

                // create file descriptor for pipe ------------------------------------------
                int filedesc[2];
                if (pipe_amount > 1) {
                        pipe(filedesc);
                }
                else {
                        close(filedesc[0]);
                        close(filedesc[1]);
                }
                // regular execution of system commands, but pipe style ----------------------
                pid_t pid = fork();
                // First child redirects output of its execution -----------------------------
                if (pid == 0) {
                        if (pipe_amount > 1){
                                close(filedesc[0]);
                                dup2(filedesc[1], STDOUT_FILENO);
                                close(filedesc[1]);
                        }
                        // I moved arguments to here since I didn't know how to include them in struct
                        char *args[] = {pipe_cmds[0].command1, pipe_cmds[0].arg1, NULL};
                        if (pipe_cmds[0].meta_char) {
                                dup2(fd, STDOUT_FILENO);
                                execvp(pipe_cmds[0].command1, args);
                                close(fd);
                        }
                        else {
                                execvp(pipe_cmds[0].command1, args);
                        }
                        perror("execvp");
                        exit(1);
                }
                // Parent waits for first child to finish --------------------------------------
                else if (pid > 0) {
                        int status;
                        waitpid(pid, &status, 0);
                        pid_t pid_2; 
                        if (pipe_amount > 1) pid_2 = fork();
                        else goto gohome;
                        if (pid_2 == 0){
                                // Second child redirects input and executes second command -----------------
                                close(filedesc[1]);
                                dup2(filedesc[0], STDIN_FILENO);
                                close(filedesc[0]);
                                char *args2[] = {pipe_cmds[1].command1, pipe_cmds[1].arg1, NULL};
                                execvp(pipe_cmds[1].command1, args2);
                                //where file goes poof
                                perror("execvp");
                                exit(1);
                        }
                        // Everyone comes here to this parent regardless of pipes ----------------------------
                        else if (pid_2 > 0){
                                int new_status;
                                if (pipe_amount == 2) {
                                        waitpid(pid_2, &new_status, 0);
                                }
                                else {
                                        gohome:
                                        new_status = status;
                                        fprintf(stderr, "+ completed '%s' [%d]\n", command_copy, WEXITSTATUS(new_status));
                                }
                                if (pipe_amount == 2){
                                        fprintf(stderr, "+ completed '%s' [%d][%d]\n", command_copy, WEXITSTATUS(status), WEXITSTATUS(new_status));
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
        return EXIT_SUCCESS;
}