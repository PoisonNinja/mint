target_sources(mint.kernel
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/intel-8253.c
    ${CMAKE_CURRENT_SOURCE_DIR}/jiffy.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tsc.c
)
