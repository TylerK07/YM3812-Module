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
now with primitive polyphonic MIDI functionality. This code pairs with
an article and YouTube video where we implement an aglogorithm that rotates
through each of the YM3812 channels to play multiple notes at the same time.


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
#include <MIDI.h>
#include <SPI.h>


//----------------------------------------- SETUP & INITIALIZATION FUNCTIONS -----------------------------------------//

/*******************************************
 * Sound Processor Definition              *
 *******************************************/

YM3812  PROC_YM3812;                                                           // Instantiate the YM3812 library



/*******************************************
 * MIDI Definition                         *
 *******************************************/

MIDI_CREATE_INSTANCE( HardwareSerial, Serial2, MIDI );                         // Create an instance of MIDI library running on Serial port 2

void handleNoteOn( byte channel, byte midiNote, byte velocity ){               // Handle MIDI Note On Events
  PROC_YM3812.noteOn( midiNote );                                              // Play midiNote on YM3812
}

void handleNoteOff( byte channel, byte midiNote, byte velocity ){              // Handle MIDI Note Off Events
  PROC_YM3812.noteOff( midiNote );                                             // Turn off midi note
}


/*******************************************
 * Primary Setup Function                  *
 *******************************************/

void setup(void) {
  PROC_YM3812.reset();

  //MIDI Setup
  MIDI.setHandleNoteOn(  handleNoteOn );                                       // Setup Note-on Handler function
  MIDI.setHandleNoteOff( handleNoteOff );                                      // Setup Note-off Handler function

  MIDI.begin();                                                                // Start listening for incoming MIDI


  uint8_t op1_index, op2_index;

  //Set up a patch
  for( uint8_t ch=0; ch<YM3812_NUM_CHANNELS; ch++){                            // Use the same patch for all channels
    op1_index = PROC_YM3812.channel_map[ch];
    op2_index = op1_index + 3;                                                 // Always 3 higher

    //Channel settings
    PROC_YM3812.regChAlgorithm(  ch, 0x1 );                                    // Algorithm (Addative synthesis)
    PROC_YM3812.regChFeedback(   ch, 0x0 );                                    // Feedback

    //Operator 1's settings
    PROC_YM3812.regOpAttack(   op1_index, 0xB );
    PROC_YM3812.regOpDecay(    op1_index, 0x6 );
    PROC_YM3812.regOpSustain(  op1_index, 0x2 );
    PROC_YM3812.regOpRelease(  op1_index, 0xA );
    PROC_YM3812.regOpLevel(    op1_index, 0x0 );                               // 0 - loudest, 64 (0x40) is softest
    PROC_YM3812.regOpWaveForm( op1_index, 0x1 );
    PROC_YM3812.regOpPercussiveEnv( op1_index, 0x1 );

    //Operator 2's settings
    PROC_YM3812.regOpAttack(   op2_index, 0xB );
    PROC_YM3812.regOpDecay(    op2_index, 0x6 );
    PROC_YM3812.regOpSustain(  op2_index, 0x2 );
    PROC_YM3812.regOpRelease(  op2_index, 0xA );
    PROC_YM3812.regOpLevel(    op2_index, 0x0 );                               // 0 - loudest, 64 (0x40) is softest
    PROC_YM3812.regOpWaveForm( op2_index, 0x1 );
    PROC_YM3812.regOpPercussiveEnv( op2_index, 0x1 );

  }
}



//-------------------------------------------------- LOOP FUNCTIONS --------------------------------------------------//

/*******************************************
 * Loop Function                          *
 *******************************************/

void loop() {                                                                  // Main Loop Function

  while( MIDI.read(0) ){}                                                      // Read all incoming data on all MIDI Channels

}
