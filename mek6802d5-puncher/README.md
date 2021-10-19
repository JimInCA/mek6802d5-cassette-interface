# MEK6802D5 Puncher

The puncher program is used to retrieve data from the MEK6802D5's memory, either RAM or ROM.

---
## Installing Required Software
---

You will need to download and install the gcc compiler along with the tools that you'll need to run make. You can do this through either Cygwin or MinGW.  The choice is up to you as to which one you want to use.  Here are some links to get you started:
```
https://sourceware.org/cygwin/
http://www.mingw.org/
```
To me, this always seems to be a little bit of a struggle for there isn't one single application installer that seems to do everything that you need.  So it gets to be a little bit of a hunt-and-peck process, but stick with it and you'll eventually get things to work properly.

---
## Building the loader program
---

Once you can run make and have it call gcc, building the program is as simple as entering the following command:
```
$ cd <path>/mek6802d5-puncher
$ make all
gcc -I./include -c -o obj/mek6802d5-puncher.o src/mek6802d5-puncher.c
Compiled obj/mek6802d5-puncher.o successfully!
gcc -o  bin/mek6802d5-puncher obj/mek6802d5-puncher.o
Linking bin/mek6802d5-puncher complete!
```

---
## Puncher Program Arguments
---

Here is an explanation of the different user arguments available for the puncher application:
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

---
## Running the Loader Program
---

Here is an example execution of the puncher program:
```
$ ./bin/mek6802d5-puncher.exe -f help_bytes.txt -p COM20 -b 115200
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
