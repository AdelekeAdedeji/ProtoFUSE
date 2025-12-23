cmake_minimum_required(VERSION 3.31.6)

project("filesystem")

add_executable(pfs pfssh.c disk_library/disk_emulator.c fs/pfs.c)

target_link_libraries(pfs readline)