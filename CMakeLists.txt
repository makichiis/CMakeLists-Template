cmake_minimum_required(VERSION 3.8...3.29)

project(Test LANGUAGES CXX)

# This is a composition of my research on writing a cross-platform CMakeLists which uses proper debugging information and
# does its best to support linkage with the AddressSanitizer library. I intend to use this in all of my important projects 
# and will iteratively refactor and build on top of it as I see need. If you notice any issues with the template, please
# feel free to open an issue and give your diagnostics and feedback. Please star this repo if you like this template. 
# Author: makichiis (Sarah-Laurie Evans)

# NOTE: While this CMakeLists *is* usable from the command line, I recommend that you know what you are doing before doing so.
# That being said, if you want to know how you would run these configurations from the command line, you may look at the cmake 
# arguments that cmake-tools for Visual Studio Code spits out when selecting certain kits and variants. If using CMake from the
# command line on a Linux or BSD (Mac, etc.) system, invoking cmake *should* be straightforward, and this cmakelists attempts to
# respect your development environment. 

# TODO: Tests, source list, include dirs, etc. 
# TODO: Check if multi-generation is not enabled (CONFIG should be empty) 
# https://stackoverflow.com/questions/31661264/how-to-check-if-generator-is-a-multi-config-generator-in-a-cmakelists-txt

# DEBUG_CONFIG and RELEASE_CONFIG are comma-delimited lists of preset profiles which satisfy either "Debug" or "Release" builds.
# These may be re-defined to satisfy user-defined profiles, but by default they will satisfy the primary Visual Studio Code 
# cmake-tools profiles. 

# -- Copy starting here if you just want to get AddressSanitizer working on Windows 

set(DEBUG_CONFIG "Debug")
set(RELEASE_CONFIG "Release")
# TODO: MinSizeRel, RelWithDebInfo

# Arguments to pass to each compiler under "Debug" and "Release" mode. Feel free to PR with additional compiler support. 

# Many clang/clang-cl arguments from https://nullprogram.com/blog/2023/04/29/

set(CLANG_DEBUG_ARGS 
    -fno-omit-frame-pointer
    -fsanitize=address,undefined
    -fsanitize-trap 
    -Wall 
    -Wextra
    -Wconversion 
    -Wdouble-promotion 
    -Wno-unused-function 
    -Wno-sign-conversion 
    -g3 
    -O1 
)

# CRT: https://learn.microsoft.com/en-us/cpp/c-runtime-library/crt-library-features?view=msvc-170
# CL args: https://learn.microsoft.com/en-us/cpp/build/reference/compiler-options-listed-by-category?view=msvc-170
# Easy-to-read ClangCL argument overview: https://clang.llvm.org/docs/UsersManual.html#windows
# If using Visual Studio: https://devblogs.microsoft.com/cppblog/addresssanitizer-asan-for-windows-with-msvc/
# https://github.com/microsoft/vscode-cmake-tools/blob/015abf52297c5319fb3e81b598f4b11dd82479b0/docs/cmake-presets.md
# https://en.wikipedia.org/wiki/Pantone_448_C

set(CLANGCL_DEBUG_ARGS 
    /Oy-                            # Disable omit frame pointer (synonymous with -fno-omit-frame-pointer)
    /MD                             # Disable debug symbol generation for CRT dynlib (3) 
    /W4                             # Maximum warning level 
    /wd4146                         # Silence warning C4146
    /wd4245                         # Silence warning C4245
    /D_CRT_SECURE_NO_WARNINGS       # https://nullprogram.com/blog/2023/02/15/
    /Z7                             # Maximum debug level 
    /RTCcsu                         # https://learn.microsoft.com/en-us/cpp/build/reference/rtc-run-time-error-checks?view=msvc-170
    -fsanitize=address,undefined    # AddressSanitizer 
    -Xclang=-O1                     # Low optimization level to prevent obscuring debug info. <-O2 recommended. 
)

set(CLANGCL_RELEASE_ARGS # TODO 
    /MT 
    -Xclang=-O3
)

set(MSVC_DEBUG_ARGS # TODO: document
    /Oy- # Disable omit frame pointer (synonymous with -fno-omit-frame-pointer)
    /MD  # Disable debug symbol generation for CRT dynlib (3) 
    /W4 
    /wd4146 
    /wd4245 
    /D_CRT_SECURE_NO_WARNINGS 
    /Z7 
    /RTCcsu 
    /fsanitize=address 
    /fsanitize=undefined 
    /Od 
)

# 1. Avoid checking CMAKE_BUILD_TYPE https://stackoverflow.com/a/66083321
# 2. Check CMAKE_<(C|CXX)>_COMPILER_ID to determine what arguments to use https://cmake.org/cmake/help/latest/variable/CMAKE_LANG_COMPILER_ID.html
# 3. The AddressSanitizer cannot link against debug versions of the C Runtime Library (CRT), specified by /MTd and /MDd. I'm choosing to use /MD since
#    Visual Studio devtools build a CRT dynlib by default. 
# 4. I have no where where I found these except through VS2022 linker arguments posted on forum threads https://stackoverflow.com/a/76703293

# COMPILERS
# ClangCL - ClangCL is the recommended Clang CLI when building for Windows. VSCode's cmake-tools extension will provide a kit for this.
# Visual C++ - If you're building with MSVC you must append libasan arguments to CMAKE_<(C|CXX)>_FLAGS. TODO check if release args are forwarded.
# AppleClang - The most straightforward setup, alongside GNU and Clang. TODO
# Clang TODO 
# GNU TODO 

# TODO: Support for Clang GNU CLI for Windows with Ninja 

# -- Debug information and debugger linkage 

# TODO: Add GNU CLI support for Windows (?) [What does Ninja use?]
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC") # Clang for Windows, must use ClangCL
    if(NOT CMAKE_GENERATOR_TOOLSET MATCHES "ClangCL") # Error if not using ClangCL 
        # Note: In the case of the GNU CLI, static linkage is sometimes necessary for sanitizers https://stackoverflow.com/a/77577172
        message(FATAL_ERROR "Must use clang-cl.exe if using CLANG with MSVC. GNU CLI not supported.")
    endif()

    message("ClangCL specified. Using MSVC/Clang arguments for debugger linkage.")

    # TODO: Move debug args elsewhere so they're actually configurable
    add_compile_options("$<$<CONFIG:${DEBUG_CONFIG}>:${CLANGCL_DEBUG_ARGS}>") 
    add_link_options("$<$<CONFIG:${DEBUG_CONFIG}>:clang_rt.asan_dynamic-x86_64.lib;clang_rt.asan_dynamic_runtime_thunk-x86_64.lib>") # (4)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC") # Visual C++ 
    message("MSVC specified. Using MSVC arguments for debugger linkage.")

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" $<$<CONFIG:${DEBUG_CONFIG}>:${MSVC_DEBUG_ARGS}>)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" $<$<CONFIG:${DEBUG_CONFIG}>:${MSVC_DEBUG_ARGS}>)
endif()

# -- End of Copy starting here if you just want to get AddressSanitizer working on Windows 

# -- Executable definition. Anything after this line is TODO 

add_executable(Test test.cpp)
