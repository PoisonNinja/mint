target_sources(mint.kernel
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/tests.c
)

add_subdirectory(fs)
add_subdirectory(lib)
add_subdirectory(mm)
