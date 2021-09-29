
# Cassette Interface for the Motorola MEK6802D5

NOTE:  This is a work in progress.  The following is just for reference for now and will be enhanced as time goes along.

The intent of this project is to emulate the cassette interface for the Motorola MEK6802D5 Development Board using modern technology.  

I originally implemented a cassette interface for the MEK6802D5 using a circuit called the bit-boffer which uses 1970's vintage technology.  The bit-boffer was a lot of fun to build and it worked perfectly.  But it is quite cumbersome requiring two solderless breadboards, eight ICs, two Teensy development boards, and countless discreet components and wires.  So I decided to duplicate the functionally of the bit-boffer but using modern technology.  I decided to use the NXP FRDM-K64F board because it has a DAC that I can use to simulate the transmitter and a comparator that I can use to simulate the receiver.  I also just happened to have this board in my vast collection of prototype/development boards, so the decision to use this was simple.  

NOTE:  At this point, I have the transmitter and receiver portions working.  But the issue is that the with this board, I can't change the baud rate for the board's interface.  I have the transmitter working at the board's 115200 baud rate, the the receiver requires 300 baud, hence the need to a Teensy to act as the UART to USB interface.  Here's a very poor photo of my current set-up:

![alt text](./images/FRDM-K64F-Board.jpg?raw=true "MEK6802D5")

I also modified my bit-boffer test program to work the the split baud rates so that I could easily test the prototype system with a known good test case.  I'll update the program and this README once I have finalized my prototype.

test_cassette_interface
---
The test_cassette_interface application program runs on a Widows host and can be built using gcc.  I also provided a solution file so that you can build the application under Visual Studio if you so desire.

test_cassette_interface's help menu lists the usage and arguments for the application as shown below.

```
$ ./bin/test_cassette_interface.exe -h
usage: test_cassette_interface [-h] -i COMPORT [-o COMPORT] [-b BAUDRATE] [-t TESTNUM] [-n LOOPNUM]

arguments:
  -h             Show this help message and exit.
  -i COMPORT     Transmitter COM Port.
  -o COMPORT     Receiver COM Port when TESTNUM > 0.
  -b BAUDRATE    Desired baudrate, default: 300.
  -t TESTNUM     Desired test to be executed.
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

All tests require that you connect the output of the transmitter directly to the input of the receiver.  This provides the ability for closed-loop testing.  

An example execution of test_cassette_interface follows:
```
$ ./bin/test_cassette_interface.exe -i COM22 -o COM9 -b 300 -t 5 -n 8 -v 1
Successfully connected to UART on port COM22 at baud rate 300.
Connected to transmitter port COM22 at baudrate 115200
Successfully connected to UART on port COM9 at baud rate 300.
Connected to receiver port COM9 at baudrate 300
sent 0x43 -> received 0x43
sent 0xe3 -> received 0xe3
sent 0xdf -> received 0xdf
sent 0x86 -> received 0x86
sent 0x30 -> received 0x30
sent 0x06 -> received 0x06
sent 0xf8 -> received 0xf8
sent 0xf4 -> received 0xf4
Test 5 Passed!
```

bit-boffer-writer  --- NOT IMPLEMENTED YET!!!
---
bit-boffer-writter is the application that you'll need to run to load code into the Motorola D5's ram.  It too can be built with either gcc or Visual Studio.

bit-boffer-writter's help menu lists the usage and arguments for the application as shown below: 

```
$  ./bin/bit-boffer-writer.exe -h
usage: bit-boffer-writer [-h] -f FILE -p COMPORT [-b BAUDRATE] [-c NUMMARKS]

arguments:
  -h             Show this help message and exit.
  -f FILE        Filename of S-Record input file.
  -p COMPORT     COM Port to which the device is connected.
  -b BAUDRATE    Desired baudrate, default: 300.
  -c NUMMARKS    Number of marker cycles. default: 819.
```

The expected output from running bit-boffer-writer should look like the following example.

```
$ ./bin/bit-boffer-writer.exe -f ./test/used5.s19 -p COM22 -b 300 -c 1024
Successfully connected to UART on port COM22 at baud rate 300.
DCB is ready for use.
Sending file ./test/used5.s19 to port COM22 at baudrate 300
```

That's it for now and most of all, have fun with all of your projects.