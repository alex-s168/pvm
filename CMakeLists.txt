cmake_minimum_required(VERSION 3.28)
project(pvm C)

set(CMAKE_C_STANDARD 11)

add_executable(pvm main.c
        vm.h
        gc.c
        types.c
        arrays.c
        class.h
        interpr.c
        strop.c
        gen.h
        gen.c)
