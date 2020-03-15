#include <stdlib.h>
#include "pms_driver.h"

#define PMS_STARTCHAR1 0x42
#define PMS_STARTCHAR2 0x4D
#define PMS_FRAMELEN 28

#define PMS_CMD_READPASSIVE 0xE2
#define PMS_CMD_CHANGEMODE 0xE1

//static pmsError_t pms3003SendCommand(uint8_t command, uint16_t data);

//pmsX003Result_t pmsX003ChangeMode(pmsX003Mode_t mode)
//{
//    pmsX003Result_t result = pms3003SendCommand(PMS_CMD_CHANGEMODE, mode);
//    if (result != PMSX003_OK) return result;
//
//    s_currentMode = mode;
//
//    return PMSX003_OK;
//}

pmsError_t pmsRead(const pmsParams_t* params, pmsData_t* data)
{
	size_t frameLen = params->model == PMS_3003 ? 24 : 32;

    uint8_t buffer[32]; // max frameLen is 32
    uint8_t* bufferPtr = buffer;
    
    if (params->dataReadFun(bufferPtr, 1) != 1) return PMS_ERR_DATA_READ;
    if (*bufferPtr != PMS_STARTCHAR1) return PMS_ERR_INVALID_STARTCHAR;
    bufferPtr++;

    if (params->dataReadFun(bufferPtr, 1) != 1) return PMS_ERR_DATA_READ;
    if (*bufferPtr != PMS_STARTCHAR2) return PMS_ERR_INVALID_STARTCHAR;
    bufferPtr++;
    
    size_t remaining = frameLen - 2;
    while (remaining > 0)
    {
        int read = params->dataReadFun(bufferPtr, remaining);
        if (read <= 0) return PMS_ERR_DATA_READ;
        remaining -= read;
        bufferPtr += read;
    }

    uint16_t dataLength = (buffer[2] << 8) | buffer[3];
    if (dataLength != frameLen - 4) return PMS_ERR_INVALID_FRAMELENGTH;

    data->pm1Indoor = (buffer[4] << 8) | buffer[5]; // Data 1
    data->pm2_5Indoor = (buffer[6] << 8) | buffer[7]; // Data 2
    data->pm10Indoor = (buffer[8] << 8) | buffer[9]; // Data 3
    data->pm1Outdoor = (buffer[10] << 8) | buffer[11]; // Data 4
    data->pm2_5Outdoor = (buffer[12] << 8) | buffer[13]; // Data 5
    data->pm10Outdoor = (buffer[14] << 8) | buffer[15]; // Data 6

    uint16_t checksum = (buffer[frameLen - 2] << 8) | buffer[frameLen - 1]; // Data and check

    for (size_t i = 0; i < frameLen - 2; i++)
    {
        checksum -= buffer[i];
    }

    if (checksum != 0) return PMS_ERR_INVALID_CHECKSUM;

    return PMS_OK;
}

//static pmsX003Result_t pmsX003SendCommand(uint8_t command, uint16_t data)
//{
//    uint8_t buffer[7];
//
//    buffer[0] = PMS_STARTCHAR1;
//    buffer[1] = PMS_STARTCHAR2;
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
