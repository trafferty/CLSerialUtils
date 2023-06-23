/*
    
  Utility to read/write the CL TapGeometry setting for Basler Racer linescan cameras
  using the Euresys framegraber and multicam library.

  This utility was needed because:
    - out of the box, the Basler CL cameras are setup for a low-bandwidth Tap Geometry
      and the only way to change this setting is to use Pylon on a Windows system 
    - Every once in a while, I found that the camera would revert back to factory
      CL Tap Geometry setting, and we could no longer communicate with it at high bandwidth
    - This utility allows us to read/write the CL Tap Geometry on Linux, therefore
      we don't have to have a windows system with Pylon around.

 * Author: trafferty
 * 
 * Created on Feb 02, 2022
*/

// #include <stdio.h>
// #include <string.h>
// #include <pthread.h>
// #include <unistd.h>

#include <vector>
#include <memory>
#include <set>

#include "Euresys_Serial.h"
#include "BBP_utils.h"

#include "Logger.h"

using namespace std;

std::shared_ptr<Logger> m_Log;

std::string getWelcomBanner();

int main(int argc, char* argv[])
{
    int status = -1;
    unsigned long size;
    void* SerialRefPtr = nullptr;
    bool initialized = false;

    m_Log = std::shared_ptr<Logger>(new Logger("BBP", true));

    m_Log->LogInfo("\n", getWelcomBanner());

    m_Log->LogInfo("First make sure we can find a valid serial port on the CL connection");
    unsigned int NumSerialPorts;
    status = clGetNumSerialPorts(&NumSerialPorts);
    if (status == CL_ERR_NO_ERR)
    {
        m_Log->LogInfo("NumSerialPorts: ", NumSerialPorts);
    } else {
        m_Log->LogError("Error with clGetNumSerialPorts" );
    }

    m_Log->LogInfo("Next check that the manufacturer is EURESYS");
    char ManufacturerName[255];
    unsigned int BufferSize, Version;

    BufferSize = 255;
    status = clGetManufacturerInfo(ManufacturerName, &BufferSize, &Version);
    if (status == CL_ERR_NO_ERR)
    {
        m_Log->LogInfo("  ManufacturerName: ", ManufacturerName);
        m_Log->LogInfo("  BufferSize:       ", BufferSize);
        m_Log->LogInfo("  Version:          ", Version);
    } else {
        m_Log->LogError("Error with clGetManufacturerInfo: ", status, ": ", Error_valueToStr(status) );
    }

    m_Log->LogInfo("Next connect to the serial port");
    unsigned int BaudRate = 9600;  // only speed that I could get to work for some reason...
    unsigned int PortId = 0;
    SerialRefPtr = connectToCamera(BaudRate, PortId, m_Log);

    if (SerialRefPtr == nullptr)
    {
        m_Log->LogError("Error connecting to camera.");
        return 0;
    }
    else
    {
        m_Log->LogInfo("Connection to camera established...");
        initialized = true;
    }

    m_Log->LogInfo("Create BBP frame for reading CL TapGeometry: ");
    // all this from the Basler Binary Protocol doc
    //  https://www.baslerweb.com/en/sales-support/downloads/software-downloads/basler-binary-protocol-library/
    bool doBCC = true;
    unsigned char dataLen = 4; 
    int addr = 0x0720;     // CL Tap Geometry
    unsigned char offset = 4;

    std::vector<unsigned char> read_frame_CLTapGeom =  createReadDataFrame( doBCC, dataLen, addr, offset);
    size = read_frame_CLTapGeom.size();

    for (auto f: read_frame_CLTapGeom)
        cout << " " << byteToStrHB(f) << endl;

    m_Log->LogInfo("Now send frame to read current CL TapGeometry ");
    std::vector<unsigned char> returnBytes;
    if (!sendReadFrameWaitForReturn(SerialRefPtr, read_frame_CLTapGeom, returnBytes, m_Log))
    {
        m_Log->LogError("sendReadFrameWaitForReturn error");
        if (initialized == true)
            clSerialClose (SerialRefPtr);
        return 0;
    }
    else
    {
        m_Log->LogInfo("Read complete. sendReadFrameWaitForReturn return bytes: ");
        for (auto b: returnBytes)
            cout << " " << byteToStrHB(b) << endl;

    }

    int tapGeometryValue = dataFieldsToInt(returnBytes);
    
    m_Log->LogInfo("Current Tap Geometry value: ", tapGeometryValue, ", ", TapGeometry_valueToStr(tapGeometryValue));

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

    std::cout << std::endl;
    std::cout << "+-----------------+---------------------+" << std::endl;
    std::cout << "| - Param Value - |   - CLGeometry -    |" << std::endl;
    std::cout << "+-----------------+---------------------+" << std::endl;
    for (auto CLGeometry: CLGeometries)
        std::cout << "|       " << std::setw(2) << std::setfill('0')  << TapGeometry_strToValue(CLGeometry)  << "        |" << std::setw(20) << setfill(' ') << CLGeometry << " |" << std::endl;
    std::cout << "+---------------------+-----------------+" << std::endl;

    std::cout << std::endl << "Please enter Param Value for desired CL TapGeometry: " << std::endl << "> ";

    int desiredVal;
    std::cin >> desiredVal;
    
    std::set<int> CLGeometry_intValues = { 0,1,6,7,9,11,12,14,16,18,20,21 };
    if (CLGeometry_intValues.find(desiredVal) == CLGeometry_intValues.end())
    {
        m_Log->LogError("ERROR: ", desiredVal, " is not a valid value!");
        if (initialized == true)
            clSerialClose (SerialRefPtr);
        return 11;
    }

    std::cout << "You entered: " << desiredVal << " which corresponds to \"" << TapGeometry_valueToStr(desiredVal) << "\"" << std::endl;
    
    m_Log->LogInfo("About to set CL TapGeometry to \"", TapGeometry_valueToStr(desiredVal), "\"");
    std::cout << ">> Is this correct? "  << std::endl << "(y/n) > ";
    std::string y_n;
    std::cin >> y_n; 
    
    if (y_n.find("n") != std::string::npos || y_n.find("N") != std::string::npos)
    {
        m_Log->LogInfo("Aborting...");
        if (initialized == true)
            clSerialClose (SerialRefPtr);
        return 0;
    }

    m_Log->LogInfo("Create BBP frame for writing CL TapGeometry with value ", desiredVal, ":");

    std::vector<unsigned char> write_frame_CLTapGeom =  createWriteDataFrame( doBCC, dataLen, addr, offset, desiredVal);
    for (auto f: write_frame_CLTapGeom)
        cout << " " << byteToStrHB(f) << endl;

    bool writeSuccess = false;
    if (!sendWriteFrameWaitForReturn(SerialRefPtr, write_frame_CLTapGeom, writeSuccess, m_Log))
    {
        m_Log->LogError("sendWriteFrameWaitForReturn error");
        if (initialized == true)
            clSerialClose (SerialRefPtr);
        return 0;
    }

    if (writeSuccess)
    {
        m_Log->LogInfo("Write success!  Now let's re-read to confirm new setting");

        if (!sendReadFrameWaitForReturn(SerialRefPtr, read_frame_CLTapGeom, returnBytes, m_Log))
        {
            m_Log->LogError("sendReadFrameWaitForReturn error");
            if (initialized == true)
                clSerialClose (SerialRefPtr);
            return 0;
        }
        else
        {
            m_Log->LogInfo("Read complete. sendReadFrameWaitForReturn return bytes: ");
            for (auto b: returnBytes)
                cout << " " << byteToStrHB(b) << endl;

        }

        int tapGeometryValue = dataFieldsToInt(returnBytes);
        
        m_Log->LogInfo("Current Tap Geometry value: ", tapGeometryValue, ", ", TapGeometry_valueToStr(tapGeometryValue));

        if (tapGeometryValue != desiredVal)
        {
            m_Log->LogWarn("Hmm...new desired value is different from current set value:");
            cout << "  New desired value: " << desiredVal << endl;
            cout << "  Current setting  : " << tapGeometryValue << endl;
        }
        else
        {
            m_Log->LogInfo("Confirmed CL TapGeometry has been updated to desired value");
        }
    }
    else
    {
        m_Log->LogError("ERROR: BBP returned NAK.  Check frame contents?");
        if (initialized == true)
            clSerialClose (SerialRefPtr);
        return 11;
    }
    

    if (initialized == true)
        clSerialClose (SerialRefPtr);

    return 0;
}

std::string getWelcomBanner() 
{
    std::stringstream ss;
    ss << "*******************************************************" << std::endl;
    ss << "** CL TapGeometry read/write utility                 **" << std::endl;
    ss << "**  - using Basler Binary Protocol II                **" << std::endl;
    ss << "**  - using Euresys clseremc.lib                     **" << std::endl;
    ss << "*******************************************************" << std::endl;
    return ss.str();
}
