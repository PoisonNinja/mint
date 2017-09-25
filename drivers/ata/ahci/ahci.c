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
#include <fs/device.h>
#include <fs/stat.h>
#include <kernel.h>
#include <kernel/init.h>
#include <lib/math.h>
#include <lib/printf.h>
#include <mm/dma.h>
#include <mm/heap.h>
#include <string.h>

static int got_interrupt = 0;

static char ahci_last_letter = 'a';

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

static void ahci_send_command(struct ahci_device* device, uint8_t command,
                              size_t size, uint8_t write, uint64_t lba,
                              void* buffer)
{
    struct hba_port* port =
        (struct hba_port*)((addr_t)&device->hba->ports[device->port_no]);
    int slot = ahci_get_command_slot(port);
    if (slot < 0) {
        AHCI_LOG(WARNING, "Failed to send command\n");
        return;
    }
    struct hba_command_header* header =
        (struct hba_command_header*)device->command_base;
    header += slot;
    header->fis_length = sizeof(struct fis_reg_host_to_device) / 4;
    header->write = write ? 1 : 0;
    header->prdt_len = ((size - 1) / AHCI_PRDT_MAX_MEMORY) + 1;
    struct hba_command_table* table =
        (struct hba_command_table*)device->command_table_base[slot];
    size_t index = 0;
    size_t to_write = size;
    while (to_write && index < 65536) {
        struct hba_prdt_entry* entry =
            (struct hba_prdt_entry*)(&table->prdt_entries[index]);
        entry->data_base_low = LOWER_32((addr_t)buffer);
        entry->data_base_high = UPPER_32((addr_t)buffer);
        entry->byte_count = (to_write <= AHCI_PRDT_MAX_MEMORY) ?
                                (to_write - 1) :
                                (AHCI_PRDT_MAX_MEMORY - 1);
        to_write -= entry->byte_count + 1;
        index++;
        buffer += entry->byte_count + 1;
    }
    struct fis_reg_host_to_device* fis =
        (struct fis_reg_host_to_device*)table->command_fis;
    memset(fis, 0, sizeof(struct fis_reg_host_to_device));
    fis->type = FIS_TYPE_REG_H2D;
    fis->command = command;
    fis->c = 1;
    size_t num_blocks = (size + AHCI_BLOCK_SIZE - 1) / AHCI_BLOCK_SIZE;
    // if (lba) {
    if (ahci_get_lba48_capacity(device->identify) > 0) {
        fis->count_low = num_blocks & 0xFF;
        fis->count_high = (num_blocks >> 8) & 0xFF;
        fis->lba0 = lba & 0xFF;
        fis->lba1 = (lba >> 8) & 0xFF;
        fis->lba2 = (lba >> 16) & 0xFF;
        fis->lba3 = (lba >> 24) & 0xFF;
        fis->lba4 = (lba >> 32) & 0xFF;
        fis->lba5 = (lba >> 40) & 0xFF;
        fis->device = (1 << 6);
    } else {
        fis->count_low = num_blocks & 0xFF;
        fis->lba0 = lba & 0xFF;
        fis->lba1 = (lba >> 8) & 0xFF;
        fis->lba2 = (lba >> 16) & 0xFF;
        fis->device = (1 << 6) | ((lba >> 24) & 0xF);
    }
    // }
    port->command_issue = 1 << slot;
}

static int ahci_await_completion(struct ahci_device* device)
{
    while (!got_interrupt)
        ;
    while (device->port->task_file_data & ATA_STATUS_BSY)
        ;
    got_interrupt = 0;
    return 0;
}

static ssize_t ahci_read(struct ahci_device* device, uint8_t* buffer,
                         size_t size, off_t offset)
{
    struct dma_region* region = dma_alloc(size);
    while (size) {
        uint64_t block_index = offset / AHCI_BLOCK_SIZE;
        // TODO: Select cmd based on LBA size
        ahci_send_command(device, ATA_CMD_READ_DMA_EXT, size, 0, block_index,
                          (void*)region->physical_base);
        ahci_await_completion(device);
        memcpy(buffer, (const void*)region->virtual_base, size);
        size -= size;
    }
    return size;
}

static void ahci_initialize_port(struct ahci_device* device)
{
    struct hba_port* port =
        (struct hba_port*)((addr_t)&device->hba->ports[device->port_no]);
    ahci_stop_command(port);
    struct dma_region* command_list_base_dma = dma_alloc(0x2000);
    struct dma_region* fis_base_dma = dma_alloc(0x1000);
    addr_t command_list_base = command_list_base_dma->physical_base;
    addr_t fis_base = fis_base_dma->physical_base;
    /*
     * TODO: Make this 32-bit ready, since if addr_t is 32 bit, shifting by 32
     * bits will be undefined behavior.
     * https://stackoverflow.com/questions/9860538/
     */
    struct hba_command_header* header =
        (struct hba_command_header*)command_list_base_dma->virtual_base;
    for (int i = 0; i < 32; i++, header++) {
        struct dma_region* command_table_base_dma = dma_alloc(0x1000);
        addr_t command_table_base = command_table_base_dma->physical_base;
        header->command_table_base_low = command_table_base & 0xFFFFFFFF;
        header->command_table_base_high =
            (command_table_base >> 32) & 0xFFFFFFFF;
        device->command_table_base[i] = command_table_base_dma->virtual_base;
    }
    port->command_list_base_low = command_list_base & 0xFFFFFFFF;
    port->command_list_base_high = (command_list_base >> 32) & 0xFFFFFFFF;
    port->fis_base_low = fis_base & 0xFFFFFFFF;
    port->fis_base_high = (fis_base >> 32) & 0xFFFFFFFF;
    device->fis_base = fis_base_dma->virtual_base;
    device->command_base = command_list_base_dma->virtual_base;
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

static void ahci_create_fs_node(struct ahci_device* device)
{
    char device_name[256];
    snprintf(device_name, 256, "/dev/sd%c", ahci_last_letter);
    AHCI_LOG(DEBUG, "Device name is: %s\n", device_name);
    struct inode* node =
        mknod(device_name, S_IFBLK, MKDEV(1, ahci_last_letter - 'a'));
    if (!node) {
        AHCI_LOG(WARNING, "Failed to create device node for port %d\n",
                 device->port_no);
        return;
    }
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
                struct dma_region* buffer = dma_alloc(512);
                if (!buffer) {
                    AHCI_LOG(ERROR, "Failed to allocate memory for IDENTIFY\n");
                    continue;
                }
                AHCI_LOG(INFO, "Sending IDENTIFY command to device...\n");
                ahci_send_command(device, ATA_CMD_IDENTIFY, 512, 0, 0,
                                  (void*)buffer->physical_base);
                // Can't use ahci_await_completion here
                uint32_t timeout = 1000000;
                while (--timeout) {
                    if (!((device->port->sata_active |
                           device->port->command_issue) &
                          1))
                        break;
                }
                memcpy(device->identify, (uint16_t*)buffer->virtual_base, 512);
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
                ahci_create_fs_node(device);
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
            abar->ports[i].interrupt_status = ~0;
            abar->interrupt_status = (1 << i);
        }
    }
    got_interrupt = 1;
    return 0;
}

static struct interrupt_handler ahci_interrupt_handler = {
    .handler = ahci_interrupt,
    .dev_name = "ahci",
    .dev_id = &ahci_interrupt_handler,
};

static int ahci_probe(struct pci_device* device)
{
    struct hba_memory* abar = (struct hba_memory*)pci_map(
        (addr_t)device->header->header00.bar[5], sizeof(struct hba_memory));
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
TEST_INITCALL(init_ahci);
