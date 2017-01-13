#ifndef EXTCMD_H
#define EXTCMD_H

#include "commandStruct.h"

/*
 * Execute external command
 */
int execute_cmd(Command *cmd, int input_fd);

/*
 * Wrapper for execute_cmd to restore stdin and stdout
 */
int execute_ext_cmd(Command *cmd);
#endif /* EXTCMD_H */
