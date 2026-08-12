include_directories(#"${CMAKE_CURRENT_SOURCE_DIR}/../include"
        "${CMAKE_CURRENT_BINARY_DIR}/../include"
        "${CMAKE_CURRENT_BINARY_DIR}/../external/libdivsufsort/include"
        )


file(GLOB libFiles RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}  "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp") # select all .cpp-files
if(MSVC)
    file(GLOB headerFiles RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}  "${CMAKE_CURRENT_SOURCE_DIR}/../include/sdsl/*.hpp") # select all .hpp-files
endif()

set( sdsl_SRCS ${libFiles} ${headerFiles})

add_library( sdsl ${sdsl_SRCS} )

install(TARGETS sdsl
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
        
math(EXPR SOVERSION "${LIBRARY_VERSION_MAJOR}+1")

set_target_properties(sdsl PROPERTIES
        VERSION "${LIBRARY_VERSION_FULL}"
        SOVERSION "${SOVERSION}"
        #	DEFINE_SYMBOL SDSL_BUILD_DLL
        #	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../examples"
)

# sdsl-lite is almost entirely header-only (int_vector.hpp and friends are
# templates), so any translation unit that includes these headers -- whether
# it's one of this library's own .cpp files or downstream consumer code --
# must see the same SDSL_NO_EXCEPTIONS/SDSL_USE_ABSEIL_LOGGING/
# SDSL_ENABLE_SHARED_MEMORY definitions, or different translation units end
# up compiling different bodies for the same template instantiations. Using
# PUBLIC here (rather than PRIVATE) makes these definitions a usage
# requirement that CMake propagates to anything that does
# target_link_libraries(... sdsl), so consumers don't have to redefine them
# by hand to stay consistent with how libsdsl itself was built.
if(NOT SDSL_USE_EXCEPTIONS)
    target_compile_definitions(sdsl PUBLIC SDSL_NO_EXCEPTIONS)
    if(SDSL_USE_ABSEIL_LOGGING)
        target_compile_definitions(sdsl PUBLIC SDSL_USE_ABSEIL_LOGGING)
        target_link_libraries(sdsl PUBLIC absl::log absl::absl_log)
    endif()
endif()

if(SDSL_ENABLE_SHARED_MEMORY)
    target_compile_definitions(sdsl PUBLIC SDSL_ENABLE_SHARED_MEMORY)
    target_link_libraries(sdsl PUBLIC Boost::boost)
    # Boost.Interprocess is header-only, but its named shared memory objects
    # need librt on Linux (not on macOS, where the equivalent is in libc).
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        target_link_libraries(sdsl PUBLIC rt)
    endif()
endif()
