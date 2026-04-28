
## Add gtest
set(gtest_dir ${CMAKE_CURRENT_LIST_DIR}/googletest)
set(gtest_file ${gtest_dir}/CMakeLists.txt)
if(NOT EXISTS ${gtest_file})
    EXECUTE_PROCESS(COMMAND git submodule update --init external/googletest
                    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/..
                    OUTPUT_QUIET
            )
endif(NOT EXISTS ${gtest_file})

if(WITH_GOOGLETEST)
option(BUILD_GMOCK OFF)
option(INSTALL_GTEST OFF)
add_subdirectory(googletest)
else(WITH_GOOGLETEST)
add_subdirectory(googletest EXCLUDE_FROM_ALL)
endif(WITH_GOOGLETEST)
