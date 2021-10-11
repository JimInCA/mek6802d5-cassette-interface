
# Cassette Interface for the Motorola MEK6802D5

The intent of this project is to emulate the cassette interface for the Motorola MEK6802D5 Development Board using modern technology.  

The MEK6802D5 has the ability to save it's current contents in memory to an external cassette tape drive.  It also has the ability to load a program saved on a cassette tape drive to it's internal memory.  The intent of this project is to replace the cassette tape drive with the NXP FRDM-K64F Development Board.

I originally implemented a cassette interface for the MEK6802D5 using a circuit called the bit-boffer which uses 1970's vintage technology.  The bit-boffer was a lot of fun to build and it worked perfectly.  But it is quite cumbersome requiring two solderless breadboards, eight ICs, two Teensy development boards, and countless discreet components and wires.  So I decided to duplicate the functionally of the bit-boffer but using modern technology.  I decided to use the NXP FRDM-K64F board because it has a DAC that I can use to implement the transmitter and a comparator that I can use to implement the receiver.  I also just happened to have one of these boards in my vast collection of prototype/development boards, so the decision to use this was simple.  You can find my original project at [Cassette Tape Emulator](https://github.com/JimInCA/cassette-tape-emulator).

Here's a photo of my set-up for the FRDM-K64F board.  I have it working as a prototype where I'm wrapping the transmitter's output on pin J4[11], into the receiver's input in pin J1[13].  

![alt text](./images/FRDM-K64F-Board.jpg?raw=true "MEK6802D5")

Once I have everything tested, I'll connect it to my MEK6802D5 to test it's functionality in a real application.

---
## MEK6802D5 Cassette Interface
---

The Cassette Interface on the MEK6802D5 uses Kansas City Standard (KCS) encoding to write and retrieve data to and from a cassette tape.  So the question is then, what is Kansas City Standard?  KCS was developed in 1975 as a means to save and retrieve computer data using an inexpensive consumer quality cassette tape player.  What is specifies is a process to convert computer data into two different audio frequencies that could then be written to or retrieved from to the tape.  

What the MEK6802D5 does is it takes the output from the TX pin on a UART and converts all low data bits to 1200 hertz and all high data to 2400 hertz.  The over all baud rate is set to 300 bits per second, so this means that a low data bit will equal four cycles at 1200 hertz and a high data bit will equal eight cycles at 2400 hertz.  It reverses the process on reading the data from a tape by converting four cycles at 1200 hertz to a zero and eight cycles at 2400 hertz to a one and sends this data stream to the RX pin on the UART.

The intent of this project it to emulate the audio tape by replacing it with the FRDM-K64F.

---
## Overall Project Structure
---

The following image shows the basic concept for this application.

![alt text](./images/cassette_interface.jpeg?raw=true "MEK6802D5")

<ul>
<li>J4[11]/DAC0_OUT Transmitter DAC Output</li>
<li>J1[13]/CMP0_IN1 Receiver Comparator Input</li>
<li>J1[15]/CMP0_OUT Receiver Comparator Output for debugging</li>
<li>J1[1]/PTB18 Receiver Decoded UART Output for debugging</li>
<li>J1[8]/PTA1 SCOPE_TRIGGER for debugging</li>
<li>J2[3] GND</li>
<li>J3[12] GND</li>
<li>J3[14] GND</li>
</ul>

There are two portions to the project; Emulation of the audio signal that would normally be coming from the tape (Transmitter) and Decoding of the audio signal tha would normally be written to the tape (Receiver).  

### **Transmitter**

The transmitter portion of the design takes the data coming from the host and converts it into the two different KCS frequencies.  As a starting point, what the code does is it caches the incoming data into a queue.  The main loop in the program than checks to see if there is any data in this queue.  If there is, it then sends the data to the routines that converts the zeros to four cycles of data at 1200Hz and the ones into eight cycles at 2400Hz.  It formats the data into what is required for a UART device by first generation the start bit, followed by the eight data bits, LSB first, and ending with two stop bits.  The code uses a timer to generate the correct period to update the 12 bit DAC.  The sign wave is divided into thirty two steps so this means that the timer must be set to the correct partial period depending on if a zero or one is being generated.  Control is returned to the main loop once the correct number of cycles have been generated.  If there is no data in the queue, the main loop just generates the signal for a one to maintain a constant signal.

The output from the DAC, DAC0_OUT, is on device pin J4[11].  The output on J4[11] will need to be connected to the "EAR" input on P3 on the D5.  There is also a signal coming from the board on J1[8] that can be used as a scope trigger.  The scope trigger is generated at the start of the output signal.

### **Receiver**

The receiver portion of the design uses a comparator to convert the incoming sign wave into a square wave.  This is done by setting the 6-bit dac to an appropriate level to generate a 50-50 duty cycle on the output of the comparator.  What the code does is it measures the period between contiguous rising edges to determine the frequency of the incoming signal.  It then sets the decoded uart output to either a zero or one based on the incoming frequency.  

This is where there was a departure from my original design.  I originally intended to send the decoded uart output signal to an on-board UART.  But there is a limitation on this board with respect to it's UART peripherals.  The on-board UART peripherals are clocked at too high of a frequency to be able to generate a 300 baud rate.  What happens is the internal UART counter overflows and you get a different frequency than you expect and the over flow bits also corrupts the state of other bits in the UART register.  I've filed a bug report with NXP on this issue (NXP support case: #00425437).

To get around the issue of the UART not supporting 300 baud, I had to add my own decoding of the input signal.  So when I detect the falling edge of the start signal, I first delay for one half of the period, sample, and then continue to delay for one full period and sample for the next ten bits (start, eight data bits, two stop bits equals eleven bits).  This is a little bit of overkill at this point for I could simple record the bits as they toggle the decoded uart signal.  But I wanted to keep my original design for now.  

Once the eight data bits are resolved, the code places the byte on an output queue and returns.  The main loops looks at this queue (as well as the transmitter queue) and if there is any data available, it sends the data to the host.

The input to the receiver's comparator is on signal COMP0_IN1 on pin J1[13].  This input needs to be connected to the "MIC' output on P2 on the D5.  For set up and debugging, you can use a scope to monitor the comparator's output on board pin J1[15] to verify the 50-50 duty cycle.  The decoded uart signal at 300 baud is also presented on the board's J1[1] pin.  If you like, you can send this signal to an external UART.  I like using the Teensy myself.  Just remember that the other device must be 3.3V compatible and that the UART must be set to 300 baud and two stop bits.  

---
## Testing
---

Just as a quick note, you can test this design by connecting DAC0_OUT on pin J4[11] to CMP0_IN1 on pin J1[13].  You can then use the following test case and verify that everything is working prior to connecting the board to an actual MEK6802D5. 

---
## test_cassette_interface
---

NOTE: I modified my bit-boffer test program to work with the FRDM-K64F board.  My design with the bit-boffer required two Teensy boards with both boards running at 300 baud.  This design requires just a single FRDM-K64F with the com port running at 115200 baud.  It is still possible to test the decoded uart signal coming from the MEK6802D5 by connecting a Teensy's RX pin to pin J1[1] on the FRDM-K64F board.  But I use this mode just for testing.

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

---
## mek6802d5-loader
---

TBD

That's it for now and most of all, have fun with all of your projects.
