
# Cassette Tape Interface for the Motorola MEK6802D5

The intent of this project is to emulate the cassette tape interface for the Motorola MEK6802D5 Development Board using modern technology.  

The MEK6802D5 has the ability to save it's current contents in memory to an external cassette tape drive.  It also has the ability to load a program saved on a cassette tape drive to it's internal memory.  The intent of this project is to replace the cassette tape drive with the NXP FRDM-K64F Development Board.

I originally implemented a cassette interface for the MEK6802D5 using a circuit called the bit-boffer which uses 1970's vintage technology.  The bit-boffer was a lot of fun to build and it worked perfectly.  But it is quite cumbersome requiring two solderless breadboards, eight ICs, two Teensy development boards, and countless discreet components and wires.  So I decided to duplicate the functionally of the bit-boffer but using modern technology.  I decided to use the NXP FRDM-K64F board because it has a DAC that I can use to implement the transmitter and a comparator that I can use to implement the receiver.  I also just happened to have one of these boards in my vast collection of prototype/development boards, so the decision to use this was simple.  You can find my original project that uses the bit-boffer at [Cassette Tape Emulator](https://github.com/JimInCA/cassette-tape-emulator).

---
## MEK6802D5 Cassette Interface
---

The Cassette Interface on the MEK6802D5 uses Kansas City Standard (KCS) encoding to write and retrieve data to and from a cassette tape.  So the question is then, what is Kansas City Standard?  KCS was developed in 1975 as a means to save and retrieve computer data using an inexpensive consumer quality cassette tape player.  What it specifies is a process to convert computer data into two different audio frequencies that could then be written to or retrieved from to the tape.  

What the MEK6802D5 does is it takes the output from the TX pin on a UART and converts all low data bits to 1200 hertz and all high data to 2400 hertz.  The over all baud rate is set to 300 bits per second, so this means that a low data bit will equal four cycles at 1200Hz and a high data bit will equal eight cycles at 2400Hz.  It reverses the process on reading the data from a tape by converting four cycles at 1200Hz to a zero and eight cycles at 2400Hz to a one and sends this data stream to the RX pin on the UART.

The intent of this project it to emulate the audio cassette tape drive by replacing it with the FRDM-K64F.

---
## Overall Project Structure
---

The following image shows the basic concept for this application.

![alt text](./images/cassette_interface.jpg?raw=true "MEK6802D5")

<ul>
<li>J4[11]/DAC0_OUT Transmitter DAC Output</li>
<li>J1[13]/CMP0_IN1 Receiver Comparator Input</li>
<li>J1[15]/CMP0_OUT Receiver Comparator Output for debugging</li>
<li>J1[1]/PTB18 Receiver Decoded UART Output for debugging</li>
<li>J1[8]/PTA1 SCOPE_TRIGGER for debugging</li>
<li>J3[4] 3.3V</li>
<li>J2[3] GND</li>
<li>J3[12] GND</li>
<li>J3[14] GND</li>
</ul>

There are two portions to the project; Emulation of the audio signal that would normally be coming from the tape (Transmitter) and Decoding of the audio signal that would normally be written to the tape (Receiver).  

### **Transmitter**

The transmitter portion of the design takes the data coming from the host and converts it into the two different KCS frequencies.  As a starting point, what the code does is it caches the incoming data into a queue.  The main loop in the program than checks to see if there is any data in this queue.  If there is, it then sends the data to the routines that converts the zeros to four cycles of data at 1200Hz and the ones into eight cycles at 2400Hz.  It formats the data into what is required for a UART device by first generation the start bit, followed by the eight data bits, LSB first, and ending with two stop bits.  The code uses a timer to generate the correct period to update the 12 bit DAC.  The sine wave is divided into thirty two steps so this means that the timer must be set to the correct partial period depending on if a zero or one is being generated.  Control is returned to the main loop once the correct number of cycles have been generated for each data bit.  If there is no data in the queue, the main loop just generates the signal for a one to maintain a constant signal.

The output from the DAC, DAC0_OUT, is on device pin J4[11].  The output on J4[11] will need to be connected to the "EAR" input on P3 on the D5.  There is also a signal coming from the board on J1[8] that can be used as a scope trigger.  The scope trigger is generated at the start of the output signal.

### **Receiver**

The receiver portion of the design uses a comparator to convert the incoming sine wave into a square wave.  This is done by setting the 6-bit dac to an appropriate level to generate a 50-50 duty cycle on the output of the comparator.  What the code does is it measures the period between consecutive rising edges to determine the frequency of the incoming signal.  It then sets the decoded uart output to either a zero or one based on the incoming frequency.  

This is where there was a departure from my original design.  I originally intended to send the decoded uart output signal to an on-board UART.  But there is a limitation on this board with respect to it's UART peripherals.  The on-board UART peripherals are clocked at too high of a frequency to be able to generate a 300 baud rate.  What happens is the internal UART counter overflows and you get a different frequency than you expect and the over flow bits also corrupts the state of other bits in the UART register.  I've filed a bug report with NXP on this issue (NXP support case: #00425437).

To get around the issue of the UART not supporting 300 baud, I had to add my own decoding of the input signal.  So when I detect the falling edge of the start signal, I first delay for one half of the period, sample the decoded uart output, and then continue to delay for one full period and sample again for the next ten bits (start, eight data bits, two stop bits equals eleven bits).  This is a little bit of overkill at this point for I could simply record the bits as they toggle the decoded uart signal.  But I wanted to keep my original design for now.  

Once the eight data bits are resolved, the code places the byte on an output queue and returns.  The main loops looks at this queue (as well as the transmitter queue) and if there is any data available, it sends the data to the host.

The input to the receiver's comparator is on signal CMP0_IN1 on pin J1[13].  This input needs to be connected to the "MIC" output on P2 on the D5.  For set up and debugging, you can use a scope to monitor the comparator's output on board pin J1[15] to verify the 50-50 duty cycle.  The decoded uart signal at 300 baud is also presented on the board's J1[1] pin.  If you like, you can send this signal to an external UART.  I like using the Teensy myself.  Just remember that the other device must be 3.3V compatible and that the UART must be set to 300 baud and two stop bits.

---
## FRDM-K64F Device Firmware Testing
---

Once the firmware is loaded onto the FRDM-K64F, it's good practice to verify the board set-up prior to connecting it to the D5.  I have a set of tests that can be run on the board where the tests assume that the transmitter output (J4[11]) is wrapped back into the receiver input (J1[13]).  Here's a photo that shows the FRDM-K64F's set-up for the firmware verification tests:  

![alt text](./images/FRDM-K64F-Board.jpg?raw=true "MEK6802D5")

The test_cassette_interface application program will be used to test the FRMD's firmware.  What the test program does is it writes data to the transmitter and then reads any data received on the receiver.  It then compares the data sent to the data received to determine if the test either passed or failed.  After the test completes, it reports the final results of the test.

An example execution of test_cassette_interface follows:
```
$ ./bin/test_cassette_interface.exe -i COM20 -bt 115200 -t 5 -n 8
Successfully connected to UART on port COM20 at baud rate 115200.
Connected to transmitter port COM20 at baud rate 115200
Connected to receiver port COM20 at baud rate 115200
Test 5 Passed!
```

A full description of test_cassette_interface is given later in this document.

Once you are satisfied with that the FRMD-K64F is working correctly, it's time to connect it to the D5 for full system testing.

---
## MEK6802D5 System Testing
---

Now it's time to connect the FRDM-K64F to the MEK6802D5 and see if this thing will actually work.  
Here is a photo with everything connected together:

![alt text](./images/final_example.jpg?raw=true "MEK6802D5 with FRDM-K64F")

As you can see in the photo, I've soldered together two audio jacks that I've used to connect the D5's "EAR" and "MIC" connectors to the FRDM-F64K pins.  The jack on the "EAR" connector, P3, is connected to pin J4[11], DAC0_OUT.  The jack on the "MIC" connector, P2, is connected to pin J1[13], CMP0_IN1.  

Once everything is connected and powered up, it's time to test out all of our work.  What I want to test first is to see if I can load a binary file into the D5's memory.  So on my host, I go to the ../mek6802d5-cassette-interface/mek6802d5-loader directory and execute the following command:

```
$ ./bin/mek6802d5-loader.exe -f ./test/help.s19 -p COM20 -b 115200 -c 1024
Successfully connected to UART on port COM20 at baud rate 115200.
DCB is ready for use.
Sending file ./test/help.s19 to port COM20 at baud rate 115200
```

On the D5, I then press the RS button, followed by the FS button, and then the P/L button.  The display on the D5 should go blank for about a minute and then show a dash on the most left digit.  Then press 0 followed by GO and you should then see HELP-- displayed across all six digits on the D5.

The only thing left is to test to see if we can save from the D5's memory to the host.  

NOTE:

At this point, this feature is not working and to be truthful, I had the same issue with the bit-boffer.  For the tape output, the D5 uses a voltage divider network with a 4.7K ohm resister in series with a 47 ohm resistor with the output being the center tap between the two resisters.  This means if we have a clean 5V input signal, we only get a 50mV signal on the output. The amplitude of the output signal isn't enough for the FRDM-K64F to detect.  To get around this issue, I decided to tap into the output data stream on the D5 and construct my own filter circuit as defined in the image below:

![alt text](./images/MEK6802D5-Tape-Output.jpg?raw=true "MEK6802D5 Tape Output")

I taped into the signal after C32, the DC isolation capacitor and prior to R9, the top of the voltage divider.  I then ran this signal through my own isolation capacitor, just to be save, to the center tap of a pair of 2.2K ohm resistors.  The resistors set up a mid point biasing voltage between Vcc and Vee (3.3V and gnd).  I then ran this signal into the CMP0_IN1 pin, J1[13] on the FRDM-K64F.  With the biasing voltage being at the mid point, I needed to adjust up the voltage on the 6-bit DAC to the complement input to CMP0, the comparator.  I could have just as easily adjusted the values to the pair of 2.2K resistors to reduce the voltage to something slightly below the mid point, but adjusting the voltage was the simpler of the two options.  Replacing the lower resister with a 5K ohm multi-turn pot may be the best option if you care to go that way.

I also had to modify the ISR routine for CMP0.  Instead of measuring the period between the rising edge and the falling edge, I had to measure the period between two consecutive rising edges to determine the period.  Determining the period is necessary in order to determine if we are receiving a one or a zero.  The reason why I originally went with half period is because of the inherent error induced by the difference in the period between a one and a zero.  With a half period, the error is reduced in half.  But it seems to work perfectly even with the increase in the error.

Now what do I mean by the inherent error.  I'm measuring the period between consecutive rising edges.  After the measurement of the second edge, it can be determined if this is a period for a one or a period for a zero.  This means that I have a one period delay between when the period actually changes until the output will change (DECODED_UART, J1[1]).  With the periods between a one and a zero being different, this means that the delay for a one will be less than the delay for a zero (zero has twice the period of a one). There is a way to remove this error by requiring two one periods before changing the output; this would make the delay equal between a one and a zero, but it seems to be working with this inherent error, so I'm going to leave it alone for now.  It's just a potential issue that the user needs to be aware of in the future.

With this modification, the punch command worked perfectly as shown in the description of the puncher application later in this document.  Now I could have modified the tape output circuit on the D5, but with this board being over forty years old, I felt that this was the safer option, even though it goes against my original design concept of having a single board solution with no external components.

I've included the following photo of my final setup that includes the modified output circuit from the D5 as an example of what I needed to do to get the complete system to work properly. 

![alt text](./images/final_example_with_mod.jpg?raw=true "MEK6802D5 with FRDM-K64F with Modified Receiver Circuit")

---
## Application Programs
---

The following is a description for the host application programs that are used to test and interact with the FRDM-K64F and through it, to the D5.

---
### test_cassette_interface
---

I modified my bit-boffer test program to work with the FRDM-K64F board.  My design with the bit-boffer required two Teensy boards with both boards running at 300 baud.  This design requires just a single FRDM-K64F with the com port running at 115200 baud.  It is still possible to test the decoded uart signal coming from the MEK6802D5 by connecting a Teensy's RX pin to pin J1[1] on the FRDM-K64F board.  But I use this mode just for verification of the decoded uart signal.

The test_cassette_interface application program runs on a Widows host and can be built using gcc.

test_cassette_interface's help menu lists the usage and arguments for the application as shown below.

```
./bin/test_cassette_interface.exe -h
usage: test-bit-boffer [-h] -i COMPORT [-o COMPORT] [-bt BAUDRATE] [-br BAUDRATE] [-t TESTNUM] [-n LOOPNUM] [-v]

arguments:
  -h             Show this help message and exit.
  -i  COMPORT    Transmitter COM Port.
  -o  COMPORT    Receiver COM Port when TESTNUM > 0.
                 Only define receiver com port when using a separate device
                 to test the DECODED_UART output on pin J1[1].
  -bt BAUDRATE   Transmitter baud rate, default: 115200.
  -br BAUDRATE   Receiver baud rate, default: 300.
                 Only valid when -o COMPORT is defined, ignored otherwise.
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
  -v             Increase output verbosity.
```

All tests require that you connect the output of the transmitter directly to the input of the receiver (J4[11] to J1[13] as shown in the photo above).  This provides the ability for closed-loop testing.  

An example execution of test_cassette_interface follows:
```
$ ./bin/test_cassette_interface.exe -i COM20 -bt 115200 -t 5 -n 8 -v
Successfully connected to UART on port COM20 at baud rate 115200.
Connected to transmitter port COM20 at baud rate 115200
Connected to receiver port COM20 at baud rate 115200
sent 0xd7 -> received 0xd7
sent 0xf8 -> received 0xf8
sent 0x6b -> received 0x6b
sent 0xc0 -> received 0xc0
sent 0x27 -> received 0x27
sent 0x6d -> received 0x6d
sent 0x15 -> received 0x15
sent 0xdf -> received 0xdf
Test 5 Passed!
```

---
### mek6802d5-loader
---
mek6802d5-loader is the application that you'll need to run to load code into the D5's ram.  It too can be built with either gcc or Visual Studio.

mek6802d5-loader's help menu lists the usage and arguments for the application as shown below: 

```
$ ./bin/mek6802d5-loader.exe -h
usage: mek6802d5-loader [-h] -f FILE -p COMPORT [-b BAUDRATE] [-c NUMMARKS] [-v]

arguments:
  -h           Show this help message and exit.
  -f FILE      Filename of S-Record input file.
  -p COMPORT   COM Port to which the device is connected.
  -b BAUDRATE  Desired baud rate, default: 115200.
  -c NUMMARKS  Number of marker cycles. default: 819.
  -v           Increase output verbosity.
```

To load a binary file into the D5's memory, you will first need to press the RS key on the D5's keypad to force the D5 back into it's boot monitor program.  The next step is to press the FS key followed by the P/L key on the D5's keypad.  This sets up the board to load data into it's memory.  You can then call mek6802d5-loader to load the binary into the D5's memory.  The expected output from running mek6802d5-loader should look like the following example.  

```
$ ./bin/mek6802d5-loader.exe -f ./test/used5.s19 -p COM20 -b 115200 -c 1024
Successfully connected to UART on port COM20 at baud rate 115200.
DCB is ready for use.
Sending file ./test/used5.s19 to port COM20 at baud rate 115200
```

When the load completes, it writes a dash to the first character of the six character display.  Depending on the size of the binary, this can take a minute or two.  Then all you need to do is press the 0 key followed by the G0 key on the D5's keypad to execute the program just loaded to the D5.

---
### mek6802d5-puncher
---

mek6802d5-puncher is an application that can be used to capture the data coming from the D5 when the punch command is executed to save a section of the D5's memory to tape.  

mek6802d5-puncher's help menu lists the usage and arguments for the application as shown below: 

```
$ ./bin/mek6802d5-puncher.exe -h
usage: mek6802d5-puncher [-h] [-f FILE] -p COMPORT [-b BAUDRATE] [-v]

arguments:
  -h           Show this help message and exit.
  -f FILE      Output Filename.
  -p COMPORT   COM Port.
  -b BAUDRATE  Baud rate, default: 115200.
  -v           Increase output verbosity.
```

To save the binary code that's in the D5's memory, you will need to first press the RS key on the D5's keypad to activate the D5's boot monitor (first character goes to a dash).  You will then press the P/L key and the board will prompt you for the begin address.  Enter the beginning address and then press the GO key.  The D5 will then prompt you for the end address.  Again, enter the ending address and press the GO key.  The D5 will immediately start to send a 30 second header consisting of just 0xff bytes.  This gives you time to enter the command for the mek6802d5-puncher program as shown below with the expected output:

```
./bin/mek6802d5-puncher.exe -f help_bytes.txt -p COM20 -b 115200
Successfully connected to UART on port COM20 at baud rate 115200.
Connected to port COM20 at baud rate 115200
0x0000  0xce
0x0001  0x76
0x0002  0x79
0x0003  0xff
0x0004  0xe4
0x0005  0x1d
0x0006  0xce
0x0007  0x38
0x0008  0x73
0x0009  0xff
0x000a  0xe4
0x000b  0x1f
0x000c  0xce
0x000d  0x40
0x000e  0x40
0x000f  0xff
0x0010  0xe4
0x0011  0x21
0x0012  0xce
0x0013  0xf0
0x0014  0xa2
0x0015  0xff
0x0016  0xe4
0x0017  0x19
0x0018  0x7e
0x0019  0xf0
0x001a  0xbb
```

If you include the -v option with the program execution, the output will include the Block Start Character, 0x53 ('S'), the start and end address, along with the checksum.  The output with the -v option is shown below:

```
Successfully connected to UART on port COM20 at baud rate 115200.
Connected to port COM20 at baud rate 115200
Opened output file: help_bytes.txt
Reading Leader Characters
Checksum passed: 0x00
        0x53
        0x00
        0x00
        0x00
        0x1a
0x0000  0xce
0x0001  0x76
0x0002  0x79
0x0003  0xff
0x0004  0xe4
0x0005  0x1d
0x0006  0xce
0x0007  0x38
0x0008  0x73
0x0009  0xff
0x000a  0xe4
0x000b  0x1f
0x000c  0xce
0x000d  0x40
0x000e  0x40
0x000f  0xff
0x0010  0xe4
0x0011  0x21
0x0012  0xce
0x0013  0xf0
0x0014  0xa2
0x0015  0xff
0x0016  0xe4
0x0017  0x19
0x0018  0x7e
0x0019  0xf0
0x001a  0xbb
        0xd7
```

As noted in the help menu, you can have the mek6802d5-puncher program save the data to a specified file.  As an example for when the puncher writes to a file, I've included the expected output file for help.s19 along with the expected output file for used5.s19 in the ./doc directory.  Just for fun, I also punched the contents for the D5DEBUG Program, ROM addresses 0xf000 through 0xf7ff, and saved it in file d5debug_bytes.txt.  This way, if I ever need to burn a new ROM for the D5, I have a listing of the binary data that I could use as input to a ROM programmer.

---
## Conclusion
---
As with all fun projects, there comes a time when we must admit that the project is completed.  This project has been a lot of fun in figuring out how to duplicate the circuits on the bit-boffer using the peripherals on the FRDM-K64F along with the challenge of getting everything to work with the MEK6802D5.  There are still a couple of minor issues that I'd like to clean up such as the need to press the reset button on the FRDM-K64F after power-up in order for my program to load on the target microprocessor.  There's also an annoying pop-up window when ever I attach the board to the host that I'd like to suppress.  But these will have to wait for another time. So I guess that this is it for now, but most of all, have fun with all of your projects.
