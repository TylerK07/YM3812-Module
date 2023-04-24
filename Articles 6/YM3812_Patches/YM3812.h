#ifndef YM3812_Hurereg
#define YM3812_H

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
This library wraps Yamaha's YM3812 sound processor with functions that allow for
direct manipulation of the chip's registers so you can build cool things like EuroRack modules!


--- Theory of Operation: ---
The code consists of a few levels of abstraction to make things easier to use

CHIP CONTROL FUNCTIONS:
Affect the processer overall, with things like resetting and sending data.

REGISTER CONTROL FUNCTIONS - These are the lowest level functions and directly manipulate
the registers of the sound processor. Registers can be either global level (1 per chip) or
channel level (1 per channel x9).

*/

#include "YMDefs.h"

#define YM3812_NUM_CHANNELS  9                                                                    // Number of channels supported by the YM3812 chip
#define YM3812_NUM_OPERATORS 18                                                                   // Number of channels for the YM3812 chip


class YM3812 {                                                                                    // YM3812 Class
  private:

    //---------------- Register Caches ----------------//
    // Global Level Caches (3 bytes)
    uint8_t reg_01 = 0;                                                                           // Wave Select Enable
    uint8_t reg_08 = 0;                                                                           // Speech Synthesis Mode / Note Select
    uint8_t reg_BD = 0;                                                                           // Deep AM/VB, Rythm flag, BD, SD, TOM, TC, HH (Drum stuff)

    // Channel Level Caches (18 bytes)
    uint8_t reg_A0[YM3812_NUM_CHANNELS]  = {0,0,0,0,0,0,0,0,0};                                   // frequency (lower 8-bits)
    uint8_t reg_B0[YM3812_NUM_CHANNELS]  = {0,0,0,0,0,0,0,0,0};                                   // key on, freq block, frequency (higher 2-bits)


    // Mapping
    uint8_t op_map[YM3812_NUM_OPERATORS] = { 0,1,2,3,4,5,8,9,10,11,12,13,16,17,18,19,20,21 };     // Map Operator Index to Memory Offset
    uint8_t channel_map[YM3812_NUM_CHANNELS] = { 0,1,2,6,7,8,12,13,14 };                          // Map channel index to operator 1's index. Add 3 to get operator 2's index.

    // Frequency Scale
    const unsigned int FRQ_SCALE[31] = {                                                          // Midi Note Frequency scale
      0x0AD, 0x0B7, 0x0C2, 0x0CD, 0x0D9, 0x0E6, 0x0F4, 0x102, 0x112, 0x122, 0x133, 0x145,
      0x159, 0x16D, 0x183, 0x19A, 0x1B2, 0x1CC, 0x1E8, 0x205, 0x224, 0x244, 0x267, 0x28B,
      0x2B2, 0x2DB, 0x306, 0x334, 0x365, 0x399, 0x3CF
    };

    // Channel State Management
    uint8_t    num_channels = YM3812_NUM_CHANNELS;                                                // The nunber of channels in the YM3812
    YM_Channel channel_states[YM3812_NUM_CHANNELS];                                               // Data structure containing the state variables for each channel
    uint8_t    last_channel = 0;                                                                  // Contains the last updated channel


  public:
    YM3812();                                                                                     // Constructor

    /***************************
    * Chip Control Functions   *
    ***************************/
    void reset();                                                                                 // Reset the sound procesor and all class settings
    void sendData(uint8_t reg, uint8_t val);                                                      // Send data to the sound processor


    /***********************
    * Patch Functions      *
    ***********************/
    void patchNoteOn(  PatchArr &patch, uint8_t midiNote );                                       // Selects a channel and plays a midi note on it
    void patchNoteOff( PatchArr &patch, uint8_t midiNote );                                       // Turns off any channel playing the midi note
    void patchAllOff(  PatchArr &patch );                                                         // Turns off any channel playing a specific patch
    void patchUpdate(  PatchArr &patch );                                                         // Updates any channels playing a patch with the current values


    /***********************
    * Channel Functions    *
    ***********************/
    uint8_t chGetNext();                                                                          // Return the next available channel
    void    chPlayNote( uint8_t ch, uint8_t midiNote );                                           // Play a midi note on channel ch
    void    chSendPatch( uint8_t ch, PatchArr &patch );                                           // Update channel on YM3812 with patch information


    /***********************
    * Register Functions   *
    ***********************/

    //Global Processor Settings:
    void regWaveset(         uint8_t val ){ SET_BITS( reg_01, 0b00000001, 5, val ); sendData(0x01, reg_01); }  // Allowable Waveforms: Sine Only (0), All Wavforms (1)
    void regSpeechSynthesis( uint8_t val ){ SET_BITS( reg_08, 0b00000001, 7, val ); sendData(0x08, reg_08); }  // Speech Synth Mode: off (0), on (1)... not too much documentation on this.
    void regKeySplit(        uint8_t val ){ SET_BITS( reg_08, 0b00000001, 6, val ); sendData(0x08, reg_08); }  // Key scaling: off (0), on (1)
    void regTremoloDepth(    uint8_t val ){ sendData(0xBD, SET_BITS( reg_BD, 0b00000001, 7, val ) ); }         // Set global Tremolo Depth to normal (0) or deep (1)
    void regVibratoDepth(    uint8_t val ){ sendData(0xBD, SET_BITS( reg_BD, 0b00000001, 6, val ) ); }         // Set global Vibrato Depth to normal (0) or deep (1)

    //Frequency Focused:
    void regKeyOn(           uint8_t ch, uint8_t val ){ sendData(0xB0+ch, SET_BITS( reg_B0[ch], 0b00000001, 5, val ) ); } // Turn channel's sound on (1) or off (0)
    void regFrqBlock(        uint8_t ch, uint8_t val ){ sendData(0xB0+ch, SET_BITS( reg_B0[ch], 0b00000111, 2, val ) ); } // Set Frequency Block / Octave offset (0-7)
    void regFrqFnum(         uint8_t ch, uint16_t frequency ){                                                            // Set Frequency nunmber within the block (0-1024)
      sendData(  0xA0+ch, SET_BITS( reg_A0[ch], 0b11111111, 0, frequency & 0xFF ) );                                      //   Lower 8 bits of left channel's frequency number
      sendData(  0xB0+ch, SET_BITS( reg_B0[ch], 0b00000011, 0, frequency >> 8 )   );                                      //   Upper 2 bits of left channel's frequency number
    }

};


#endif  // YM3812_H
