#ifndef _PMS3003_H
#define _PMS3003_H

#include <stdint.h>

typedef enum
{
	PMS_3003,
	PMS_7003
} pmsDeviceModel_t;

typedef struct
{
    uint16_t pm1Indoor;
    uint16_t pm2_5Indoor;
    uint16_t pm10Indoor;
    uint16_t pm1Outdoor;
    uint16_t pm2_5Outdoor;
    uint16_t pm10Outdoor;
} pmsData_t;

typedef enum
{
    PMS_MODE_PASSIVE = 0x00,
    PMS_MODE_ACTIVE = 0x01
} pmsMode_t;

typedef enum {
    PMS_OK = 0,
    PMS_ERR_INVALID_CHECKSUM = 1,
    PMS_ERR_INVALID_STARTCHAR = 2,
    PMS_ERR_INVALID_FRAMELENGTH = 3,
    PMS_ERR_DATA_READ = 0xFF,
} pmsError_t;


typedef int (*pmsDataReadFun_t)(uint8_t* buffer, size_t len);

typedef struct
{
	pmsDeviceModel_t model;
	pmsDataReadFun_t dataReadFun;

} pmsParams_t;

//pmsX003Result_t pmsX003ChangeMode(pmsX003Mode_t mode);
pmsError_t pmsRead(const pmsParams_t* params, pmsData_t* data);

#endif
