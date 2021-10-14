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

#include <Windows.h>
#include <stdio.h>
#include <stdint.h>

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

    unsigned long baudrate = 115200;
    unsigned long help = 0;

    int16_t i, j;
    long unsigned int num_read;
    uint8_t buff[16];

    // simple command line parser
    for (i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-p") == 0)
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
        else
        {
            printf("Unknown input paramter: %s\n", argv[i]);
            return -1;
        }
    }

    if ((help == 1) || (portname == NULL))
    {
        int result = 0;
        printf("usage: mek6802d5-puncher [-h] -p COMPORT [-b BAUDRATE]\n\n");
        if (help == 1)
        {
            printf("arguments:\n");
            printf("  -h             Show this help message and exit.\n");
            printf("  -p  COMPORT    COM Port.\n");
            printf("  -b  BAUDRATE   Baud rate, default: %d.\n", baudrate);
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

    // let's see if we can flush any waiting bytes from the uart buffer before we begin
    do
    {
        ReadFile(uart, buff, sizeof(buff), &num_read, NULL);
    } while (num_read != 0);

    while (1)
    {
        ReadFile(uart, buff, sizeof(buff), &num_read, NULL);
        for (j = 0; j < num_read; j++)
        {
            printf("0x%02x\n", buff[j]);
            fflush(stdout);
        }
    }

    // clean up
    CloseHandle(uart);

    return 0;
}

