#ifndef EXTCMD_H
#define EXTCMD_H

#include "commandStruct.h"

/*
 * Execute external command
 */
int execute_cmd(Command *cmd);

/*
 * Wrapper function for execute_cmd null terminates argv before executing
 */
int execute_ext_cmd(Command *cmd);

#endif /* EXTCMD_H */
