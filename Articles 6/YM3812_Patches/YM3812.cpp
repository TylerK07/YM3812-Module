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

Description:
This library wraps Yamaha's YM3812 sound processor with functions that allow for
direct manipulation of the chip's registers so you can build cool things like EuroRack modules!

*/

#include "Arduino.h"
#include <SPI.h>
#include "YM3812.h"


//Port Bits defined for control bus:
#define YM_WR    0b00000001                                                    // Pin 0, Port D - Write
#define YM_A0    0b00000010                                                    // Pin 1, Port D - Differentiates between address and data
#define YM_IC    0b00000100                                                    // Pin 2, Port D - Reset Pin
#define YM_LATCH 0b00001000                                                    // Pin 3, Port D - Output Latch
#define YM_CS    0b00010000                                                    // Pin 4, Port D - Left YM3812 Chip Select

// Optional debug light shows when information gets written to the YM3812
#define DATA_LED 0b10000000                                                    // We can use this to see activity when data is being sent

/**************
* Constructor *
**************/

YM3812::YM3812(){                                                              // Constructor
}


/************************
* Patch Functions       *
************************/

void YM3812::patchNoteOn( PatchArr &patch, uint8_t midiNote ){
  last_channel = chGetNext();
  channel_states[ last_channel ].pPatch  = &patch;                             // Store pointer to the patch
  channel_states[ last_channel ].midi_note  = midiNote;                        // Store midi note associated with the channel
  channel_states[ last_channel ].note_state = true;                            // Indicate that the note is turned on
  channel_states[ last_channel ].state_changed = millis();                     // save the time that the note was turned on
  chPlayNote( last_channel, midiNote );                                        // Play the note on the correct YM3812 channel
}


void YM3812::patchNoteOff( PatchArr &patch, uint8_t midiNote ){
  for( uint8_t ch = 0; ch<num_channels; ch++ ){
    if( channel_states[ch].pPatch == &patch ){
      if( channel_states[ch].midi_note == midiNote ){
        channel_states[ ch ].state_changed = millis();                         // Save the time that the state changed
        channel_states[ ch ].note_state = false;                               // Indicate that the note is currently off
        regKeyOn( ch, 0 );                                                     // Turn off any channels associated with the midiNote
      }
    }
  }
}

void YM3812::patchAllOff( PatchArr &patch ){
  for( uint8_t ch = 0; ch<num_channels; ch++ ){
    if( channel_states[ch].pPatch == &patch ){
      channel_states[ ch ].state_changed = millis();                           // Save the time that the state changed
      channel_states[ ch ].note_state = false;                                 // Indicate that the note is currently off
      regKeyOn( ch, 0 );                                                       // Turn off any channels associated with the midiNote
    }
  }
}

void YM3812::patchUpdate( PatchArr &patch ){                                   // Update the patch data of any active channels assocaited with the patch
  for( byte ch = 0; ch < num_channels; ch++ ){                                 // Loop through each channel
    if( channel_states[ch].pPatch == &patch ) chSendPatch( ch, patch );        // If the channel uses the patch, update the patch data on the chip
  }
}


/************************
* Channel Functions     *
************************/

// chSendPatch Theory of Operation:
// This function is the heart of the class, and without too many modifications can be updated to work with any of the
// Yamaha synthesis chips. The function accepts a generalized patch array with values in order according to the
// indicies outlined in YMDefs.h. Because all values in the array are between 0 and 127, this function scales
// those values to the correct bit depth, combines them with other values to produce the correct combinations for the register
// map of the YM3812 and then sends them to the sound chip.

void YM3812::chSendPatch( byte ch, PatchArr &patch ){
  byte mem_offset, patch_offset;

  //Channel Settings
  sendData( 0xC0+ch,  ((patch[PATCH_FEEDBACK]>>4)<<1) |                        // Compose the feedback and Algorithm values
                       (patch[PATCH_ALGORITHM]>>6)    );                       // into a single byte and send to YM3812

  for( uint8_t op = 0; op<2; op++ ){
    mem_offset = op_map[channel_map[ch] + op*3];                               // Determine memory offset for slot 1 for the channel
    patch_offset = PATCH_OP_SETTINGS * op;                                     // Determine operator's index offset for the patch properties 

    sendData( 0x20+mem_offset,      ((patch[patch_offset + PATCH_TREMOLO        ]>>6)<<7) | // Compose Tremolo, vibrato, percussive envelope
                                    ((patch[patch_offset + PATCH_VIBRATO        ]>>6)<<6) | // envelope scaling flags along with frequency
                                    ((patch[patch_offset + PATCH_PERCUSSIVE_ENV ]>>6)<<5) | // multiple into a single 8-bit register value
                                    ((patch[patch_offset + PATCH_ENV_SCALING    ]>>6)<<4) | // and send to the YM3812
                                    ((patch[patch_offset + PATCH_FREQUENCY_MULT ]>>3)<<0) );

    sendData( 0x40+mem_offset,      ((patch[patch_offset + PATCH_LEVEL_SCALING ]>>5)<<6) |  // Compose level scaling and level value for the left
                                     (patch[patch_offset + PATCH_LEVEL         ]>>1)     ); // Output channel and send to the LEFT YM3812
    sendData( 0x60+mem_offset,      ((patch[patch_offset + PATCH_ATTACK        ]>>3)<<4) |  // Compose attack and decay settings into a single
                                    ((patch[patch_offset + PATCH_DECAY         ]>>3)<<0) ); // 8-bit register and send to YM3812
    sendData( 0x80+mem_offset, (0xF-((patch[patch_offset + PATCH_SUSTAIN_LEVEL ]>>3)<<4)) | // Compose sustain level (inverted) and release rate settings into a single
                                    ((patch[patch_offset + PATCH_RELEASE_RATE  ]>>3)<<0) ); // 8-bit register and send to YM3812
    sendData( 0xE0+mem_offset,      ((patch[patch_offset + PATCH_WAVEFORM      ]>>5)<<0) ); // Send waveform register to YM3812
  }

}

uint8_t YM3812::chGetNext(){
  uint8_t on_channel = 0xFF;                                                   // The channel that has been on the longest 
  uint8_t off_channel = 0xFF;                                                  // The channel that has been OFF the longest
  unsigned long oldest_on_time = millis(); 
  unsigned long oldest_off_time = millis(); 

  for( uint8_t ch=0; ch < num_channels; ch++ ){                                // Loop through all of the channels
    if( channel_states[ch].note_state ){                                       // If the note is turned on...
      if( channel_states[ch].state_changed < oldest_on_time ){                 // Is this the longest running note?
        oldest_on_time = channel_states[ch].state_changed;                     // save the current time
        on_channel = ch;                                                       // save this as our longest running channel
      } 
    } else {                                                                   // If the note is turned off...
      if( channel_states[ch].state_changed < oldest_off_time ){                // Is this the longest turned off note?
        oldest_off_time = channel_states[ch].state_changed;                    // save the current time
        off_channel = ch;                                                      // save this as our longest off channel
      } 
    }
  }

  if( off_channel < 0xFF ){                                                    // If it isn't FF, then we changed... so at least one channel is currently off.
    return( off_channel );                                                     // Return the channel that has been off the longest
  }
  return( on_channel );                                                        // Return the channel that has been ON the longest
}


void YM3812::chPlayNote( uint8_t ch, uint8_t midiNote ){                       // Play a note on channel ch with pitch midiNote
  regKeyOn( ch, 0 );                                                           // Turn off the channel if it is on
  if( midiNote > 114 ) return;                                                 // Note is out of range, so return

  chSendPatch( ch, *channel_states[ch].pPatch );                               // Send the patch to the YM3812

  if( midiNote < 19 ){                                                         // Note is at the bottom of range
    regFrqBlock( ch, 0 );                                                      // So use block zero
    regFrqFnum( ch, FRQ_SCALE[midiNote] );                                     // and pull the value from the s
  } else {                                                                     // If in the normal range
    regFrqBlock( ch, (midiNote - 19) / 12 );                                   // Increment block every 12 notes from zero to 7
    regFrqFnum( ch, FRQ_SCALE[((midiNote - 19) % 12) + 19] );                  // Increment F-Num from 19 through 30 over and over
  }

  regKeyOn( ch, 1 );                                                           // Turn the channel back on
}



/********************************
* Processor Control Functions   *
********************************/

void YM3812::reset(){
  PORTD.DIRSET = YM_IC | YM_A0 | YM_WR | YM_LATCH | YM_CS | DATA_LED;          // Set control lines high to output mode
  PORTD.OUTCLR = YM_LATCH;                                                     // Set the latch low to start
  PORTD.OUTSET = YM_WR | YM_CS;                                                // Set chip select and write lines high
  
  SPI.begin();
  //Hard Reset the YM3812
  PORTD.OUTCLR = YM_IC; delay(10);                                             // Hard Reset the processor by bringing Initialize / Clear line low
  PORTD.OUTSET = YM_IC; delay(10);                                             // Complete process by bringing line high and allowing a short moment to reset

  PORTD.OUTCLR = DATA_LED;                                                     // Turn the data LED off


  //Clear all of our register caches
  reg_01 = reg_08 = reg_BD = 0;                                                // Clear out global settings
  for( uint8_t ch = 0; ch < YM3812_NUM_CHANNELS; ch++ ){                       // Loop through all of the channels
    reg_A0[ch] = reg_B0[ch] = 0;                                               // Set all register info to zero
  }
  regWaveset( 1 );                                                             // Enable all wave forms (not just sine waves)
}

void YM3812::sendData( uint8_t reg, uint8_t val ){
  PORTD.OUTSET = DATA_LED;

  PORTD.OUTCLR = YM_CS;                                                        // Enable the chip
  PORTD.OUTCLR = YM_A0;                                                        // Put chip into register select mode

  SPI.transfer(reg);                                                           // Put register location onto the data bus through SPI port
  PORTD.OUTSET = YM_LATCH;                                                     // Latch register location into the 74HC595
  delayMicroseconds(1);                                                        // wait a tic.
  PORTD.OUTCLR = YM_LATCH;                                                     // Bring latch low now that the location is latched

  PORTD.OUTCLR = YM_WR;                                                        // Bring write low to begin the write cycle
  delayMicroseconds(10);                                                       // Delay so chip completes write cycle
  PORTD.OUTSET = YM_WR;                                                        // Bring write high
  delayMicroseconds(10);                                                       // Delay until the chip is ready to continue

  PORTD.OUTSET = YM_A0;                                                        // Put chip into data write mode

  SPI.transfer(val);                                                           // Put value onto the data bus through SPI port
  PORTD.OUTSET = YM_LATCH;                                                     // Latch the value into the 74HC595
  delayMicroseconds(1);                                                        // wait a tic.
  PORTD.OUTCLR = YM_LATCH;                                                     // Bring latch low now that the value is latched

  PORTD.OUTCLR = YM_WR;                                                        // Bring write low to begin the write cycle
  delayMicroseconds(10);                                                       // Delay so chip completes write cycle
  PORTD.OUTSET = YM_WR;                                                        // Bring write high
  delayMicroseconds(10);                                                       // Delay until the chip is ready to continue

  PORTD.OUTSET = YM_CS;                                                        // Bring Chip Select high to disable the YM3812

  PORTD.OUTCLR = DATA_LED;

}
