cmake_minimum_required(VERSION 3.8...3.29)

project(CMakeLists-Template-Project LANGUAGES CXX)

# This is a composition of my research on writing a cross-platform CMakeLists which uses proper debugging information and
# does its best to support linkage with the AddressSanitizer library. I intend to use this in all of my important projects 
# and will iteratively refactor and build on top of it as I see need. If you notice any issues with the template, please
# feel free to open an issue and give your diagnostics and feedback. Please star this repo if you like this template. 
# Author: makichiis (sarahstar)

# NOTE to novice developers: While this CMakeLists *is* usable from the command line, I recommend that you know what you are doing 
# before doing so. That being said, if you want to know how you would run these configurations from the command line, you may look  
# at the cmake arguments that cmake-tools for Visual Studio Code spits out when selecting certain kits and variants. If using CMake 
# from the command line on a Linux or BSD (Mac, etc.) system, invoking cmake *should* be straightforward, and this cmakelists 
# attempts to respect your development environment. 

# TODO: Tests, source list, include dirs, etc. 
# Evaluating configuration type(s): https://stackoverflow.com/questions/31661264/how-to-check-if-generator-is-a-multi-config-generator-in-a-cmakelists-txt

# If building on Windows from command line with MSVC back-end: 
# https://learn.microsoft.com/en-us/cpp/build/cmake-presets-vs?view=msvc-160#run-cmake-from-the-command-line-or-a-ci-pipeline

# 1. Avoid checking CMAKE_BUILD_TYPE https://stackoverflow.com/a/66083321
# 2. Check CMAKE_<(C|CXX)>_COMPILER_ID to determine what arguments to use https://cmake.org/cmake/help/latest/variable/CMAKE_LANG_COMPILER_ID.html
# 3. The AddressSanitizer cannot link against debug versions of the C Runtime Library (CRT), specified by /MTd and /MDd. I'm choosing to use /MD since
#    Visual Studio devtools builds a CRT dynlib by default. 
# 4. I have no where where I found these except through VS2022 linker arguments posted on forum threads https://stackoverflow.com/a/76703293

# COMPILERS
# ClangCL - ClangCL is the recommended Clang CLI when building for Windows. VSCode's cmake-tools extension will provide a kit for this. Supported.
# Visual C++ - Supported for single-configuration and multi-configuration 
# AppleClang - The most straightforward setup, alongside GNU and Clang. TODO
# GNU/Clang - Mostly supported
# GNU/GCC - Mostly supported

# https://learn.microsoft.com/en-us/cpp/build/cmake-presets-vs?view=msvc-160#run-cmake-from-the-command-line-or-a-ci-pipeline

get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

# https://stackoverflow.com/a/24767451
if(NOT is_multi_config AND NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Release.")
    set(CMAKE_BUILD_TYPE "Release")
endif()

add_executable(CMakeLists-Template-Project test.cpp)

# Many gnu/msvc arguments from https://nullprogram.com/blog/2023/04/29/

# CRT: https://learn.microsoft.com/en-us/cpp/c-runtime-library/crt-library-features?view=msvc-170
# CL args: https://learn.microsoft.com/en-us/cpp/build/reference/compiler-options-listed-by-category?view=msvc-170
# Easy-to-read ClangCL argument overview: https://clang.llvm.org/docs/UsersManual.html#windows
# If using Visual Studio: https://devblogs.microsoft.com/cppblog/addresssanitizer-asan-for-windows-with-msvc/
# https://github.com/microsoft/vscode-cmake-tools/blob/015abf52297c5319fb3e81b598f4b11dd82479b0/docs/cmake-presets.md
# https://en.wikipedia.org/wiki/Pantone_448_C

# -- Additional compiler-gnostic arguments. <PLATFORM>_DEBUG_ADDITIONAL_ARGS and <PLATFORM>_RELEASE_ADDITIONAL_ARGS supported.
# Platforms:
#   MSVC - Visual C++ or ClangCL.
#   GNU - GNU/Clang or GNU/GCC.

set(MSVC_DEBUG_ADDITIONAL_ARGS 
    /W4                             # Maximum warning level 
    /wd4146                         # Silence warning C4146
    /wd4245                         # Silence warning C4245
    /D_CRT_SECURE_NO_WARNINGS       # https://nullprogram.com/blog/2023/02/15/
    /Z7                             # Maximum debug level
    /RTCsu                          # https://learn.microsoft.com/en-us/cpp/build/reference/rtc-run-time-error-checks?view=msvc-170
)

set(GNU_DEBUG_ADDITIONAL_ARGS 
    -mno-omit-leaf-frame-pointer    # Required for frame pointer omission in clang
    -fno-omit-frame-pointer         # Omitting the frame pointer makes debugging on some distributions impossible 
    -Wall                           # Enable most warning flags 
    -Wextra                         # Enable additional warning flags 
    -Wconversion                    # https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html#index-Wconversion
    -Wdouble-promotion              # https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html#index-Wno-double-promotion
    -Wno-unused-function            # Suppress unused function warning 
    -Wno-sign-conversion            # Suppress sign conversion warning 
    -g3                             # Maximum debug level
)

message(STATUS "Using generator ${CMAKE_GENERATOR} with ${CMAKE_CXX_COMPILER_ID}, build type '${CMAKE_BUILD_TYPE}'")

# -- Build environment flags defined at the highest level in order to easily refactor flag conditions.

# Workaround to compare clang and clangcl when using Ninja. https://learn.microsoft.com/en-us/cpp/build/cmake-presets-vs?view=msvc-160  
if(CMAKE_GENERATOR_TOOLSET MATCHES "ClangCL" OR CMAKE_CXX_COMPILER MATCHES "clang-cl") 
    set(is_clangcl TRUE)
endif()

if(NOT MSVC AND (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU"))
    set(is_gnu_clang TRUE)
endif()

if(NOT (MSVC OR is_gnu_clang))
    message(WARNING "Compiler toolset ${CMAKE_CXX_COMPILER_ID} is not explicitly supported by this template, so AddressSanitizer may be unsupported.")
endif()

if(WIN32 AND is_gnu_clang) # https://stackoverflow.com/questions/14714877/mismatch-detected-for-runtimelibrary
    message("NOTE: GNU/Clang will only work with the AddressSanitizer if you have a non-MSVC C runtime. If you are using Clang with MSVC, use ClangCL instead.")
    message("Current compiler: ${CMAKE_CXX_COMPILER}")
endif()

# -- Additional compiler-gnostic arguments for each configuration

# https://ecrafter.wordpress.com/2012/04/24/cmake-tutorial-part-2/
# https://web.archive.org/web/20190722182127/http://voices.canonical.com/jussi.pakkanen/2013/03/26/a-list-of-common-cmake-antipatterns/

if(MSVC)
    message(STATUS "Using MSVC arguments.")

    set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG} ${MSVC_DEBUG_ADDITIONAL_ARGS})
    set(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE} ${MSVC_RELEASE_ADDITIONAL_ARGS})
elseif(is_gnu_clang)
    message(STATUS "Using GNU arguments.")

    set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG} ${GNU_DEBUG_ADDITIONAL_ARGS})
    set(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE} ${GNU_RELEASE_ADDITIONAL_ARGS})
endif()

# https://learn.microsoft.com/en-us/cpp/build/cmake-presets-vs?view=msvc-160#enable-addresssanitizer-for-windows-and-linux
option(ASAN_ENABLED "Build this target with AddressSanitizer" ON)

# -- Configures AddressSanitizer linkage arguments based on the compiler toolset.  

if(MSVC)
    set(ASAN_COMPILE_ARGS /fsanitize=address)
    if(is_clangcl) # ClangCL linkage workaround TODO: Check if asan lib names are widely-supported and/or search for libasan
        set(ASAN_COMPILE_ARGS ${ASAN_COMPILE_ARGS} /MD /Oy-)
        set(ASAN_LINK_ARGS clang_rt.asan_dynamic-x86_64.lib clang_rt.asan_dynamic_runtime_thunk-x86_64.lib)
    endif()
else()
    set(ASAN_COMPILE_ARGS -fsanitize=address)
    set(ASAN_LINK_ARGS -fsanitize=address)
endif()

# -- Bootstrapped AddressSanitizer linkage. Configurations to arguments are done in the previous step.

if(ASAN_ENABLED)
    if(is_multi_config)
        target_compile_options(CMakeLists-Template-Project PUBLIC $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:${ASAN_COMPILE_ARGS}>)
        target_link_options(CMakeLists-Template-Project PUBLIC $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:${ASAN_LINK_ARGS}>)
    elseif(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
        target_compile_options(CMakeLists-Template-Project PUBLIC ${ASAN_COMPILE_ARGS})
        target_link_options(CMakeLists-Template-Project PUBLIC ${ASAN_LINK_ARGS})
    endif()
endif()
