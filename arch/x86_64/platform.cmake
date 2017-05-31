add_definitions(
    -DX86
    -DX86_64
)

# Temporarily use the Linux target because otherwise Clang will pass it to GCC instead of LD
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --target=x86_64-pc-linux-elf -mcmodel=kernel -mno-red-zone")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld -Wl,-T${CMAKE_SOURCE_DIR}/arch/x86_64/mint.lds")
set(CMAKE_ASM_NASM_COMPILE_OBJECT "${CMAKE_ASM_NASM_COMPILE_OBJECT} -felf64")
