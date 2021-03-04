/*
 * PROJECT:     ReactOS ISA PnP Bus driver
 * LICENSE:     GPL-2.0-or-later (https://spdx.org/licenses/GPL-2.0-or-later)
 * PURPOSE:     Hardware definitions
 * COPYRIGHT:   Copyright 2010 Cameron Gutman <cameron.gutman@reactos.org>
 *              Copyright 2020 Hervé Poussineau <hpoussin@reactos.org>
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define ISAPNP_ADDRESS 0x279
#define ISAPNP_WRITE_DATA 0xA79

#define ISAPNP_READ_PORT_MIN 0x203
#define ISAPNP_READ_PORT_START 0x213
#define ISAPNP_READ_PORT_MAX 0x3FF
#define ISAPNP_READ_PORT_STEP 0x10

#define ISAPNP_CSN_MIN 0x01
#define ISAPNP_CSN_MAX 0x0F

#define ISAPNP_READPORT 0x00
#define ISAPNP_SERIALISOLATION 0x01
#define ISAPNP_CONFIGCONTROL 0x02
#define ISAPNP_WAKE 0x03
#define ISAPNP_RESOURCEDATA 0x04
#define ISAPNP_STATUS 0x05
#define ISAPNP_CARDSELECTNUMBER 0x06
#define ISAPNP_LOGICALDEVICENUMBER 0x07

#define ISAPNP_ACTIVATE 0x30
#define ISAPNP_IORANGECHECK 0x31

#define ISAPNP_IOBASE(n) (0x60 + ((n)*2))
#define ISAPNP_IRQNO(n) (0x70 + ((n)*2))
#define ISAPNP_IRQTYPE(n) (0x71 + ((n) * 2))
#define ISAPNP_DMACHANNEL(n) (0x74 + (n))

#define ISAPNP_CONFIG_RESET (1 << 0)
#define ISAPNP_CONFIG_WAIT_FOR_KEY (1 << 1)
#define ISAPNP_CONFIG_RESET_CSN (1 << 2)

#define ISAPNP_LFSR_SEED 0x6A

#define ISAPNP_IS_SMALL_TAG(t) (!((t) & 0x80))
#define ISAPNP_SMALL_TAG_NAME(t) (((t) >> 3) & 0xF)
#define ISAPNP_SMALL_TAG_LEN(t) (((t) & 0x7))
#define ISAPNP_TAG_PNPVERNO 0x01
#define ISAPNP_TAG_LOGDEVID 0x02
#define ISAPNP_TAG_COMPATDEVID 0x03
#define ISAPNP_TAG_IRQ 0x04
#define ISAPNP_TAG_DMA 0x05
#define ISAPNP_TAG_STARTDEP 0x06
#define ISAPNP_TAG_ENDDEP 0x07
#define ISAPNP_TAG_IOPORT 0x08
#define ISAPNP_TAG_FIXEDIO 0x09
#define ISAPNP_TAG_RSVDSHORTA 0x0A
#define ISAPNP_TAG_RSVDSHORTB 0x0B
#define ISAPNP_TAG_RSVDSHORTC 0x0C
#define ISAPNP_TAG_RSVDSHORTD 0x0D
#define ISAPNP_TAG_VENDORSHORT 0x0E
#define ISAPNP_TAG_END 0x0F

#define ISAPNP_IS_LARGE_TAG(t) (((t) & 0x80))
#define ISAPNP_LARGE_TAG_NAME(t) (t)
#define ISAPNP_TAG_MEMRANGE 0x81
#define ISAPNP_TAG_ANSISTR 0x82
#define ISAPNP_TAG_UNICODESTR 0x83
#define ISAPNP_TAG_VENDORLONG 0x84
#define ISAPNP_TAG_MEM32RANGE 0x85
#define ISAPNP_TAG_FIXEDMEM32RANGE 0x86
#define ISAPNP_TAG_RSVDLONG0 0xF0
#define ISAPNP_TAG_RSVDLONG1 0xF1
#define ISAPNP_TAG_RSVDLONG2 0xF2
#define ISAPNP_TAG_RSVDLONG3 0xF3
#define ISAPNP_TAG_RSVDLONG4 0xF4
#define ISAPNP_TAG_RSVDLONG5 0xF5
#define ISAPNP_TAG_RSVDLONG6 0xF6
#define ISAPNP_TAG_RSVDLONG7 0xF7
#define ISAPNP_TAG_RSVDLONG8 0xF8
#define ISAPNP_TAG_RSVDLONG9 0xF9
#define ISAPNP_TAG_RSVDLONGA 0xFA
#define ISAPNP_TAG_RSVDLONGB 0xFB
#define ISAPNP_TAG_RSVDLONGC 0xFC
#define ISAPNP_TAG_RSVDLONGD 0xFD
#define ISAPNP_TAG_RSVDLONGE 0xFE
#define ISAPNP_TAG_RSVDLONGF 0xFF
#define ISAPNP_TAG_PSEUDO_NEWBOARD 0x100

typedef struct _ISAPNP_IDENTIFIER
{
    USHORT VendorId;
    USHORT ProdId;
    ULONG Serial;
    UCHAR Checksum;
} ISAPNP_IDENTIFIER, *PISAPNP_IDENTIFIER;

typedef struct _ISAPNP_LOGDEVID
{
    USHORT VendorId;
    USHORT ProdId;
    USHORT Flags;
} ISAPNP_LOGDEVID, *PISAPNP_LOGDEVID;

typedef struct _ISAPNP_DEVICEID
{
    CHAR* Name;
    USHORT VendorId;
    USHORT ProdId;
} ISAPNP_DEVICEID, *PISAPNP_DEVICEID;

#include <pshpack1.h>

typedef struct _ISAPNP_IO_DESCRIPTION
{
    UCHAR Information;
    USHORT Minimum;
    USHORT Maximum;
    UCHAR Alignment;
    UCHAR Length;
} ISAPNP_IO_DESCRIPTION, *PISAPNP_IO_DESCRIPTION;

typedef struct _ISAPNP_IRQ_DESCRIPTION
{
    USHORT Mask;
    UCHAR Information;
} ISAPNP_IRQ_DESCRIPTION, *PISAPNP_IRQ_DESCRIPTION;

typedef struct _ISAPNP_DMA_DESCRIPTION
{
    UCHAR Mask;
    UCHAR Information;
} ISAPNP_DMA_DESCRIPTION, *PISAPNP_DMA_DESCRIPTION;

#include <poppack.h>

#ifdef __cplusplus
}
#endif
