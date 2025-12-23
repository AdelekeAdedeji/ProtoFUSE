#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "include/pfs.h"


static char *shell_cache, *input, *disk_path;


void do_format(int args, char* disk_path);
void do_mount(int args);
void do_create(int args);
void do_remove(int args, int inode);
void do_cat(int args, int inode);
void do_echo(int args, char* input, int inode);
void do_echoa(int args, char* input, int inode);
void do_cpy(int args, int src, int dst);
void do_copyin();
void do_copyout(int args, int src, int dst);
void do_stat(int args, int inode);
void do_help();
void do_exit();



int main(int argc, char* argv[]) {
    char cmd[512], arg1[512], arg2[512];

    if (argc > 2) {
        fprintf(stderr, "pfs requires only one argument, disk_path\n");
        exit(EXIT_FAILURE);
    }

    disk_path = argv[1];

    if (disk_path == NULL) {
        fprintf(stderr, "pfs requires disk_path\n");
        return EXIT_FAILURE;
    }

    // fs_format(disk_path);
    // fs_mount();
    // fs_create();
    // fs_create();
    // fs_write(0, "hello world how are you doing\n", 31, 0);
    // fs_create();


    printf("%s\n", disk_path);

    shell_cache = calloc(1024, sizeof(char));

    while (1) {
        input = readline("pfs> ");

        if (input == NULL || strcmp(input, "exit") == 0 || strcmp(input, "quit") == 0) {
            fs_destroy();
            do_exit();
            break;
        }

        if (strcmp(input, "") == 0) {
            free(input);
            continue;
        }

        int no_args = sscanf(input, "%s %s > %s", cmd, arg1, arg2);

        if (strcmp(cmd, "format") == 0) {
            do_format(no_args, disk_path);
        }
        else if (strcmp(cmd, "mount") == 0) {
            do_mount(no_args);
        }
        else if (strcmp(cmd, "create") == 0) {
            do_create(no_args);
        }
        else if (strcmp(cmd, "remove") == 0) {
            do_remove(no_args, (int) strtol(arg1, NULL, 10));
        }
        else if (strcmp(cmd, "cat") == 0) {
            do_cat(no_args, (int) strtol(arg1, NULL, 10));
        }
        else if (strcmp(cmd, "echo") == 0) {
            do_echo(no_args, arg1, (int) strtol(arg2, NULL, 10));
        }
        else if (strcmp(cmd, "echoa") == 0) {
            do_echoa(no_args, arg1, (int) strtol(arg2, NULL, 10));
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
            do_stat(no_args, strtol(arg1, NULL, 10));
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


void do_format(int args, char* disk_path) {
    if (args != 1) {
        printf("Usage: format\n");
        return;
    }

    if (!fs_format(disk_path)) return;

    printf("File system successfully formatted\n");

}
void do_mount(int args) {
    if (args != 1) {
        printf("Usage: mount\n");
        return;
    }

    if (!fs_mount()) return;

    printf("File system successfully registered\n");
}

void do_create(int args) {
    if (args != 1) {
        printf("Usage: create\n");
        return;
    }

    fs_create();
}

void do_remove(int args, int inode) {
    if (args != 2) {
        printf("Usage: remove <inode>\n");
        return;
    }

    fs_remove(inode);
}

void do_cat(int args, int inode) {
    ssize_t offset = 0, bytes_read, bytes_written, size;
    if (args != 2) {
        printf("Usage: cat <inode>\n");
        return;
    }
    while (1) {
        if ((size = fs_stat(inode)) == -1) break;
        if ((bytes_read = fs_read(inode, shell_cache, (int) size, offset)) <= 0){
            break;
        }

        if ((bytes_written = (ssize_t) fwrite(shell_cache, sizeof(char), bytes_read, stdout)) != bytes_read) {
            printf("\nError: Could not write all bytes to stdout for some reason\n");
            break;
        }

        offset += bytes_written;
    }

    printf("\n");
}

void do_echo(int args, char* input, int inode) {
    if (args != 3) {
        printf("Usage: echo <text> <inode>\n");
        return;
    }

    fs_write(inode, input, (int) strlen(input), 0);
}

void do_echoa(int args, char* input, int inode) {
    if (args != 3) {
        printf("Usage: echoa <text> <inode>\n");
        return;
    }

    fs_write(inode, input, (int) strlen(input), fs_stat(inode));
}

void do_cpy(int args, int src, int dst) {
    ssize_t offset = 0, bytes_read, bytes_written, size;

    if (args != 3) {
        printf("Usage: cpy <inode> > <inode>\n");
        return;
    }

    while (1) {
        if ((size = fs_stat(src)) == -1) break;
        if ((bytes_read = fs_read(src, shell_cache, (int) size, offset)) <= 0)
            break;
        if ((bytes_written = fs_write(dst, shell_cache, (int) bytes_read, offset)) <= 0)
            break;

        offset += bytes_written;
    }

    printf("Wrote %ld bytes to inode %d\n", offset, dst);
}

void do_copyin() {

}

void do_copyout(int args, int src, int dst) {
    if (args != 3) {
        printf("Usage: copyout <inode> <inode/file>\n");
        return;
    }
    // copyout(src, dst);
}

void do_stat(int args, int inode) {
    ssize_t size = 0;
    if (args != 2) {
        printf("Usage: stat <inode>\n");
        return;
    }

    size = fs_stat(inode);

    if (size == -1) return;

    printf("inode %d is %ld bytes\n", inode, size);
}

void do_help() {
    printf("Commands are:\n");
    printf("    format\n");
    printf("    mount\n");
    printf("    debug\n");
    printf("    create\n");
    printf("    remove   <inode>\n");
    printf("    cat   <inode>\n");
    printf("    echo   <text> <inode>\n");
    printf("    echoa   <text> <inode>\n");
    printf("    cpy   <inode> <inode>\n");
    printf("    stat   <inode>\n");
    printf("    copyin   <file> <inode>\n");
    printf("    copyout   <inode> <file>\n");
    printf("    help\n");
    printf("    quit\n");
    printf("    exit\n");
}

void do_exit() {
    printf("Bye Bye\n");
    free(input);
    free(shell_cache);
}

