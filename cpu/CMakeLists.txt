target_sources(mint.kernel
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/exception.c
    ${CMAKE_CURRENT_SOURCE_DIR}/interrupt.c
    ${CMAKE_CURRENT_SOURCE_DIR}/power.c
)
