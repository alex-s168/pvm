cmake_minimum_required(VERSION 3.28)
project(pvm C)

set(CMAKE_C_STANDARD 11)

add_executable(pvm
        kollektions/alloc/libc.c
        kollektions/dynamic_list/add_and_addAll.c
        kollektions/dynamic_list/init_and_clear.c
        kollektions/dynamic_list/insertAt_and_insertAllAt.c
        kollektions/dynamic_list/removeAt_and_removeRange.c
        kollektions/dynamic_list/reserve_and_shrink.c
        kollektions/fixed_list/get_and_set.c
        kollektions/fixed_list/indexOf.c
        kollektions/lists/copy.c
        kollektions/static_list/add_and_addAll.c
        kollektions/static_list/init_and_clear.c
        kollektions/static_list/insertAt_and_insertAllAt.c
        kollektions/static_list/removeAt_and_removeRange.c
        kollektions/linked_list/add.c
        kollektions/linked_list/addAll.c
        kollektions/linked_list/clear.c
        kollektions/linked_list/find.c
        kollektions/linked_list/findLast.c
        kollektions/linked_list/flatten.c
        kollektions/linked_list/fromFixed.c
        kollektions/linked_list/fromLinks.c
        kollektions/linked_list/linkAt.c
        kollektions/linked_list/remove.c
        kollektions/linked_list/removeMultiple.c
        kollektions/blocking_list/access.c
        kollektions/blocking_list/init_and_destroy.c

        main.c
        vm.h
        gc.c
        types.c
        arrays.c
        class.h
        interpr.c
        strop.c
        gen.h
        gen.c
        kollektions/alloc/statistic.c)
