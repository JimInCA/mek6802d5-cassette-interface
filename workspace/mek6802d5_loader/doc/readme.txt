Overview
========
This program is intended to be used to load an assembled program into the Motorola MEK6802D5
development board.  

What this program does is it simulates the audio signal coming from a cassette tape drive.
This provides the ability to write your assembly code in a standard text editor and use 
the Motorola 6800 assembler to generate your binary file that can then be loaded using
this program.

Toolchain supported
===================
- MCUXpresso  11.4.0

Hardware requirements
=====================
- Mini/Micro USB cable
- FRDM-K64F board
- Personal Computer

Board settings
==============
No special is needed.

Preparing the software
======================
You will first need to install MCUXpresso Version 11.4.0. This is the version that I'm using,
but other versions may work as well.  Once you have MCUXpresso installed, execute the program.
Once MCUXpresso comes up, you will need to select 'Switch Workspace -> Other' from the File menu.
Once the pop-up has completed loading, select ./mek6802d5-loader/workspace as your workspace.
You will then need to import from directory the mek6802d5_loader into MCUXpresso's workspace.
Them simply compile the program.

Prepare the board
=================
1.  Connect a USB cable between the PC host and the OpenSDA USB port on the board.
2.  Open a serial terminal on PC for OpenSDA serial device with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.
5.  A scope may be used to verify the output signal on pin J4[11]:DAC output using an occiloscope.  
6.  Pin J1[8] can be used as a trigger for the scope.

Running the loader
==================

TBD

