add_definitions(
    -DX86
    -DX86_64
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcmodel=kernel -mno-red-zone")

# Be careful of what you pass in here. Input will go directly to LLD, so no Clang flags allowed
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -zmax-page-size=0x1000 -T${CMAKE_SOURCE_DIR}/arch/x86_64/mint.lds")

set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)
