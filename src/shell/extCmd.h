#ifndef EXTCMD_H
#define EXTCMD_H

/*
 * Execute external command
 */
int execute_cmd(char **args);

/*
 * Wrapper function for execute_cmd null terminates argv before executing
 */
int execute_ext_cmd(int argc, char **argv);

#endif /* EXTCMD_H */
