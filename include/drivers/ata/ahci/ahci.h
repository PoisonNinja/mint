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

#pragma once

#include <kernel.h>
#include <types.h>

#define AHCI_LOG(level, ...) printk(level, "ahci: " __VA_ARGS__)

typedef enum {
    FIS_TYPE_REG_H2D = 0x27,    // Register FIS - host to device
    FIS_TYPE_REG_D2H = 0x34,    // Register FIS - device to host
    FIS_TYPE_DMA_ACT = 0x39,    // DMA activate FIS - device to host
    FIS_TYPE_DMA_SETUP = 0x41,  // DMA setup FIS - bidirectional
    FIS_TYPE_DATA = 0x46,       // Data FIS - bidirectional
    FIS_TYPE_BIST = 0x58,       // BIST activate FIS - bidirectional
    FIS_TYPE_PIO_SETUP = 0x5F,  // PIO setup FIS - device to host
    FIS_TYPE_DEV_BITS = 0xA1,   // Set device bits FIS - device to host
} FIS_TYPE;

#define ATA_STATUS_ERR (1 << 0)  /* Error. */
#define ATA_STATUS_DRQ (1 << 3)  /* Data Request. */
#define ATA_STATUS_DF (1 << 5)   /* Device Fault. */
#define ATA_STATUS_DRDY (1 << 6) /* Device Ready. */
#define ATA_STATUS_BSY (1 << 7)  /* Busy. */

/** ATA Commands. */
#define ATA_CMD_READ_DMA 0xC8          /**< READ DMA. */
#define ATA_CMD_READ_DMA_EXT 0x25      /**< READ DMA EXT. */
#define ATA_CMD_READ_SECTORS 0x20      /**< READ SECTORS. */
#define ATA_CMD_READ_SECTORS_EXT 0x24  /**< READ SECTORS EXT. */
#define ATA_CMD_WRITE_DMA 0xCA         /**< WRITE DMA. */
#define ATA_CMD_WRITE_DMA_EXT 0x35     /**< WRITE DMA EXT. */
#define ATA_CMD_WRITE_SECTORS 0x30     /**< WRITE SECTORS. */
#define ATA_CMD_WRITE_SECTORS_EXT 0x34 /**< WRITE SECTORS EXT. */
#define ATA_CMD_PACKET 0xA0            /**< PACKET. */
#define ATA_CMD_IDENTIFY_PACKET 0xA1   /**< IDENTIFY PACKET DEVICE. */
#define ATA_CMD_FLUSH_CACHE 0xE7       /**< FLUSH CACHE. */
#define ATA_CMD_FLUSH_CACHE_EXT 0xEA   /**< FLUSH CACHE EXT. */
#define ATA_CMD_IDENTIFY 0xEC          /**< IDENTIFY DEVICE. */

/** Bits in the Port x Interrupt Enable register. */
#define PXIE_DHRE (1 << 0)  /**< Device to Host Register Enable. */
#define PXIE_PSE (1 << 1)   /**< PIO Setup FIS Enable. */
#define PXIE_DSE (1 << 2)   /**< DMA Setup FIS Enable. */
#define PXIE_SDBE (1 << 3)  /**< Set Device Bits Enable. */
#define PXIE_UFE (1 << 4)   /**< Unknown FIS Enable. */
#define PXIE_DPE (1 << 5)   /**< Descriptor Processed Enable. */
#define PXIE_PCE (1 << 6)   /**< Port Connect Change Enable. */
#define PXIE_DMPE (1 << 7)  /**< Device Mechanical Presence Enable. */
#define PXIE_PRCE (1 << 22) /**< PhyRdy Change Enable. */
#define PXIE_IPME (1 << 23) /**< Incorrect Port Multiplier Enable. */
#define PXIE_OFE (1 << 24)  /**< Overflow Enable. */
#define PXIE_INFE (1 << 26) /**< Interface Non-Fatal Error Enable. */
#define PXIE_IFE (1 << 27)  /**< Interface Fatal Error Enable. */
#define PXIE_HBDE (1 << 28) /**< Host Bus Data Error Enable. */
#define PXIE_HBFE (1 << 29) /**< Host Bus Fatal Error Enable. */
#define PXIE_TFEE (1 << 30) /**< Task File Error Enable. */
#define PXIE_CPDE (1 << 31) /**< Cold Port Detect Enable. */

#define PORT_INTR_ERROR                                                   \
    (PXIE_UFE | PXIE_PCE | PXIE_PRCE | PXIE_IPME | PXIE_OFE | PXIE_INFE | \
     PXIE_IFE | PXIE_HBDE | PXIE_HBFE | PXIE_TFEE)

static const uint32_t PXCMD_ST = 1 << 0;  /* 0x00000001 */
static const uint32_t PXCMD_SUD = 1 << 1; /* 0x00000002 */
static const uint32_t PXCMD_POD = 1 << 2; /* 0x00000004 */
static const uint32_t PXCMD_CLO = 1 << 3; /* 0x00000008 */
static const uint32_t PXCMD_FRE = 1 << 4; /* 0x00000010 */
static inline uint32_t PXCMD_CSS(uint32_t val)
{
    return (val >> 8) % 32;
}
static const uint32_t PXCMD_MPSS = 1 << 13;  /* 0x00002000 */
static const uint32_t PXCMD_FR = 1 << 14;    /* 0x00004000 */
static const uint32_t PXCMD_CR = 1 << 15;    /* 0x00008000 */
static const uint32_t PXCMD_CPS = 1 << 16;   /* 0x00010000 */
static const uint32_t PXCMD_PMA = 1 << 17;   /* 0x00020000 */
static const uint32_t PXCMD_HPCP = 1 << 18;  /* 0x00040000 */
static const uint32_t PXCMD_MPSP = 1 << 19;  /* 0x00080000 */
static const uint32_t PXCMD_CPD = 1 << 20;   /* 0x00100000 */
static const uint32_t PXCMD_ESP = 1 << 21;   /* 0x00200000 */
static const uint32_t PXCMD_FBSCP = 1 << 22; /* 0x00400000 */
static const uint32_t PXCMD_APSTE = 1 << 23; /* 0x00800000 */
static const uint32_t PXCMD_ATAPI = 1 << 24; /* 0x01000000 */
static const uint32_t PXCMD_DLAE = 1 << 25;  /* 0x02000000 */
static const uint32_t PXCMD_ALPE = 1 << 26;  /* 0x04000000 */
static const uint32_t PXCMD_ASP = 1 << 27;   /* 0x08000000 */
static inline uint32_t PXCMD_ICC(uint32_t val)
{
    return (val >> 28) % 16;
}

#define AHCI_TYPE_NULL 0x0
#define AHCI_TYPE_SATA 0x00000101
#define AHCI_TYPE_ATAPI 0xEB140101
#define AHCI_TYPE_SEMB 0xC33C0101
#define AHCI_TYPE_PM 0x96690101

static const uint32_t GHC_AE = 1U << 31;
static const uint32_t GHC_MRSM = 1U << 2;
static const uint32_t GHC_IE = 1U << 1;
static const uint32_t GHC_HR = 1U << 0;

#define AHCI_PRDT_MAX_MEMORY 0x1000
#define AHCI_BLOCK_SIZE 512

struct fis_reg_host_to_device {
    uint8_t type;
    uint8_t pmport : 4;
    uint8_t reserved_0 : 3;
    uint8_t c : 1;
    uint8_t command;
    uint8_t feature_low;
    uint8_t lba0;
    uint8_t lba1;
    uint8_t lba2;
    uint8_t device;
    uint8_t lba3;
    uint8_t lba4;
    uint8_t lba5;
    uint8_t feature_high;

    uint8_t count_low;
    uint8_t count_high;
    uint8_t icc;
    uint8_t control;
    uint8_t reserved_1[4];
} __attribute__((packed));

struct fis_reg_device_to_host {
    uint8_t type;
    uint8_t pmport : 4;
    uint8_t reserved_0 : 2;
    uint8_t interrupt : 1;
    uint8_t reserved_1 : 1;
    uint8_t status;
    uint8_t error;
    uint8_t lba0;
    uint8_t lba1;
    uint8_t lba2;
    uint8_t device;
    uint8_t lba3;
    uint8_t lba4;
    uint8_t lba5;
    uint8_t reserved_2;
    uint8_t count_low;
    uint8_t count_high;
    uint8_t reserved_3[2];
    uint8_t reserved_4[4];
} __attribute__((packed));

struct fis_data {
    uint8_t type;
    uint8_t pmport : 4;
    uint8_t reserved_0 : 4;
    uint8_t reserved_1[2];
    uint32_t data[1];
} __attribute__((packed));

struct fis_pio_setup {
    uint8_t type;
    uint8_t pmport : 4;
    uint8_t reserved_0 : 1;
    uint8_t direction : 1;
    uint8_t interrupt : 1;
    uint8_t reserved_1 : 1;
    uint8_t status;
    uint8_t error;
    uint8_t lba0;
    uint8_t lba1;
    uint8_t lba2;
    uint8_t device;
    uint8_t lba3;
    uint8_t lba4;
    uint8_t lba5;
    uint8_t reserved2;
    uint8_t count_low;
    uint8_t count_high;
    uint8_t reserved3;
    uint8_t e_status;
    uint16_t transfer_count;
    uint8_t reserved_[2];
} __attribute__((packed));

struct fis_dma_setup {
    uint8_t type;
    uint8_t pmport : 4;
    uint8_t reserved_0 : 1;
    uint8_t direction : 1;
    uint8_t interrupt : 1;
    uint8_t auto_activate : 1;
    uint8_t reserved_1[2];
    uint64_t dma_buffer_id;
    uint32_t reserved_2;
    uint32_t dma_buffer_offset;
    uint32_t transfer_count;
    uint32_t reserved_3;
} __attribute__((packed));

struct fis_dev_bits {
    volatile uint8_t type;
    volatile uint8_t pmport : 4;
    volatile uint8_t reserved0 : 2;
    volatile uint8_t interrupt : 1;
    volatile uint8_t notification : 1;
    volatile uint8_t status;
    volatile uint8_t error;
    volatile uint32_t protocol;
} __attribute__((packed));

struct hba_port {
    volatile uint32_t command_list_base_low;
    volatile uint32_t command_list_base_high;
    volatile uint32_t fis_base_low;
    volatile uint32_t fis_base_high;
    volatile uint32_t interrupt_status;
    volatile uint32_t interrupt_enable;
    volatile uint32_t command;
    volatile uint32_t reserved_0;
    volatile uint32_t task_file_data;
    volatile uint32_t signature;
    volatile uint32_t sata_status;
    volatile uint32_t sata_control;
    volatile uint32_t sata_error;
    volatile uint32_t sata_active;
    volatile uint32_t command_issue;
    volatile uint32_t sata_notification;
    volatile uint32_t fis_based_switch_control;
    volatile uint32_t reserved_1[11];
    volatile uint32_t vendor[4];
} __attribute__((packed));

struct hba_memory {
    volatile uint32_t capability;
    volatile uint32_t global_host_control;
    volatile uint32_t interrupt_status;
    volatile uint32_t port_implemented;
    volatile uint32_t version;
    volatile uint32_t ccc_control;
    volatile uint32_t ccc_ports;
    volatile uint32_t em_location;
    volatile uint32_t em_control;
    volatile uint32_t ext_capabilities;
    volatile uint32_t bohc;
    volatile uint8_t reserved[0xA0 - 0x2C];
    volatile uint8_t vendor[0x100 - 0xA0];
    volatile struct hba_port ports[1];
} __attribute__((packed));

struct hba_received_fis {
    volatile struct fis_dma_setup fis_ds;
    volatile uint8_t pad_0[4];
    volatile struct fis_pio_setup fis_ps;
    volatile uint8_t pad_1[12];
    volatile struct fis_reg_device_to_host fis_r;
    volatile uint8_t pad_2[4];
    volatile struct fis_dev_bits fis_sdb;
    volatile uint8_t ufis[64];
    volatile uint8_t reserved[0x100 - 0xA0];
} __attribute__((packed));

struct hba_command_header {
    uint8_t fis_length : 5;
    uint8_t atapi : 1;
    uint8_t write : 1;
    uint8_t prefetchable : 1;
    uint8_t reset : 1;
    uint8_t bist : 1;
    uint8_t clear_busy_upon_r_ok : 1;
    uint8_t reserved_0 : 1;
    uint8_t pmport : 4;
    uint16_t prdt_len;
    volatile uint32_t prdb_count;
    uint32_t command_table_base_low;
    uint32_t command_table_base_high;
    uint32_t reserved_1[4];
} __attribute__((packed));

struct hba_prdt_entry {
    uint32_t data_base_low;
    uint32_t data_base_high;
    uint32_t reserved_0;
    uint32_t byte_count : 22;
    uint32_t reserved_1 : 9;
    uint32_t interrupt_on_complete : 1;
} __attribute__((packed));

struct hba_command_table {
    uint8_t command_fis[64];
    uint8_t acmd[16];
    uint8_t reserved[48];
    struct hba_prdt_entry prdt_entries[1];
} __attribute__((packed));

/*
 * ATA identify response data, per the ATA spec at
 * http://www.t13.org/Documents/UploadedDocuments/docs2009/d2015r1a-ATAATAPI_Command_Set_-_2_ACS-2.pdf
 *
 * TODO: Move this to generic ATA header
 */
enum ahci_identify {
    ATA_GENERAL_CONFIGURATION = 0,
    ATA_SPECIFIC_CONFIGURATION = 2,
    ATA_SERIAL_NUMBER = 10,
    ATA_FIRMWARE_REVISION = 23,
    ATA_MODEL_NUMBER = 27,
    ATA_TRUSTED_COMPUTING = 48,
    ATA_CAPABILITY = 49,
    ATA_FIELD_VALID = 53,
    ATA_MULTIPLE_SECTOR = 59,
    ATA_LBA28_CAPACITY = 60,
    ATA_MULTIWORD_MODES = 63,
    ATA_PIO_MODES = 64,
    ATA_MAJOR_VERSION = 80,
    ATA_MINOR_VERSION = 81,
    ATA_COMMANDSET_1 = 82,
    ATA_COMMANDSET_2 = 83,
    ATA_COMMANDSET_EXTENDED = 84,
    ATA_CFS_ENABLE_1 = 85,
    ATA_CFS_ENABLE_2 = 86,
    ATA_CFS_DEFAULT = 87,
    ATA_UDMA_MODES = 88,
    ATA_HW_RESET = 93,
    ATA_ACOUSTIC = 94,
    ATA_LBA48_CAPACITY = 100,
    ATA_REMOVABLE = 127,
    ATA_SECURITY_STATUS = 128,
    ATA_CFA_POWER_MODE = 160,
    ATA_MEDIA_SERIAL_NUMBER = 176,
    ATA_INTEGRITY = 255,
};

struct ahci_device {
    uint32_t port_no;
    struct hba_memory* hba;
    struct hba_port* port;
    addr_t fis_base, command_base;
    addr_t command_table_base[32];
    uint16_t identify[256];
};

static inline uint32_t ahci_get_lba28_capacity(uint16_t* identify)
{
    uint32_t lba_cap = identify[ATA_LBA28_CAPACITY + 1];
    return lba_cap << 16 | identify[ATA_LBA28_CAPACITY];
}

static inline uint64_t ahci_get_lba48_capacity(uint16_t* identify)
{
    uint64_t lba48_cap = identify[ATA_LBA48_CAPACITY + 3];
    return ((lba48_cap << 16 | identify[ATA_LBA48_CAPACITY + 2]) << 16 |
            identify[ATA_LBA48_CAPACITY + 1])
               << 16 |
           identify[ATA_LBA48_CAPACITY];
}
