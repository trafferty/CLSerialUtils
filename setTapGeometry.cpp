/*
    

*/

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include <vector>
#include <memory>
#include <set>

#include "Euresys_Serial.h"
#include "BBP_utils.h"

#include "Logger.h"

using namespace std;

std::shared_ptr<Logger> m_Log;

int main(int argc, char* argv[])
{
    int status = -1;
    unsigned long size;
    void* SerialRefPtr = nullptr;
    bool initialized = false;

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

    status = clGetManufacturerInfo(ManufacturerName, &BufferSize, &Version);
    if (status == CL_ERR_NO_ERR)
    {
        m_Log->LogInfo("ManufacturerName: ", ManufacturerName);
        m_Log->LogInfo("BufferSize:       ", BufferSize);
        m_Log->LogInfo("Version:          ", Version);
    } else {
        m_Log->LogError("Error with clGetManufacturerInfo: ", status, ": ", Error_valueToStr(status) );
    }

    unsigned int BaudRate = 9600;
    unsigned int PortId = 0;
    SerialRefPtr = connectToCamera(BaudRate, PortId, m_Log);

    if (SerialRefPtr == nullptr)
    {
        m_Log->LogError("Error connecting to camera.");
        return 0;
    }
    else
    {
    	initialized = true;
    }

    // all this from the Basler Binary Protocol doc
    // 
    bool doBCC = true;
    unsigned char dataLen = 4; 
    int addr = 0x0720;     // CL Tap Geometry
    unsigned char offset = 4;

    std::vector<unsigned char> read_frame_CLTapGeom =  createReadDataFrame( doBCC, dataLen, addr, offset);
    size = read_frame_CLTapGeom.size();

    m_Log->LogInfo("read_frame_CLTapGeom: ");
    for (auto f: read_frame_CLTapGeom)
        cout << " " << byteToStrHB(f) << endl;

    std::vector<unsigned char> returnBytes;
    if (!sendReadFrameWaitForReturn(SerialRefPtr, read_frame_CLTapGeom, returnBytes, m_Log))
    {
        m_Log->LogError("sendFrameWaitForReturn error");
        if (initialized == true)
            clSerialClose (SerialRefPtr);
        return 0;
    }
    else
    {
        m_Log->LogInfo("sendFrameWaitForReturn return bytes: ");
        for (auto b: returnBytes)
            cout << " " << byteToStrHB(b) << endl;

    }

    int tapGeometryValue = dataFieldsToInt(returnBytes);
    
    cout << "Current Tap Geometry value: " << tapGeometryValue << ", " << TapGeometry_valueToStr(tapGeometryValue) << endl;

    std::vector< std::string > CLGeometries = {
        "CLGeometry1X_1Y",
        "CLGeometry1X2_1Y",
        "CLGeometry1X_2YE",
        "CLGeometry1X3_1Y",
        "CLGeometry1X4_1Y",
        "CLGeometry1X6_1Y",
        "CLGeometry1X8_1Y",
        "CLGeometry1X10_1Y",
        "CLGeometry1X2",
        "CLGeometry1X4",
        "CLGeometry1X8",
        "CLGeometry1X10"
    };

    std::cout << "+-----------------+---------------------+" << std::endl;
    std::cout << "| - Param Value - |   - CLGeometry -    |" << std::endl;
    std::cout << "+-----------------+---------------------+" << std::endl;
    for (auto CLGeometry: CLGeometries)
        std::cout << "|       " << std::setw(2) << std::setfill('0')  << TapGeometry_strToValue(CLGeometry)  << "        |" << std::setw(20) << setfill(' ') << CLGeometry << " |" << std::endl;
    std::cout << "+---------------------+-----------------+" << std::endl;

    std::cout << std::endl << "Please enter Param Value for desired CL Tap Geometry: " << std::endl << "> ";

    int desiredVal;
    std::cin >> desiredVal;
    
    std::set<int> CLGeometry_intValues = { 0,1,6,7,9,11,12,14,16,18,20,21 };
    if (CLGeometry_intValues.find(desiredVal) == CLGeometry_intValues.end())
    {
        std::cout << "ERROR: " << desiredVal << " is not a valid value!" << std::endl;
        return 11;
    }

    std::cout << "You entered: " << desiredVal << " which corresponds to \"" << TapGeometry_valueToStr(desiredVal) << "\"" << std::endl;
    
    std::cout << "About to set CL TapGeometry to \"" << TapGeometry_valueToStr(desiredVal) << "\"" << std::endl;
    std::cout << "-- Is this correct? "  << std::endl << "(y/n) > ";
    std::string y_n;
    std::cin >> y_n; 
    
    if (y_n.find("y") != std::string::npos || y_n.find("Y") != std::string::npos)
    {
        std::vector<unsigned char> write_frame =  createWriteDataFrame( doBCC, dataLen, addr, offset, desiredVal);

        cout << "write_frame: " << endl;
        for (auto f: write_frame)
            cout << " " << byteToStrHB(f) << endl;

    }
    else
    {
        std::cout << "Ok, we quit now " << std::endl;
    }

    return 0;
}