target_sources(mint.kernel
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/clocksource.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tick.c
    ${CMAKE_CURRENT_SOURCE_DIR}/time.c
)
