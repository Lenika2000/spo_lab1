#include "includes/part2.h"
#include "includes/part1.h"

int main(int argc, char** argv) {
    if (argc == 1) {
        fprintf(stderr, "Для выбора режима работы программы в качестве аргумента добавьте TASK_1 или TASK_2.\n");
        return 1;
    }

    if (argc > 3) {
        fprintf(stderr, "Ошибка. Превышено допустимое количество аргументов командной строки.\n");
        return 1;
    }

    if (argc == 2) {
        if (!strcmp(argv[1], "TASK_1")) {
            do_task_1();
            return 0;
        } else if (strcmp(argv[1], "TASK_2") == 0) {
            fprintf(stderr, "За аргументом '%s' должен располагаться путь к файловой системе xfs\n", argv[1]);
            return 1;
        }
    } else if (argc == 3 && strcmp(argv[1], "TASK_2") == 0) {
        printf("Выбран режим работы с файловой системой xfs. Для получения информации о всех командах введите help.");
        do_task_2(argv[2]);
        return 0;
    } else {
        fprintf(stderr, "Данный формат аргумента не поддерживается '%s'\n", argv[1]);
        return 1;
    }
}