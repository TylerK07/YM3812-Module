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


--- Theory of Operation: ---

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

#define DATA_LED 0b10000000                                                    // We can use this to see activity when data is being sent

/********************************
* Processor Control Functions   *
********************************/
YM3812::YM3812(){                                                              // Configure hardware to commuinicate with the YM3812
  PORTD.DIRSET = YM_IC | YM_A0 | YM_WR | YM_LATCH | YM_CS;                     // Set control lines high to output mode
  PORTD.OUTCLR = YM_LATCH;                                                     // Set the latch low to start
  PORTD.OUTSET = YM_WR | YM_CS;                                                // Set chip select and write lines high

  PORTD.DIRSET = DATA_LED;
  PORTD.OUTCLR = DATA_LED;
}

void YM3812::reset(){
  SPI.begin();
  //Hard Reset the YM3812
  PORTD.OUTCLR = YM_IC; delay(10);                                             // Hard Reset the processor by bringing Initialize / Clear line low
  PORTD.OUTSET = YM_IC; delay(10);                                             // Complete process by bringing line high and allowing a short moment to reset

  //Clear all of our register caches
  reg_01 = reg_08 = reg_BD = 0;                                                // Clear out global settings
  for( uint8_t ch = 0; ch < YM3812_NUM_CHANNELS; ch++ ){                       // Loop through all of the channels
    reg_A0[ch] = reg_B0[ch] = reg_C0[ch] = 0;                                  // Set all register info to zero
  }
  for( uint8_t op = 0; op < YM3812_NUM_OPERATORS; op++ ){                      // Loop through all of the operators
    reg_20[op] = reg_40[op] = reg_60[op] = reg_80[op] = reg_E0[op] = 0;        // Set all register info to zero
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

/************************
* Channel Functions     *
************************/


/***********************
* Register Functions   *
***********************/
