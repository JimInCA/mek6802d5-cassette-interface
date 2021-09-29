
# Cassette Interface for the Motorola MEK6802D5

The intent of this program is to emulate the cassette interface for the Motorola MEK6802D5 Development Board.  This will allow you to either load an assembled binary program in to the ram memory of the MEK6802D5 or save the binary data located in either ram or rom memory of the MEK6802D5 to your host computer.

For loading, what this program does is it simulates the audio signal that would normally be coming from a cassette tape drive.  This provides the ability to write your assembly code in a standard text editor and use the Motorola 6800 assembler to generate your binary file that can then be loaded using this program.  You can also use this program to save the code or data located in either the board's ram or rom.  

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
Nothing special is needed.

Preparing the software
======================
You will first need to install MCUXpresso Version 11.4.0. This is the version that I'm using,
but other versions may work as well.  Once you have MCUXpresso installed, execute the program.
Once MCUXpresso comes up, you will need to select 'Switch Workspace -> Other...' from the File 
menu.  Once the pop-up has completed loading, select ./mek6802d5-cassette-interface/workspace as your 
workspace.  You will then need to select "Import projects..." in the Project Explorer tab.  
Then under General, select 'Existing Projects into Workspace" and press the Next button.   
Select <path>.\mek6802d5-cassette-interface\workspace as your root directory, make sure that the 
mek6802d5_cassette_interface project is selected and then press the Finish button.  This should load the
mek6802d5_cassette_interface program into the project Explorer tab.  Then simply left click on the 
mek6802d5_cassette_interface project, right click to bring up the menu, and then select Build Project.
This should compile the program.

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
5.  A scope may be used to verify the output signal on pin J4[11]:DAC output using an oscilloscope.  
6.  Pin J1[8] can be used as a trigger for the scope.

Running the loader
==================

TBD

