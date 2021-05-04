#include "includes/utils.h"

static struct operation operations[] = {
        { LS,   "ls",   "Вывести файлы с их атрибутами в директории" },
        { COPY, "cp",   "Скопировать файлы или директории" },
        { PWD,  "pwd",  "Показать текущую директорию" },
        { CD,   "cd",   "Переход в другую директорию" },
        { HELP, "help",    "Получить список доступных команд" },
        { EXIT, "exit",    "Выход" }
};

struct operation* get_operations() {
    return operations;
}

// получение числа доступных операций
size_t get_operations_size() {
    return sizeof(operations) / sizeof(struct operation);
}

void execute_xfs_operation(enum command command_type, char* output_buf, int argc, char** argv, struct xfs_state* xfs_state) {
    switch (command_type) {
        case LS:
            xfs_ls(output_buf, xfs_state);
            break;
        case COPY:
            if (argc == 2) {
//                xfs_copy(output_buf, argv[0], argv[1], xfs_state);
            } else {
                printf("Данное количество аргументов не поддерживается.\n");
            }
            break;
        case PWD:
            xfs_pwd(output_buf, xfs_state);
            break;
        case CD:
            if (argc == 1) {
                xfs_cd(output_buf, argv[0], xfs_state);
            } else {
                printf("Данное количество аргументов не поддерживается.\n");
            }
            break;
        case EXIT:
            break;
        default:
            execute_help(output_buf);
    }
}
