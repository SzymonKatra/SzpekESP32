#ifndef _PMS3003_H
#define _PMS3003_H

#include <freertos/FreeRTOS.h>
#include <stdint.h>
#include <driver/uart.h>

typedef enum
{
	PMS3003,
	PMS7003
} pmsX003Device_t;

typedef struct
{
    uint16_t pm1Indoor;
    uint16_t pm2_5Indoor;
    uint16_t pm10Indoor;
    uint16_t pm1Outdoor;
    uint16_t pm2_5Outdoor;
    uint16_t pm10Outdoor;
} pmsX003Data_t;

typedef enum
{
    PMSX003_MODE_PASSIVE = 0x00,
    PMSX003_MODE_ACTIVE = 0x01
} pmsX003Mode_t;

typedef enum {
    PMSX003_OK = 0,
    PMSX003_INVALID_CHECKSUM = 1,
    PMSX003_INVALID_STARTCHAR = 2,
    PMSX003_INVALID_FRAMELENGTH = 3,
    PMSX003_UART_ERROR = 0xFF,
} pmsX003Result_t;

pmsX003Result_t pmsX003_init(pmsX003Device_t device, uart_port_t uartPort, TickType_t uartTimeoutTicks, pmsX003Mode_t mode);
//pmsX003Result_t pmsX003ChangeMode(pmsX003Mode_t mode);
pmsX003Result_t pmsX003_read(pmsX003Data_t* data);

#endif
