# Implementation Description
Out of the current work we've done, we were able to fully implement Phases 0, 
1, 2, and 4. Phase 3 and 6 are similar in concepts, but were a little shaky in 
how effective they consistently were (at least according to the autograder). 
Phase 5 piping was unable to be implemented in a satisfactory way, but we 
believe that with a bit more time, the pipeline could've at least been 
implemented for two commands.

In terms of coding style, this code utilizes camelCase conventions.

## Phase 0 - Understanding and Makefile
The Makefile generates the executable shell with gcc as  requested, and there
is also a clean rule included.

## Phase 1
We used the Professor's suggested workflow to begin implementing this project. 
We replaced the system() command with fork(), exec(), and wait() from the 
lecture slides. 

## Phase 2
We used [this link](https://beginnersbook.com/2014/01/c-strings-string-functions/)
to better understand the functions of how to manipulate strings with C functions.
Next, we used the functions strchr() and strtok() to parse the input from the
terminal and separate it into command and arguments and passed to execvp(). 
Using our knowledge of pointers, we also accounted for additional spaces with 
a loop that kept moving the pointer along as long as it was pointing to a 
space or not pointing to a null character. We then split whatever arguments were
in there and assigned them to different members of a structure. While we used a
helper function for space removal work, we had to change it from a void function
to a char * returning function, as the scope of the changes made in there would
not return outside.

## Phase 3
We implemented cd and pwd by following the sample code given in lecture and also 
following the libc documentation, as there doesn't appear to be major issues, 
yet the autograder still returns incorrectly. We used given class functions like chdir() 
and getcwd() for the work.

## Phase 4
Continuing with the string parsing from Phase 2, we used strchr() to search 
for the metacharacter '>'. If this character was found, we would parse the string 
for the filename. Then we used the appropriate open() function and dup2() to set up
the redirection. After the command had been executed, we closed files.

## Phase 5
The pipe-work branch was the closest we got to implementing pipelining. The last
issue we ran into was that although the pipeline was implemented between two
commands, and we would get the output needed from the second command, the last
execution would never finish and we would be forced to kill the shell in order 
to continue.

The initial plan was to utilize a loop to count the number of pipe meta
characters, then use that number to loop through the full command to split
into smaller raw commands. We would then create an array of command line structs
where certain indexes would hold those commands in order. The process of parsing
through commands to assign an individual struct its own argument members would
also be a loop dependent on the number of commands given on the line. After the
process, we then enter the execution phase. I was assuming sm

## Phase 6
We were able to implement standard input redirection by using similar logic to
output redirection. We parsed the command line for the "<" character. The only 
difference was we had to implement error handling for if the file did not exist
or could not be opened. We had a more difficult time implementing the stack 
directory because the stack data was being altered everytime something was pushed
onto the stack. Later, we realized this was because the address of the stack was
being pushed in and our function would alter all the values that were associated 
with the stack. If given more time, we would have a better chance of properly
implementing this feature. It also took a long time and a lot of research to figure
out how to pass in the arguments correctly with pointers and references.

## The Debugging Process
We didn't utilize gdb for much debugging, but instead utilized print statements. 
There was a short time where we had to do some string concatenation in order to 
execute commands (adding "/bin/" to the front of first commands) before we 
realized we could've used execvp(). The bulk of the project was last spent on 
trying to fix phases 3, 5, and 6. Space characters were also a boon, as we tried 
to remove whatever amounts remained without causing segmentation faults.
