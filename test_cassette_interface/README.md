MEK6802D5 Cassette Interface Test Program
===

Installing Required Software
---
You will need to download and install the gcc compiler along with the tools that you'll need to run make. You can do this through either Cygwin or MinGW.  The choice is up to you as to which one you want to use.  Here are some links to get you started:
```
https://sourceware.org/cygwin/
http://www.mingw.org/
```
To me, this always seems to be a little bit of a struggle for there isn't one single application installer that seems to do everything that you need.  So it gets to be a little bit of a hunt-and-peck process, but stick with it and you'll eventually get things to work properly.

Building the test program
---
Once you can run make and have it call gcc, building the program is as simple as entering the following command:
```
$ cd <path>/test_cassette_interface 
$ make all
gcc -I./include -c -o obj/test_cassette_interface.o src/test_cassette_interface.c
Compiled obj/test_cassette_interface.o successfully!
gcc -o  bin/test_cassette_interface obj/test_cassette_interface.o
Linking bin/test_cassette_interface complete!
```
Loader Program Arguments
---
Here is an explanation of the different user arguments available for the test application:
```
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
Please be aware that the file and com port are mandatory.

Running the Test Program
---
Here is an example execution of the test program:
```
$ ./bin/test_cassette_interface.exe -i COM20 -bt 115200 -t 5 -n 5 -v 1
Successfully connected to UART on port COM20 at baud rate 115200.
Connected to transmitter port COM20 at baud rate 115200
sent 0x74 -> received 0x74
sent 0x1b -> received 0x1b
sent 0x91 -> received 0x91
sent 0x19 -> received 0x19
sent 0x32 -> received 0x32
Test 5 Passed!
```

