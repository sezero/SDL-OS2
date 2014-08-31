/* libFLAC - Free Lossless Audio Codec library
 * Copyright (C) 2000-2009  Josh Coalson
 * Copyright (C) 2011-2013  Xiph.Org Foundation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * - Neither the name of the Xiph.org Foundation nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef FLAC__PRIVATE__BITREADER_H
#define FLAC__PRIVATE__BITREADER_H

#include <stdio.h> /* for FILE */
#include "FLAC/ordinals.h"
#include "cpu.h"

/*
 * opaque structure definition
 */
#ifndef __WATCOMC__
struct FLAC__BitReader;
#else
typedef FLAC__bool (*FLAC__BitReaderReadCallback)(FLAC__byte buffer[], size_t *bytes, void *client_data);

/* WATCHOUT: assembly routines rely on the order in which these fields are declared */
struct FLAC__BitReader {
	/* any partially-consumed word at the head will stay right-justified as bits are consumed from the left */
	/* any incomplete word at the tail will be left-justified, and bytes from the read callback are added on the right */
	uint32_t *buffer;
	unsigned capacity; /* in words */
	unsigned words; /* # of completed words in buffer */
	unsigned bytes; /* # of bytes in incomplete word at buffer[words] */
	unsigned consumed_words; /* #words ... */
	unsigned consumed_bits; /* ... + (#bits of head word) already consumed from the front of buffer */
	unsigned read_crc16; /* the running frame CRC */
	unsigned crc16_align; /* the number of bits in the current consumed word that should not be CRC'd */
	FLAC__BitReaderReadCallback read_callback;
	void *client_data;
	FLAC__CPUInfo cpu_info;
};
#endif
typedef struct FLAC__BitReader FLAC__BitReader;

#ifndef __WATCOMC__
typedef FLAC__bool (*FLAC__BitReaderReadCallback)(FLAC__byte buffer[], size_t *bytes, void *client_data);
#endif

/*
 * construction, deletion, initialization, etc functions
 */
FLAC__BitReader *FLAC__bitreader_new(void);
void FLAC__bitreader_delete(FLAC__BitReader *br);
FLAC__bool FLAC__bitreader_init(FLAC__BitReader *br, FLAC__CPUInfo cpu, FLAC__BitReaderReadCallback rcb, void *cd);
void FLAC__bitreader_free(FLAC__BitReader *br); /* does not 'free(br)' */
FLAC__bool FLAC__bitreader_clear(FLAC__BitReader *br);
void FLAC__bitreader_dump(const FLAC__BitReader *br, FILE *out);

/*
 * CRC functions
 */
void FLAC__bitreader_reset_read_crc16(FLAC__BitReader *br, FLAC__uint16 seed);
FLAC__uint16 FLAC__bitreader_get_read_crc16(FLAC__BitReader *br);

/*
 * info functions
 */
#ifdef __WATCOMC__
#define FLAC__bitreader_is_consumed_byte_aligned(_br) ((_br->consumed_bits & 7) == 0)
#define FLAC__bitreader_bits_left_for_byte_alignment(_br) (8 - (_br->consumed_bits & 7))
#define FLAC__bitreader_get_input_bits_unconsumed(_br) ((_br->words-_br->consumed_words)*FLAC__BITS_PER_WORD + _br->bytes*8 - _br->consumed_bits)
#else
FLAC__bool FLAC__bitreader_is_consumed_byte_aligned(const FLAC__BitReader *br);
unsigned FLAC__bitreader_bits_left_for_byte_alignment(const FLAC__BitReader *br);
unsigned FLAC__bitreader_get_input_bits_unconsumed(const FLAC__BitReader *br);
#endif

/*
 * read functions
 */

FLAC__bool FLAC__bitreader_read_raw_uint32(FLAC__BitReader *br, FLAC__uint32 *val, unsigned bits);
FLAC__bool FLAC__bitreader_read_raw_int32(FLAC__BitReader *br, FLAC__int32 *val, unsigned bits);
FLAC__bool FLAC__bitreader_read_raw_uint64(FLAC__BitReader *br, FLAC__uint64 *val, unsigned bits);
FLAC__bool FLAC__bitreader_read_uint32_little_endian(FLAC__BitReader *br, FLAC__uint32 *val); /*only for bits=32*/
FLAC__bool FLAC__bitreader_skip_bits_no_crc(FLAC__BitReader *br, unsigned bits); /* WATCHOUT: does not CRC the skipped data! */ /*@@@@ add to unit tests */
FLAC__bool FLAC__bitreader_skip_byte_block_aligned_no_crc(FLAC__BitReader *br, unsigned nvals); /* WATCHOUT: does not CRC the read data! */
FLAC__bool FLAC__bitreader_read_byte_block_aligned_no_crc(FLAC__BitReader *br, FLAC__byte *val, unsigned nvals); /* WATCHOUT: does not CRC the read data! */
FLAC__bool FLAC__bitreader_read_unary_unsigned(FLAC__BitReader *br, unsigned *val);
FLAC__bool FLAC__bitreader_read_rice_signed(FLAC__BitReader *br, int *val, unsigned parameter);
FLAC__bool FLAC__bitreader_read_rice_signed_block(FLAC__BitReader *br, int vals[], unsigned nvals, unsigned parameter);
#ifndef FLAC__NO_ASM
#  ifdef FLAC__CPU_IA32
#    ifdef FLAC__HAS_NASM
FLAC__bool FLAC__bitreader_read_rice_signed_block_asm_ia32_bswap(FLAC__BitReader *br, int vals[], unsigned nvals, unsigned parameter);
#    endif
#  endif
#endif
#if 0 /* UNUSED */
FLAC__bool FLAC__bitreader_read_golomb_signed(FLAC__BitReader *br, int *val, unsigned parameter);
FLAC__bool FLAC__bitreader_read_golomb_unsigned(FLAC__BitReader *br, unsigned *val, unsigned parameter);
#endif
FLAC__bool FLAC__bitreader_read_utf8_uint32(FLAC__BitReader *br, FLAC__uint32 *val, FLAC__byte *raw, unsigned *rawlen);
FLAC__bool FLAC__bitreader_read_utf8_uint64(FLAC__BitReader *br, FLAC__uint64 *val, FLAC__byte *raw, unsigned *rawlen);

FLAC__bool bitreader_read_from_client_(FLAC__BitReader *br);
#endif
