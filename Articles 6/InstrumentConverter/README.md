# InstrumentConverter
This nodejs code extracts sound patch settings from .op2 files and converts them into the instruments.h file

## Usage:
node op2.js GENMIDI.op2 instruments.h

This will extract any sound patches from GENMIDI.op2 and output a new file called instruments.h.

Once you have the instruments.h file, copy it into the folder with your .ino file to compile it into the program.


op2 files can be created and manipulated using another opensource project——the OPL3 Bank Editor:
https://github.com/Wohlstand/OPL3BankEditor
