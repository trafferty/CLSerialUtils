/*
+-------------------------------- DISCLAIMER ---------------------------------+
|                                                                             |
| This application program is provided to you free of charge as an example.   |
| Despite the considerable efforts of Euresys personnel to create a usable    |
| example, you should not assume that this program is error-free or suitable  |
| for any purpose whatsoever.                                                 |
|                                                                             |
| EURESYS does not give any representation, warranty or undertaking that this |
| program is free of any defect or error or suitable for any purpose. EURESYS |
| shall not be liable, in contract, in torts or otherwise, for any damages,   |
| loss, costs, expenses or other claims for compensation, including those     |
| asserted by third parties, arising out of or in connection with the use of  |
| this program.                                                               |
|                                                                             |
+-----------------------------------------------------------------------------+
*/

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include <vector>
#include <memory>

#include "Euresys_Serial.h"
#include "BBP_utils.h"

#include "Logger.h"

#define BOOL int
#define TRUE 1
#define FALSE 0
#define TERMINAL_BUFFER_SIZE 1024
#define TERMINAL_STRING_SIZE 128

using namespace std;

void *ReadDataThread (void * SerialRefPtr);

std::shared_ptr<Logger> m_Log;

int main(int argc, char* argv[])
{
    char ReadBuffer[TERMINAL_BUFFER_SIZE], ReadData[TERMINAL_BUFFER_SIZE];
    int status = -1, i;
    unsigned int SupportedBaudRate, BaudRate, BaudRateId;
    pthread_t ReadDataThreadId;
    unsigned long size;
    unsigned int NumPorts, PortId;
    char PortName[TERMINAL_STRING_SIZE];
    void* SerialRefPtr;
    BOOL inited = FALSE;
    char c;

    m_Log = std::shared_ptr<Logger>(new Logger("BBP", true));

    unsigned int NumSerialPorts;
    status = clGetNumSerialPorts(&NumSerialPorts);
    if (status == CL_ERR_NO_ERR)
    {
        m_Log->LogInfo("NumSerialPorts: ", NumSerialPorts);
    } else {
        m_Log->LogError("Error with clGetNumSerialPorts" );
    }

    char ManufacturerName[255];
    unsigned int BufferSize, Version;

    BufferSize = 255;
    status = clGetManufacturerInfo(ManufacturerName, &BufferSize, &Version);
    if (status == CL_ERR_NO_ERR)
    {
        m_Log->LogInfo("ManufacturerName: ", ManufacturerName);
        m_Log->LogInfo("BufferSize:       ", BufferSize);
        m_Log->LogInfo("Version:          ", Version);
    } else {
        m_Log->LogError("Error with clGetManufacturerInfo" );
    }

    // Enumerate Camera Link Serial Ports
    status = clGetNumSerialPorts (&NumPorts);
    if (status != CL_ERR_NO_ERR)
    {
        m_Log->LogError("clGetNumSerialPorts error: ", status, ": ", Error_valueToStr(status));
        if (inited == TRUE)
            clSerialClose (SerialRefPtr);
        return 0;
    }
    if (NumPorts == 0)
    {
        m_Log->LogWarn("\nSorry, no serial port detected.\n");
        m_Log->LogWarn("Check if a GrabLink is present in your system and if the drivers are correctly loaded...\n");
        if (inited == TRUE)
            clSerialClose (SerialRefPtr);
        return 0;
    }

    m_Log->LogInfo("\nDetected ports:\n");
    for (i=0; i<NumPorts; i++)
    {
        size = TERMINAL_STRING_SIZE;
        status = clGetSerialPortIdentifier (i, PortName, &size);
        cout << " - Serial Index : " << i << " - " << PortName << endl;
    }

    // Camera Link Serial Port Selection
    PortId = NumPorts;
    // while (PortId >= NumPorts)
    // {
    //  printf ("\nPlease, select a port in the list above by entering its serial index...\n> ");
    //  if (scanf("%d", &PortId) == 0 || PortId >= NumPorts)
    //  {
    //      while (getchar() != '\n'); // Flush stdin buffer
    //      printf ("This is not a valid serial index.\n");
    //  }
    // }
    PortId = 0;
    m_Log->LogInfo ("Port selected : ", PortId);

    // Initialize Camera Link Serial Connetion
    status = clSerialInit (PortId, &SerialRefPtr);
    if (status != CL_ERR_NO_ERR)
    {
        m_Log->LogError("clSerialInit error ", status, ": ", Error_valueToStr(status));
        if (inited == TRUE)
            clSerialClose (SerialRefPtr);
        return 0;
    }
    inited = TRUE;

    // Camera Link Serial Port Baudrate Selection
    status = clGetSupportedBaudRates (SerialRefPtr, &SupportedBaudRate);
    if (status != CL_ERR_NO_ERR)
    {
        m_Log->LogError("clGetSupportedBaudRates error: ", status, ": ", Error_valueToStr(status));
        if (inited == TRUE)
            clSerialClose (SerialRefPtr);
        return 0;
    }
    
    BaudRate = 9600; // the only baudrate that seems to work...

    BaudRateId=BaudRate2Id(BaudRate);
    m_Log->LogInfo ("Baudrate selected : ", BaudRate, ", BaudRateId=", BaudRateId);


    // Set Camera Link Serial Port Baudrate 
    status = clSetBaudRate (SerialRefPtr, BaudRateId);
    if (status != CL_ERR_NO_ERR)
    {
        m_Log->LogError("clSetBaudRate error: ", status, ": ", Error_valueToStr(status));
        if (inited == TRUE)
            clSerialClose (SerialRefPtr);
        return 0;
    }

    bool doBCC = true;
    unsigned char dataLen = 4; 
    //int base_addr = 0x00030160; //test image selector
    int base_addr = 0x0720;     // CL Tap Geometry
    unsigned char offset = 4;

    std::vector<unsigned char> read_frame =  createReadDataFrame( doBCC, dataLen, base_addr, offset);
    size = read_frame.size();

    m_Log->LogInfo("read_frame: ");
    for (auto f: read_frame)
        cout << " " << byteToStrHB(f) << endl;



#if 0
    // Terminal loop -> process lines and send data to the camera.
    m_Log->LogInfo ("Starting read thread...");
    pthread_create (&ReadDataThreadId, NULL, ReadDataThread, SerialRefPtr);
    usleep (100000);

    m_Log->LogInfo ("sending cmd 1...");
    status = clSerialWrite (SerialRefPtr,  (char*)read_frame.data(), &size, 1000);
    if (status != CL_ERR_NO_ERR)
    {
        m_Log->LogError("clSerialWrite error: ", status);
        if (inited == TRUE)
            clSerialClose (SerialRefPtr);
        return 0;
    }
#else

    std::vector<unsigned char> returnBytes;
    if (!sendReadFrameWaitForReturn(SerialRefPtr, read_frame, returnBytes, m_Log, true))
    {
        m_Log->LogError("sendFrameWaitForReturn error");
        if (inited == TRUE)
            clSerialClose (SerialRefPtr);
        return 0;
    }
    else
    {
        m_Log->LogInfo("read_frame return bytes: ");
        for (auto b: returnBytes)
            cout << " " << byteToStrHB(b) << endl;

    }


    int tapGeometryValue = dataFieldsToInt(returnBytes);

    m_Log->LogInfo ("CL TapGeopmetry: ", TapGeometry_valueToStr(tapGeometryValue));


#endif

    return 0;
}

// Read Data Thread
//      This thread will read data (if available) from the 
//      Camera Link Serial port and display it in the console.
void *ReadDataThread (void * SerialRefPtr)
{
    int status;
    unsigned long size;
    char inByte;

    bool msgInProgress = false;
    std::vector<unsigned char> bytes;

    while (1)
    {
        // Read Data 
        size = 1;
        //status = clSerialRead (SerialRefPtr, ReadBuffer, &size, 1000);
        status = clSerialRead (SerialRefPtr, &inByte, &size, 1000);
        if (status != CL_ERR_NO_ERR)
        {
            m_Log->LogError("clSerialRead error: " , status, ": ", Error_valueToStr(status));
            return NULL;
        }

        if (size>0)
        {
            // unsigned char inByte = ReadBuffer[0];

            if (msgInProgress)
            {
                bytes.push_back(inByte);

                if (inByte == 0x03)
                {
                    //BFE
                    m_Log->LogInfo ("Received BFE - End Of Msg.  Bytes: " );
                    msgInProgress = false;

                    for (auto b: bytes)
                        cerr << "> " << byteToStrHB(b) << endl;
                        
                    bytes.clear();
                }
            }
            else
            {
                switch (inByte)
                {
                    case 0x06:
                        //ACK
                        m_Log->LogInfo ("Received ACK");
                        break;

                    case 0x15:
                        //NAK
                        m_Log->LogInfo ("Received NAK" );
                        break;

                    case 0x01:
                        //BFS
                        m_Log->LogInfo ("Received BFS - Start Of Msg");
                        msgInProgress = true;
                        bytes.push_back(inByte);
                        break;

                    default:
                        m_Log->LogWarn("Received unexpected byte: ", byteToStrHB(inByte));

                }
            }

            //fflush(stdout);
        }

        //printf(".");
        usleep (100);
    }
    return NULL;
}


