/*
 * Copyright (C) 2017 Jason Lu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <arch/drivers/io.h>
#include <arch/mm/mmap.h>
#include <drivers/pci/pci.h>
#include <drivers/pci/pci_list.h>
#include <kernel.h>
#include <lib/math.h>
#include <mm/heap.h>
#include <mm/valloc.h>
#include <mm/virtual.h>

static struct list_element pci_driver_list = LIST_COMPILE_INIT(pci_driver_list);
static struct list_element pci_device_list = LIST_COMPILE_INIT(pci_device_list);

static struct valloc_region* pci_region = NULL;

struct pci_device* pci_driver_match_device(struct pci_driver* driver)
{
    if (!driver || !driver->filter)
        return NULL;
    struct pci_device_filter* filter = driver->filter;
    struct pci_device* device = NULL;
    list_for_each(&pci_device_list, list, device)
    {
        if (filter->vendor_id && filter->device_id && !filter->class &&
            !filter->subclass) {
            if (device->header->vendor_id == filter->vendor_id &&
                device->header->device_id == filter->device_id) {
                return device;
            }
        } else if (!filter->vendor_id && !filter->device_id && filter->class &&
                   filter->subclass) {
            if (device->header->class == filter->class &&
                device->header->subclass == filter->subclass) {
                return device;
            }
        } else if (filter->vendor_id && filter->device_id && filter->class &&
                   filter->subclass) {
            if (device->header->vendor_id == filter->vendor_id &&
                device->header->device_id == filter->device_id &&
                device->header->class == filter->class &&
                device->header->subclass == filter->subclass) {
                return device;
            }
        } else {
            return NULL;
        }
    }
    return NULL;
}

int pci_driver_register(struct pci_driver* driver)
{
    if (!driver->name || !driver->filter || !driver->probe)
        return 1;
    list_add(&pci_driver_list, &driver->list);
    struct pci_device* device = pci_driver_match_device(driver);
    if (device) {
        driver->probe(device);
    }
    return 0;
}

uint32_t pci_read_dword(const uint16_t bus, const uint16_t dev,
                        const uint16_t func, const uint32_t reg)
{
    outl(0xCF8, 0x80000000L | ((uint32_t)bus << 16) | ((uint32_t)dev << 11) |
                    ((uint32_t)func << 8) | (reg & ~3));
    return inl(0xCFC + (reg & 3));
}

void pci_write_dword(const uint16_t bus, const uint16_t dev,
                     const uint16_t func, const uint32_t reg, unsigned data)
{
    outl(0xCF8, 0x80000000L | ((uint32_t)bus << 16) | ((uint32_t)dev << 11) |
                    ((uint32_t)func << 8) | (reg & ~3));
    outl(0xCFC + (reg & 3), data);
}

static void pci_dump_header(struct pci_header* header)
{
    PCI_VENTABLE pci_vendor = {
        .VenShort = "Unknown",
        .VenFull = "Unknown",
    };
    for (uint16_t i = 0; i < PCI_VENTABLE_LEN; i++) {
        if (PciVenTable[i].VenId == header->vendor_id)
            pci_vendor = PciVenTable[i];
    }
    PCI_DEVTABLE pci_device = {
        .Chip = "Unknown",
        .ChipDesc = "Unknown device",
    };
    for (uint16_t i = 0; i < PCI_DEVTABLE_LEN; i++) {
        if (PciDevTable[i].VenId == header->vendor_id) {
            if (PciDevTable[i].DevId == header->device_id)
                pci_device = PciDevTable[i];
        }
    }
    printk(INFO, "PCI: Found device: 0x%04X:0x%04X (%s %s)\n",
           header->vendor_id, header->device_id, pci_vendor.VenFull,
           pci_device.ChipDesc);
}

struct pci_header* pci_probe(uint16_t bus, uint16_t dev, uint16_t func)
{
    if (pci_read_dword(bus, dev, func, 0) == 0xFFFFFFFF)
        return NULL;
    struct pci_header* header = kzalloc(sizeof(struct pci_header));
    for (uint32_t i = 0; i < 64; i += 16) {
        *(uint32_t*)((addr_t)header + i) = pci_read_dword(bus, dev, func, i);
        *(uint32_t*)((addr_t)header + i + 4) =
            pci_read_dword(bus, dev, func, i + 4);
        *(uint32_t*)((addr_t)header + i + 8) =
            pci_read_dword(bus, dev, func, i + 8);
        *(uint32_t*)((addr_t)header + i + 12) =
            pci_read_dword(bus, dev, func, i + 12);
    }
    return header;
}

void* pci_map(addr_t physical, size_t size)
{
    size = ROUND_UP(size, 0x1000);
    addr_t virtual = valloc_alloc(pci_region, size);
    virtual_map(&kernel_context, virtual, physical, size,
                PAGE_PRESENT | PAGE_WRITABLE);
    return (void*)virtual;
}

void pci_detect(void)
{
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint16_t dev = 0; dev < 32; dev++) {
            struct pci_header* header = pci_probe(bus, dev, 0);
            if (!header)
                continue;
            pci_dump_header(header);
            struct pci_device* device = kzalloc(sizeof(struct pci_device));
            device->header = header;
            device->bus = bus;
            device->device = dev;
            device->function = 0;
            list_add(&pci_device_list, &device->list);
            if (header->type & 0x80) {
                for (uint16_t func = 1; func < 8; func++) {
                    header = pci_probe(bus, dev, func);
                    if (!header)
                        continue;
                    pci_dump_header(header);
                    device = kzalloc(sizeof(struct pci_device));
                    device->header = header;
                    device->bus = bus;
                    device->device = dev;
                    device->function = func;
                    list_add(&pci_device_list, &device->list);
                }
            }
        }
    }
}

void pci_init(void)
{
    pci_region = valloc_create(PCI_START, PCI_END - PCI_START);
    pci_detect();
}
