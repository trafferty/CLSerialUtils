/*
    
  Utility to write register values via Camera Link serial port on Euresys Grablink CL board.  Arg 1 is 
  a JSON file containing an array of parameters to write, with values; these are setup according to camera manufacturer
  settings (in CNT's case: Basler Racer).

  This utility was needed because:
    - out of the box, the Basler CL cameras are setup for a low-bandwidth Tap Geometry
      and the only way to change this setting is to use Pylon on a Windows system 
    - Every once in a while, I found that the camera would revert back to factory
      CL Tap Geometry setting, and we could no longer communicate with it at high bandwidth
    - This utility allows us to read/write the CL Tap Geometry on Linux, therefore
      we don't have to have a windows system with Pylon around.
    - As the linescan project progressed, I also discovered a couple of other factory settings
      of the Basler cameras also needed to change.

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
#include <limits>       // std::numeric_limits

#include "Euresys_Serial.h"
#include "BBP_utils.h"

#include "utils.h"
#include "Logger.h"
#include "CNT_JSON.h"

using namespace std;

std::shared_ptr<Logger> m_Log;

std::string getWelcomBanner();


int main(int argc, char* argv[])
{
    int status = -1;
    unsigned long size;
    void* SerialRefPtr = nullptr;
    bool initialized = false;

    cJSON *config;
    string filename("");
    bool debug = false;

    if (argc >= 2)
    {
        filename = argv[1];
    }

    if (argc >= 3)
    {
        string debug_str(argv[2]);
        if (debug_str.find("debug") != std::string::npos)
            debug = true;
    }

    m_Log = std::shared_ptr<Logger>(new Logger("BBP", debug));
    m_Log->LogInfo("\n", getWelcomBanner());

    if (filename.length() == 0)
    {
        m_Log->LogError("Must pass config file as argument");
        return 1;
    }

    if (!readJSONFromFile(&config, filename))
    {
        m_Log->LogError("Error! Could not read config file: ", filename);
        return 1;
    }

    if (config->type != cJSON_Array)
    {
        m_Log->LogError("Error! JSON in config must be a JSON array");
        return 1;
    }

    m_Log->LogDebug("Loading config:");
    printJSON(config);

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

    cJSON* reg_config;
    for (int idx = 0; idx < cJSON_GetArraySize(config); idx++)
    {
        reg_config = cJSON_GetArrayItem(config, idx);

        Reg_Enum_set_t reg_set;

        if (!convertJSONtoEnumStruct(reg_config, reg_set, m_Log))
        {
            m_Log->LogError("Error converting JSON to struct");
            return 1;
        }

        m_Log->LogInfo("Create BBP frame for reading register for ", reg_set.name);
        // all this from the Basler Binary Protocol doc
        //  https://www.baslerweb.com/en/sales-support/downloads/software-downloads/basler-binary-protocol-library/
        bool doBCC = true;
        std::vector<unsigned char> read_frame =  createReadDataFrame( doBCC, reg_set.dataLen, reg_set.base_addr, reg_set.offset);
        size = read_frame.size();

        for (auto f: read_frame)
            cout << " " << byteToStrHB(f) << endl;

        m_Log->LogInfo("Now send frame to read current ", reg_set.name);
        std::vector<unsigned char> returnBytes;
        if (!sendReadFrameWaitForReturn(SerialRefPtr, read_frame, returnBytes, m_Log, debug))
        {
            m_Log->LogError("sendReadFrameWaitForReturn error");
            if (initialized == true)
                clSerialClose (SerialRefPtr);
            return 0;
        }
        else
        {
            if (debug)
            {
                m_Log->LogInfo("Read complete. sendReadFrameWaitForReturn return bytes: ");
                for (auto b: returnBytes)
                    cout << " " << byteToStrHB(b) << endl;
            }
            else
                m_Log->LogInfo("Read complete.");

        }

        int initialValue= dataFieldsToInt(returnBytes);

        printRegValue(reg_set.name, initialValue, m_Log);

        // Now write the new value to register

        int desiredVal = reg_set.value;
        m_Log->LogInfo("Create BBP frame for writing ", reg_set.name, " with value ", desiredVal, ":");

        std::vector<unsigned char> write_frame =  createWriteDataFrame( doBCC, reg_set.dataLen, reg_set.base_addr, reg_set.offset, desiredVal);
        if (debug)
            for (auto f: write_frame)
                cout << " " << byteToStrHB(f) << endl;

        bool writeSuccess = false;
        if (!sendWriteFrameWaitForReturn(SerialRefPtr, write_frame, writeSuccess, m_Log, debug))
        {
            m_Log->LogError("sendWriteFrameWaitForReturn error");
            if (initialized == true)
                clSerialClose (SerialRefPtr);
            return 0;
        }

        if (writeSuccess)
        {
            m_Log->LogInfo("Write success!  Now let's re-read to confirm new setting");

            if (!sendReadFrameWaitForReturn(SerialRefPtr, read_frame, returnBytes, m_Log, debug))
            {
                m_Log->LogError("sendReadFrameWaitForReturn error");
                if (initialized == true)
                    clSerialClose (SerialRefPtr);
                return 0;
            }
            else
            {
                if (debug)
                {
                    m_Log->LogInfo("Read complete. sendReadFrameWaitForReturn return bytes: ");
                    for (auto b: returnBytes)
                        cout << " " << byteToStrHB(b) << endl;
                }
                else
                    m_Log->LogInfo("Read complete.");

            }

            int postWriteValue= dataFieldsToInt(returnBytes);

            printRegValue(reg_set.name, postWriteValue, m_Log);

            if (postWriteValue != desiredVal)
            {
                m_Log->LogWarn("Hmm...new desired value is different from current set value:");
                cout << "  New desired value: " << desiredVal << endl;
                cout << "  Current setting  : " << postWriteValue << endl;
            }
            else
            {
                m_Log->LogInfo("Confirmed ", reg_set.name, " has been updated to desired value");
            }
        }
        else
        {
            m_Log->LogError("ERROR: BBP returned NAK.  Check frame contents?");
            if (initialized == true)
                clSerialClose (SerialRefPtr);
            return 11;
        }
    }    

    if (initialized == true)
        clSerialClose (SerialRefPtr);

    return 0;
}

std::string getWelcomBanner() 
{
    std::stringstream ss;
    ss << "*******************************************************" << std::endl;
    ss << "** CameraLink Serial write Utility                   **" << std::endl;
    ss << "**  - using Basler Binary Protocol II                **" << std::endl;
    ss << "**  - using Euresys clseremc.lib                     **" << std::endl;
    ss << "*******************************************************" << std::endl;
    return ss.str();
}
