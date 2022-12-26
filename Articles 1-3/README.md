# YM3812_Breadboard
This code repository parallels a series of articles about a YM3812 module written on thingsmadesimple.com

With each new article, the functionality of the module will grow, and the associated code and schematics will be available here.

<hr/>

## Current Articles

### <a href='https://www.thingsmadesimple.com/2022/11/28/ym3812-part-1-register-basics/'>YM3812 Part 1: Register Basics</a> 
Focuses on FM Synthesis basics, the YM3812's register map and electrically controlling the chip's pins.

### YM3812 Part 2: FACE Reveal
Introduces a basic implementation of the YM3812 circuit controlled by an AVR128DA28 microcontroller that we will build on in future articles. Think of it as a "Hello World" starting point that ensure the circuit works and that we can upload code to the microcontroller.

<hr/>

## Articles Coming Soon

### YM3812 Part 3: Getting Classy
Walks through the implementation of the YM3812 class used in Part 2 and describes how each of the register manipulation functions work.

### YM3812 Part 4: MIDI Control
Adds a MIDI input to our schematic, and discusses how to translate MIDI note numbers into frequencies the YM3812 understands

### YM3812 Part 5: MIDI Poliphony
Adds poliphony to the module, automatically choosing the best channel on the YM3812 to use.

### YM3812 Part 6: Instrument Support
Revise the YM3812 class implementation to support more generic instrument definition

and more...
