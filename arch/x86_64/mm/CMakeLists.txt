target_sources(mint.kernel
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/clone.c
    ${CMAKE_CURRENT_SOURCE_DIR}/fault.c
    ${CMAKE_CURRENT_SOURCE_DIR}/init.c
    ${CMAKE_CURRENT_SOURCE_DIR}/map.c
    ${CMAKE_CURRENT_SOURCE_DIR}/unmap.c
)
