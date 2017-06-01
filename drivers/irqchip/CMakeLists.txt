target_sources(mint.kernel
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/intel-8259.c
    ${CMAKE_CURRENT_SOURCE_DIR}/irqchip.c
)
