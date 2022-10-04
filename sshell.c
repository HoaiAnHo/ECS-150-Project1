#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CMDLINE_MAX 512

// char* parse() {
//         /* Take in arguments from terminal input */


// }
char* cmd_convert(char* command){
        /* add /bin/ to command */
        printf("got in");
        char* final_cmd = strcat("/bin/", command);
        printf("got concat");
        printf("final command: '%s'", final_cmd);
        return final_cmd;
}
int main(void)
{
        char cmd[CMDLINE_MAX];

        while (1) {
                char *nl;
                //int retval;
                pid_t pid;
                //char path = "";
                char *args[] = {"Hi", NULL};


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
                //cmd_convert(cmd);

                /* Regular command */
                // retval = system(cmd);
                // fprintf(stdout, "Return status value for '%s': %d\n",
                //         cmd, retval);

                /* add "/bin/" to command */
                //char *final_cmd;
                //char src[] = "/bin/";
                char bin[CMDLINE_MAX] = "/bin/";
                strcat(bin, cmd);
                printf("%s",bin);
                //printf("final command: '%s'", src);
        


                /* fork() + exec() + wait() */
                pid = fork();
                if (pid == 0) {
                        /* Child */
                        //printf("cmd: '%s'", cmd);
                        execv(bin,args);
                        perror("execv");
                        exit(1);
                } else if (pid > 0) {
                        /* Parent */
                        int status;
                        waitpid(pid, &status, 0);
                        printf( "Return status value for '%s' : %d\n", bin, WEXITSTATUS(status));
                } else {
                        perror("fork");
                        exit(1);
                }
        }

        return EXIT_SUCCESS;
}
