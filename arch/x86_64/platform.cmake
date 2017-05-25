add_definitions(
    -DX86
    -DX86_64
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcmodel=kernel -mno-red-zone")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -zmax-page-size=0x1000 -T${CMAKE_SOURCE_DIR}/arch/x86_64/mint.lds")
set(CMAKE_ASM_NASM_COMPILE_OBJECT "${CMAKE_ASM_NASM_COMPILE_OBJECT} -felf64")
