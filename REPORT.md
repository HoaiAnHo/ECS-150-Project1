# Implementation Description

## Phase 0 - Understanding and Makefile


## Phase 1
We used the Professor's suggested workflow to begin implementing this project. We replaced the system() command with fork(), exec(), and wait() from the lecture slides.

## Phase 2
We used [this link](https://beginnersbook.com/2014/01/c-strings-string-functions/) to better understand the functions of how to manipulate strings with C functions. Next, we used the functions strchr() and strtok() to parse the input from the terminal and separate it into command and arguments and passed to execvp().

## Phase 3
wip

## Phase 4
Continuing with the string parsing from Phase 2, we used strchr() to search for the metacharacter '>'.

## Phase 5
The pipe-work branch was the closest we got to implementing pipelining. The last issue we ran into was that although the pipeline was implemented between two commands, and we would get the output needed from the second command, the last execution would never finish and we would be forced to kill the shell in order to continue.

## Phase 6
wip
