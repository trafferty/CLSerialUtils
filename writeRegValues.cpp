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

    cJSON *config=nullptr;
    cJSON *reg_config_array=nullptr;
    cJSON *user_sets_array=nullptr;
    string filename("");
    bool debug = false;

    if (argc >= 2)
    {
        filename = argv[1];
    }

    if (filename.length() == 0)
    {
        std::cerr << "Error! Must pass config file as argument" << std::endl;
        return 1;
    }

    if (!readJSONFromFile(&config, filename))
    {
        std::cerr << "Error! Could not read config file: " << filename << std::endl;
        return 1;
    }

    debug = getAttributeDefault_Bool(config, "debug", debug);

    m_Log = std::shared_ptr<Logger>(new Logger("BBP", debug));
    m_Log->LogInfo("\n", getWelcomBanner());

    reg_config_array = cJSON_GetObjectItem(config, "reg_config");

    if ((reg_config_array == nullptr) || (reg_config_array->type != cJSON_Array))
    {
        m_Log->LogError("Error! reg_config must be a JSON array");
        printJSON(reg_config_array);
        return 1;
    }

    m_Log->LogDebug("Loaded config:");
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

    user_sets_array = cJSON_GetObjectItem(config, "user_sets");
    if ((user_sets_array == nullptr) || (user_sets_array->type != cJSON_Array))
    {
        m_Log->LogError("Error! user_sets_array must be a JSON array");
        printJSON(user_sets_array);
        return 0;
    }

    int default_user_set = getAttributeDefault_Int(config, "default_user_set", 1);

    // First let's see what the default User Set is (the one loaded upon power up)
    int currentDefaultUserSetNum;
    if (!userSetGetDefault(currentDefaultUserSetNum, SerialRefPtr, m_Log, debug))
    {
        m_Log->LogError("userSetGetDefault error");
        if (initialized == true)
            clSerialClose (SerialRefPtr);
        return 0;
    }
    m_Log->LogInfo("Current default user set is ", currentDefaultUserSetNum);

    if (currentDefaultUserSetNum != default_user_set)
    {
        m_Log->LogInfo("Setting default user set to num: ", default_user_set);
        if (!userSetDefaultSelect(default_user_set, SerialRefPtr, m_Log, debug))
        {
            m_Log->LogError("userSetDefaultSelect error");
            if (initialized == true)
                clSerialClose (SerialRefPtr);
            return 0;
        }

        if (!userSetGetDefault(currentDefaultUserSetNum, SerialRefPtr, m_Log, debug))
        {
            m_Log->LogError("userSetGetDefault error");
            if (initialized == true)
                clSerialClose (SerialRefPtr);
            return 0;
        }
        m_Log->LogInfo("Current default user set now set to ", currentDefaultUserSetNum);
    }

    m_Log->LogInfo("Setting Default Set (factory default) to customer set 0" );
    if (!writeEnumValue(0x00060080, SerialRefPtr, 16, m_Log, debug))
    {
        m_Log->LogError("writeEnumValue error");
        if (initialized == true)
            clSerialClose (SerialRefPtr);
        return 0;
    }

    int user_set_num;
    for (int idx = 0; idx < cJSON_GetArraySize(user_sets_array); idx++)
    {
        user_set_num = cJSON_GetArrayItem(user_sets_array, idx)->valueint;
        
        m_Log->LogInfo("**** Loading user set #", user_set_num);
        if (!userSetLoad(user_set_num, true, SerialRefPtr, m_Log, debug))
        {
            m_Log->LogError("userSetLoad error");
            if (initialized == true)
                clSerialClose (SerialRefPtr);
            return 0;
        }

        // Let's see what the current user set is
        int userSetNum;
        if (!userSetGetCurrent(userSetNum, SerialRefPtr, m_Log, debug))
        {
            m_Log->LogError("userSetGetCurrent error");
            if (initialized == true)
                clSerialClose (SerialRefPtr);
            return 0;
        }
        m_Log->LogInfo("Current user set is ", userSetNum);

        cJSON* reg_config;
        for (int idx = 0; idx < cJSON_GetArraySize(reg_config_array); idx++)
        {
            reg_config = cJSON_GetArrayItem(reg_config_array, idx);

            if(!validateFields(reg_config, true))
            {
                m_Log->LogError("JSON fields missing or incorrect: ");
                printJSON(reg_config);
                return 1;
            }

            string        reg_name;
            string        type;
            unsigned int  base_addr;
            string        base_addr_str;
            int           new_value;
            
            getAttributeValue_String(reg_config, "name", reg_name);
            getAttributeValue_String(reg_config, "type", type);
            getAttributeValue_String(reg_config, "base_addr", base_addr_str);
            getAttributeValue_Int(reg_config, "value", new_value);

            base_addr = std::stoul(base_addr_str, nullptr, 16);

            if (type.find("enum_value") != std::string::npos)
            {
                m_Log->LogInfo("******* Reading enum register: ", reg_name);

                int enumValue_before;
                if (!readEnumValue(base_addr, SerialRefPtr, enumValue_before, m_Log, debug))
                {
                    m_Log->LogError("readEnumValue error");
                    if (initialized == true)
                        clSerialClose (SerialRefPtr);
                    return 0;
                }

                m_Log->LogInfo("Reg value before write: ", enumValue_before);
                printRegValue(reg_name, enumValue_before, m_Log);

                m_Log->LogInfo("Now writing new value: ", new_value);
                printRegValue(reg_name, new_value, m_Log);

                if (!writeEnumValue(base_addr, SerialRefPtr, new_value, m_Log, debug))
                {
                    m_Log->LogError("writeEnumValue error");
                    if (initialized == true)
                        clSerialClose (SerialRefPtr);
                    return 0;
                }

                m_Log->LogInfo("------- Verifying after write...");

                int enumValue_after;
                if (!readEnumValue(base_addr, SerialRefPtr, enumValue_after, m_Log, debug))
                {
                    m_Log->LogError("readEnumValue error");
                    if (initialized == true)
                        clSerialClose (SerialRefPtr);
                    return 0;
                }

                m_Log->LogInfo("Reg value after write: ", enumValue_after);
                printRegValue(reg_name, enumValue_after, m_Log);

                if (enumValue_after != new_value)
                {
                    m_Log->LogError("After write operation, read value not equal to set value!!!");
                    if (initialized == true)
                        clSerialClose (SerialRefPtr);
                    return 0;
                }

            }
            else if (type.find("bool_value") != std::string::npos)
            {
                m_Log->LogInfo("******* Reading bool register: ", reg_name);

                bool boolValue_before;
                if (!readBoolValue(base_addr, SerialRefPtr, boolValue_before, m_Log, debug))
                {
                    m_Log->LogError("readBoolValue error");
                    if (initialized == true)
                        clSerialClose (SerialRefPtr);
                    return 0;
                }

                m_Log->LogInfo("Reg value before write: ", (boolValue_before == true ? "True" : "False"));

                m_Log->LogInfo("Now writing new value: ", new_value);

                if (!writeBoolValue(base_addr, SerialRefPtr, new_value, m_Log, debug))
                {
                    m_Log->LogError("writeEnumValue error");
                    if (initialized == true)
                        clSerialClose (SerialRefPtr);
                    return 0;
                }

                m_Log->LogInfo("------- Verifying after write...");

                bool boolValue_after;
                if (!readBoolValue(base_addr, SerialRefPtr, boolValue_after, m_Log, debug))
                {
                    m_Log->LogError("readBoolValue error");
                    if (initialized == true)
                        clSerialClose (SerialRefPtr);
                    return 0;
                }

                m_Log->LogInfo("Reg value after write: ", (boolValue_after == true ? "True" : "False"));

                if (boolValue_after != new_value)
                {
                    m_Log->LogError("After write operation, read value not equal to set value!!!");
                    if (initialized == true)
                        clSerialClose (SerialRefPtr);
                    return 0;
                }
            }
            else 
            {
                m_Log->LogWarn("Currenly only writing to enum and bool values implemented.  Skipping ", reg_name);
            }
            std::cout << "-----------------------------------------------------------------------------------" << std::endl;
        }    

        m_Log->LogInfo("**** Saving user set #", user_set_num);
        if (!userSetSave(user_set_num, false, SerialRefPtr, m_Log, debug))
        {
            m_Log->LogError("userSetSave error");
            if (initialized == true)
                clSerialClose (SerialRefPtr);
            return 0;
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
    ss << "** CameraLink Serial Write Utility                   **" << std::endl;
    ss << "**  - using Basler Binary Protocol II                **" << std::endl;
    ss << "**  - using Euresys clseremc.lib                     **" << std::endl;
    ss << "*******************************************************" << std::endl;
    return ss.str();
}
