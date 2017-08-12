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

#pragma once

#include <lib/list.h>
#include <types.h>

struct pci_header_00 {
    uint32_t bar[6];
    uint32_t cis;
    uint16_t sub_vendor_id;
    uint16_t sub_id;
    uint32_t expansion_rom;
    uint8_t cap;
    uint8_t reserved[7];
    uint8_t interrupt_line;
    uint8_t interrupt_pin;
    uint8_t min_grant;
    uint8_t max_latency;
} __attribute__((packed));

struct pci_header_01 {
    uint32_t bar[2];
    uint8_t primary_bus;
    uint8_t secondary_bus;
    uint8_t subordinate_bus;
    uint8_t secondary_latency;
    uint8_t io_base;
    uint8_t io_limit;
    uint16_t secondary_status;
    uint16_t memory_base;
    uint16_t memory_limit;
    uint16_t prefetch_base;
    uint16_t prefetch_limit;
    uint32_t prefetch_base_upper;
    uint32_t prefetch_limit_upper;
    uint16_t io_base_upper;
    uint16_t io_limit_upper;
    uint8_t cap;
    uint8_t reserved[3];
    uint32_t expansion_rom;
    uint8_t interrupt_line;
    uint8_t interrupt_pin;
    uint16_t bridge_control;
} __attribute__((packed));

struct pci_header {
    uint16_t vendor_id;
    uint16_t device_id;
    uint16_t command;
    uint16_t status;
    uint8_t revision;
    uint8_t prog_if;
    uint8_t subclass;
    uint8_t class;
    uint8_t cache_sz;
    uint8_t latency;
    uint8_t type;
    uint8_t bist;
    union {
        struct pci_header_00 header00;
        struct pci_header_01 header01;
    };
} __attribute__((packed));

struct pci_device {
    uint8_t bus, device, function;
    struct pci_header* header;
    struct list_element list;
};

struct pci_device_filter {
    uint32_t vendor_id, device_id;
    uint32_t class, subclass;
};

struct pci_driver {
    const char* name;
    struct pci_device_filter* filter;
    int (*probe)(struct pci_device*);
    struct list_element list;
};

extern int pci_driver_register(struct pci_driver* driver);
extern void pci_init(void);
