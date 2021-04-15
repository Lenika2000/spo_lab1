#ifndef PART2_H_
#define PART2_H_

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "utils.h"
#include "xfs_operations.h"

#define MAX_INPUT_LENGTH 256
#define MAX_OPERANDS 100

void read_user_input(char* input);
void do_operation(char* input, struct xfs_state* xfs_state);
void do_task_2(char* start_path);

#endif