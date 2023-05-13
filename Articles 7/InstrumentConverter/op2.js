const DB = require('./lib/jsdb.js');
const FS = require('fs');
const crypto = require('crypto');

const PATCH_GEN_SETTINGS    = 10 //Number of general and channel settings
const PATCH_OP_SETTINGS     = 17 //Number of operator settings
const PATCH_SIZE            = PATCH_GEN_SETTINGS + PATCH_OP_SETTINGS * 4 // Total Patch Size

// Instrument Level / Virtual
const PATCH_PROC_CONF       =  0 // Indicates the type of processor this patch connects to
const PATCH_NOTE_NUMBER     =  1 // Indicates the pitch to use when playing as a drum sound
const PATCH_PEG_INIT_LEVEL  =  2 // Pitch Envelope Attack (Virtual setting - none of the YM chips have this by default)
const PATCH_PEG_ATTACK      =  3 // Pitch Envelope Attack (Virtual setting - none of the YM chips have this by default)
const PATCH_PEG_RELEASE     =  4 // Pitch Envelope Release (Virtual setting - none of the YM chips have this by default)
const PATCH_PEG_REL_LEVEL   =  5 // Pitch Envelope Attack (Virtual setting - none of the YM chips have this by default)

// Channel Level
const PATCH_FEEDBACK        =  6 // Used by: YM3526 | YM3812 | YMF262 | YM2413 | YM2151 |        | YM2612
const PATCH_ALGORITHM       =  7 // Used by: YM3526 | YM3812 | YMF262 |        | YM2151 |        | YM2612
const PATCH_TREMOLO_SENS    =  8 // Used by:        |        |        |        | YM2151 |        | YM2612
const PATCH_VIBRATO_SENS    =  9 // Used by:        |        |        |        | YM2151 |        | YM2612

// Operator Level
const PATCH_WAVEFORM        = 10 // Used by:        | YM3812 | YMF262 | YM2413 |        |        |
const PATCH_LEVEL           = 11 // Used by: YM3526 | YM3812 | YMF262 | YM2413 | YM2151 | YM2149 | YM2612
const PATCH_LEVEL_SCALING   = 12 // Used by: YM3526 | YM3812 | YMF262 | YM2413 |        |        |
const PATCH_ENV_SCALING     = 13 // Used by: YM3526 | YM3812 | YMF262 | YM2413 | YM2151 |        | YM2612
const PATCH_PERCUSSIVE_ENV  = 14 // Used by: YM3526 | YM3812 | YMF262 | YM2413 |        |        |
const PATCH_ATTACK          = 15 // Used by: YM3526 | YM3812 | YMF262 | YM2413 | YM2151 |        | YM2612
const PATCH_DECAY           = 16 // Used by: YM3526 | YM3812 | YMF262 | YM2413 | YM2151 |        | YM2612
const PATCH_SUSTAIN_LEVEL   = 17 // Used by: YM3526 | YM3812 | YMF262 | YM2413 | YM2151 |        | YM2612
const PATCH_SUSTAIN_DECAY   = 18 // Used by:        |        |        |        | YM2151 |        | YM2612
const PATCH_RELEASE_RATE    = 19 // Used by: YM3526 | YM3812 | YMF262 | YM2413 | YM2151 |        | YM2612
const PATCH_TREMOLO         = 20 // Used by: YM3526 | YM3812 | YMF262 | YM2413 | YM2151 |        | YM2612
const PATCH_VIBRATO         = 21 // Used by: YM3526 | YM3812 | YMF262 | YM2413 |        |        |
const PATCH_FREQUENCY_MULT  = 22 // Used by: YM3526 | YM3812 | YMF262 | YM2413 | YM2151 |        | YM2612
const PATCH_DETUNE_FINE     = 23 // Used by:        |        |        |        | YM2151 | YM2149 | YM2612
const PATCH_DETUNE_GROSS    = 24 // Used by:        |        |        |        | YM2151 | YM2149 | YM2612
const PATCH_SSGENV_ENABLE   = 25 // Used by:        |        |        |        |        | YM2149 | YM2612
const PATCH_SSGENV_WAVEFORM = 26 // Used by:        |        |        |        |        |        | YM2612


// Processor Types
const PROC_ANYPROC       = 0x00  // Use any available sound processor
const PROC_YM3526        = 0x10  // YM3526 OPL  Supports: 1OP, 2OP, 2x2OP
const PROC_YM3812        = 0x20  // YM3812 OPL2 Supports: 1OP, 2OP, 2x2OP
const PROC_YMF262        = 0x30  // YMF262 OPL3 Supports: 1OP, 2OP, 2x2OP, 4OP
const PROC_YM2151        = 0x40  // YM2151 OPM  Supports: 4OP (emulated 2OP)
const PROC_YM2612        = 0x50  // YM2612 OPN2 Supports: 4OP
const PROC_YM2149        = 0x60  // YM2149 SSG  Supports: 1OP
const PROC_SN76489       = 0x70  // SN76489     Supports: 1OP

// Config Options
const CONF_1OP          = 0x1 // 1 operator voice
const CONF_2OP          = 0x2 // 2 operator voice
const CONF_2x2OP        = 0x3 // Dual 2 operator voices
const CONF_4OP          = 0x4 // 4 operoator voice


// Light weight binary file reading class
function binStream( data ){
  this.data = data;
  this.ptr  = 0;
  this.readChar = () => {
    let chr = data.readUInt8( this.ptr++ )
    return( chr != 0 ? String.fromCharCode( chr ) : "" );
  }
  this.readString = (length) => {
    let str = "";
    for( let i=0; i<length; i++ ){ str += this.readChar(); }
    return( str );
  }

  this.readUInt8    = () => data.readUInt8( this.ptr++ );
  this.readUInt16BE = () => { this.ptr+=2; return( data.readUInt16BE(this.ptr-2) ); }
  this.readUInt16LE = () => { this.ptr+=2; return( data.readUInt16LE(this.ptr-2) ); }
  this.readUInt32BE = () => { this.ptr+=4; return( data.readUInt32BE(this.ptr-4) ); }
  this.readUInt32LE = () => { this.ptr+=4; return( data.readUInt32LE(this.ptr-4) ); }
  this.readInt8     = () => data.readInt8( this.ptr++ );
  this.readInt16BE  = () => { this.ptr+=2; return( data.readInt16BE(this.ptr-2) );  }
  this.readInt16LE  = () => { this.ptr+=2; return( data.readInt16LE(this.ptr-2) );  }
  this.readInt32BE  = () => { this.ptr+=4; return( data.readInt32BE(this.ptr-4) );  }
  this.readInt32LE  = () => { this.ptr+=4; return( data.readInt32LE(this.ptr-4) );  }
  this.readDoubleBE = () => { this.ptr+=8; return( data.readDoubleBE(this.ptr-8) ); }
  this.readDoubleLE = () => { this.ptr+=8; return( data.readDoubleLE(this.ptr-8) ); }
}

// Convert a value into a 2-digit hex value with a leading zero if necessary
function toHex( v ){
  let val = v.toString(16).toUpperCase();
  if( val.length < 2 ) val = "0"+val;
  return( val )
}

function processOP2File( fileName, data ){
  let fileData = new binStream( FS.readFileSync( `./${fileName}`) );

  //Read the patch data from the file
  if( fileData.readString(8) == "#OPL_II#"){
  	console.log( "OPL2 Sound Font File Detected. Processing...")
  }
  patchList = [];

  for( let inst = 0; inst < 175; inst++ ){

    patch = {};
    patch.flags             = fileData.readUInt16LE();
    patch.fineTune          = fileData.readUInt8();
    patch.noteNum           = fileData.readUInt8();

    //Voice 1
    patch.v1_op1_adr_20     = fileData.readUInt8();
    patch.v1_op1_adr_60     = fileData.readUInt8();
    patch.v1_op1_adr_80     = fileData.readUInt8();
    patch.v1_op1_waveform   = fileData.readUInt8();
    patch.v1_op1_lvlScale   = fileData.readUInt8();
    patch.v1_op1_level      = fileData.readUInt8();
    patch.v1_feedback       = fileData.readUInt8();
    patch.v1_op2_adr_20     = fileData.readUInt8();
    patch.v1_op2_adr_60     = fileData.readUInt8();
    patch.v1_op2_adr_80     = fileData.readUInt8();
    patch.v1_op2_waveform   = fileData.readUInt8();
    patch.v1_op2_keyScale   = fileData.readUInt8();
    patch.v1_op2_level      = fileData.readUInt8();
    fileData.readUInt8();                                // Unused
    patch.v1_noteOffset     = fileData.readInt16LE();

    //Voice 2
    patch.v2_op1_adr_20     = fileData.readUInt8();
    patch.v2_op1_adr_60     = fileData.readUInt8();
    patch.v2_op1_adr_80     = fileData.readUInt8();
    patch.v2_op1_waveform   = fileData.readUInt8();
    patch.v2_op1_lvlScale   = fileData.readUInt8();
    patch.v2_op1_level      = fileData.readUInt8();
    patch.v2_feedback       = fileData.readUInt8();
    patch.v2_op2_adr_20     = fileData.readUInt8();
    patch.v2_op2_adr_60     = fileData.readUInt8();
    patch.v2_op2_adr_80     = fileData.readUInt8();
    patch.v2_op2_waveform   = fileData.readUInt8();
    patch.v2_op2_keyScale   = fileData.readUInt8();
    patch.v2_op2_level      = fileData.readUInt8();
    fileData.readUInt8();                                // Unused
    patch.v2_noteOffset     = fileData.readInt16LE();
    patchList.push( patch );
  }
  for( let inst = 0; inst < 175; inst++ ){
    patchList[inst].name = fileData.readString(32);
  }

  let firstDrum = 128; //Can't have more than 128 patches, so this has to be highest patch

  outStr = "#ifndef INSTRUMENTS_H\n#define INSTRUMENTS_H\n";
  patchList.map( (patch, inst) => {
    // Voice level
    outStr += `const unsigned char ym_patch_${inst}[${PATCH_SIZE}] PROGMEM = {`;
    if( patch.flags & 0b100 ){
    	outStr += `0x${toHex( PROC_YM3812 | CONF_2x2OP ) },`;                  // PATCH_PROC_CONF (Dual 2OP Voice YM3812)
	} else {
    	outStr += `0x${toHex( PROC_YM3812 | CONF_2OP ) },`;                    // PATCH_PROC_CONF (Single 2OP Voice YM3812)
	}
    outStr += `0x${toHex( ((patch.noteNum         >> 0) & 0b01111111 )<<0)},`; // PATCH_NOTE_NUMBER
    outStr += "0x00,";                                                         // PATCH_PEG_INIT_LEVEL
    outStr += "0x00,";                                                         // PATCH_PEG_ATTACK
    outStr += "0x00,";                                                         // PATCH_PEG_RELEASE
    outStr += "0x00,";                                                         // PATCH_PEG_REL_LEVEL
    outStr += `0x${toHex( ((patch.v1_feedback >> 1) & 0b111 )<<4)},`;          // PATCH_FEEDBACK
    outStr += `0x${toHex( ((patch.v1_feedback >> 0) & 0b1   )<<6)},`;          // PATCH_ALGORITHM
    outStr += "0x00,";                                                         // PATCH_TREMOLO_SENS
    outStr += "0x00,";                                                         // PATCH_VIBRATO_SENS

    // Operator Level
    outStr += `0x${toHex( ((patch.v1_op1_waveform >> 0) & 0b00000011 )<<5)},`; // PATCH_WAVEFORM
    outStr += `0x${toHex( ((patch.v1_op1_level    >> 0) & 0b00111111 )<<1)},`; // PATCH_LEVEL
    outStr += `0x${toHex( ((patch.v1_op1_lvlScale >> 6) & 0b00000011 )<<5)},`; // PATCH_LEVEL_SCALING
    outStr += `0x${toHex( ((patch.v1_op1_adr_20   >> 4) & 0b00000001 )<<6)},`; // PATCH_ENV_SCALING
    outStr += `0x${toHex( ((patch.v1_op1_adr_20   >> 5) & 0b00000001 )<<6)},`; // PATCH_PERCUSSIVE_ENV
    outStr += `0x${toHex( ((patch.v1_op1_adr_60   >> 4) & 0b00001111 )<<3)},`; // PATCH_ATTACK
    outStr += `0x${toHex( ((patch.v1_op1_adr_60   >> 0) & 0b00001111 )<<3)},`; // PATCH_DECAY
    outStr += `0x${toHex( ((0xF-(patch.v1_op1_adr_80   >> 4)) & 0b00001111 )<<3)},`; // PATCH_SUSTAIN_LEVEL
    outStr += "0x00,";                                                         // PATCH_SUSTAIN_DECAY
    outStr += `0x${toHex( ((patch.v1_op1_adr_80   >> 0) & 0b00001111 )<<3)},`; // PATCH_RELEASE_RATE
    outStr += `0x${toHex( ((patch.v1_op1_adr_20   >> 7) & 0b00000001 )<<6)},`; // PATCH_TREMOLO
    outStr += `0x${toHex( ((patch.v1_op1_adr_20   >> 6) & 0b00000001 )<<6)},`; // PATCH_VIBRATO
    outStr += `0x${toHex( ((patch.v1_op1_adr_20   >> 0) & 0b00001111 )<<3)},`; // PATCH_FREQUENCY_MULT
    outStr += `0x00,`;                                                         // PATCH_DETUNE_FINE
    outStr += "0x00,";                                                         // PATCH_DETUNE_GROSS
    outStr += "0x00,";                                                         // PATCH_SSGENV_ENABLE
    outStr += "0x00,";                                                         // PATCH_SSGENV_WAVEFORM

    // Operator Level (OP2)
    outStr += `0x${toHex( ((patch.v1_op2_waveform >> 0) & 0b00000011 )<<5)},`; // PATCH_WAVEFORM
    outStr += `0x${toHex( ((patch.v1_op2_level    >> 0) & 0b00111111 )<<1)},`; // PATCH_LEVEL
    outStr += `0x${toHex( ((patch.v1_op2_lvlScale >> 6) & 0b00000011 )<<5)},`; // PATCH_LEVEL_SCALING
    outStr += `0x${toHex( ((patch.v1_op2_adr_20   >> 4) & 0b00000001 )<<6)},`; // PATCH_ENV_SCALING
    outStr += `0x${toHex( ((patch.v1_op2_adr_20   >> 5) & 0b00000001 )<<6)},`; // PATCH_PERCUSSIVE_ENV
    outStr += `0x${toHex( ((patch.v1_op2_adr_60   >> 4) & 0b00001111 )<<3)},`; // PATCH_ATTACK
    outStr += `0x${toHex( ((patch.v1_op2_adr_60   >> 0) & 0b00001111 )<<3)},`; // PATCH_DECAY
    outStr += `0x${toHex( ((0xF-(patch.v1_op2_adr_80   >> 4)) & 0b00001111 )<<3)},`; // PATCH_SUSTAIN_LEVEL
    outStr += "0x00,";                                                         // PATCH_SUSTAIN_DECAY
    outStr += `0x${toHex( ((patch.v1_op2_adr_80   >> 0) & 0b00001111 )<<3)},`; // PATCH_RELEASE_RATE
    outStr += `0x${toHex( ((patch.v1_op2_adr_20   >> 7) & 0b00000001 )<<6)},`; // PATCH_TREMOLO
    outStr += `0x${toHex( ((patch.v1_op2_adr_20   >> 6) & 0b00000001 )<<6)},`; // PATCH_VIBRATO
    outStr += `0x${toHex( ((patch.v1_op2_adr_20   >> 0) & 0b00001111 )<<3)},`; // PATCH_FREQUENCY_MULT
    outStr += `0x00,`;                                                         // PATCH_DETUNE_FINE
    outStr += "0x00,";                                                         // PATCH_DETUNE_GROSS
    outStr += "0x00,";                                                         // PATCH_SSGENV_ENABLE
    outStr += "0x00,";                                                         // PATCH_SSGENV_WAVEFORM

    // Operator Level Voice 2
    outStr += `0x${toHex( ((patch.v2_op1_waveform >> 0) & 0b00000011 )<<5)},`; // PATCH_WAVEFORM
    outStr += `0x${toHex( ((patch.v2_op1_level    >> 0) & 0b00111111 )<<1)},`; // PATCH_LEVEL
    outStr += `0x${toHex( ((patch.v2_op1_lvlScale >> 6) & 0b00000011 )<<5)},`; // PATCH_LEVEL_SCALING
    outStr += `0x${toHex( ((patch.v2_op1_adr_20   >> 4) & 0b00000001 )<<6)},`; // PATCH_ENV_SCALING
    outStr += `0x${toHex( ((patch.v2_op1_adr_20   >> 5) & 0b00000001 )<<6)},`; // PATCH_PERCUSSIVE_ENV
    outStr += `0x${toHex( ((patch.v2_op1_adr_60   >> 4) & 0b00001111 )<<3)},`; // PATCH_ATTACK
    outStr += `0x${toHex( ((patch.v2_op1_adr_60   >> 0) & 0b00001111 )<<3)},`; // PATCH_DECAY
    outStr += `0x${toHex( ((0xF-(patch.v2_op1_adr_80   >> 4)) & 0b00001111 )<<3)},`; // PATCH_SUSTAIN_LEVEL
    outStr += "0x00,";                                                         // PATCH_SUSTAIN_DECAY
    outStr += `0x${toHex( ((patch.v2_op1_adr_80   >> 0) & 0b00001111 )<<3)},`; // PATCH_RELEASE_RATE
    outStr += `0x${toHex( ((patch.v2_op1_adr_20   >> 7) & 0b00000001 )<<6)},`; // PATCH_TREMOLO
    outStr += `0x${toHex( ((patch.v2_op1_adr_20   >> 6) & 0b00000001 )<<6)},`; // PATCH_VIBRATO
    outStr += `0x${toHex( ((patch.v2_op1_adr_20   >> 0) & 0b00001111 )<<3)},`; // PATCH_FREQUENCY_MULT
    outStr += `0x${toHex( ((patch.fineTune        >> 0) & 0b00001111 )<<3)},`; // PATCH_DETUNE_FINE
    outStr += "0x00,";                                                         // PATCH_DETUNE_GROSS
    outStr += "0x00,";                                                         // PATCH_SSGENV_ENABLE
    outStr += "0x00,";                                                         // PATCH_SSGENV_WAVEFORM

    // Operator Level (OP2) Voice 2
    outStr += `0x${toHex( ((patch.v2_op2_waveform >> 0) & 0b00000011 )<<5)},`; // PATCH_WAVEFORM
    outStr += `0x${toHex( ((patch.v2_op2_level    >> 0) & 0b00111111 )<<1)},`; // PATCH_LEVEL
    outStr += `0x${toHex( ((patch.v2_op2_lvlScale >> 6) & 0b00000011 )<<5)},`; // PATCH_LEVEL_SCALING
    outStr += `0x${toHex( ((patch.v2_op2_adr_20   >> 4) & 0b00000001 )<<6)},`; // PATCH_ENV_SCALING
    outStr += `0x${toHex( ((patch.v2_op2_adr_20   >> 5) & 0b00000001 )<<6)},`; // PATCH_PERCUSSIVE_ENV
    outStr += `0x${toHex( ((patch.v2_op2_adr_60   >> 4) & 0b00001111 )<<3)},`; // PATCH_ATTACK
    outStr += `0x${toHex( ((patch.v2_op2_adr_60   >> 0) & 0b00001111 )<<3)},`; // PATCH_DECAY
    outStr += `0x${toHex( ((0xF-(patch.v2_op2_adr_80   >> 4)) & 0b00001111 )<<3)},`; // PATCH_SUSTAIN_LEVEL
    outStr += "0x00,";                                                         // PATCH_SUSTAIN_DECAY
    outStr += `0x${toHex( ((patch.v2_op2_adr_80   >> 0) & 0b00001111 )<<3)},`; // PATCH_RELEASE_RATE
    outStr += `0x${toHex( ((patch.v2_op2_adr_20   >> 7) & 0b00000001 )<<6)},`; // PATCH_TREMOLO
    outStr += `0x${toHex( ((patch.v2_op2_adr_20   >> 6) & 0b00000001 )<<6)},`; // PATCH_VIBRATO
    outStr += `0x${toHex( ((patch.v2_op2_adr_20   >> 0) & 0b00001111 )<<3)},`; // PATCH_FREQUENCY_MULT
    outStr += `0x00,`;                                                         // PATCH_DETUNE_FINE
    outStr += "0x00,";                                                         // PATCH_DETUNE_GROSS
    outStr += "0x00,";                                                         // PATCH_SSGENV_ENABLE
    outStr += "0x00";                                                          // PATCH_SSGENV_WAVEFORM
    outStr += `}; // ${patch.name}\n`;

  });

  outStr += "const unsigned char *patches[] = {\n";
  patchList.map( (patch, inst) => {
    outStr += `ym_patch_${inst},`
  });
  outStr += "};\n"

  patchList.map( (patch, inst) => {
    outStr += `const char PName${inst}[] PROGMEM = "${patch.name}";\n`
  });

  outStr += `const char* const patchNames[] = {`
  patchList.map( (patch, inst) => { outStr += `PName${inst},`} );
  outStr += `};\n`;

  outStr += `#define NUM_MELODIC       ${firstDrum}\n`;
  outStr += `#define NUM_DRUMS         ${patchList.length - firstDrum}\n`;
  outStr += "#endif"; 


  return( outStr );
}



data = new DB();

let inputFile = ""
let outputFile = "instruments.h"
if (process.argv[3]) {
  outputFile = process.argv[3];
}
if (process.argv[2]) {
  inputFile = process.argv[2];
  FS.writeFileSync(outputFile, processOP2File(inputFile, data ));
  console.log( "Done!");
	
} else {
  console.log('\n');
  console.log('------------------------------------------------');
  console.log(' OPL2 FILE PARSER                               ');
  console.log(' Convert op2 files into a .h files that contain ');
  console.log(' a cross compatible YM sound font               ');
  console.log('------------------------------------------------');
  console.log('\n');


  console.log('Please specify an input file using syntax: \n');
  console.log('node op2.js GENMIDI.op2 \n');
}