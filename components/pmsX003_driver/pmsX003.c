#include "pmsX003.h"

#include <driver/uart.h>
#include <esp_log.h>
#include <stdlib.h>

#define PMS3003_STARTCHAR1 0x42
#define PMS3003_STARTCHAR2 0x4D
#define PMS3003_FRAMELEN 28

#define PMS3003_COMMAND_READPASSIVE 0xE2
#define PMS3003_COMMAND_CHANGEMODE 0xE1

static pmsX003Device_t s_device;
static uint8_t s_frameLen;
static uart_port_t s_uartPort;
static TickType_t s_uartTimeoutTicks;
static pmsX003Mode_t s_currentMode;

static pmsX003Result_t pms3003SendCommand(uint8_t command, uint16_t data);

pmsX003Result_t pmsX003_init(pmsX003Device_t device, uart_port_t uartPort, TickType_t uartTimeoutTicks, pmsX003Mode_t mode)
{
	s_device = device;
	s_frameLen = device == PMS3003 ? 24 : 32; // pms3003 = 20, pms7003 = 28
    s_uartPort = uartPort;
    s_uartTimeoutTicks = uartTimeoutTicks;

    //pmsX003Result_t result = pms3003ChangeMode(mode);
    //if (result != PMSX003_OK) return result;

    return PMSX003_OK;
}

//pmsX003Result_t pmsX003ChangeMode(pmsX003Mode_t mode)
//{
//    pmsX003Result_t result = pms3003SendCommand(PMS3003_COMMAND_CHANGEMODE, mode);
//    if (result != PMSX003_OK) return result;
//
//    s_currentMode = mode;
//
//    return PMSX003_OK;
//}

pmsX003Result_t pmsX003_read(pmsX003Data_t* data)
{
//    if (s_currentMode == PMSX003_MODE_PASSIVE)
//    {
//        pms3003SendCommand(PMS3003_COMMAND_READPASSIVE, 0);
//    }

    uint8_t buffer[32]; // max framelen is 32
    uint8_t* bufferPtr = buffer;
    
    if (uart_read_bytes(s_uartPort, bufferPtr, 1, s_uartTimeoutTicks) != 1) return PMSX003_UART_ERROR;
    if (*bufferPtr != PMS3003_STARTCHAR1) return PMSX003_INVALID_STARTCHAR;
    bufferPtr++;

    if (uart_read_bytes(s_uartPort, bufferPtr, 1, s_uartTimeoutTicks) != 1) return PMSX003_UART_ERROR;
    if (*bufferPtr != PMS3003_STARTCHAR2) return PMSX003_INVALID_STARTCHAR;
    bufferPtr++;
    
    size_t remaining = s_frameLen - 2;
    while (remaining > 0)
    {
        int read = uart_read_bytes(s_uartPort, bufferPtr, remaining, s_uartTimeoutTicks);
        if (read == -1) return PMSX003_UART_ERROR;
        remaining -= read;
        bufferPtr += read;
    }

    uint16_t dataLength = (buffer[2] << 8) | buffer[3];
    if (dataLength != s_frameLen - 4) return PMSX003_INVALID_FRAMELENGTH;

    data->pm1Indoor = (buffer[4] << 8) | buffer[5]; // Data 1
    data->pm2_5Indoor = (buffer[6] << 8) | buffer[7]; // Data 2
    data->pm10Indoor = (buffer[8] << 8) | buffer[9]; // Data 3
    data->pm1Outdoor = (buffer[10] << 8) | buffer[11]; // Data 4
    data->pm2_5Outdoor = (buffer[12] << 8) | buffer[13]; // Data 5
    data->pm10Outdoor = (buffer[14] << 8) | buffer[15]; // Data 6

    uint16_t checksum = (buffer[s_frameLen - 2] << 8) | buffer[s_frameLen - 1]; // Data and check

    for (size_t i = 0; i < s_frameLen - 2; i++)
    {
        checksum -= buffer[i];
    }

    if (checksum != 0) return PMSX003_INVALID_CHECKSUM;

    return PMSX003_OK;
}

//static pmsX003Result_t pmsX003SendCommand(uint8_t command, uint16_t data)
//{
//    uint8_t buffer[7];
//
//    buffer[0] = PMS3003_STARTCHAR1;
//    buffer[1] = PMS3003_STARTCHAR2;
//    buffer[2] = command;
//    buffer[3] = (uint8_t)(data >> 8);
//    buffer[4] = (uint8_t)data;
//
//    uint16_t checksum = buffer[0] + buffer[1] + buffer[2] + buffer[3] + buffer[4];
//
//    buffer[5] = (uint8_t)(checksum >> 8);
//    buffer[6] = (uint8_t)checksum;
//
//    char* bufferPtr = (char*)buffer;
//    size_t remaining = 7;
//
//    while (remaining > 0)
//    {
//        int written = uart_write_bytes(s_uartPort, bufferPtr, remaining);
//        if (written == -1) return PMSX003_UART_ERROR;
//        remaining -= written;
//        bufferPtr += written;
//    }
//
//    return PMSX003_OK;
//}
