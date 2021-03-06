/* libFLAC - Free Lossless Audio Codec library
 * Copyright (C) 2012  Xiph.org Foundation
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

/* It is assumed that this header will be included after "config.h". */

#if HAVE_BSWAP32			/* GCC and Clang */

#define	ENDSWAP_32(x)		(__builtin_bswap32 (x))

#elif defined _MSC_VER		/* Windows. Apparently in <stdlib.h>. */

#define	ENDSWAP_32(x)		(_byteswap_ulong (x))

#elif defined __WATCOMC__ && defined __386__

extern uint32_t _watcom_bswap32(uint32_t);
#pragma aux _watcom_bswap32 = \
    "bswap eax"  \
    parm   [eax] \
    modify [eax];
#define	ENDSWAP_32(x)		(_watcom_bswap32 (x))

#elif defined HAVE_BYTESWAP_H		/* Linux */

#include <byteswap.h>

#define	ENDSWAP_32(x)		(bswap_32 (x))

#else

#define	ENDSWAP_32(x)		((((x) >> 24) & 0xFF) + (((x) >> 8) & 0xFF00) + (((x) & 0xFF00) << 8) + (((x) & 0xFF) << 24))

#endif

