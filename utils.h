#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <dirent.h>
#include "xfs_operations.h"

#define MAX_OPERATION_NAME_LENGTH 20
#define MAX_OPERATION_DESCRIPTION_LENGTH 100
#define SYS_BLOCK_DIR "/sys/block/"
#define MAX_OPERATION_LENGTH (PATH_MAX + MAX_OPERATION_NAME_LENGTH)

enum command {
    LS,
    COPY,
    PWD,
    CD,
    HELP,
    EXIT
};

struct operation {
    enum command command_type;
    char name[MAX_OPERATION_NAME_LENGTH];
    char description[MAX_OPERATION_DESCRIPTION_LENGTH];
};

size_t get_operations_size();
struct operation* get_operations();
void execute_xfs_operation(enum command command_type, char* output_buf, int argc, char** argv, struct xfs_state* xfs_state);
void execute_operation(enum command command_type, char* output_buf, int argc, char** argv);

#endif