/*

    Utility functions for use with the Euresys Multicam FG Serial Port communications.
    These functions allow communication with CameraLink cameras using the Euresys
    serial library clseremc.lib 

    Note: This file was largely stolen from the Euresys sample program called Terminal
*/

#include <unistd.h>  //usleep

#include "Euresys_Serial.h"
#include "BBP_utils.h"

int BaudRate2Id (int BaudRate)
{
    switch(BaudRate)
    {
    case 9600:
        return CL_BAUDRATE_9600;
    case 19200:
        return CL_BAUDRATE_19200;
    case 38400:
        return CL_BAUDRATE_38400;
    case 57600:
        return CL_BAUDRATE_57600;
    case 115200:
        return CL_BAUDRATE_115200;
    case 230400:
        return CL_BAUDRATE_230400;
    case 460800:
        return CL_BAUDRATE_460800;
    case 921600:
        return CL_BAUDRATE_921600;
    default:
        return 0;
    }
}

std::string Error_valueToStr(int errorValue)
{
    std::string str;
    switch (errorValue)
    {
        case 0:      str = "CL_ERR_NO_ERR"; break;
        case -10001: str = "CL_ERR_BUFFER_TOO_SMALL"; break;                 
        case -10002: str = "CL_ERR_MANU_DOES_NOT_EXIST"; break;              
        case -10003: str = "CL_ERR_UNABLE_TO_OPEN_PORT"; break;              
        case -10004: str = "CL_ERR_TIMEOUT"; break;                          
        case -10005: str = "CL_ERR_INVALID_INDEX"; break;                    
        case -10006: str = "CL_ERR_INVALID_REFERENCE"; break;                
        case -10007: str = "CL_ERR_ERROR_NOT_FOUND"; break;                  
        case -10008: str = "CL_ERR_BAUD_RATE_NOT_SUPPORTED"; break;          
        case -10098: str = "CL_ERR_UNABLE_TO_LOAD_DLL"; break;               
        case -10099: str = "CL_ERR_FUNCTION_NOT_FOUND"; break;               
        default: str = "unknown error value";
    }
    return str;
}


void* connectToCamera(unsigned int BaudRate, unsigned int PortId, std::shared_ptr<Logger> logger)
{
    unsigned int SupportedBaudRate, BaudRateId;
    unsigned int NumPorts;
    int status;
    unsigned long size;    
    char PortName[TERMINAL_STRING_SIZE];
    void* SerialRefPtr = nullptr;

    // Enumerate Camera Link Serial Ports
    status = clGetNumSerialPorts (&NumPorts);
    if (status != CL_ERR_NO_ERR)
    {
        logger->LogError("clGetNumSerialPorts error: ", status, ": ", Error_valueToStr(status));
        return nullptr;
    }

    if (NumPorts == 0)
    {
        logger->LogWarn("\nSorry, no serial port detected.\n");
        logger->LogWarn("Check if a GrabLink is present in your system and if the drivers are correctly loaded...\n");
        return nullptr;
    }

    logger->LogInfo("\nDetected ports:\n");
    for (int i=0; i<NumPorts; i++)
    {
        size = TERMINAL_STRING_SIZE;
        status = clGetSerialPortIdentifier (i, PortName, &size);
        std::cout << " - Serial Index : " << i << " - " << PortName << std::endl;
    }

    // Camera Link Serial Port Selection
    logger->LogInfo ("Port selected : ", PortId);

    // Initialize Camera Link Serial Connetion
    status = clSerialInit (PortId, &SerialRefPtr);
    if (status != CL_ERR_NO_ERR)
    {
        logger->LogError("clSerialInit error ", status, ": ", Error_valueToStr(status));
        return nullptr;
    }

    // Camera Link Serial Port Baudrate Selection
    status = clGetSupportedBaudRates (SerialRefPtr, &SupportedBaudRate);
    if (status != CL_ERR_NO_ERR)
    {
        logger->LogError("clGetSupportedBaudRates error: ", status, ": ", Error_valueToStr(status));
        return nullptr;
    }
    
    BaudRateId=BaudRate2Id(BaudRate);
    logger->LogInfo ("Baudrate selected : ", BaudRate, ", BaudRateId=", BaudRateId);

    // Set Camera Link Serial Port Baudrate 
    status = clSetBaudRate (SerialRefPtr, BaudRateId);
    if (status != CL_ERR_NO_ERR)
    {
        logger->LogError("clSetBaudRate error: ", status, ": ", Error_valueToStr(status));
        return nullptr;
    }

    return SerialRefPtr;
}

bool sendReadFrameWaitForReturn(void* SerialRefPtr, std::vector<unsigned char> readFrame, std::vector<unsigned char>& returnBytes, std::shared_ptr<Logger> logger)
{
    int status;
    unsigned long size;
    char inByte;

    bool msgInProgress = false;
    std::vector<unsigned char> inBytes;

    int dataStartByte = 3;
    int dataLen = 4;  //default

    size = readFrame.size();
    status = clSerialWrite (SerialRefPtr,  (char*)readFrame.data(), &size, 1000);
    if (status != CL_ERR_NO_ERR)
    {
        logger->LogError("clSerialWrite error: ", status, ": ", Error_valueToStr(status));
        return false;
    }

    // now wait for the reply...
    while (1)
    {
        // Read Data 
        size = 1;
        status = clSerialRead (SerialRefPtr, &inByte, &size, 1000);
        if (status != CL_ERR_NO_ERR)
        {
            logger->LogError("clSerialRead error: " , status, ": ", Error_valueToStr(status));
            return false;
        }

        if (size>0)
        {
            if (msgInProgress)
            {
                inBytes.push_back(inByte);

                if (inBytes.size() == 3)
                    dataLen = (int)inByte; // 3rd byte is dataLen


                if (inByte == 0x03)
                {
                    //BFE
                    logger->LogInfo ("Received BFE - End Of Msg.  Bytes: " );
                    msgInProgress = false;

                    for (auto b: inBytes)
                        std::cerr << "> " << byteToStrHB(b) << std::endl;

                    returnBytes.clear();
                    for(int i = dataStartByte; i < (dataStartByte+dataLen); i++)
                        returnBytes.push_back(inBytes[i]);

                    break;
                }
            }
            else
            {
                switch (inByte)
                {
                    case 0x06:
                        //ACK
                        logger->LogInfo ("Received ACK");
                        break;

                    case 0x15:
                        //NAK
                        logger->LogInfo ("Received NAK. Read error: check frame contents" );

                        for (auto f: readFrame)
                            std::cout << " " << byteToStrHB(f) << std::endl;
                            break;

                    case 0x01:
                        //BFS
                        logger->LogInfo ("Received BFS - Start Of Msg");
                        msgInProgress = true;
                        inBytes.push_back(inByte);
                        break;

                    default:
                        logger->LogWarn("Received unexpected byte: ", byteToStrHB(inByte));

                }
            }
        }

        usleep (100);
    }

    return true;
}

bool sendWriteFrameWaitForReturn(void* SerialRefPtr, std::vector<unsigned char> writeFrame, std::vector<unsigned char>& returnBytes, std::shared_ptr<Logger> logger)
{
    int status;
    unsigned long size;
    char inByte;

    bool msgInProgress = false;
    std::vector<unsigned char> inBytes;

    int dataStartByte = 3;
    int dataLen = 4;  //default

    size = writeFrame.size();
    status = clSerialWrite (SerialRefPtr,  (char*)writeFrame.data(), &size, 1000);
    if (status != CL_ERR_NO_ERR)
    {
        logger->LogError("clSerialWrite error: ", status, ": ", Error_valueToStr(status));
        return false;
    }

    // now wait for the reply...
    while (1)
    {
        // Read Data 
        size = 1;
        status = clSerialRead (SerialRefPtr, &inByte, &size, 1000);
        if (status != CL_ERR_NO_ERR)
        {
            logger->LogError("clSerialRead error: " , status, ": ", Error_valueToStr(status));
            return false;
        }

        if (size>0)
        {
            switch (inByte)
            {
                case 0x06:
                    //ACK
                    logger->LogInfo ("Received ACK.  Write complete");
                    break;

                case 0x15:
                    //NAK
                    logger->LogInfo ("Received NAK. Write error: check frame contents" );

                    for (auto f: writeFrame)
                        std::cout << " " << byteToStrHB(f) << std::endl;

                    break;

                default:
                    logger->LogWarn("Received unexpected byte: ", byteToStrHB(inByte));

            }
        }

        usleep (100);
    }

    return true;
}
