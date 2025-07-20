cmake_minimum_required(VERSION 3.8...3.29)

project(CMakeLists-Template-Project LANGUAGES CXX)

# Avoid checking CMAKE_BUILD_TYPE with multi-config https://stackoverflow.com/a/66083321
get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

if(NOT is_multi_config AND NOT CMAKE_BUILD_TYPE) # https://stackoverflow.com/a/24767451
    message(STATUS "No build type selected, default to Release.")
    set(CMAKE_BUILD_TYPE "Release")
endif()

add_executable(CMakeLists-Template-Project test.cpp)

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

if(MSVC)
    message(STATUS "Using MSVC arguments.")

    set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG} ${MSVC_DEBUG_ADDITIONAL_ARGS})
    set(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE} ${MSVC_RELEASE_ADDITIONAL_ARGS})
elseif(is_gnu_clang)
    message(STATUS "Using GNU arguments.")

    set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG} ${GNU_DEBUG_ADDITIONAL_ARGS})
    set(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE} ${GNU_RELEASE_ADDITIONAL_ARGS})
endif()

# -- Configures AddressSanitizer linkage arguments based on the compiler toolset.  

# https://learn.microsoft.com/en-us/cpp/build/cmake-presets-vs?view=msvc-160#enable-addresssanitizer-for-windows-and-linux
option(ASAN_ENABLED "Build this target with AddressSanitizer" ON)

if(MSVC)
    set(ASAN_COMPILE_ARGS /fsanitize=address)
    if(is_clangcl) # ClangCL linkage workaround TODO: Check if asan lib names are widely-supported and/or search for libasan
        # The AddressSanitizer cannot link against debug versions of the C Runtime Library (CRT), specified by /MTd and /MDd. 
        # I'm choosing to use /MD since Visual Studio devtools builds a CRT dynlib by default. 
        set(ASAN_COMPILE_ARGS ${ASAN_COMPILE_ARGS} /MD /Oy-)
        set(ASAN_LINK_ARGS clang_rt.asan_dynamic-x86_64.lib clang_rt.asan_dynamic_runtime_thunk-x86_64.lib) # https://stackoverflow.com/a/76703293
    endif()
else()
    set(ASAN_COMPILE_ARGS -fsanitize=address -fno-omit-frame-pointer -mno-omit-leaf-frame-pointer)
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
