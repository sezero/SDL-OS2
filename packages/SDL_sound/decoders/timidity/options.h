/*
    TiMidity -- Experimental MIDI to WAVE converter
    Copyright (C) 1995 Tuukka Toivonen <toivonen@clinet.fi>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/* When a patch file can't be opened, one of these extensions is
   appended to the filename and the open is tried again.
 */
#define PATCH_EXT_LIST { ".pat", 0 }

/* Acoustic Grand Piano seems to be the usual default instrument. */
#define DEFAULT_PROGRAM 0

/* 9 here is MIDI channel 10, which is the standard percussion channel.
   Some files (notably C:\WINDOWS\CANYON.MID) think that 16 is one too. 
   On the other hand, some files know that 16 is not a drum channel and
   try to play music on it. This is now a runtime option, so this isn't
   a critical choice anymore. */
#define DEFAULT_DRUMCHANNELS (1<<9) /* | (1<<15) */

/* In percent. */
#define DEFAULT_AMPLIFICATION 	70

/* Default polyphony */
#define DEFAULT_VOICES	32

/* 1000 here will give a control ratio of 22:1 with 22 kHz output.
   Higher CONTROLS_PER_SECOND values allow more accurate rendering
   of envelopes and tremolo. The cost is CPU time. */
#define CONTROLS_PER_SECOND 1000

/* Make envelopes twice as fast. Saves ~20% CPU time (notes decay
   faster) and sounds more like a GUS. There is now a command line
   option to toggle this as well. */
#define FAST_DECAY

/* How many bits to use for the fractional part of sample positions.
   This affects tonal accuracy. The entire position counter must fit
   in 32 bits, so with FRACTION_BITS equal to 12, the maximum size of
   a sample is 1048576 samples (2 megabytes in memory). The GUS gets
   by with just 9 bits and a little help from its friends...
   "The GUS does not SUCK!!!" -- a happy user :) */
#define FRACTION_BITS 12

/* For some reason the sample volume is always set to maximum in all
   patch files. Define this for a crude adjustment that may help
   equalize instrument volumes. */
#define ADJUST_SAMPLE_VOLUMES

/* The number of samples to use for ramping out a dying note. Affects
   click removal. */
#define MAX_DIE_TIME 20

/**************************************************************************/
/* Anything below this shouldn't need to be changed unless you're porting
   to a new machine with other than 32-bit, big-endian words. */
/**************************************************************************/

/* change FRACTION_BITS above, not these */
#define INTEGER_MASK (0xFFFFFFFF << FRACTION_BITS)
#define FRACTION_MASK (~ INTEGER_MASK)

/* This is enforced by some computations that must fit in an int */
#define MAX_CONTROL_RATIO 255

#define MAX_AMPLIFICATION 800

/* The TiMidity configuration file */
#define CONFIG_FILE "timidity.cfg"

/* These affect general volume */
#define GUARD_BITS 3
#define AMP_BITS (15-GUARD_BITS)

#define MAX_AMP_VALUE ((1<<(AMP_BITS+1))-1)

#define FSCALE(a,b) (float)((a) * (double)(1<<(b)))
#define FSCALENEG(a,b) (float)((a) * (1.0L / (double)(1<<(b))))

/* Vibrato and tremolo Choices of the Day */
#define SWEEP_TUNING 38
#define VIBRATO_AMPLITUDE_TUNING 1.0L
#define VIBRATO_RATE_TUNING 38
#define TREMOLO_AMPLITUDE_TUNING 1.0L
#define TREMOLO_RATE_TUNING 38

#define SWEEP_SHIFT 16
#define RATE_SHIFT 5

#ifndef PI
  #define PI 3.14159265358979323846
#endif
