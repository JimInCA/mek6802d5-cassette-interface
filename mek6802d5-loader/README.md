MEK6802D5 Loader
===

Installing Required Software
---
You will need to download and install the gcc compiler along with the tools that you'll need to run make. You can do this through either Cygwin or MinGW.  The choice is up to you as to which one you want to use.  Here are some links to get you started:
```
https://sourceware.org/cygwin/
http://www.mingw.org/
```
To me, this always seems to be a little bit of a struggle for there isn't one single application installer that seems to do everything that you need.  So it gets to be a little bit of a hunt-and-peck process, but stick with it and you'll eventually get things to work properly.

Building the loader program
---
Once you can run make and have it call gcc, building the program is as simple as entering the following command:
```
$ cd <path>/mek6802d5-loader
$ make all
gcc -I./include -c -o obj/mek6802d5-loader.o src/mek6802d5-loader.c
Compiled obj/mek6802d5-loader.o successfully!
gcc -o  bin/mek6802d5-loader obj/mek6802d5-loader.o
Linking bin/mek6802d5-loader complete!
```

Loader Program Arguments
---
Here is an explanation of the different user arguments available for the loader application:
```
usage: mek6802d5-loader [-h] -f FILE -p COMPORT [-b BAUDRATE]

arguments:
  -h             Show this help message and exit.
  -f FILE        Filename of S-Record input file.
  -p COMPORT     COM Port to which the device is connected.
  -b BAUDRATE    Desired baud rate, default: 115200.
```
Please be aware that the file and com port are mandatory.

Running the Loader Program
---
TBD

TBD:
```
$ ./bin/mek6802d5-loader.exe -f ./test/used5.s19 -p COM22
Successfully connected to UART on port COM20 at baud rate 115200.
DCB is ready for use.
Sending file ./test/used5.s19 to port COM20 at baudrate 115200
```

