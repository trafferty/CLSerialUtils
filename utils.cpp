#include "utils.h"
#include "BBP_utils.h"

bool convertJSONtoEnumStruct(cJSON* reg_config, Reg_Enum_set_t& reg_set, std::shared_ptr<Logger> log)
{
    /*
        {
            "name": "Tap Geometry",
            "type": "enum_value",
            "size": 4,
            "base_addr" : "0x0720",
            "value": 21
        },
    */

    string reg_name;
    if (!getAttributeValue_String(reg_config, "name", reg_name))
    {
        log->LogError("Could not get name from reg_config ");
        return false;
    }
    else
        reg_set.name = reg_name;

    string type;
    if (!getAttributeValue_String(reg_config, "type", type))
    {
        log->LogError("Could not get type from reg_config");
        return false;
    }
    else
    {
        if (type.find("enum_value") != std::string::npos)
            reg_set.offset = 4;
        else
        {
            log->LogError("enum_value field not supported");
            return false;
        }
    }

    int size;
    if (!getAttributeValue_Int(reg_config, "size", size))
    {
        log->LogError("Could not get type from reg_config");
        return false;
    }
    else
    {
        if ((size > std::numeric_limits<unsigned char>::max()) ||
            (size < std::numeric_limits<unsigned char>::min()))
        {
            log->LogError("Size field out of range: ", size);
            return false;
        }
        reg_set.dataLen = size;
    }

    string base_addr;
    if (!getAttributeValue_String(reg_config, "base_addr", base_addr))
    {
        log->LogError("Could not get base_addr from reg_config");
        return false;
    }
    else
    {
        try
        {
            reg_set.base_addr = std::stoul(base_addr, nullptr, 16);
        }
        catch (const std::invalid_argument& e)
        {
            log->LogError("Could not convert base addr field to number: ", base_addr);
            return false;
        }
        catch (const std::out_of_range& e)
        {
            log->LogError("Could not convert base addr field to number (out of range): ", base_addr);
            return false;
        }
    }
    
    int value;
    if (!getAttributeValue_Int(reg_config, "value", value))
    {
        log->LogWarn("No field for value from reg_config");
    }
    else
        reg_set.value = value;    

    return true;
}

bool convertJSONtoStringStruct(cJSON* reg_config, Reg_String_set_t& reg_set, std::shared_ptr<Logger> log)
{
    /*
        {
            "name": "Tap Geometry",
            "type": "enum_value",
            "size": 4,
            "base_addr" : "0x0720",
            "value": 21
        },
    */

    string reg_name;
    if (!getAttributeValue_String(reg_config, "name", reg_name))
    {
        log->LogError("Could not get name from reg_config ");
        return false;
    }
    else
        reg_set.name = reg_name;

    string type;
    if (!getAttributeValue_String(reg_config, "type", type))
    {
        log->LogError("Could not get type from reg_config");
        return false;
    }
    else
    {
        if (type.find("enum_value") != std::string::npos)
            reg_set.offset = 4;
        else
        {
            log->LogError("enum_value field not supported");
            return false;
        }
    }

    int size;
    if (!getAttributeValue_Int(reg_config, "size", size))
    {
        log->LogError("Could not get type from reg_config");
        return false;
    }
    else
    {
        if ((size > std::numeric_limits<unsigned char>::max()) ||
            (size < std::numeric_limits<unsigned char>::min()))
        {
            log->LogError("Size field out of range: ", size);
            return false;
        }
        reg_set.dataLen = size;
    }

    string base_addr;
    if (!getAttributeValue_String(reg_config, "base_addr", base_addr))
    {
        log->LogError("Could not get base_addr from reg_config");
        return false;
    }
    else
    {
        try
        {
            reg_set.base_addr = std::stoul(base_addr, nullptr, 16);
        }
        catch (const std::invalid_argument& e)
        {
            log->LogError("Could not convert base addr field to number: ", base_addr);
            return false;
        }
        catch (const std::out_of_range& e)
        {
            log->LogError("Could not convert base addr field to number (out of range): ", base_addr);
            return false;
        }
    }
    
    int value;
    if (!getAttributeValue_Int(reg_config, "value", value))
    {
        log->LogWarn("No field for value from reg_config");
    }
    else
        reg_set.value = value;    

    return true;
}



void printRegValue(std::string reg_name, int value, std::shared_ptr<Logger> log)
{
    // for registers that have helper functions, then read them out
    if (reg_name.find("Tap Geometry") != std::string::npos)
        log->LogInfo("Current Tap Geometry value: ", value, ", ", TapGeometry_valueToStr(value));
    else if (reg_name.find("Trigger Mode") != std::string::npos)
        log->LogInfo("Current Trigger Mode value: ", value, ", ", TriggerMode_valueToStr(value));
    else if (reg_name.find("Exposure Mode") != std::string::npos)
        log->LogInfo("Current Exposure Mode value: ", value, ", ", ExposureMode_valueToStr(value));
    else if (reg_name.find("Device Scan Type") != std::string::npos)
        log->LogInfo("Current Device Scan Type value: ", value, ", ", DeviceScanType_valueToStr(value));

    return;
}