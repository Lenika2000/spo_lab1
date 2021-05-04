#include "xfs_operations.h"

static inline __uint64_t xfs_mask64lo(int n) {
    return ((__uint64_t) 1 << (n)) - 1;
}

static inline xfs_uint8_t* xfs_dir2_sf_ftype(struct xfs_dir2_sf_entry* entry) {
    return (entry->name + entry->namelen);
}

static inline long inou_t_to_long(xfs_dir2_inou_t* inou) {
    return inou->i4.i[0] * 255 * 255 * 255 + inou->i4.i[1] * 255 * 255 + inou->i4.i[2] * 255 + inou->i4.i[3];
}

static inline long xfs_dir2_sf_inumber(struct xfs_dir2_sf_entry* entry) {
    return inou_t_to_long((xfs_dir2_inou_t *) (entry->name + entry->namelen + sizeof(entry->ftype)));
}

// из заголовка родительской директории получаем ее inode
static inline long xfs_dir2_sf_parent(struct xfs_dir2_sf_hdr* hdr) {
    return inou_t_to_long(&hdr->parent);
}

static inline unsigned long xfs_dir2_sf_entry_true_size(struct xfs_dir2_sf_entry* entry) {
    return sizeof(struct xfs_dir2_sf_entry) + entry->namelen - sizeof(xfs_dir2_ino4_t) - 1;
}

static inline unsigned long xfs_dir2_sf_entry_address_offset() {
    return sizeof(struct xfs_dinode_core) + sizeof(struct xfs_dir2_sf_hdr) - (sizeof(xfs_dir2_ino8_t) - sizeof(xfs_dir2_ino4_t));
}

void xfs_dir2_sf_filename(struct xfs_dir2_sf_entry* entry, char* filename) {
    // сохраняем имя файла в массив символов filename
    for (xfs_uint8_t i = 0; i < entry->namelen; i++) {
        filename[i] = entry->name[i];
    }

    if (*xfs_dir2_sf_ftype(entry) == 2) {
        // если это директория, то добавляем слэш
        filename[entry->namelen] = '/';
        filename[entry->namelen + 1] = '\0';
    } else {
        // если просто файл, то просто добавляем символ окончания строки
        filename[entry->namelen] = '\0';
    }
}

// в наше состояние записываем путь, по которому перешли по команде cd
void construct_path(char* path, char* result) {
    char* token;
    unsigned long current_path_len;
    char* path_copy;

    if (path[0] == '/') {
        result[1] = '\0';
    }
    path_copy = calloc(strlen(path) + 1, sizeof(char));
    strcpy(path_copy, path);
    token = strtok(path_copy, "/");

    while (token != NULL) {
        // в конец результирующего пути добавляем текущую директорию
        strcat(result, token);
        current_path_len = strlen(result);
        if (strcmp(token, ".") == 0) {
            // убираем . и /
            result[current_path_len - 1] = '\0';
            result[current_path_len] = '\0';
        } else
            // если нам нужно вернуться на директорию выше
            if (strcmp(token, "..") == 0) {
                // убираем .. и /
            current_path_len -= 4;
            // далее добираемся до родительской директории
            while (result[current_path_len] != '/') current_path_len--;
            result[++current_path_len] = '\0';
        } else {
            strcat(result, "/");
        }
        // выделение очередной части строки
        token = strtok(NULL, "/");
    }

    if (strcmp(path_copy, "/..") == 0) {
        path[1] = '\0';
    }

    free(path_copy);
}

// todo
void split_path(char* path, char* dir) {
    unsigned long path_len = strlen(path);
    if (path[--path_len] == '/') path[path_len] = '\0';
    while (path[--path_len] != '/' && path_len >= 0);
    path[path_len+1] = '\0';
    if (path_len == 0) {
        dir = NULL;
    } else {
        dir = path;
    }
}

// чтение информации из суперблока
void read_sb(struct xfs_state* xfs_state) {
    // если нет файла по переданному пути, то мы не можем прочесть его содержимое
    if (xfs_state->file_pointer == NULL) {
        xfs_state->error = CANT_READ_SB;
        return;
    }
    // из указанного файла считываем данные о суперблоке
    fread(&xfs_state->sb, sizeof(struct xfs_sb), 1, xfs_state->file_pointer);
    // исходя из размерности типа данных выбираем функцию по вычислению обратного прядка байт
    // magicnum идентифицирует фс. Для XFS это (0x58465342).
    xfs_state->sb.sb_magicnum = bswap_32(xfs_state->sb.sb_magicnum);
    // blocksize - размер базовой единицы выделяемого пространства в байтах.
    xfs_state->sb.sb_blocksize = bswap_32(xfs_state->sb.sb_blocksize);
    // rootino - номер корневого индекса файловой системы
    xfs_state->sb.sb_rootino = bswap_64(xfs_state->sb.sb_rootino);
    // inodesize - размер inode в байтах. Значение по умолчанию - 256
    xfs_state->sb.sb_inodesize = bswap_16(xfs_state->sb.sb_inodesize);
    // если значение magicnum не совпадает, то данные организованы не в том формате, что нам требуются
    if (xfs_state->sb.sb_magicnum != XFS_SB_MAGIC) {
        xfs_state->error = UNSUPPORTED_FILESYSTEM;
    }
}

// чтение индексного дескриптора
void read_dinode(struct xfs_state* xfs_state, struct xfs_dinode* dinode) {
    // устанавливаем указатель в файле на начало расположения корневого индекса фс (address от начала файла)
    fseek(xfs_state->file_pointer, xfs_state->address, SEEK_SET);
    // аналогично суперблоку считываем информацию о dinode
    fread(dinode, sizeof(struct xfs_dinode), 1, xfs_state->file_pointer);
    dinode->di_core.di_magic = bswap_16(dinode->di_core.di_magic);
    // ino полный номер inode этого узла
    dinode->di_core.di_ino = bswap_64(dinode->di_core.di_ino);
    // nextents задает количество связанных с этим индексом данных данных
    dinode->di_core.di_nextents = bswap_32(dinode->di_core.di_nextents);
    // nblocks число блоков, необходимых для хранения данных inode
    dinode->di_core.di_nblocks = bswap_64(dinode->di_core.di_nblocks);
    // di_size размер файла в байтах. Для обычных файлов это размер файла в байтах, каталоги, пространство,
    // занимаемое записями каталога, а для ссылок - длина символической ссылки.
    dinode->di_core.di_size = bswap_64(dinode->di_core.di_size);
    if (dinode->di_core.di_magic != XFS_DINODE_MAGIC) {
        xfs_state->error = CANT_READ_DINODE;
    }
}

// инициализация фс
struct xfs_state* init(char* fs_path, struct xfs_state* xfs_state) {
    // если структура xfs_state еще не создана выделяем под нее память
    if (xfs_state == NULL) {
        xfs_state = malloc(sizeof(struct xfs_state));
    }
    xfs_state->error = OK;
    // добавляем в структуру путь до нашей файловой системы
    strcpy(xfs_state->path, fs_path);
    // отквываем двоичный файл для чтения
    xfs_state->file_pointer = fopen(fs_path, "rb");
    // производим чтение суперблока для получения информации об организации остальных данных на диске
    read_sb(xfs_state);
    // если мы смогли прочитать файл и все впорядке с магическим числом, то переходим далее
    if (xfs_state->error) {
        return xfs_state;
    }
    // получаем адрес корневого индекса фс
    xfs_state->address = xfs_state->sb.sb_rootino * xfs_state->sb.sb_inodesize;
    // производим чтение первого индексного дескриптора
    read_dinode(xfs_state, &xfs_state->dinode);
    // установка корня
    xfs_state->path[0] = '/';
    xfs_state->path[1] = '\0';
    return xfs_state;
}

void destroy(struct xfs_state* xfs_state) {
    fclose(xfs_state->file_pointer);
    free(xfs_state);
}

// чтение содержимого директории
void xfs_readdir(char* output_buf, struct xfs_state* xfs_state) {
    char filename[FILENAME_MAX+2] = {'\0'};
    unsigned long long address = xfs_state->address;
    // выделяем память под содержимое директории
    xfs_dir2_sf_entry_t* entry = malloc(sizeof(struct xfs_dir2_sf_entry) + FILENAME_MAX);
    output_buf[0] = '\0';
    address += xfs_dir2_sf_entry_address_offset();
    fseek(xfs_state->file_pointer, address, SEEK_SET);
    // проходим по всем внутренним сущностям директории
    for (unsigned char i = 0; i < xfs_state->dinode.di_u.di_dir2sf.hdr.count; i++) {
        fread(entry, sizeof(struct xfs_dir2_sf_entry) + FILENAME_MAX, 1, xfs_state->file_pointer);
        // если это просто файл, то выводим имя, если директория то добавляем слэш
        xfs_dir2_sf_filename(entry, filename);
        strcat(output_buf, filename);
        strcat(output_buf, "\n");
        // переходим к следующей сущности
        address += xfs_dir2_sf_entry_true_size(entry);
        fseek(xfs_state->file_pointer, address, SEEK_SET);
    }
    free(entry);
}

static long find_inode(char* name, int ftype, struct xfs_state* xfs_state) {
    char filename[FILENAME_MAX+2] = {'\0'};
    unsigned long long address = xfs_state->address;
    // сущность родительского каталога
    xfs_dir2_sf_entry_t* entry = malloc(sizeof(struct xfs_dir2_sf_entry) + FILENAME_MAX);

    // ftype = 2 , то есть мы находим inode для директории
    if (ftype == 2) {
        if (strcmp(name, ".") == 0) {
            // возвращаем полный номер inode того узла, где находимся
            return xfs_state->dinode.di_core.di_ino;
        } else if (strcmp(name, "..") == 0) {
            // возвращаем номер inode родительской директории
            return xfs_dir2_sf_parent(&xfs_state->dinode.di_u.di_dir2sf.hdr);
        }
    }

    address += xfs_dir2_sf_entry_address_offset();
    fseek(xfs_state->file_pointer, address, SEEK_SET);
    // проходим по всем сущностям родительского каталога
    for (unsigned char i = 0; i < xfs_state->dinode.di_u.di_dir2sf.hdr.count; i++) {
        // читаем содержимое сущности внутри каталога
        fread(entry, sizeof(struct xfs_dir2_sf_entry) + FILENAME_MAX, 1, xfs_state->file_pointer);
        xfs_dir2_sf_filename(entry, filename);
        if (ftype == 2)
            filename[entry->namelen] = '\0';
        // если тип искомой сущности совпадает(файл или директория), а также совпадает искомое имя
        if (*xfs_dir2_sf_ftype(entry) == ftype && strcmp(filename, name) == 0) {
            // то мы возвращаем искомый inode
            long inode = xfs_dir2_sf_inumber(entry);
            free(entry);
            return inode;
        }
        // иначе продолжаем поиск
        address += xfs_dir2_sf_entry_true_size(entry);
        // перемещаем указатель на следующую сущность
        fseek(xfs_state->file_pointer, address, SEEK_SET);
    }
    free(entry);
    return -1;
}

static long find_dir_inode(char* dir, struct xfs_state* xfs_state) {
    return find_inode(dir, 2, xfs_state);
}

static long find_file_inode(char* filename, struct xfs_state* xfs_state) {
    return find_inode(filename, 1, xfs_state);
}

void read_file_data(char* to, char* filename, struct xfs_state* xfs_state) {
    struct xfs_dinode dinode;
    // сохраняем наш изначальный адрес
    unsigned long long saved_address = xfs_state->address;
    // находим inode, связанный с искомым файлом
    long finode = find_file_inode(filename, xfs_state);
    // переходим к данному inod'у
    xfs_state->address = finode * xfs_state->sb.sb_inodesize;
    read_dinode(xfs_state, &dinode);
    // возвращаемся на изначальный адрес
    xfs_state->address = saved_address;
    struct xfs_bmbt_irec bmbt;
    xfs_uint64_t l0 = bswap_64(dinode.di_u.di_bmx->l0);
    xfs_uint64_t l1 = bswap_64(dinode.di_u.di_bmx->l1);
    swap(l0, l1);
    // преобразование экстента в несжатую форму (Извлеките начальное поле из записи экстента bmap в памяти.)
    bmbt.br_state = XFS_EXT_NORM;
    bmbt.br_startoff = ((xfs_fileoff_t)l0 &
                        xfs_mask64lo(64 - BMBT_EXNTFLAG_BITLEN)) >> 9; // EXACT (no * sb inodesize) address of data
    bmbt.br_startblock = (((xfs_fsblock_t)l0 & xfs_mask64lo(9)) << 43) |
                         (((xfs_fsblock_t)l1) >> 21);
    bmbt.br_blockcount = (xfs_filblks_t)(l1 & xfs_mask64lo(21));

    if (bmbt.br_startoff == 0) {
        return;
    }
    fseek(xfs_state->file_pointer, bmbt.br_startoff, SEEK_SET);
    int buf_size = 512;
    char *buffer = malloc(sizeof(char) * buf_size);
//    strcat(to, xfs_state->path);
    strcat(to, filename);
    FILE* fp = fopen(to, "wb");
    int file_size = dinode.di_core.di_size;
    while (file_size > 0) {
        if (file_size < buf_size) {
            fread(buffer, sizeof(char), file_size, xfs_state->file_pointer);
            fwrite(buffer, sizeof(char), file_size, fp);//запись копии в новый файл
        } else {
            fread(buffer, sizeof(char), buf_size, xfs_state->file_pointer);
            fwrite(buffer, sizeof(char), buf_size, fp);//запись копии в новый файл
        }
        file_size = file_size - buf_size;
    }
    free(buffer);
    fclose(fp);
}

void dir_copy(char* output_buf, char* to, struct xfs_state* xfs_state) {
    char files_list[TMP_MAX] = {'\0'};
    char buffer[TMP_MAX] = {'\0'};
    char out_buf[PATH_MAX*2 + 20] = {'\0'};
    char buffer2[TMP_MAX] = {'\0'};
    // получаем список всех файлов и директорий в директории(перенос на каждую строку)
    xfs_readdir(files_list, xfs_state);
    char* token = strtok(files_list, "\n");
    while (token != NULL) {
        strcpy(buffer2, token);
        // текущая сущность - директория
        if (buffer2[strlen(buffer2) - 1] == '/') {
            //переходим в директорию
            xfs_cd(buffer, buffer2, xfs_state);
            strcat(to, buffer2);
            // создаем директорию ACCESSPERMS: Equivalent of 777 = rwxrwxrwx
            mkdir(to, ACCESSPERMS);
            token += strlen(token) + 1;
            // рекурсивно вызываем этот же метод
            dir_copy(output_buf, to, xfs_state);
            // возвращаемся обратно в родительскую директорию
            xfs_cd(buffer, "..", xfs_state);
            split_path(to, to);
            buffer[0] = '\0';
            token = strtok(token, "\n");
        } else {
            // текущая сущность - файл
            read_file_data(to ,buffer2, xfs_state);
            snprintf(out_buf, PATH_MAX*2 + 20, "Извлечение %s%s в %s\n", xfs_state->path, buffer2, to);
            // убираем скопированный файл
            token = strtok(NULL, "\n");
//            if (token == NULL) {
                split_path(to, NULL);
//            }
            strcat(output_buf, out_buf);
            buffer[0] = '\0';
            // выделение очередной части строки
//            token = strtok(NULL, "\n");
        }
    }
}

void xfs_ls(char* output_buf, struct xfs_state* xfs_state) {
    xfs_readdir(output_buf, xfs_state);
}

void xfs_copy(char* output_buf, char* from, char* to, struct xfs_state* xfs_state) {
    char user_path[PATH_MAX];

    strcpy(user_path, xfs_state->path);
    // переходим в папку, откуда будем осуществлять копирование
    if (xfs_cd(output_buf, from, xfs_state) == 0) {
        read_file_data(to ,from, xfs_state);
        snprintf(output_buf, PATH_MAX*2 + 20, "Извлечение %s%s в %s\n", xfs_state->path, from, to);
    };
    // если была найдена неизвестная директория
    if (output_buf[0] != '\0') {
        return;
    }
    // создаем директорию, которую копируем
    mkdir(strcat(to, xfs_state->path) , ACCESSPERMS);
    dir_copy(output_buf, to, xfs_state);
    // возвращаемся в ту директорию, где и начинали операцию копирования
    xfs_cd(output_buf, user_path, xfs_state);
}

void xfs_pwd(char* output_buf, struct xfs_state* xfs_state) {
    // получаем путь, записанный в поле path
    strcpy(output_buf, xfs_state->path);
    strcat(output_buf, "\n");
}

int xfs_cd(char* output_buf, char* path, struct xfs_state* xfs_state) {
    struct xfs_dinode saved_dinode = xfs_state->dinode;
    unsigned long long saved_address = xfs_state->address;
    unsigned long long inode;
    char* tempstr = calloc(strlen(path) + 1, sizeof(char));
    strcpy(tempstr, path);
    char* token = strtok(tempstr, "/");
    // если мы стартуем от корня
    if (path[0] == '/') {
        // очищаем путь и инициализируем root dinode
        xfs_state->path[0] = '/';
        xfs_state->path[1] = '\0';
        // получаем адрес корневого индекса фс
        xfs_state->address = xfs_state->sb.sb_rootino * xfs_state->sb.sb_inodesize;
        // производим чтение первого индексного дескриптора
        read_dinode(xfs_state, &xfs_state->dinode);
    }
    // если помимо корня в пути присутствуют еще директории
    while (token != NULL) {
        // получаем id  директории в пути
        inode = find_dir_inode(token, xfs_state);
        long fileInode = find_file_inode(token, xfs_state);
        // если директория оказалась файлом
        if (fileInode != -1) {
//            xfs_state->address = inode * xfs_state->sb.sb_inodesize;
//            // читаем информацию о полученном inode, перемещая указатель
//            read_dinode(xfs_state, &xfs_state->dinode);
            return 0;
        }
        // если такой директории не существует
        if (inode == -1) {
            // восстанавлием состояние полей dinode и address
            xfs_state->dinode = saved_dinode;
            xfs_state->address = saved_address;
            strcpy(output_buf, "Неизвестная директория\n");
            free(tempstr);
            return -1;
        }
        xfs_state->address = inode * xfs_state->sb.sb_inodesize;
        // читаем информацию о полученном inode, перемещая указатель
        read_dinode(xfs_state, &xfs_state->dinode);
        // Выделение очередной части строки
        token = strtok(NULL, "/");
    }
    // в поле path записываем значение пути из аргумента команды cd
    construct_path(path, xfs_state->path);
    free(tempstr);
    return 1;
}

void execute_help(char* output_buf) {
    struct operation* operations = get_operations();
    size_t operations_size = get_operations_size();
    output_buf[0] = '\0';
    for (size_t i = 0; i < operations_size; i++) {
        strcat(output_buf, operations[i].name);
        strcat(output_buf, "\t");
        strcat(output_buf, operations[i].description);
        strcat(output_buf, "\n");
    }
}