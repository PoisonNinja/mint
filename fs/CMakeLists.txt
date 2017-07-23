target_sources(mint.kernel
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/dentry.c
    ${CMAKE_CURRENT_SOURCE_DIR}/file.c
    ${CMAKE_CURRENT_SOURCE_DIR}/fs.c
    ${CMAKE_CURRENT_SOURCE_DIR}/inode.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mount.c
    ${CMAKE_CURRENT_SOURCE_DIR}/open.c
)

add_subdirectory(initfs)
