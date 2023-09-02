# YM3812-Module
This code repository parallels a series of articles about a YM3812 module written on thingsmadesimple.com

With each new article, the functionality of the module will grow, and the associated code and schematics will be available here.

<hr/>

## Current Articles

### <a href='https://www.thingsmadesimple.com/2022/11/28/ym3812-part-1-register-basics/'>YM3812 Part 1: Register Basics</a>
Focuses on FM Synthesis basics, the YM3812's register map and electrically controlling the chip's pins.

### <a href='https://thingsmadesimple.com/2023/01/02/ym3812-part-2-face-reveal/'>YM3812 Part 2: FACE Reveal</a>
Introduces a basic implementation of the YM3812 circuit controlled by an AVR128DA28 microcontroller that we will build on in future articles. Think of it as a "Hello World" starting point that ensure the circuit works and that we can upload code to the microcontroller.

### <a href='https://www.thingsmadesimple.com/2023/01/21/ym3812-part-3-behind-the-code/'>YM3812 Part 3: Behind the Code</a>
Walks through the implementation of the YM3812 class used in Part 2 and describes how each of the register manipulation functions work.

### <a href='https://www.thingsmadesimple.com/2023/02/11/ym3812-part-4-midi-journey/'>YM3812 Part 4: MIDI Journey</a>
Adds a MIDI input to our schematic, and discusses how to translate MIDI note numbers into frequencies the YM3812 understands

### <a href='https://www.thingsmadesimple.com/2023/03/05/ym3812-part-5-midi-polyphony/'>YM3812 Part 5: MIDI Polyphony</a>
Adds Polyphony to our breadboard module, walking through a couple of algorithms in the process.

### <a href='https://www.thingsmadesimple.com/2023/04/29/ym3812-part-6-patches/'>YM3812 Part 6: Patches</a>
Revise the YM3812 class implementation to support more generic instrument definition

### <a href='https://www.thingsmadesimple.com/2023/05/13/ym3812-part-7-percussive-maintenance/'>YM3812 Part 7: Percussive Maintenance</a>
Add General MIDI drum support by linking different drum sounds to different notes on the keyboard

### <a href='https://www.thingsmadesimple.com/2023/05/30/ym3812-part-8-velocity-sensitivity/'>YM3812 Part 8: Velocity Sensitivity</a>
Add touch sensitivity to the module by selectively scaling the right operator levels

### <a href='https://www.thingsmadesimple.com/2023/07/28/ym3812-part-9-synth-card/'>YM3812 Part 9: SynthCard</a>
Turn that breadboard into a business card sized PCB!

### <a href='https://www.thingsmadesimple.com/2023/09/02/ym3812-part-10-pitch-bend/'>YM3812 Part 10: Pitch Bend</a>
Add pitch bend capabilities and shred on a YM3812




<hr/>

## Article Backlog:

### YM3812 Part ?: OPL2 in STEREO!
Add another YM3812 chip and some stereo effects for panning and detuning

### YM3812 Part ?: Adding a menu to swap patches
Add a TFT Screen and a Rotary Encoder so we can adjust patch assignments to MIDI channels

### YM3812 Part ?: Adjust Patch Properties
Add buttons and a menu system that lets us adjust patch settings individually

### YM3812 Part ?: Visualizing 2-OP envelopes
Add a graphical visualization that helps us interpret patch settings

### YM3812 Part ?: Virtual 4-OP Voices
Add support for virtual four-operator voices


and more...
