/*

    Utility functions for use with the Euresys Multicam FG Serial Port communications.
    These functions allow communication with CameraLink cameras using the Euresys
    serial library clseremc.lib 

    Note: This file was largely stolen from the Euresys sample program called Terminal
*/


#ifndef _EURESYS_SERIAL_H_
#define _EURESYS_SERIAL_H_

#include <vector>
#include <string>
#include <memory>

#include "Logger.h"

#ifdef __GNUC__
#define CLSEREMC_API
#define CLSEREMC_CC
#else

#ifdef CLSEREMC_EXPORTS
#define CLSEREMC_API __declspec(dllexport)
#else
#define CLSEREMC_API __declspec(dllimport)
#endif

#define CLSEREMC_CC __cdecl

#endif 


//////////////////////////////////////////////////////////////////////
// Constants
//////////////////////////////////////////////////////////////////////

// Baud rates
#define CL_BAUDRATE_9600                        1
#define CL_BAUDRATE_19200                       2
#define CL_BAUDRATE_38400                       4
#define CL_BAUDRATE_57600                       8
#define CL_BAUDRATE_115200                      16
#define CL_BAUDRATE_230400                      32
#define CL_BAUDRATE_460800                      64
#define CL_BAUDRATE_921600                      128

//////////////////////////////////////////////////////////////////////
//  Error Codes
//////////////////////////////////////////////////////////////////////
#define CL_ERR_NO_ERR                           0
#define CL_ERR_BUFFER_TOO_SMALL                 -10001
#define CL_ERR_MANU_DOES_NOT_EXIST              -10002
#define CL_ERR_UNABLE_TO_OPEN_PORT              -10003
#define CL_ERR_PORT_IN_USE                      -10003
#define CL_ERR_TIMEOUT                          -10004
#define CL_ERR_INVALID_INDEX                    -10005
#define CL_ERR_INVALID_REFERENCE                -10006
#define CL_ERR_ERROR_NOT_FOUND                  -10007
#define CL_ERR_BAUD_RATE_NOT_SUPPORTED          -10008
#define CL_ERR_UNABLE_TO_LOAD_DLL               -10098
#define CL_ERR_FUNCTION_NOT_FOUND               -10099

#define TERMINAL_STRING_SIZE 128

// ***************************************************************************
#ifdef __cplusplus
extern "C" {
#endif
    CLSEREMC_API int CLSEREMC_CC clSerialInit(unsigned long SerialIndex, void** SerialRefPtr);
    CLSEREMC_API int CLSEREMC_CC clSerialWrite(void* SerialRef, char* Buffer, unsigned long* BufferSize, unsigned long SerialTimeout);
    CLSEREMC_API int CLSEREMC_CC clSerialRead(void* SerialRef, char* Buffer, unsigned long* BufferSize, unsigned long SerialTimeout);
    CLSEREMC_API int CLSEREMC_CC clSerialClose(void* SerialRef);

    CLSEREMC_API int CLSEREMC_CC clGetManufacturerInfo(char* ManufacturerName, unsigned int* BufferSize, unsigned int *Version);
    CLSEREMC_API int CLSEREMC_CC clGetNumSerialPorts(unsigned int* NumSerialPorts);
    CLSEREMC_API int CLSEREMC_CC clGetSerialPortIdentifier(unsigned long SerialIndex, char* PortId, unsigned long* BufferSize);
    CLSEREMC_API int CLSEREMC_CC clGetSupportedBaudRates(void *SerialRef, unsigned int* BaudRates);
    CLSEREMC_API int CLSEREMC_CC clSetBaudRate(void* SerialRef, unsigned int BaudRate);
    CLSEREMC_API int CLSEREMC_CC clGetErrorText(int ErrorCode, char *ErrorText, unsigned int *ErrorTextSize);
    CLSEREMC_API int CLSEREMC_CC clGetNumBytesAvail(void *SerialRef, unsigned int *NumBytes);
    CLSEREMC_API int CLSEREMC_CC clFlushInputBuffer(void *SerialRef);
#ifdef __cplusplus
};
#endif

std::string Error_valueToStr(int errorValue);
int BaudRate2Id (int BaudRate);

void* connectToCamera(unsigned int BaudRate, unsigned int PortId, std::shared_ptr<Logger> logger);

bool sendReadFrameWaitForReturn(void* SerialRefPtr, std::vector<unsigned char> frame, std::vector<unsigned char>& returnBytes, std::shared_ptr<Logger> logger);
bool sendWriteFrameWaitForReturn(void* SerialRefPtr, std::vector<unsigned char> frame, std::vector<unsigned char>& returnBytes, std::shared_ptr<Logger> logger);

//--------------------------------------------------------------------------------------------------------


#endif //_EURESYS_SERIAL_H_
