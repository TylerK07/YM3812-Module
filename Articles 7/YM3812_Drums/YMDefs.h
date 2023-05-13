#ifndef YMDEFS_H
#define YMDEFS_H

/*
     _____.___.  _____  ________    ______  ____________
     \__  |   | /     \ \_____  \  /  __  \/_   \_____  \
      /   |   |/  \ /  \  _(__  <  >      < |   |/  ____/
      \____   /    Y    \/       \/   --   \|   /       \
      / ______\____|__  /______  /\______  /|___\_______ \
      \/              \/       \/        \/             \/
            ________ __________.____    ________
            \_____  \\______   \    |   \_____  \
             /   |   \|     ___/    |    /  ____/
            /    |    \    |   |    |___/       \
            \_______  /____|   |_______ \_______ \
                    \/                 \/       \/
.____    ._______________________    _____ _______________.___.
|    |   |   \______   \______   \  /  _  \\______   \__  |   |
|    |   |   ||    |  _/|       _/ /  /_\  \|       _//   |   |
|    |___|   ||    |   \|    |   \/    |    \    |   \\____   |
|_______ \___||______  /|____|_  /\____|__  /____|_  // ______|
        \/           \/        \/         \/       \/ \/


YM3182 OPL2 LIBRARY source code designed to run on the AVR128DA28.
Copyright (C) 2022 Tyler Klein

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <https://www.gnu.org/licenses/>.

--- Description: ---
This library wraps Yamaha's YM3812 sound processor with functions that allow for direct
manipulation of the chip's registers so you can build cool things like EuroRack modules.
This file contains all of the general definitions used throughout the YM3812 library.
Some of the functions, data structure and macro definitions are in this file because
I want to be able to create projects with multiple types of sound chips. Since they use
the same general variables, I had to pull the more general things into this file.

*/


/*--------- SET & GET bits with a helper macro! ---------//
Set the bits in an 8-bit variable using a bit mask. The bit mask should be right-aligned
for the number of bits you want to include. The offset argument left shifts the bits into the
right position to insert into the register. For example, if you wanted to update bits 4-6 on
variable "regVal", you would go:

SET_BITS( regVal, 0b00000111, 4, newVal );

To get the bits, mask out the ones you want to keep, and use offset to right-shift them to end.
To retreive the bits we just set above, use:

val = GET_BITS( regVal, 0b01110000, 4 );
*/
#define SET_BITS( regVal, mask, offset, newVal) ((regVal) = ((regVal) & (~((mask)<<(offset)))) | (((newVal) & (mask)) << (offset)))
#define GET_BITS( regVal, mask, shift ) ( ((regVal) & (mask)) >> (shift) )


//---------------- NAMES FOR GENERIC PATCH VALUE OFFSETS ----------------//
// The following definitions outline the function of each index in our patch array.
// Note: These indexes go beyond just the functionality of the YM3812 and have been chosen to support a variety
// of Yamaha sound chips. The compatability matrix is shown for each setting below.

#define PATCH_GEN_SETTINGS    10 // Number of non-operator settings (channel, general, etc.)
#define PATCH_OP_SETTINGS     17 // Number of operator settings
#define PATCH_SIZE            PATCH_GEN_SETTINGS + PATCH_OP_SETTINGS * 4 // Total Patch Size

typedef uint8_t PatchArr[PATCH_SIZE]; // Define the PatchArr type as a uint8_t array

// Instrument Level / Virtual
#define PATCH_PROC_CONF        0 // Specifies the processor and desired processor configuration
#define PATCH_NOTE_NUMBER      1 // Indicates the pitch to use when playing as a drum sound
#define PATCH_PEG_INIT_LEVEL   2 // Initial pitch shift before attack begins (Virtual setting - none of the YM chips have this by default)
#define PATCH_PEG_ATTACK       3 // Pitch Envelope Attack (Virtual setting - none of the YM chips have this by default)
#define PATCH_PEG_RELEASE      4 // Pitch Envelope Release (Virtual setting - none of the YM chips have this by default)
#define PATCH_PEG_REL_LEVEL    5 // Final Release Level (Virtual setting - none of the YM chips have this by default)

// Channel Level
#define PATCH_FEEDBACK         6 // Used by: YM3526 | YM3812 | YMF262 | YM2413 | YM2151 |        | YM2612 |
#define PATCH_ALGORITHM        7 // Used by: YM3526 | YM3812 | YMF262 |        | YM2151 |        | YM2612 |
#define PATCH_TREMOLO_SENS     8 // Used by:        |        |        |        | YM2151 |        | YM2612 | *SN76489
#define PATCH_VIBRATO_SENS     9 // Used by:        |        |        |        | YM2151 |        | YM2612 | *SN76489

// Operator Level
#define PATCH_WAVEFORM        10 // Used by:        | YM3812 | YMF262 | YM2413 |        |        |        |
#define PATCH_LEVEL           11 // Used by: YM3526 | YM3812 | YMF262 | YM2413 | YM2151 | YM2149 | YM2612 | *SN76489
#define PATCH_LEVEL_SCALING   12 // Used by: YM3526 | YM3812 | YMF262 | YM2413 |        |        |        |
#define PATCH_ENV_SCALING     13 // Used by: YM3526 | YM3812 | YMF262 | YM2413 | YM2151 |        | YM2612 |
#define PATCH_PERCUSSIVE_ENV  14 // Used by: YM3526 | YM3812 | YMF262 | YM2413 |        |        |        | *SN76489
#define PATCH_ATTACK          15 // Used by: YM3526 | YM3812 | YMF262 | YM2413 | YM2151 |        | YM2612 | *SN76489
#define PATCH_DECAY           16 // Used by: YM3526 | YM3812 | YMF262 | YM2413 | YM2151 |        | YM2612 | *SN76489
#define PATCH_SUSTAIN_LEVEL   17 // Used by: YM3526 | YM3812 | YMF262 | YM2413 | YM2151 |        | YM2612 | *SN76489
#define PATCH_SUSTAIN_DECAY   18 // Used by:        |        |        |        | YM2151 |        | YM2612
#define PATCH_RELEASE_RATE    19 // Used by: YM3526 | YM3812 | YMF262 | YM2413 | YM2151 |        | YM2612 | *SN76489
#define PATCH_TREMOLO         20 // Used by: YM3526 | YM3812 | YMF262 | YM2413 | YM2151 |        | YM2612
#define PATCH_VIBRATO         21 // Used by: YM3526 | YM3812 | YMF262 | YM2413 |        |        |
#define PATCH_FREQUENCY_MULT  22 // Used by: YM3526 | YM3812 | YMF262 | YM2413 | YM2151 |        | YM2612 |
#define PATCH_DETUNE_FINE     23 // Used by:        |        |        |        | YM2151 |        | YM2612 | *SN76489
#define PATCH_DETUNE_GROSS    24 // Used by:        |        |        |        | YM2151 |        | YM2612 | *SN76489
#define PATCH_SSGENV_ENABLE   25 // Used by:        |        |        |        |        | YM2149 | YM2612 | *SN76489
#define PATCH_SSGENV_WAVEFORM 26 // Used by:        |        |        |        |        |        | YM2612 | *SN76489


// Processor IDs
// Processor type field is composed of two nibbles. The upper nibble indicates the desired processor
// type, or can be zero so the system can choose any available processor capable of rendering the correct configuration.
// The lower nibble indicates they type of voice configuration, which can be 1 operator, 2 operator or 4 operator.
// A virtual 2x2 operator option allows 4 operators to be distributed virtually across two channels.

#define PID_ANYPROC        0x00  // Use any available sound processor
#define PID_YM3526         0x10  // YM3526 OPL  Supports: 1OP, 2OP, 2x2OP
#define PID_YM3812         0x20  // YM3812 OPL2 Supports: 1OP, 2OP, 2x2OP
#define PID_YMF262         0x30  // YMF262 OPL3 Supports: 1OP, 2OP, 2x2OP, 4OP
#define PID_YM2151         0x40  // YM2151 OPM  Supports: 4OP (emulated 2OP)
#define PID_YM2612         0x50  // YM2612 OPN2 Supports: 4OP
#define PID_YM2149         0x60  // YM2149 SSG  Supports: 1OP
#define PID_SN76489        0x70  // SN76489     Supports: 1OP

#define CONF_1OP           0b0001 // 1 operator voice
#define CONF_2OP           0b0010 // 2 operator voice
#define CONF_2x2OP         0b0011 // Dual 2 operator voices
#define CONF_4OP           0b0100 // 4 operoator voice


//---------------- Common Data Structures ----------------//
// The YM_Channel structure is used to track which of the channels on the YM sound chip
// are being used at any given time. This enables us to run the alrgorithm that selects
// the best channel to use when playing a new note.

struct YM_Channel{
  PatchArr      *pPatch = NULL;                                                                   // Pointer to the patch playing on the channel
  uint8_t       midi_note  = 0;                                                                   // The pitch of the note associated with the channel
  bool          note_state = false;                                                               // Whether the note is on (true) or off (false)
  unsigned long state_changed;                                                                    // The time that the note state changed (millis)
};




#endif  // YMDEFS_H
