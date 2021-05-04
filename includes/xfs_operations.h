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
    UNSUPPORTED_FILESYSTEM = 1,
    CANT_READ_SB = 2,
    CANT_READ_DINODE = 3
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

char* xfs_ls();
void xfs_copy(char* output_buf, char* from, char* to, struct xfs_state* xfs_state);
char* xfs_pwd();
int xfs_cd(char* output_buf, char* path, struct xfs_state* xfs_state);
char* xfs_cd_perl(char* path);
void execute_help(char* output_buf);
int init_xfs(char* xfs_path);

#endif