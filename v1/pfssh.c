#include "include/pfs.h"

int main() {
    char data_buff[5096] = {0};

    fs_format();

    if (!fs_mount()) {
        printf("Mounting failed\n");
        return -1;
    }

    fs_create();

    fs_create();

    fs_create();

    fs_remove(0);

    fs_remove(1);

    // fs_remove(2);

    fs_read(2, data_buff, 4096, 5);

    data_buff[3999] = '\0';

    printf("%s\n", data_buff);

    return 0;
}

// ed fe 00 10 9a 01 00 cd  00 00 9b 01 9c 01 9d 01