/* 
* Internal cd command.
* 
* Changes current working directory of the shell. The shell changes to the
* directory specified by the first and only argument. If no argument is
* specified, the shell changes to user's home directory.
*/
int in_cd(char **args);

/* 
* Internal exit command.
* 
* Terminates the shell.
*/
int in_exit(char **args);
