target_sources(mint.kernel
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/heap.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/physical.c
)
