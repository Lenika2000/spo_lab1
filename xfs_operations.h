#ifndef XFS_OPERATIONS_H_
#define XFS_OPERATIONS_H_

#define XFS_SB_MAGIC 0x58465342 // from docs

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <byteswap.h>
#include <sys/stat.h>
#include "xfs.h"
#include "utils.h"

#define swap(a, b) typeof(a) __temp = a; a = b; b = __temp;

enum xfs_error {
    OK = 0,
    UNSUPPORTED_FILESYSTEM,
    CANT_READ_SB,
    CANT_READ_DINODE
};

struct xfs_state {
    enum xfs_error error;
    // суперблок
    struct xfs_sb sb;
    struct xfs_dinode dinode;
    unsigned long long address;
    char path[PATH_MAX];
    FILE* file_pointer;
};

struct xfs_state* init(char* fs_path, struct xfs_state* xfs_state);
void destroy(struct xfs_state* xfs_state);

void xfs_ls(char* output_buf, struct xfs_state* xfs_state);
void xfs_copy(char* output_buf, char* from, char* to, struct xfs_state* xfs_state);
void xfs_pwd(char* output_buf, struct xfs_state* xfs_state);
void xfs_cd(char* output_buf, char* path, struct xfs_state* xfs_state);
void execute_help(char* output_buf);

#endif