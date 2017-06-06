target_sources(mint.kernel
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/early_console.c
    ${CMAKE_CURRENT_SOURCE_DIR}/setup.c
    ${CMAKE_CURRENT_SOURCE_DIR}/stacktrace.c
)
