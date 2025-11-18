add_subdirectory(iop)

add_cmocka_test(test_sample
                SOURCES test_sample.c
                LINK_LIBRARIES cmocka)

# Windows: main-method requires the wrapper provided by lib-darktable
if(WIN32)
    target_link_libraries(test_sample PRIVATE lib_darktable)
    _copy_required_library(test_sample lib_darktable)
endif(WIN32)

add_cmocka_test(test_memory_safety
                SOURCES test_memory_safety.c
                LINK_LIBRARIES cmocka)

# Windows: main-method requires the wrapper provided by lib-darktable
if(WIN32)
    target_link_libraries(test_memory_safety PRIVATE lib_darktable)
    _copy_required_library(test_memory_safety lib_darktable)
endif(WIN32)

add_cmocka_test(test_path_handling
                SOURCES test_path_handling.c
                LINK_LIBRARIES cmocka)

# Windows: main-method requires the wrapper provided by lib-darktable
if(WIN32)
    target_link_libraries(test_path_handling PRIVATE lib_darktable)
    _copy_required_library(test_path_handling lib_darktable)
endif(WIN32)
