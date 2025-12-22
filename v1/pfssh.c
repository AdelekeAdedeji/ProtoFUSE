#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "include/pfs.h"


char *shell_cache, *input, *disk_path;

void do_copyout(int args, int src, int dst);
void do_exit();
void do_cpy(int args, int src, int dst);

int main(int argc, char* argv[]) {
    char cmd[512], arg1[512], arg2[512], buff[780];

    if (argc > 2) {
        fprintf(stderr, "pfs requires only one argument, disk_path\n");
        exit(EXIT_FAILURE);
    }

    disk_path = argv[1];

    if (disk_path == NULL) {
        fprintf(stderr, "pfs requires disk_path\n");
        return EXIT_FAILURE;
    }

    fs_format(disk_path);
    fs_mount();
    fs_create();
    fs_create();
    fs_write(0, "hello world how are you doing\n", 31, 0);
    // fs_create();


    printf("%s\n", disk_path);

    shell_cache = calloc(1024, sizeof(char));

    while (1) {
        input = readline("pfs> ");

        if (input == NULL || strcmp(input, "exit") == 0) {
            fs_destroy();
            do_exit();
            break;
        }

        if (strcmp(input, "") == 0) {
            free(input);
            continue;
        }

        int no_args = sscanf(input, "%s %s %s", cmd, arg1, arg2);

        if (strcmp(cmd, "format") == 0) {
            do_format();
        }
        else if (strcmp(cmd, "mount") == 0) {
            do_mount();
        }
        else if (strcmp(cmd, "create") == 0) {
            do_create();
        }
        else if (strcmp(cmd, "remove") == 0) {
            do_remove();
        }
        else if (strcmp(cmd, "cat") == 0) {
            do_cat();
        }
        else if (strcmp(cmd, "cpy") == 0) {
            do_cpy(no_args, (int) strtol(arg1, NULL, 10), (int) strtol(arg2, NULL, 10));
        }
        else if (strcmp(cmd, "copyin") == 0) {
            do_copyin(no_args, atoi(arg1), atoi(arg2));
        }
        else if (strcmp(cmd, "copyout") == 0) {
            do_copyout(no_args, atoi(arg1), atoi(arg2));
        }
        else if (strcmp(cmd, "stat") == 0) {
            do_stat();
        }
        else if (strcmp(cmd, "help") == 0) {
            do_help();
        }
        else {
            printf("Unknown command %s\nType help to see supported commands\n", cmd);
        }


        free(input);
    }

    return EXIT_SUCCESS;

}


void do_copyout(int args, int src, int dst) {
    if (args != 3) {
        printf("Usage: copyout <inode> <inode/file>\n");
        return;
    }


    // copyout(src, dst);
}


void do_cpy(int args, int src, int dst) {
    ssize_t offset = 0, bytes_read, bytes_written;

    if (args != 3) {
        printf("Usage: cpy <inode> <inode>");
        return;
    }

    while (1) {
        if ((bytes_read = fs_read(src, shell_cache, 31, offset)) <= 0)
            break;
        if ((bytes_written = fs_write(dst, shell_cache, (int) bytes_read, offset)) <= 0)
            break;

        offset += bytes_written;
    }

    printf("Wrote %ld bytes to inode %d", offset, dst);
}


void do_exit() {
    printf("Bye Bye\n");
    free(input);
    free(shell_cache);
}

