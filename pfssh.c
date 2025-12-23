#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "include/pfs.h"
#include "include/pfssh.h"


static char *shell_cache, *input, *disk_path, *token_arr[MAX_ARGS];


int main(int argc, char* argv[]) {

    if (argc != 2) {
        fprintf(stderr, "pfs requires only one argument, disk_path\n");
        exit(EXIT_FAILURE);
    }

    disk_path = argv[1];

    if (disk_path == NULL) {
        fprintf(stderr, "pfs requires disk_path\n");
        return EXIT_FAILURE;
    }

    shell_cache = calloc(PFSSH_CACHE_NMEMBERS, sizeof(char));

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

        int no_args = tokenize_input(input);

        if (strcmp(*token_arr, "format") == 0) {
            do_format(no_args, disk_path);
        }
        else if (strcmp(*token_arr, "mount") == 0) {
            do_mount(no_args);
        }
        else if (strcmp(*token_arr, "create") == 0) {
            do_create(no_args);
        }
        else if (strcmp(*token_arr, "remove") == 0) {
            do_remove(no_args, (int) strtol(*(token_arr + (no_args - 1)), NULL, 10));
        }
        else if (strcmp(*token_arr, "cat") == 0) {
            do_cat(no_args, (int) strtol(*(token_arr + (no_args - 1)), NULL, 10));
        }
        else if (strcmp(*token_arr, "echo") == 0) {
            do_echo(no_args, (int) strtol(*(token_arr + (no_args - 1)), NULL, 10));
        }
        else if (strcmp(*token_arr, "echoa") == 0) {
            do_echo_append(no_args, (int) strtol(*(token_arr + (no_args - 1)), NULL, 10));
        }
        else if (strcmp(*token_arr, "cp") == 0) {
            if (no_args != 3) {
                printf("Usage: cp <inode> > <inode>\n");
                continue;
            }

            do_copy((int) strtol(*(token_arr + (no_args - 2)), NULL, 10), (int) strtol(*(token_arr + (no_args - 1)), NULL, 10));
        }
        else if (strcmp(*token_arr, "stat") == 0) {
            do_stat(no_args, (int) strtol(*(token_arr + (no_args - 1)), NULL, 10));
        }
        else if (strcmp(*token_arr, "help") == 0) {
            do_help();
        }
        else {
            printf("Unknown command %s\nType help to see supported commands\n", *token_arr);
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
    ssize_t offset = 0, bytes_read, bytes_written;

    if (args != 2) {
        printf("Usage: cat <inode>\n");
        return;
    }

    while (1) {

        if ((bytes_read = fs_read(inode, shell_cache, CAT_BUFSIZ, offset)) <= 0){
            break;
        }

        if ((bytes_written = (ssize_t) fwrite(shell_cache, sizeof(char), bytes_read, stdout)) != bytes_read) {
            printf("\nError: Could not write all bytes to stdout for some reason\n");
            break;
        }

        offset += bytes_written;
    }

    printf("\n");

    memset(shell_cache, 0 , PFSSH_CACHE_NMEMBERS);
}

void do_echo(int args, int inode) {
    if (args > MAX_ARGS - 1) {
        printf("Too many arguments specified for echo try reducing length of text\n");
        return;
    }

    if (args < 3) {
        printf("Usage: echo <text> <inode>\n");
        return;
    }

    for (int i = 1; i <= args - 2; i++) {
        strncat(shell_cache, *(token_arr + i), PFSSH_CACHE_NMEMBERS - strlen(shell_cache) - 1);

        strncat(shell_cache, " ", PFSSH_CACHE_NMEMBERS - strlen(shell_cache) - 1);

    }

    fs_write(inode, shell_cache, (int) strlen(shell_cache), 0);

    memset(shell_cache, 0, PFSSH_CACHE_NMEMBERS);

}

void do_echo_append(int args, int inode) {
    if (args > MAX_ARGS - 1) {
        printf("Too many arguments specified for echo try reducing length of text\n");
        return;
    }

    if (args < 3) {
        printf("Usage: echoa <text> <inode>\n");
        return;
    }

    for (int i = 1; i <= args - 2; i++) {
        strncat(shell_cache, *(token_arr + i), PFSSH_CACHE_NMEMBERS - strlen(shell_cache) - 1);

        strncat(shell_cache, " ", PFSSH_CACHE_NMEMBERS - strlen(shell_cache) - 1);

    }

    fs_write(inode, shell_cache, (int) strlen(shell_cache), fs_stat(inode));

    memset(shell_cache, 0, PFSSH_CACHE_NMEMBERS);

}

void do_copy(int src, int dst) {
    ssize_t offset = 0, bytes_read, bytes_written;

    while (1) {
        if ((bytes_read = fs_read(src, shell_cache, CP_BUFSIZ, offset)) <= 0)
            break;
        if ((bytes_written = fs_write(dst, shell_cache, (int) bytes_read, offset)) <= 0)
            break;

        offset += bytes_written;
    }

    memset(shell_cache, 0 , PFSSH_CACHE_NMEMBERS);

    printf("Wrote %ld bytes to inode %d\n", offset, dst);
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
    printf("    cp   <inode> <inode>\n");
    printf("    stat   <inode>\n");
    printf("    copyin   <file> <inode>\n");
    printf("    copyout   <inode> <file>\n");
    printf("    help\n");
    printf("    quit\n");
    printf("    exit\n");
}

void do_exit() {
    printf("Exiting file system.....\n");
    free(input);
    free(shell_cache);
}

int tokenize_input(char* input) {
    int argc = 0;

    char* token = strtok(input, " \t\n");

    while (token && argc < MAX_ARGS - 1) {
        token_arr[argc++] = token;
        token = strtok(NULL, " \t\n");
    }

    token_arr[argc] = NULL;

    return argc;
}