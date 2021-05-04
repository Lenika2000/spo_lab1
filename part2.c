#include "includes/part2.h"

void read_user_input(char* input) {
    input[0] = '\0';
    do {
        printf("> ");
        // считывем данные из потока, пока не наступит символ новой строки
        fgets(input, MAX_INPUT_LENGTH, stdin);
    } while (strlen(input) == 1);
    // заменяем симовол новой строки концом строки
    input[strlen(input) - 1] = '\0';
}

void do_operation(char* input, struct xfs_state* xfs_state) {
    struct operation* operations = get_operations();
    int argc = 0;
    char* argv[MAX_OPERANDS] = { NULL };
    char input_copy[MAX_INPUT_LENGTH];
    char output_buf[TMP_MAX] = {'\0'};
    enum command selected_command = HELP;
    // получаем количество доступных команд для фс
    size_t operations_size = get_operations_size();

    strcpy(input_copy, input);
    // разбивка строки с командой и аргументами(при наличии)
    char* input_ptr = strtok(input_copy, " ");
    // проходим по всем операциям и ищем совпадение со введенной строкой
    for (size_t i = 0; i < operations_size; i++) {
        if (strcmp(operations[i].name, input_ptr) == 0) {
            selected_command = operations[i].command_type;
            break;
        }
    }
    // заносим необходимые аргументы в массов аргументов + ведем подсчет их количества
    while ((input_ptr = strtok(NULL, " ")) != NULL) {
        // выделяем память под каждый аргумент
        argv[argc] = malloc((strlen(input_ptr) + 1) * sizeof(char));
        strcpy(argv[argc], input_ptr);
        // подсчитываем количество аргументов
        argc += 1;
    }
//    execute_xfs_operation(selected_command, output_buf, argc, argv, xfs_state);
    printf("%s", output_buf);
//    for (int i = 0; i < argc; i++) {
//        free(argv[i]);
//    }
}

void do_task_2(char* start_path) {
    char input[MAX_INPUT_LENGTH] = {'\0'};
    struct xfs_state* xfs_state = NULL;
    // инициализация фс, считывание информации из суперблока и первого inode
    xfs_state = init(start_path, NULL);
    do {
        read_user_input(input);
        do_operation(input, xfs_state);
    } while (strcmp(input, "exit") != 0);
}