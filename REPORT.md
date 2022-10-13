# Implementation Description

We used the Professor's suggested workflow to begin implementing this project. We replaced the system() command with fork(), exec(), and wait() from the lecture slides. We used [this link](https://beginnersbook.com/2014/01/c-strings-string-functions/) to better understand the functions of how to manipulate strings with C functions. Next, we used the functions strchr() and strtok() to parse the input from the terminal and separate it into command and arguments and passed to execvp().
