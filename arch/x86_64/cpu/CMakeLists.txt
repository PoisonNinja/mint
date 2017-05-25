target_sources(mint.kernel
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/flush.asm
    ${CMAKE_CURRENT_SOURCE_DIR}/gdt.c
)
