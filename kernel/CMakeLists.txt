target_sources(mint.kernel
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/console.c
    ${CMAKE_CURRENT_SOURCE_DIR}/init.c
    ${CMAKE_CURRENT_SOURCE_DIR}/kmain.c
    ${CMAKE_CURRENT_SOURCE_DIR}/panic.c
    ${CMAKE_CURRENT_SOURCE_DIR}/printk.c
    ${CMAKE_CURRENT_SOURCE_DIR}/stacktrace.c
    ${CMAKE_CURRENT_SOURCE_DIR}/symbol.c
)

add_subdirectory(lock)
add_subdirectory(time)
