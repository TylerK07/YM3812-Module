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
This file contains the code for a simplified breadboard YM3812 circuit. This video
pairs with an article and YouTube video.


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
     Not Connected | PD6       RX | Not Connected
     Not Connected | PD7       TX | Not Connected
               +5V | AVCC     GND | Ground
                   ----------------
*/

#include "Arduino.h"
#include "YM3812.h"
#include <SPI.h>


//----------------------------------------- SETUP & INITIALIZATION FUNCTIONS -----------------------------------------//

/*******************************************
 * Sound Processor Definition              *
 *******************************************/

YM3812  PROC_YM3812;                                                           // Instantiate the YM3812 library


/*******************************************
 * Primary Setup Function                  *
 *******************************************/

void setup(void) {
  PROC_YM3812.reset();

  uint8_t op1_index, op2_index;

  //Set up a patch
  for( uint8_t ch=0; ch<YM3812_NUM_CHANNELS; ch++){   // Use the same patch for all channels
    op1_index = PROC_YM3812.channel_map[ch];
    op2_index = op1_index + 3;                        // Always 3 higher

    //Channel settings
    PROC_YM3812.regChAlgorithm(  ch, 0x1 ); // Algorithm (Addative synthesis)
    PROC_YM3812.regChFeedback(   ch, 0x0 ); // Feedback

    //Operator 1's settings
    PROC_YM3812.regOpAttack(   op1_index, 0xB );
    PROC_YM3812.regOpDecay(    op1_index, 0x6 );
    PROC_YM3812.regOpSustain(  op1_index, 0xA );
    PROC_YM3812.regOpRelease(  op1_index, 0x2 );
    PROC_YM3812.regOpLevel(    op1_index, 0x0 );  // 0 - loudest, 64 (0x40) is softest
    PROC_YM3812.regOpWaveForm( op1_index, 0x1 );

    //Operator 2's settings
    PROC_YM3812.regOpAttack(   op2_index, 0xB );
    PROC_YM3812.regOpDecay(    op2_index, 0x6 );
    PROC_YM3812.regOpSustain(  op2_index, 0xA );
    PROC_YM3812.regOpRelease(  op2_index, 0x2 );
    PROC_YM3812.regOpLevel(    op2_index, 0x0 );  // 0 - loudest, 64 (0x40) is softest
    PROC_YM3812.regOpWaveForm( op2_index, 0x1 );
    
  }
}



//-------------------------------------------------- LOOP FUNCTIONS --------------------------------------------------//

/*******************************************
 * Loop Function                          *
 *******************************************/

uint8_t count = 0;

void loop() {                                                                  // Main Loop Function

  uint8_t op1_index, op2_index;

  //Turn all of the keys off
  PROC_YM3812.regKeyOn(  0, false ); //Turn Channel 0 off
  PROC_YM3812.regKeyOn(  1, false ); //Turn Channel 1 off
  PROC_YM3812.regKeyOn(  2, false ); //Turn Channel 2 off
  PROC_YM3812.regKeyOn(  3, false ); //Turn Channel 3 off

  //Set all of the octaves of the keys
  PROC_YM3812.regFrqBlock( 0, 4); //Fourth Octave
  PROC_YM3812.regFrqBlock( 1, 4); //Fourth Octave
  PROC_YM3812.regFrqBlock( 2, 4); //Fourth Octave
  PROC_YM3812.regFrqBlock( 3, 4); //Fourth Octave

  //Set the frequency of each note:
  PROC_YM3812.regFrqFnum( 0, 0x1C9 ); // F
  PROC_YM3812.regFrqFnum( 1, 0x240 ); // A
  PROC_YM3812.regFrqFnum( 2, 0x2AD ); // C
  PROC_YM3812.regFrqFnum( 3, 0x360 ); // E

  //Turn on all of the notes one at a time
  PROC_YM3812.regKeyOn(  0, true );
  delay(100);
  PROC_YM3812.regKeyOn(  1, true );
  delay(100);
  PROC_YM3812.regKeyOn(  2, true );
  delay(100);
  PROC_YM3812.regKeyOn(  3, true );
  delay(1000);

  //Turn all of the keys off
  PROC_YM3812.regKeyOn(  0, false ); //Turn Channel 0 off
  PROC_YM3812.regKeyOn(  1, false ); //Turn Channel 1 off
  PROC_YM3812.regKeyOn(  2, false ); //Turn Channel 2 off
  PROC_YM3812.regKeyOn(  3, false ); //Turn Channel 3 off

  //Turn all of the keys on at the same time (like playing a chord)
  PROC_YM3812.regKeyOn(  0, true );
  PROC_YM3812.regKeyOn(  1, true );
  PROC_YM3812.regKeyOn(  2, true );
  PROC_YM3812.regKeyOn(  3, true );
  delay(1000);

  count = (count + 4) % 16;

  //Update our patch settings
  for( uint8_t ch=0; ch<YM3812_NUM_CHANNELS; ch++){   // Use the same patch for all channels
    op1_index = PROC_YM3812.channel_map[ch];
    op2_index = op1_index + 3;                        // Always 3 higher

    PROC_YM3812.regOpSustain(  op1_index, 16 - count );
    PROC_YM3812.regOpSustain(  op2_index, 16 - count );
  
  }
  
}
