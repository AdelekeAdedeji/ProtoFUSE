#pragma once

#define MAX_ARGS                        1024
#define PFSSH_CACHE_NMEMBERS            1024
#define CAT_BUFSIZ                      1024
#define CP_BUFSIZ                      1024

void do_format(int args, char* disk_path);
void do_mount(int args);
void do_create(int args);
void do_remove(int args, int inode);
void do_cat(int args, int inode);
void do_echo(int args, int inode);
void do_echo_append(int args, int inode);
void do_copy(int src, int dst);
void do_stat(int args, int inode);
void do_help();
void do_exit();
int tokenize_input(char* input);