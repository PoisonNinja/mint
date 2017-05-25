target_sources(mint.kernel
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/bootstrap32.asm
    ${CMAKE_CURRENT_SOURCE_DIR}/bootstrap64.asm
    ${CMAKE_CURRENT_SOURCE_DIR}/x86-64_init.c
)
