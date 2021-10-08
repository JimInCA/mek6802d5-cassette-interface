
# Cassette Interface for the Motorola MEK6802D5

The intent of this project is to emulate the cassette interface for the Motorola MEK6802D5 Development Board using modern technology.  

The MEK6802D5 has the ability to save it's current contents in memory to an external cassette tape drive.  It also has the ability to load a program saved on a cassette tape drive to it's internal memory.  The intent of this project is to replace the cassette tape drive with the NXP FRDM-K64F Development Board.

I originally implemented a cassette interface for the MEK6802D5 using a circuit called the bit-boffer which uses 1970's vintage technology.  The bit-boffer was a lot of fun to build and it worked perfectly.  But it is quite cumbersome requiring two solderless breadboards, eight ICs, two Teensy development boards, and countless discreet components and wires.  So I decided to duplicate the functionally of the bit-boffer but using modern technology.  I decided to use the NXP FRDM-K64F board because it has a DAC that I can use to implement the transmitter and a comparator that I can use to implement the receiver.  I also just happened to have one of these boards in my vast collection of prototype/development boards, so the decision to use this was simple.  You can find my original project at [Cassette Tape Emulator](https://github.com/JimInCA/cassette-tape-emulator).

Here's a photo of my set-up for the FRDM-K64F board.  I have it working as a prototype where I'm wrapping the transmitter's output on pin J4[11], into the receiver's input in pin J1[13].  

![alt text](./images/FRDM-K64F-Board.jpg?raw=true "MEK6802D5")

Once I have everything tested, I'll connect it to my MEK6802D5 to test it's functionality in a real application.

test_cassette_interface
---
```
NOTE: I modified my bit-boffer test program to work with the FRDM-K64F board.  My design with the bit-boffer required two Teensy boards with both boards running at 300 baud.  This design requires just a single FRDM-K64F with the com port running at 115200 baud.  It is still possible to test the decoded uart signal coming from the MEK6802D5 by connecting a Teensy's RX pin to pin J1[1] on the FRDM-K64F board.  But I use this mode just for testing.  
 ```

The test_cassette_interface application program runs on a Widows host and can be built using gcc.

test_cassette_interface's help menu lists the usage and arguments for the application as shown below.

```
$ ./bin/test_cassette_interface.exe -h
usage: test-bit-boffer [-h] -i COMPORT [-o COMPORT] [-bt BAUDRATE] [-br BAUDRATE] [-t TESTNUM] [-n LOOPNUM]

arguments:
  -h             Show this help message and exit.
  -i  COMPORT    Transmitter COM Port.
  -o  COMPORT    Receiver COM Port when TESTNUM > 0.
  -bt BAUDRATE   Transmitter baud rate, default: 115200.
  -br BAUDRATE   Receiver baud rate, default: 300.
  -t  TESTNUM    Desired test to be executed.
                 0: Generate a count from 0x00 to 0xff and send to transmitter.
                    Connect a uart terminal window to receiver's com port.
                 1: Generate a count from 0x00 to 0xff, send to transmitter
                    and verify that the receiver received the correct data.
                 2: Send 'n' number of 0xff markers to transmitter and verify on receiver.
                 3: Send code for USED5 program to transmitter and verify on receiver.
                 4: Combine test 2 followed by test 3. This emulates a Load sequence.
                 5: Generate 'n' number of random bytes, send to transmitter and
                    verify on receiver.
  -n LOOPNUM     Number of test cycles in test loop.
```

All tests require that you connect the output of the transmitter directly to the input of the receiver (J4[11] to J1[13] as shown in the photo above).  This provides the ability for closed-loop testing.  

An example execution of test_cassette_interface follows:
```
$ ./bin/test_cassette_interface.exe -i COM20 -bt 115200 -t 5 -n 8 -v 1
Successfully connected to UART on port COM20 at baud rate 115200.
Connected to transmitter port COM20 at baud rate 115200
sent 0x90 -> received 0x90
sent 0xd7 -> received 0xd7
sent 0x72 -> received 0x72
sent 0x80 -> received 0x80
sent 0x27 -> received 0x27
sent 0x6b -> received 0x6b
sent 0xe8 -> received 0xe8
sent 0xc7 -> received 0xc7
Test 5 Passed!
```

bit-boffer-writer  --- NOT IMPLEMENTED YET!!!
---
TBD

That's it for now and most of all, have fun with all of your projects.
