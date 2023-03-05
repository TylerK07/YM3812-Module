#ifndef YM3812_H
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
the registers of the sound processor. Registers can be either global level (1 per chip),
channel level (1 per channel x9) or operator level (1 per operator x2 per channel x9).
To update a register, you have to know which channel and operator's setting you want to change.

*/


#define YM3812_NUM_CHANNELS  9                                                 // Number of channels supported by the YM3812 chip
#define YM3812_NUM_OPERATORS 18                                                // Number of channels for the YM3812 chip


/*--------- SET & GET bits with a helper macro! ---------
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



class YM3812 {                                                                                    // YM3812 Class
  private:

    //---------------- Register Caches ----------------//
    // Global Level Caches (3 bytes)
    uint8_t reg_01 = 0;                                                                           // Wave Select Enable
    uint8_t reg_08 = 0;                                                                           // Speech Synthesis Mode / Note Select
    uint8_t reg_BD = 0;                                                                           // Deep AM/VB, Rythm flag, BD, SD, TOM, TC, HH (Drum stuff)

    // Channel Level Caches (27 bytes)
    uint8_t reg_A0[YM3812_NUM_CHANNELS]      = {0,0,0,0,0,0,0,0,0};                               // frequency (lower 8-bits)
    uint8_t reg_B0[YM3812_NUM_CHANNELS]      = {0,0,0,0,0,0,0,0,0};                               // key on, freq block, frequency (higher 2-bits)
    uint8_t reg_C0[YM3812_NUM_CHANNELS]      = {0,0,0,0,0,0,0,0,0};                               // feedback, algorithm

    // Operator Level Caches (90 bytes)
    uint8_t reg_20[YM3812_NUM_OPERATORS]     = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};             // tremolo, vibrato, envelope type, envelope scale, frequency multiplier
    uint8_t reg_40[YM3812_NUM_OPERATORS]     = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};             // Level Scaling, Total Level
    uint8_t reg_60[YM3812_NUM_OPERATORS]     = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};             // Attack Rate, Decay Rate
    uint8_t reg_80[YM3812_NUM_OPERATORS]     = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};             // Sustain Level, Release Rate
    uint8_t reg_E0[YM3812_NUM_OPERATORS]     = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};             // Waveform

    // Mapping
    uint8_t op_map[YM3812_NUM_OPERATORS]     = { 0,1,2,3,4,5,8,9,10,11,12,13,16,17,18,19,20,21 }; // Map Operator Index to Memory Offset

    // Frequency Scale
    const unsigned int FRQ_SCALE[31] = {
      0x0AD, 0x0B7, 0x0C2, 0x0CD, 0x0D9, 0x0E6, 0x0F4, 0x102, 0x112, 0x122, 0x133, 0x145,
      0x159, 0x16D, 0x183, 0x19A, 0x1B2, 0x1CC, 0x1E8, 0x205, 0x224, 0x244, 0x267, 0x28B,
      0x2B2, 0x2DB, 0x306, 0x334, 0x365, 0x399, 0x3CF
    };

    uint8_t num_channels = 3;                                                                     // The nnumber of channels (we are using three fro demonstration purposes)
    uint8_t channel_notes[3] = {0,0,0};                                                           // Contains the midi notes each channel is playing
    uint8_t last_channel = 0;                                                                     // Tracks the index of the last played channel

  public:                                                                                         // Eventually this will become private... but we are hacking this right now.
    uint8_t channel_map[YM3812_NUM_CHANNELS] = { 0,1,2,6,7,8,12,13,14 };                          // Map channel index to operator 1's index. Add 3 to get operator 2's index.


  public:
    YM3812();                                                                                     // Constructor

    /***************************
    * Chip Control Functions   *
    ***************************/
    void reset();                                                                                 // Reset the sound procesor and all class settings
    void sendData(uint8_t reg, uint8_t val);                                                      // Send data to the sound processor


    /***********************
    * Note Functions       *
    ***********************/
    void noteOn(  uint8_t midiNote );
    void noteOff( uint8_t midiNote );



    /***********************
    * Channel Functions    *
    ***********************/
    void chPlayNote( uint8_t ch, uint8_t midiNote );                                              // Play a midi note on channel ch


    /***********************
    * Register Functions   *
    ***********************/

    //Global Processor Settings:
    void regWaveset(         uint8_t val ){ SET_BITS( reg_01, 0b00000001, 5, val ); sendData(0x01, reg_01); }  // Allowable Waveforms: Sine Only (0), All Wavforms (1)
    void regSpeechSynthesis( uint8_t val ){ SET_BITS( reg_08, 0b00000001, 7, val ); sendData(0x08, reg_08); }  // Speech Synth Mode: off (0), on (1)... not too much documentation on this.
    void regKeySplit(        uint8_t val ){ SET_BITS( reg_08, 0b00000001, 6, val ); sendData(0x08, reg_08); }  // Key scaling: off (0), on (1)
    void regTremoloDepth(    uint8_t val ){ sendData(0xBD, SET_BITS( reg_BD, 0b00000001, 7, val ) ); }  // Set global Tremolo Depth to normal (0) or deep (1)
    void regVibratoDepth(    uint8_t val ){ sendData(0xBD, SET_BITS( reg_BD, 0b00000001, 6, val ) ); }  // Set global Vibrato Depth to normal (0) or deep (1)

    //Channel Settings
    void regChAlgorithm(     uint8_t ch, uint8_t val ){ sendData(0xC0+ch, SET_BITS( reg_C0[ch], 0b00000001, 0, val ) ); }         // Set Algorithm: 0 - FM, 1 - Addative
    void regChFeedback(      uint8_t ch, uint8_t val ){ sendData(0xC0+ch, SET_BITS( reg_C0[ch], 0b00000111, 1, val ) ); }         // Set Operator 1's Feedback (0-7)

    //Frequency Focused:
    void regKeyOn(           uint8_t ch, uint8_t val ){ sendData(0xB0+ch, SET_BITS( reg_B0[ch], 0b00000001, 5, val ) ); }         // Turn channel's sound on (1) or off (0)
    void regFrqBlock(        uint8_t ch, uint8_t val ){ sendData(0xB0+ch, SET_BITS( reg_B0[ch], 0b00000111, 2, val ) ); }         // Set Frequency Block / Octave offset (0-7)
    void regFrqFnum(         uint8_t ch, uint16_t frequency ){                                                                    // Set Frequency nunmber within the block (0-1024)
      sendData(  0xA0+ch, SET_BITS( reg_A0[ch], 0b11111111, 0, frequency & 0xFF ) );                                              //   Lower 8 bits of left channel's frequency number
      sendData(  0xB0+ch, SET_BITS( reg_B0[ch], 0b00000011, 0, frequency >> 8 )   );                                              //   Upper 2 bits of left channel's frequency number
    }

    //Operator Settings
    void regOpTremolo(       uint8_t op, uint8_t val ){ sendData(0x20+op_map[op], SET_BITS( reg_20[op], 0b00000001, 7, val ) ); } // Turn Tremolo on (1) or off (0)
    void regOpVibrato(       uint8_t op, uint8_t val ){ sendData(0x20+op_map[op], SET_BITS( reg_20[op], 0b00000001, 6, val ) ); } // Turn Vibrato on (1) or off (0)
    void regOpPercussiveEnv( uint8_t op, uint8_t val ){ sendData(0x20+op_map[op], SET_BITS( reg_20[op], 0b00000001, 5, val ) ); } // Set Envelope to Percussive (0) or Normal (1)
    void regOpEnvScaling(    uint8_t op, uint8_t val ){ sendData(0x20+op_map[op], SET_BITS( reg_20[op], 0b00000001, 4, val ) ); } // Turn Envelope Scaling on (1) or off (0)
    void regOpFrequencyMult( uint8_t op, uint8_t val ){ sendData(0x20+op_map[op], SET_BITS( reg_20[op], 0b00001111, 0, val ) ); } // Set frequyency multiple (0-7)

    void regOpLevel(         uint8_t op, uint8_t val ){ sendData(0x40+op_map[op], SET_BITS( reg_40[op], 0b00111111, 0, val ) ); } // Change level from loudest (0) to softest (64)
    void regOpLevelScaling(  uint8_t op, uint8_t val ){ sendData(0x40+op_map[op], SET_BITS( reg_40[op], 0b00000011, 6, val ) ); } // Change level scaling (0-3)

    void regOpAttack(        uint8_t op, uint8_t val ){ sendData(0x60+op_map[op], SET_BITS( reg_60[op], 0b00001111, 4, val ) ); } // Set Attack from longest (0) to quickest (15)
    void regOpDecay(         uint8_t op, uint8_t val ){ sendData(0x60+op_map[op], SET_BITS( reg_60[op], 0b00001111, 0, val ) ); } // Set Decay from quickest (0) to longest (15)
    void regOpSustain(       uint8_t op, uint8_t val ){ sendData(0x80+op_map[op], SET_BITS( reg_80[op], 0b00001111, 4, val ) ); } // Set Sustain Level from Loudest (0) to quietest (15)
    void regOpRelease(       uint8_t op, uint8_t val ){ sendData(0x80+op_map[op], SET_BITS( reg_80[op], 0b00001111, 0, val ) ); } // Set Attack from longest (0) to quickest (15)
    void regOpWaveForm(      uint8_t op, uint8_t val ){ sendData(0xE0+op_map[op], SET_BITS( reg_E0[op], 0b00000011, 0, val ) ); } // Set Wave to Sine (0), Half Sine (1), ABS Sine (2), Quarter Sine (3)

};


#endif  // YM3812_H
