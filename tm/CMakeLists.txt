target_sources(mint.kernel
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/process.c
    ${CMAKE_CURRENT_SOURCE_DIR}/sched.c
    ${CMAKE_CURRENT_SOURCE_DIR}/thread.c
)
