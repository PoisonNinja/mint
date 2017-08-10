#include <drivers/ata/ahci/ahci.h>
#include <drivers/pci/pci.h>
#include <kernel.h>
#include <kernel/init.h>

static int ahci_probe(struct pci_device* device)
{
}

static struct pci_device_filter ahci_device_filter = {
    .vendor_id = 0, .device_id = 0, .class = 1, .subclass = 6,
};

static struct pci_driver ahci_driver = {
    .name = "ahci", .filter = &ahci_device_filter, .probe = ahci_probe,
};

static int init_ahci(void)
{
    pci_driver_register(&ahci_driver);
}
DEVICE_INITCALL(init_ahci);
