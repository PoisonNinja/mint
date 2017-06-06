#include <drivers/irqchip/irqchip.h>

void setup_arch()
{
    interrupt_controller_set(IC_INTEL_8259);
}
