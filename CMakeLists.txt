cmake_minimum_required(VERSION 3.1)

set(MINT_VERSION_MAJOR "0")
set(MINT_VERSION_MINOR "0")
set(MINT_VERSION_PATCH "1")
set(MINT_VERSION_SUFFIX "-rc1")
set(MINT_VERSION "${MINT_VERSION_MAJOR}.${MINT_VERSION_MINOR}.${MINT_VERSION_PATCH}${MINT_VERSION_SUFFIX}")

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source builds are not allowed.")
endif("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")

# Targeting baremetal so we need a generic system.
#
# Setting something like Linux or a real system name causes CMake to inject
# additional flags that break builds
set(CMAKE_SYSTEM_NAME       Generic)
set(CMAKE_SYSTEM_VERSION    1)

set(ARCH "x86_64" CACHE STRING "Architecture to build for")
message(STATUS "Architecture: " ${ARCH})

set(ARCH_PATH "${CMAKE_SOURCE_DIR}/arch/${ARCH}")
if(NOT EXISTS ${ARCH_PATH})
    message(FATAL_ERROR "Unable to locate architecture directory. Perhaps your ARCH is wrong?")
endif()
message(STATUS "Architecture folder: " ${ARCH_PATH})

set(ARCH_INCLUDE_PATH "${CMAKE_SOURCE_DIR}/arch/include/${ARCH}")
if(NOT EXISTS ${ARCH_INCLUDE_PATH})
    message(FATAL_ERROR "Unable to locate architecture include directory. Perhaps your ARCH is wrong?")
endif()
message(STATUS "Architecture include folder: " ${ARCH_INCLUDE_PATH})

if(NOT DEFINED ${PLATFORM_FILE})
    set(PLATFORM_FILE ${ARCH_PATH}/platform.cmake)
endif()
if(NOT EXISTS ${PLATFORM_FILE})
    message(FATAL_ERROR "Platform file does not exist: " ${PLATFORM_FILE})
endif()
message(STATUS "Platform file: " ${PLATFORM_FILE})

if(NOT DEFINED SYSROOT)
    set(SYSROOT "${CMAKE_SOURCE_DIR}/../hdd")
endif()
message(STATUS "Sysroot: " ${SYSROOT})

find_program(CMAKE_C_COMPILER ${ARCH}-pepper-gcc)
if(NOT CMAKE_C_COMPILER)
    message(FATAL_ERROR "Compiler not found! Run the \
                         toolchain script in Pepper to build a toolchain. If \
                         you have already built it, make sure that you have run \
                         envsetup.sh before configuring CMake.")
endif()
message(STATUS "C Compiler: " ${CMAKE_C_COMPILER})

project(Mint C)

include(${PLATFORM_FILE})

enable_language(ASM_NASM)

# HACK: Setting CMAKE_ASM_NASM_FLAGS before enabling ASM_NASM overwrites the
#       flags we set in the platform file.
#
#       To work around this, platforms will set CMAKE_ASM_FLAGS and then we
#       will set it into ASM_NASM for them. This has the advantage of making
#       transitions to new assemblers slightly easier
set(CMAKE_ASM_NASM_FLAGS ${CMAKE_ASM_FLAGS})

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -ffreestanding -fno-builtin -Iinclude -fdiagnostics-color=always -O3")
message(STATUS "CFLAGS: " ${CMAKE_C_FLAGS})

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -nostdlib")
message(STATUS "LDFLAGS: " ${CMAKE_EXE_LINKER_FLAGS})

message(STATUS "ASMFLAGS: " ${CMAKE_ASM_NASM_FLAGS})

set(INCLUDE_DIRS "${INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/include/")
include_directories(${INCLUDE_DIRS})
message(STATUS "Include directories: " ${INCLUDE_DIRS})

add_executable(mint.kernel "")
add_subdirectory(${ARCH_PATH})
add_subdirectory(block)
add_subdirectory(cpu)
add_subdirectory(drivers)
add_subdirectory(fs)
add_subdirectory(kernel)
add_subdirectory(lib)
add_subdirectory(mm)
add_subdirectory(tests)
add_subdirectory(tm)

set(LN_ARGS ln -sf ${ARCH_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}/include/arch)
execute_process(
    COMMAND ${LN_ARGS}
)

add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/include/kernel/version.h
    COMMAND ${CMAKE_SOURCE_DIR}/tools/version.sh
        ${MINT_VERSION}
        ${ARCH}
        ${CMAKE_C_COMPILER}
    COMMENT "Generating version header"
)

add_custom_target(
    version-header
    DEPENDS "${CMAKE_SOURCE_DIR}/include/kernel/version.h"
)

add_dependencies(mint.kernel version-header)

install(TARGETS mint.kernel RUNTIME DESTINATION "${SYSROOT}/boot")
