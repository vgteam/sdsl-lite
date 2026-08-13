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

# PUBLIC: sdsl-lite's headers are templates, so consumer code needs these
# same defines or it'll compile different template bodies than libsdsl.
if(NOT SDSL_USE_EXCEPTIONS)
    target_compile_definitions(sdsl PUBLIC SDSL_NO_EXCEPTIONS)
    if(SDSL_USE_ABSEIL_LOGGING)
        target_compile_definitions(sdsl PUBLIC SDSL_USE_ABSEIL_LOGGING)
        target_link_libraries(sdsl PUBLIC absl::log absl::absl_log)
    endif()
endif()

if(SDSL_ENABLE_SHARED_MEMORY)
    target_compile_definitions(sdsl PUBLIC SDSL_ENABLE_SHARED_MEMORY)
endif()
