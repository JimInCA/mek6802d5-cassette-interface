/*
 * Copyright(c) 2021 - Jim Newman
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following
 * conditions:

 * The above copyright notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*   The format for the incomming data stream should be as follows:

     a) 30 seconds of 0xff characters as leader.
     b) Block start character "S" (0x53).
     c) Begin address high byte.
     d) Begin address low byte.
     e) End address high byte.
     f) End address low byte.
     g) Data in binary for starting with the data at the Begin
        address to and including data at the End address.
     h) One byte checksum.  Checksum is the two's complement 
        summation of all data bytes plus the begin and end
        address characters.  When loading, the sample sum of 
        all data in the file starting with the first byte of 
        the begin address and including the checksum character 
        should be zero.
*/

#include <Windows.h>
#include <stdio.h>
#include <stdint.h>

#define MAX_ARRAY 4096
#define DEFAULT_BAUD_RATE 115200

HANDLE uart;

int16_t init_uart(HANDLE *uart, uint8_t *serial_port, uint32_t baud, uint32_t stopbits)
{
    COMMTIMEOUTS timeouts = { 0 };

    timeouts.ReadIntervalTimeout = 20;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.ReadTotalTimeoutConstant = 100;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 100;

    uint8_t adjusted_serial_port[16];

    sprintf(adjusted_serial_port, "\\\\.\\%s", serial_port);
    *uart = CreateFile(adjusted_serial_port,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (*uart == INVALID_HANDLE_VALUE)
    {
        printf("Failed to connet to UART on port %s\n", serial_port);
        return 1;
    }
    else
    {
        printf("Successfully connected to UART on port %s at baud rate %d.\n", serial_port, baud);
    }

    DCB dcb = { 0 };

    dcb.DCBlength = sizeof(dcb);

    BOOL ret = GetCommState(*uart, &dcb);

    if (ret == FALSE)
    {
        ret = GetLastError();
        printf("Error getting current DCB settings: %d\n", ret);
    }
    //else
    //    printf("DCB is ready for use.\n");

    FillMemory(&dcb, sizeof(dcb), 0);
    if (!GetCommState(*uart, &dcb))     // get current DCB
    {
        printf("Error in GetCommState.\n");
        return 1;
    }

    // Update DCB rate.
    dcb.BaudRate = baud;
    dcb.StopBits = stopbits;

    // Set new state.
    if (!SetCommState(*uart, &dcb))
    {
        printf("Error in SetCommState. Possibly a problem with the communications,\n");
        printf("port handle, or a problem with the DCB structure itself.\n");
        return 1;
    }

    if (!SetCommTimeouts(*uart, &timeouts))
        printf("Error setting time-outs.\n");

    return 0;
}

int main(int argc, char **argv)
{
    int16_t error = 0;

    char *portname = NULL;
    char *file_name = NULL;
    FILE *fp = NULL;

    unsigned long baudrate = DEFAULT_BAUD_RATE;
    unsigned long help = 0;
    unsigned long verbose = 0;

    int16_t i, j, k;
    long unsigned int num_read;
    uint8_t buff[16];

    uint8_t array[MAX_ARRAY];
    uint16_t array_num = 0;
    uint16_t start_address;
    uint16_t end_address;
    uint16_t num_bytes;
    uint8_t checksum;

    // simple command line parser
    for (i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-f") == 0)
        {
            file_name = argv[++i];
        }
        else if (strcmp(argv[i], "-p") == 0)
        {
            portname = argv[++i];
        }
        else if (strcmp(argv[i], "-b") == 0)
        {
            baudrate = atol(argv[++i]);
        }
        else if (strcmp(argv[i], "-h") == 0)
        {
            help = 1;
        }
        else if (strcmp(argv[i], "-v") == 0)
        {
            verbose = 1;
        }
        else
        {
            printf("Unknown input paramter: %s\n", argv[i]);
            return -1;
        }
    }

    if ((help == 1) || (portname == NULL))
    {
        int result = 0;
        printf("usage: mek6802d5-puncher [-h] [-f FILE] -p COMPORT [-b BAUDRATE] [-v]\n\n");
        if (help == 1)
        {
            printf("arguments:\n");
            printf("  -h           Show this help message and exit.\n");
            printf("  -f FILE      Output Filename.\n");
            printf("  -p COMPORT   COM Port.\n");
            printf("  -b BAUDRATE  Baud rate, default: %d.\n", baudrate);
            printf("  -v           Increase output verbosity.\n");
        }
        else
        {
            if (portname == NULL)
                printf("mek6802d5-puncher error: argument -p COMPORT is required.\n");
            result = -1;
        }
        return result;
    }

    if (init_uart(&uart, portname, baudrate, ONESTOPBIT) != 0)
    {
        printf("Error opening %s: %s\n", portname, strerror(errno));
        return -1;
    }

    printf("Connected to port %s at baud rate %ld\n", portname, baudrate);
    fflush(stdout);

    if (file_name != NULL)
    {
        // let's open the file and check for errors
        fp = fopen(file_name, "w");
        if (fp == NULL)
        {
            printf("Error while opening file %s\n", file_name);
            return -1;
        }
        if (verbose)
            printf("Opened output file: %s\n", file_name);
    }

    // let's see if we can flush any waiting bytes from the uart buffer before we begin
    do
    {
        ReadFile(uart, buff, sizeof(buff), &num_read, NULL);
    } while ((num_read != 0) && (buff[0] != 0xff));

    buff[0] = 0x00;
    while (buff[0] != 0xff)
        ReadFile(uart, buff, sizeof(buff), &num_read, NULL);

    if (verbose)
    {
        printf("Reading Leader Characters\n");
    }
    fflush(stdout);

    // flush out the leader characters 
    while (buff[0] == 0xff)
    {
        ReadFile(uart, buff, sizeof(buff), &num_read, NULL);
        for (j = 0, k = 0; j < num_read; j++)
        {
            if (buff[j] != 0xff)  // this may be the start of the data
            {
                for ( ; j < num_read; j++)
                {
                    array[array_num++] = buff[j];  // so save to array
                }
                buff[0] = 0x00;
            }
        }
    }

    // We need to read at least the first five bytes so we can check the 
    // block start character and get the start and end addresses.
    while (array_num < 5)
    {
        ReadFile(uart, buff, sizeof(buff), &num_read, NULL);
        for (j = 0, k = 0; j < num_read; j++)
        {
            array[array_num++] = buff[j];
        }
    }

    if (array[0] == 0x53)                        // verify the block start character
    {
        start_address = (array[1] << 8) + array[2];  // get 16 bit start address
        end_address = (array[3] << 8) + array[4];    // get 16 bit end address
        num_bytes = end_address - start_address + 7; // calculate total num of bytes
    }
    else
    {
        printf("Error: Invalid Block Start Character 0x%02x\n", array[0]);
    }

    while (array_num < num_bytes)   // get the rest of the data and the checksum
    {
        ReadFile(uart, buff, sizeof(buff), &num_read, NULL);
        for (j = 0; j < num_read; j++)
        {
            array[array_num++] = buff[j];
        }
    }

    // we need to add a check for the checksum
    for (i = 1, checksum = 0; i < array_num; i++)
    {
        checksum += array[i];
    }
    if (checksum != 0)  // checksum should be equal to zero
    {
        printf("Checksum Error: 0x%02x\n", checksum);
        if (!verbose)
            return -1;
    }
    else if (verbose)
    {
        printf("Checksum passed: 0x%02x\n", checksum);
    }

    // for now, let's print out the data to the screen
    uint8_t out_string[32];
    for (i = 0, j = 0; i < array_num; i++)
    {
        if ((i < 5) || (i == (array_num - 1)))
        {
            sprintf(out_string, "        0x%02x\n", array[i]);
            if (verbose)
            {
                printf("%s", out_string);
                if (fp != NULL)
                    fputs(out_string, fp);
            }
        }
        else
        {   // just for fun, let's include the memory address
            sprintf(out_string, "0x%04x  0x%02x\n", (start_address + j++), array[i]);
            printf("%s", out_string);
            if (fp != NULL)
                fputs(out_string, fp);
        }
    }

    // clean up
    if (fp != NULL)
    {
        fclose(fp);
    }
    CloseHandle(uart);

    return 0;
}

