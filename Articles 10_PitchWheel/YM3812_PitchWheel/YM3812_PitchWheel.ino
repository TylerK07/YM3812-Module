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


YM3182 OPL2 EuroRack Module source code designed to run on the AVR128DA28.
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

--- Description ---
This file contains the code for a simplified breadboard YM3812 circuit,
now with patch functionality. This code pairs with an article and YouTube video 
where we rework the YM3812 class implementation to support patches.


AVR128DA28 Pinout:
                   -------__-------
     Not Connected | PA7      PA6 | SPI Bus Clock 74HC595
     Not Connected | PC0      PA5 | Not Connected
     Not Connected | PC1      PA4 | SPI Bus MOSI 74HC595
     Not Connected | PC2      PA3 | Not Connected
     Not Connected | PC3      PA2 | Not Connected
      YM3812 Write | PD0      PA1 | Not Connected
         YM3812 A0 | PD1      PA0 | Not Connected
         YM3812 IC | PD2      GND | Ground
74HC595 Data Latch | PD3      VCC | +5V
YM3812 Chip Select | PD4     UPDI | UPDI Port
     Not Connected | PD5    RESET | Reset Button
     Not Connected | PD6       RX | MIDI In (Serial2)
     Not Connected | PD7       TX | Not Connected
               +5V | AVCC     GND | Ground
                   ----------------
*/

#include "Arduino.h"
#include "YM3812.h"
#include "YMDefs.h"
#include "instruments.h"
#include <MIDI.h>
#include <SPI.h>


//----------------------------------------- SETUP & INITIALIZATION FUNCTIONS -----------------------------------------//

/*******************************************
 * Sound Processor Definition              *
 *******************************************/

YM3812  PROC_YM3812;                                                           // Instantiate the YM3812 library


/*******************************************
 * Instrument Definitions                  *
 *******************************************/
#define  MAX_INSTRUMENTS   16                                                  // Total MIDI instruments to support (one per midi channel)

uint8_t  inst_patch_index[ MAX_INSTRUMENTS ];                                  // Contains index of the patch used for each midi instrument / channel
PatchArr inst_patch_data[  MAX_INSTRUMENTS ];                                  // Contains one patch per instrument

void loadPatchFromProgMem( byte instIndex, byte patchIndex ){                  // Load patch data from program memory into inst_patch_data array
  for( byte i=0; i<PATCH_SIZE; i++ ){                                          // Loop through instrument data
    inst_patch_data[instIndex][i] = pgm_read_byte_near( patches[patchIndex]+i ); // Copy each byte into ram_data
  }
}

#define  DRUM_CHANNEL      10                                                  // The MIDI channel to use for drums
#define  FIRST_DRUM_NOTE   35                                                  // Conforming to GM patch standard, notes [35-81]

PatchArr drum_patch_data[ NUM_DRUMS ];                                         // Patch data for the drums
uint8_t  drum_patch_index[ NUM_DRUMS ];                                        // Array that translates between drum index and drum patchIndex (allows reassignment)

void loadDrumPatchFromProgMem( byte trackIndex, byte patchIndex ){             // Load a patch from program memory into drum_patach_data
  for( byte i=0; i<PATCH_SIZE; i++ ){                                          // Loop through instrument data
    drum_patch_data[trackIndex][i] = pgm_read_byte_near( patches[patchIndex+NUM_MELODIC] + i ); // Copy each byte into ram_data
  }                                                                            // Worth noting that drum patches are stored after instrument
}                                                                              // patches, hence why we add NUM_PATCHES

uint16_t inst_pitch_bend[ MAX_INSTRUMENTS ];                                    // holds current pitch bend value


/*******************************************
 * MIDI Definition                         *
 *******************************************/

MIDI_CREATE_INSTANCE( HardwareSerial, Serial2, MIDI );                         // Create an instance of MIDI library running on Serial port 2

#define RPNMSB  101                                                            // Command ID for RPN Command's Most Significant Byte
#define RPNLSB  100                                                            // Command ID for RPN Command's Least Significant Byte
#define DATAMSB 6                                                              // Command ID for RPN Value's Most Significant Byte
#define DATALSB 38                                                             // Command ID for RPN Value's Least Significant Byte

uint16_t RPN_command = 0x7F7F;                                                 // Holds the current RPN command while bytes are coming in


void handleNoteOn( byte channel, byte midiNote, byte velocity ){               // Handle MIDI Note On Events
  uint8_t ch = channel - 1;                                                    // Convert to 0-indexed from MIDI's 1-indexed channel nonsense
  uint8_t drumIndex;

  if( DRUM_CHANNEL == channel ){                                               // See if the note being played is on the drum channel
    drumIndex = (midiNote - FIRST_DRUM_NOTE) % NUM_DRUMS;                      // Calculate the index of the drum based on the midi note
    PROC_YM3812.patchNoteOn( drum_patch_data[drumIndex], velocity );           // Play the drum patch
  } else {                                                                     // If not a drum channel
    PROC_YM3812.patchNoteOn( inst_patch_data[ch], midiNote, velocity, inst_pitch_bend[ch]); // Pass the patch information for the channel and note to the YM3812
  }

}

void handleNoteOff( byte channel, byte midiNote, byte velocity ){              // Handle MIDI Note Off Events
  uint8_t ch = channel - 1;                                                    // Convert to 0-indexed from MIDI's 1-indexed channel nonsense
  uint8_t drumIndex;

  if( DRUM_CHANNEL == channel ){                                               // See if the note being played is on the drum channel
    drumIndex = (midiNote - FIRST_DRUM_NOTE) % NUM_DRUMS;                      // Calculate the index of the drum based on the midi note
    PROC_YM3812.patchNoteOff( drum_patch_data[drumIndex] );                    // Turn off the drum patch
  } else {                                                                     // If not a drum channel
    PROC_YM3812.patchNoteOff( inst_patch_data[ch], midiNote );                 // Pass the patch information for the channel and note to the YM3812
  }

}

void handleProgramChange( byte channel, byte patchIndex ){
  uint8_t ch = channel-1;                                                      // Convert to 0-indexed from MIDI's 1-indexed channel nonsense
  inst_patch_index[ch] = patchIndex;                                           // Store the patch index
  loadPatchFromProgMem( ch, inst_patch_index[ch] );                            // Load the patch from progmem into regular memory
}

void handlePitchBend( byte channel, int16_t pitchBend ){
  uint8_t ch = channel-1;                                                      // Convert to 0-indexed from MIDI's 1-indexed channel nonsense
  inst_pitch_bend[ch] = pitchBend + 0x2000;                                    // Shift pitchBend value by 0x2000 so it is always positive (and runs 0 ... 0x3FFF)
  PROC_YM3812.patchPitchBend( inst_patch_data[ch], inst_pitch_bend[ch]);       // Update pitch of any currently playing notes
}

void handleControlChange( byte channel, byte command, byte val ){              // Respond to ControlChange commands so we can pick out RPN commands
  switch( command ){
    case RPNMSB:  RPN_command = (RPN_command & 0x007F) | (val << 7);    break; // Capture the MSB value in bits 7 through 13 of RPN_command
    case RPNLSB:  RPN_command = (RPN_command & 0xFF80) | val;           break; // Capture the LSB value in bits 0 through 6 of RPN_command
    case DATAMSB: if( RPN_command==0 ) PROC_YM3812.setBendRange( val ); break; // Check if command is pitch bend sensitivity and then send value to YM3812 library
    case DATALSB:                                                       break; // LSB sets range to less than a semitone. We can ignore that
  }
}



/*******************************************
 * Primary Setup Function                  *
 *******************************************/

void setup(void) {

  // Initialize Patches
  for( byte i=0; i<MAX_INSTRUMENTS; i++ ){                                     // Load patch data from channel_patches into the
    inst_patch_index[i] = i;                                                   // By default, use a different patch for each midi channel
    loadPatchFromProgMem( i, inst_patch_index[i] );                            // instruments array (inst_patch_data)
    inst_pitch_bend[i] = 0x2000;                                               // Set pitch bend to middle value by default
  }
  for( byte i=0; i<NUM_DRUMS; i++ ){                                           // Loop through all of the drum patches and by default,
    drum_patch_index[i] = i;                                                   // Map each drum to another patch, wrap around if more notes than patches
    loadDrumPatchFromProgMem( i, drum_patch_index[i] );                        // Load the drum patches into memory for each drum track
  }

  PROC_YM3812.reset();

  //MIDI Setup
  MIDI.setHandleNoteOn(  handleNoteOn );                                       // Setup Note-on Handler function
  MIDI.setHandleNoteOff( handleNoteOff );                                      // Setup Note-off Handler function
  MIDI.setHandleProgramChange( handleProgramChange );                          // Setup Program Change Handler function
  MIDI.setHandlePitchBend( handlePitchBend );
  MIDI.setHandleControlChange( handleControlChange );

  MIDI.begin();                                                                // Start listening for incoming MIDI

}



//-------------------------------------------------- LOOP FUNCTIONS --------------------------------------------------//

/*******************************************
 * Loop Function                          *
 *******************************************/

void loop() {                                                                  // Main Loop Function

  while( MIDI.read(0) ){}                                                      // Read all incoming data on all MIDI Channels

}
