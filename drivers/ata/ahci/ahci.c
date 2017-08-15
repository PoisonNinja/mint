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
 *
 * This driver uses code from the Sortix operating system. The license is
 * reproduced below:
 *
 * Copyright (c) 2013, 2014, 2015, 2016 Jonas 'Sortie' Termansen.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <arch/mm/mmap.h>
#include <cpu/interrupt.h>
#include <drivers/ata/ahci/ahci.h>
#include <drivers/pci/pci.h>
#include <kernel.h>
#include <kernel/init.h>
#include <lib/math.h>
#include <mm/heap.h>
#include <string.h>

/*
 * Copies a AHCI string into a char buffer
 *
 * Necessary because for some reason ATA devices according to Section 3.3.10
 * "ATA string convention" in the ATA command set states that each pair of
 * bytes are swapped. Also, they aren't NULL terminated, so we need to do this
 * manually.
 *
 * In essence, this behaves like memcpy but it automatically swaps the bytes
 * and inserts a NULL terminator for you.
 */
static inline void ahci_string_copy(char* target, char* source, size_t length)
{
    for (size_t i = 0; i < length; i += 2) {
        target[i] = source[i + 1];
        target[i + 1] = source[i];
    }
    target[length] = '\0';
}

static struct ahci_device* ports[32];

static void ahci_start_command(volatile struct hba_port* port)
{
    while (port->command & PXCMD_CR)
        ;
    port->command |= PXCMD_FRE;
    port->command |= PXCMD_ST;
}

static void ahci_stop_command(volatile struct hba_port* port)
{
    port->command &= ~PXCMD_ST;
    port->command &= ~PXCMD_FRE;
    while ((port->command & PXCMD_CR) || (port->command & PXCMD_FR))
        ;
}

static int ahci_get_command_slot(volatile struct hba_port* port)
{
    uint32_t slots = (port->sata_active | port->command_issue);
    for (int i = 0; i < 32; i++) {
        if ((slots & 1) == 0)
            return i;
        slots >>= 1;
    }
    AHCI_LOG(WARNING, "Cannot find free command list entry\n");
    return -1;
}

static void* ahci_send_command(struct ahci_device* device, uint8_t command,
                               size_t size, uint8_t write)
{
    struct hba_port* port =
        (struct hba_port*)((addr_t)&device->hba->ports[device->port_no]);
    int slot = ahci_get_command_slot(port);
    if (slot < 0) {
        AHCI_LOG(WARNING, "Failed to send command\n");
        return NULL;
    }
    struct hba_command_header* header =
        (struct hba_command_header*)device->command_base;
    header += slot;
    header->fis_length = sizeof(struct fis_reg_host_to_device) / 4;
    header->write = write ? 1 : 0;
    header->prdt_len = 1;
    struct hba_command_table* table =
        (struct hba_command_table*)((addr_t)MERGE_64(
                                        header->command_table_base_high,
                                        header->command_table_base_low) +
                                    PHYS_START);
    struct hba_prdt_entry* entry =
        (struct hba_prdt_entry*)(&table->prdt_entries[0]);
    addr_t dma = (addr_t)kmalloc(0x1000) - PHYS_START;
    entry->data_base_low = LOWER_32(dma);
    entry->data_base_high = UPPER_32(dma);
    entry->byte_count = size - 1;
    struct fis_reg_host_to_device* fis =
        (struct fis_reg_host_to_device*)table->command_fis;
    memset(fis, 0, sizeof(struct fis_reg_host_to_device));
    fis->type = FIS_TYPE_REG_H2D;
    fis->command = command;
    fis->c = 1;
    port->command_issue = 1 << slot;
    return (void*)(dma + PHYS_START);
}

static void ahci_initialize_port(struct ahci_device* device)
{
    struct hba_port* port =
        (struct hba_port*)((addr_t)&device->hba->ports[device->port_no]);
    ahci_stop_command(port);
    addr_t command_list_base = (addr_t)kzalloc(0x2000) - PHYS_START;
    addr_t fis_base = (addr_t)kzalloc(0x1000) - PHYS_START;
    /*
     * TODO: Make this 32-bit ready, since if addr_t is 32 bit, shifting by 32
     * bits will be undefined behavior.
     * https://stackoverflow.com/questions/9860538/
     */
    struct hba_command_header* header =
        (struct hba_command_header*)(command_list_base + PHYS_START);
    for (int i = 0; i < 32; i++, header++) {
        addr_t command_table_base = (addr_t)kzalloc(0x1000) - PHYS_START;
        header->command_table_base_low = command_table_base & 0xFFFFFFFF;
        header->command_table_base_high =
            (command_table_base >> 32) & 0xFFFFFFFF;
    }
    port->command_list_base_low = command_list_base & 0xFFFFFFFF;
    port->command_list_base_high = (command_list_base >> 32) & 0xFFFFFFFF;
    port->fis_base_low = fis_base & 0xFFFFFFFF;
    port->fis_base_high = (fis_base >> 32) & 0xFFFFFFFF;
    device->fis_base = fis_base + PHYS_START;
    device->command_base = command_list_base + PHYS_START;
    ahci_start_command(port);
    port->interrupt_enable =
        PXIE_DHRE | PXIE_PSE | PXIE_DSE | PXIE_SDBE | PXIE_DPE;
}

static int ahci_check_type(volatile struct hba_port* port)
{
    uint32_t ssts = port->sata_status;
    uint8_t ipm = (ssts >> 8) & 0x0F;
    uint8_t det = ssts & 0x0F;
    if (ipm != 1 || det != 3)
        return AHCI_TYPE_NULL;
    return port->signature;
}

static void ahci_detect_ports(struct hba_memory* abar)
{
    AHCI_LOG(INFO, "0x%X ports available\n", (abar->capability & 0xF) + 1);
    uint32_t pi = abar->port_implemented;
    for (int i = 0; i < 32; i++) {
        if (pi & 1) {
            int type = ahci_check_type(&abar->ports[i]);
            if (type == AHCI_TYPE_SATA) {
                AHCI_LOG(INFO, "Got SATA device attached to port %u\n", i);
                struct ahci_device* device =
                    kzalloc(sizeof(struct ahci_device));
                device->port_no = i;
                device->hba = abar;
                device->port =
                    (struct hba_port*)((addr_t)&abar->ports[device->port_no]);
                ports[i] = device;
                ahci_initialize_port(ports[i]);
                void* buffer =
                    ahci_send_command(device, ATA_CMD_IDENTIFY, 512, 0);
                uint32_t timeout = 1000000;
                while (--timeout) {
                    if (!((device->port->sata_active |
                           device->port->command_issue) &
                          1))
                        break;
                }
                memcpy(device->identify, (uint16_t*)buffer, 512);
                char model[41];
                char serial[21];
                ahci_string_copy(
                    serial, (char*)(&device->identify[ATA_SERIAL_NUMBER]), 20);
                ahci_string_copy(
                    model, (char*)(&device->identify[ATA_MODEL_NUMBER]), 40);
                AHCI_LOG(INFO, "Serial: %s\n", serial);
                AHCI_LOG(INFO, "Model: %s\n", model);
                AHCI_LOG(INFO, "LBA28 max addressable: %X\n",
                         ahci_get_lba28_capacity(device->identify));
                AHCI_LOG(INFO, "LBA48 max addressable: %llX\n",
                         ahci_get_lba48_capacity(device->identify));
            }
        }
        pi >>= 1;
    }
}

static int ahci_interrupt(struct interrupt_ctx* ctx, void* dev_id)
{
    struct hba_memory* abar = (struct hba_memory*)dev_id;
    for (uint32_t i = 0; i < 32; i++) {
        if (abar->interrupt_status & (1 << i)) {
            AHCI_LOG(DEBUG, "Received interrupt from port %u!\n", i);
            abar->ports[i].interrupt_status = ~0;
            abar->interrupt_status = (1 << i);
        }
    }
    return 0;
}

static struct interrupt_handler ahci_interrupt_handler = {
    .handler = ahci_interrupt,
    .dev_name = "ahci",
    .dev_id = &ahci_interrupt_handler,
};

static int ahci_probe(struct pci_device* device)
{
    struct hba_memory* abar =
        (struct hba_memory*)((addr_t)device->header->header00.bar[5] +
                             PHYS_START);
    AHCI_LOG(INFO, "Got controller with HBA memory located at %p\n", abar);
    if (!(device->header->command & 0x4)) {
        AHCI_LOG(INFO, "Enabling bus mastering mode\n");
        device->header->command |= 0x4;
    }
    if (abar->ext_capabilities & 0x1) {
        AHCI_LOG(INFO, "BIOS owns AHCI device. Requesting transfer...\n");
        abar->bohc |= 0x2;
        while (!(abar->bohc & 0x2) || (abar->bohc & 0x1))
            ;
        AHCI_LOG(INFO, "Transfer complete\n");
    } else {
        AHCI_LOG(INFO, "BIOS handoff not supported. Skipping...\n");
    }
    AHCI_LOG(INFO, "Interrupt line is %u\n",
             device->header->header00.interrupt_line);
    ahci_interrupt_handler.dev_id = abar;
    interrupt_handler_register(device->header->header00.interrupt_line,
                               &ahci_interrupt_handler);
    abar->global_host_control |= GHC_AE;
    abar->global_host_control |= GHC_IE;
    ahci_detect_ports(abar);
    return 0;
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
    return 0;
}
DEVICE_INITCALL(init_ahci);
