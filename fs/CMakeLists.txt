target_sources(mint.kernel
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/fs.c
    ${CMAKE_CURRENT_SOURCE_DIR}/dentry.c
    ${CMAKE_CURRENT_SOURCE_DIR}/inode.c
    ${CMAKE_CURRENT_SOURCE_DIR}/path.c
)

add_subdirectory(initfs)
