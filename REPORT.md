**# Implementation Description**
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
Implementing pwd and cd was interesting, as there doesn't appear to be major 
issues, yet the autograder still returns incorrectly. Of course, we used given
class functions like chdir() for the work.

## Phase 4
Continuing with the string parsing from Phase 2, we used strchr() to search 
for the metacharacter '>'.

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
While implementing standard input redirection wasn't feasible for this project,
we were able to utilize a combination of cd/pwd work and some helper functions 
for linked lists to implement the extra pushd, popd, and dirs commands.

As there were issues with the scope of changes made within helper functions, we
tried changing the newStack, push, and pop functions into returning structure
pointer functions.

## The Debugging Process
We didn't utilize gdb for much debugging, but instead did some good
old-fashioned print statements. There was a short time where we had to do some
string concatenation in order to execute commands (adding "/bin/" to the front
of first commands) before we realized we could've used execvp(). The bulk of 
the project was last spent on trying to fix phases 3, 5, and 6. Space
characters were also a boon, as we tried to remove whatever amounts remained 
without causing segmentation faults.
