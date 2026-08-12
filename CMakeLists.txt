cmake_minimum_required(VERSION 3.13)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules")
include(AppendCompilerFlags)

## Project information ##
project(sdsl CXX C)
set(PROJECT_VENDOR "Jouni Siren")
set(PROJECT_CONTACT "jouni.siren@iki.fi")
set(PROJECT_URL "https://github.com/vgteam/sdsl-lite")
set(PROJECT_DESCRIPTION "SDSL: Succinct Data Structure Library (vgteam fork)")
set(CMAKE_BUILD_TYPE "Release")

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" PROJECT_VERSION_FULL)
string(REGEX REPLACE "[\n\r]" "" PROJECT_VERSION_FULL "${PROJECT_VERSION_FULL}")
string(REGEX REPLACE "^([0-9]+)\\.[0-9]+\\.[0-9]+$" "\\1" PROJECT_VERSION_MAJOR "${PROJECT_VERSION_FULL}")
string(REGEX REPLACE "^[0-9]+\\.([0-9]+)\\.[0-9]+$" "\\1" PROJECT_VERSION_MINOR "${PROJECT_VERSION_FULL}")
string(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.([0-9]+)$" "\\1" PROJECT_VERSION_PATCH "${PROJECT_VERSION_FULL}")
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")
math(EXPR LIBRARY_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(LIBRARY_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(LIBRARY_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(LIBRARY_VERSION "${LIBRARY_VERSION_MAJOR}.${LIBRARY_VERSION_MINOR}")
set(LIBRARY_VERSION_FULL "${LIBRARY_VERSION}.${LIBRARY_VERSION_PATCH}")


option(CODE_COVERAGE "Set ON to add code coverage compile options" OFF)
option(GENERATE_DOC "Set ON to generate doxygen API reference in build/doc directory" OFF)
option(WITH_GOOGLETEST "Set ON to install google test along with the library" ON)

# By default, sdsl-lite reports fatal errors by throwing C++ exceptions, as
# it always has. Some embedders (e.g. Google's DeepVariant, which builds
# with Bazel and -fno-exceptions) need a build that never throws instead.
option(SDSL_USE_EXCEPTIONS "Report fatal errors by throwing C++ exceptions" ON)
# When exceptions are off, fatal errors are reported through Abseil's
# ABSL_LOG(FATAL) if this is on, or through stderr + std::abort() if it is
# off. It has no effect when SDSL_USE_EXCEPTIONS is on.
option(SDSL_USE_ABSEIL_LOGGING "Use Abseil logging to report fatal errors when exceptions are off" OFF)

if(NOT SDSL_USE_EXCEPTIONS)
    message("sdsl-lite: building without C++ exceptions")
    if(SDSL_USE_ABSEIL_LOGGING)
        find_package(absl REQUIRED)
        message("sdsl-lite: fatal errors reported via Abseil ABSL_LOG(FATAL)")
    else()
        message("sdsl-lite: fatal errors reported via stderr and std::abort()")
    endif()
endif()

# int_vector normally always allocates and owns its own memory. Turning this
# on adds an additional int_vector constructor (see include/sdsl/int_vector.hpp)
# that instead wraps a caller-supplied uint64_t buffer -- e.g. one obtained
# from a Boost.Interprocess shared memory segment -- without copying or
# taking ownership of it, plus the sdsl::shared_memory_int_vector helpers
# (see include/sdsl/shared_memory_int_vector.hpp) that place an int_vector's
# bits into such a segment under a name a separate process can attach to.
# Off by default because it changes int_vector's memory layout (an extra
# bool member) and requires Boost; on, everything about int_vector's default
# (owning) behavior is unchanged.
option(SDSL_ENABLE_SHARED_MEMORY "Enable int_vector's shared-memory-wrapping constructor and the shared_memory_int_vector helpers, backed by Boost.Interprocess" OFF)
if(SDSL_ENABLE_SHARED_MEMORY)
    find_package(Boost REQUIRED)
    message("sdsl-lite: Boost found, shared-memory int_vector support enabled")
endif()

# C++17 compiler Check
if(CMAKE_CXX_COMPILER MATCHES ".*clang.*" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
      set(CMAKE_COMPILER_IS_CLANGXX 1)
endif()
if(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    set(CMAKE_COMPILER_IS_INTEL 1)
endif()
if( (CMAKE_COMPILER_IS_GNUCXX AND ${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS 7) OR
    (CMAKE_COMPILER_IS_CLANGXX AND ${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS 5))
  message(FATAL_ERROR "Your C++ compiler does not support C++17. Please install g++ 7 (or greater) or clang 5 (or greater)")
else()
  message(STATUS "Compiler is recent enough to support C++17.")
endif()

if( CMAKE_COMPILER_IS_GNUCXX )
    append_cxx_compiler_flags("-std=c++17 -Wall -Wextra -DNDEBUG" "GCC" CMAKE_CXX_FLAGS)
    append_cxx_compiler_flags("-O3 -ffast-math -funroll-loops" "GCC" CMAKE_CXX_OPT_FLAGS)
    if ( CODE_COVERAGE )
        append_cxx_compiler_flags("-g -fprofile-arcs -ftest-coverage -lgcov" "GCC" CMAKE_CXX_FLAGS)
    endif()

else()
    if( CMAKE_COMPILER_IS_CLANGXX )
		append_cxx_compiler_flags("-std=c++17 -DNDEBUG" "CLANG" CMAKE_CXX_FLAGS)
		append_cxx_compiler_flags("-stdlib=libc++" "CLANG" CMAKE_CXX_FLAGS)
		append_cxx_compiler_flags("-O3 -ffast-math -funroll-loops -D__extern_always_inline=\"extern __always_inline\" " "CLANG" CMAKE_CXX_OPT_FLAGS)
	else()
        if( CMAKE_COMPILER_IS_INTEL )
            append_cxx_compiler_flags("-std=c++17 -Wall -Wextra -DNDEBUG" "INTEL" CMAKE_CXX_FLAGS)
            append_cxx_compiler_flags("-O3 -ffast-math -funroll-loops -no-inline-min-size -no-inline-max-size" "INTEL" CMAKE_CXX_OPT_FLAGS)
        else()
            append_cxx_compiler_flags("/EHsc" "MSVC" CMAKE_CXX_FLAGS)
            append_cxx_compiler_flags("/Od" "MSVC" CMAKE_CXX_FLAGS_DEBUG)
            append_cxx_compiler_flags("/Ox" "MSVC" CMAKE_CXX_FLAGS_RELEASE)
            set(vars CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
            foreach(var ${vars})
                string(REPLACE "/MD" "-MT" ${var} "${${var}}")
            endforeach(var)

            add_definitions("/DMSVC_COMPILER")
        endif()
	endif()
endif()

try_compile(MODE_TI "${PROJECT_BINARY_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/check_mode_ti.cpp")
if( MODE_TI )
  if(MSVC)
    add_definitions("/DMODE_TI")
  else()
    add_definitions("-DMODE_TI")
  endif()
endif()

include(CheckSSE4_2)
if( BUILTIN_POPCNT )
  if( CMAKE_COMPILER_IS_GNUCXX )
    append_cxx_compiler_flags("-msse4.2 -march=native" "GCC" CMAKE_CXX_OPT_FLAGS)
  endif()
  if( CMAKE_COMPILER_IS_GNUCXX )
    append_cxx_compiler_flags("-msse4.2 -march=native" "CLANG" CMAKE_CXX_OPT_FLAGS)
  endif()
  if( CMAKE_COMPILER_IS_INTEL )
    append_cxx_compiler_flags("-msse4.2 -march=native" "INTEL" CMAKE_CXX_FLAGS)
  endif()
endif()

# check for demangle support to get pretty C++ class names
include(FindCxaDemangle)
if( HAVE_CXA_DEMANGLE )
    message(STATUS "Compiler supports DEMANGLE.")
    if( CMAKE_COMPILER_IS_GNUCXX )
        append_cxx_compiler_flags("-DHAVE_CXA_DEMANGLE" "GCC" CMAKE_CXX_OPT_FLAGS)
    else()
        append_cxx_compiler_flags("-DHAVE_CXA_DEMANGLE" "CLANG" CMAKE_CXX_OPT_FLAGS)
    endif()
endif()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Make.helper.cmake"
		       "${CMAKE_CURRENT_SOURCE_DIR}/Make.helper" @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/lib/structure_tree.cpp.cmake"
               "${CMAKE_CURRENT_SOURCE_DIR}/lib/structure_tree.cpp" @ONLY)

find_package(Git)
if(GIT_FOUND)
    message("git found: ${GIT_EXECUTABLE}")
else()
    message(WARNING "git not found. Cloning of submodules will not work.")
endif()

add_subdirectory(external)
add_subdirectory(include)
add_subdirectory(lib)

## Add doxygen documentation generation target ##
find_package(Doxygen)
if(GENERATE_DOC AND DOXYGEN_FOUND)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/extras/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
add_custom_target(docs ALL
${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
COMMENT "Generating API documentation with Doxygen" VERBATIM
)
endif(GENERATE_DOC AND DOXYGEN_FOUND)

## Add 'uninstall' target ##
CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/CMakeModules/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)
ADD_CUSTOM_TARGET(uninstall-sdsl
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/CMakeModules/cmake_uninstall.cmake")

target_include_directories(sdsl PUBLIC include)

# pkg-config consumers don't see the sdsl CMake target's PUBLIC compile
# definitions (see lib/CMakeLists.txt), so -- same reasoning as there, since
# sdsl-lite's headers are what actually read these defines -- Cflags needs
# to carry them explicitly, or a pkg-config consumer built without
# redeclaring these flags by hand would silently disagree with how libsdsl
# itself was compiled.
set(SDSL_PC_CFLAGS_EXTRA "")
if(NOT SDSL_USE_EXCEPTIONS)
    string(APPEND SDSL_PC_CFLAGS_EXTRA " -DSDSL_NO_EXCEPTIONS")
    if(SDSL_USE_ABSEIL_LOGGING)
        string(APPEND SDSL_PC_CFLAGS_EXTRA " -DSDSL_USE_ABSEIL_LOGGING")
    endif()
endif()
if(SDSL_ENABLE_SHARED_MEMORY)
    string(APPEND SDSL_PC_CFLAGS_EXTRA " -DSDSL_ENABLE_SHARED_MEMORY")
endif()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/sdsl-lite.pc.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/sdsl-lite.pc" @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/sdsl-lite.pc"
        DESTINATION "lib/pkgconfig")

add_subdirectory(test)
