#include "utils.h"
#include "BBP_utils.h"
#include "Euresys_Serial.h"

bool validateFields(cJSON* reg_config, bool checkValue)
{
    if (!doesAttributeExist(reg_config, "name", false))
        return false;
        
    if (!doesAttributeExist(reg_config, "type", false))
        return false;

    string base_addr;
    if (!getAttributeValue_String(reg_config, "base_addr", base_addr))
    {
        return false;
    }
    else
    {
        try
        {
            unsigned int addr = std::stoul(base_addr, nullptr, 16);
        }
        catch (const std::invalid_argument& e)
        {
            return false;
        }
        catch (const std::out_of_range& e)
        {
            return false;
        }
    }

    if (checkValue)
    {
        int value;
        if (!getAttributeValue_Int(reg_config, "value", value))
        {
            return false;
        }
        else
        {
            if ((value > std::numeric_limits<int>::max()) ||
                (value < std::numeric_limits<int>::min()))
            {
                return false;
            }
        }
    }

    return true;
}

std::string byteToStrH(unsigned char inByte)
{
  std::stringstream ss;
  // hex part:
  ss << "0x" << std::setw(2) << std::setfill('0') << std::hex << (int)(inByte) << std::dec;
  return ss.str();
}

std::string byteToStrHB(unsigned char inByte)
{
  std::stringstream ss;
  
  // hex part:
  ss << byteToStrH(inByte);

  // binstr part:
  ss << " - " << std::bitset<8>(inByte);

  return ss.str();
}

unsigned char calc_xor_sum( std::vector<unsigned char> bytes)
{
    unsigned char xor_sum = bytes[0];
    for(int i = 1; i < bytes.size(); i++)
        xor_sum = xor_sum ^ bytes[i];

    return xor_sum;
}

int dataFieldsToInt(std::vector<unsigned char> bytes)
{
    int data = bytes[0];
    data += (bytes[1] << 8);
    data += (bytes[2] << 16);
    data += (bytes[3] << 24);
    return data;
}

std::string dataFieldsToStr(std::vector<unsigned char> bytes)
{
    std::string str(bytes.begin(), bytes.end());
    return str;
}

std::vector<unsigned char> intToDataField(int data)
{
    std::vector<unsigned char> bytes;
    if (data >= 2^16)
    {
        bytes.push_back(  data & 0xFF);
        bytes.push_back( (data >> 8) & 0xFF);
        bytes.push_back( (data >> 16) & 0xFF);
        bytes.push_back( (data >> 24) & 0xFF);
    }
    else
    {
        bytes.push_back(  data & 0xFF);
        bytes.push_back( (data >> 8) & 0xFF);
    }
    return bytes;
}

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
        log->LogInfo("  ", reg_name, ": ", value, ", ", TapGeometry_valueToStr(value));
    else if (reg_name.find("Trigger Mode") != std::string::npos)
        log->LogInfo("  ", reg_name, ": ", value, ", ", TriggerMode_valueToStr(value));
    else if (reg_name.find("Exposure Mode") != std::string::npos)
        log->LogInfo("  ", reg_name, ": ", value, ", ", ExposureMode_valueToStr(value));
    else if (reg_name.find("Device Scan Type") != std::string::npos)
        log->LogInfo("  ", reg_name, ": ", value, ", ", DeviceScanType_valueToStr(value));

    return;
}

bool readBoolValue(unsigned int base_addr, void* SerialRefPtr, bool& boolValue, std::shared_ptr<Logger> log, bool debug)
{
    int           size = 4;
    unsigned char data_offset = 4;
    bool          doBCC = true;

    std::vector<unsigned char> read_frame =  createReadDataFrame( doBCC, size, base_addr, data_offset);

    if (debug)
    {
        std::cout << "createReadDataFrame bytes:" << std::endl;
        for (auto f: read_frame)
            std::cout << " " << byteToStrHB(f)<< std::endl;
    }

    log->LogDebug("Now send frame to read enum value");
    std::vector<unsigned char> returnBytes;
    if (!sendReadFrameWaitForReturn(SerialRefPtr, read_frame, returnBytes, log, debug))
    {
        log->LogError("sendReadFrameWaitForReturn error");
        return 0;
    }
    else
    {
        if (debug)
        {
            std::cout << "Read complete. sendReadFrameWaitForReturn return bytes:" << std::endl;
            for (auto b: returnBytes)
                std::cout << " " << byteToStrHB(b)<< std::endl;
        }
    }

    boolValue = (returnBytes[0] == 1 ? true : false);
    return true;
}

bool readEnumValue(unsigned int base_addr, void* SerialRefPtr, int& enumValue, std::shared_ptr<Logger> log, bool debug)
{
    int           size = 4;
    unsigned char data_offset = 4;
    bool          doBCC = true;

    std::vector<unsigned char> read_frame =  createReadDataFrame( doBCC, size, base_addr, data_offset);

    if (debug)
    {
        std::cout << "createReadDataFrame bytes:" << std::endl;
        for (auto f: read_frame)
            std::cout << " " << byteToStrHB(f)<< std::endl;
    }

    log->LogDebug("Now send frame to read enum value");
    std::vector<unsigned char> returnBytes;
    if (!sendReadFrameWaitForReturn(SerialRefPtr, read_frame, returnBytes, log, debug))
    {
        log->LogError("sendReadFrameWaitForReturn error");
        return false;
    }
    else
    {
        if (debug)
        {
            std::cout << "Read complete. sendReadFrameWaitForReturn return bytes:" << std::endl;
            for (auto b: returnBytes)
                std::cout << " " << byteToStrHB(b)<< std::endl;
        }
    }

    enumValue = dataFieldsToInt(returnBytes);
    return true;
}

bool readInfoValue(unsigned int base_addr, void* SerialRefPtr, int& infoValue, std::shared_ptr<Logger> log, bool debug)
{
    // First read the size part of the info reg
    int           size = 4;
    unsigned char data_offset = 4;
    bool          doBCC = true;

    log->LogDebug("First, read part of register that stores the number of bytes for info");

    std::vector<unsigned char> read_frame =  createReadDataFrame( doBCC, size, base_addr, data_offset);

    std::vector<unsigned char> returnBytes;
    if (!sendReadFrameWaitForReturn(SerialRefPtr, read_frame, returnBytes, log, debug))
    {
        log->LogError("sendReadFrameWaitForReturn error");
        return false;
    }
    else
    {
        if (debug)
        {
            std::cout << "Read complete. sendReadFrameWaitForReturn return bytes:" << std::endl;
            for (auto b: returnBytes)
                std::cout << " " << byteToStrHB(b)<< std::endl;
        }
    }

    int infoSize = dataFieldsToInt(returnBytes);
    log->LogDebug("infoSize = ", infoSize, " bytes");

    if (infoSize > 4)
        log->LogWarn("Info Reg size is larger than 4 bytes; returned data is not accurate");

    log->LogDebug("Now read part of register that stores the string data");

    read_frame.clear();
    data_offset = 8;
    read_frame =  createReadDataFrame( doBCC, infoSize, base_addr, data_offset);

    returnBytes.clear();
    if (!sendReadFrameWaitForReturn(SerialRefPtr, read_frame, returnBytes, log, debug))
    {
        log->LogError("sendReadFrameWaitForReturn error");
        return false;
    }
    else
    {
        if (debug)
        {
            std::cout << "Read complete. sendReadFrameWaitForReturn return bytes:" << std::endl;
            for (auto b: returnBytes)
                std::cout << " " << byteToStrHB(b)<< std::endl;
        }
    }

    infoValue = dataFieldsToInt(returnBytes);
    log->LogDebug("returnBytes converted to int: ", infoValue);

    return true;
}

bool readStringValue(unsigned int base_addr, void* SerialRefPtr, string& strValue, std::shared_ptr<Logger> log, bool debug)
{
    // First read the size part of the string reg
    int           size = 4;
    unsigned char data_offset = 4;
    bool          doBCC = true;

    log->LogDebug("First, read part of register that stores the number of chars for string");

    std::vector<unsigned char> read_frame =  createReadDataFrame( doBCC, size, base_addr, data_offset);

    std::vector<unsigned char> returnBytes;
    if (!sendReadFrameWaitForReturn(SerialRefPtr, read_frame, returnBytes, log, debug))
    {
        log->LogError("sendReadFrameWaitForReturn error");
        return false;
    }
    else
    {
        if (debug)
        {
            std::cout << "Read complete. sendReadFrameWaitForReturn return bytes:" << std::endl;
            for (auto b: returnBytes)
                std::cout << " " << byteToStrHB(b)<< std::endl;
        }
    }

    int stringSize = dataFieldsToInt(returnBytes);
    log->LogDebug("stringSize = ", stringSize, " bytes");

    log->LogDebug("Now read part of register that stores the string data");

    read_frame.clear();
    data_offset = 8;
    read_frame =  createReadDataFrame( doBCC, stringSize, base_addr, data_offset);

    returnBytes.clear();
    if (!sendReadFrameWaitForReturn(SerialRefPtr, read_frame, returnBytes, log, debug))
    {
        log->LogError("sendReadFrameWaitForReturn error");
        return false;
    }
    else
    {
        if (debug)
        {
            std::cout << "Read complete. sendReadFrameWaitForReturn return bytes:" << std::endl;
            for (auto b: returnBytes)
                std::cout << " " << byteToStrHB(b) << std::endl;
        }
    }

    strValue = dataFieldsToStr(returnBytes);
    log->LogDebug("returnBytes converted to string: ", strValue);

    return true;
}

bool writeEnumValue(unsigned int base_addr, void* SerialRefPtr, int enumValue, std::shared_ptr<Logger> log, bool debug)
{
    int           size = 4;
    unsigned char data_offset = 4;
    bool          doBCC = true;

    std::vector<unsigned char> write_frame = createWriteDataFrame( doBCC, size, base_addr, data_offset, enumValue);
    if (debug)
    {
        std::cout << "Created WriteDataFrame with contents:" << std::endl;
        for (auto f: write_frame)
            std::cout << " " << byteToStrHB(f) << std::endl;
    }

    bool writeSuccess;
    if (!sendWriteFrameWaitForReturn(SerialRefPtr, write_frame, writeSuccess, log, debug))
    {
        log->LogError("sendWriteFrameWaitForReturn error");
        return false;
    }

    return writeSuccess;
}

bool writeBoolValue(unsigned int base_addr, void* SerialRefPtr, bool enumValue, std::shared_ptr<Logger> log, bool debug)
{
    int           size = 4;
    unsigned char data_offset = 4;
    bool          doBCC = true;

    // first, read the 32 bits (4 bytes) of current value
    std::vector<unsigned char> read_frame =  createReadDataFrame( doBCC, size, base_addr, data_offset);
    std::vector<unsigned char> returnBytes;
    if (!sendReadFrameWaitForReturn(SerialRefPtr, read_frame, returnBytes, log, debug))
    {
        log->LogError("sendReadFrameWaitForReturn error");
        return 0;
    }

    // set the LSB of lowest byte with new enum value by ORing byte
    returnBytes[0] |= (enumValue ? 1 : 0);

    // create an int version for the data frame
    int newValue = dataFieldsToInt(returnBytes);

    std::vector<unsigned char> write_frame = createWriteDataFrame( doBCC, size, base_addr, data_offset, newValue);
    if (debug)
    {
        std::cout << "Created WriteDataFrame with contents:" << std::endl;
        for (auto f: write_frame)
            std::cout << " " << byteToStrHB(f) << std::endl;
    }

    bool writeSuccess;
    if (!sendWriteFrameWaitForReturn(SerialRefPtr, write_frame, writeSuccess, log, debug))
    {
        log->LogError("sendWriteFrameWaitForReturn error");
        return false;
    }

    return writeSuccess;
}

bool writeCommandReg(unsigned int base_addr, void* SerialRefPtr, std::shared_ptr<Logger> log, bool debug)
{
    int           size = 4;
    unsigned char data_offset = 4;
    bool          doBCC = true;

#if 0   // Not sure if we need this part...removing for now

    // first, read the 32 bits (4 bytes) of current value
    std::vector<unsigned char> read_frame =  createReadDataFrame( doBCC, size, base_addr, data_offset);
    std::vector<unsigned char> returnBytes;
    if (!sendReadFrameWaitForReturn(SerialRefPtr, read_frame, returnBytes, log, debug))
    {
        log->LogError("sendReadFrameWaitForReturn error");
        return 0;
    }

    // set the LSB of lowest byte with new enum value by ORing byte
    returnBytes[0] |= 1;

    // create an int version for the data frame
    int newValue = dataFieldsToInt(returnBytes);
#endif

    std::vector<unsigned char> write_frame = createWriteDataFrame( doBCC, size, base_addr, data_offset, 1);
    if (debug)
    {
        std::cout << "Created WriteDataFrame with contents:" << std::endl;
        for (auto f: write_frame)
            std::cout << " " << byteToStrHB(f) << std::endl;
    }

    bool writeSuccess;
    if (!sendWriteFrameWaitForReturn(SerialRefPtr, write_frame, writeSuccess, log, debug))
    {
        log->LogError("sendWriteFrameWaitForReturn error");
        return false;
    }

    return writeSuccess;

}

bool userSetSelect(int userSetNum, void* SerialRefPtr, std::shared_ptr<Logger> log, bool debug)
{
    if ((userSetNum < 0 ) || (userSetNum > 3))
    {
        log->LogError("Error!  userSetNum out of range");
        std::cout << " 0 - Default user set" << std::endl;
        std::cout << " 1 - User set 1" << std::endl;
        std::cout << " 2 - User set 2" << std::endl;
        std::cout << " 3 - User set 3" << std::endl;
        return false;
    }

    // "User Set Selector"
    unsigned int base_addr = 0x00060040;
    if (!writeEnumValue(base_addr, SerialRefPtr, userSetNum, log, debug))
    {
        log->LogError("writeEnumValue error");
        return false;
    }

    return true;
}

bool userSetLoad(int userSetNum, bool doSelect, void* SerialRefPtr, std::shared_ptr<Logger> log, bool debug)
{
    if (doSelect)
    {
        log->LogDebug("Selecting user set ", userSetNum);
        if (!userSetSelect(userSetNum, SerialRefPtr, log, debug))
        {
            log->LogError("userSetSelect error");
            return false;
        }
    }

    // "User Set Load"
    unsigned int base_addr = 0x00060000;
    if (!writeCommandReg(base_addr, SerialRefPtr, log, debug))
    {
        log->LogError("writeCommandReg error");
        return false;
    }

    return true;
}

bool userSetSave(int userSetNum, bool doSelect, void* SerialRefPtr, std::shared_ptr<Logger> log, bool debug)
{
    if (doSelect)
    {
        log->LogDebug("Selecting user set ", userSetNum);
        if (!userSetSelect(userSetNum, SerialRefPtr, log, debug))
        {
            log->LogError("userSetSelect error");
            return false;
        }
    }

    // "User Set Save"
    unsigned int base_addr = 0x00060020;
    if (!writeCommandReg(base_addr, SerialRefPtr, log, debug))
    {
        log->LogError("writeCommandReg error");
        return false;
    }

    return true;
}

bool userSetGetCurrent(int& userSetNum, void* SerialRefPtr, std::shared_ptr<Logger> log, bool debug)
{
    unsigned int base_addr = 0x00060040;
    if (!readEnumValue(base_addr, SerialRefPtr, userSetNum, log, debug))
    {
        log->LogError("readEnumValue error");
        return false;
    }
    return true;
}

bool userSetGetDefault(int& userSetNum, void* SerialRefPtr, std::shared_ptr<Logger> log, bool debug)
{
    unsigned int base_addr = 0x00060060;
    if (!readEnumValue(base_addr, SerialRefPtr, userSetNum, log, debug))
    {
        log->LogError("readEnumValue error");
        return false;
    }
    return true;
}

bool userSetDefaultSelect(int userSetNum, void* SerialRefPtr, std::shared_ptr<Logger> log, bool debug)
{
    if ((userSetNum < 0 ) || (userSetNum > 3))
    {
        log->LogError("Error!  userSetNum out of range");
        std::cout << " 0 - Default user set" << std::endl;
        std::cout << " 1 - User set 1" << std::endl;
        std::cout << " 2 - User set 2" << std::endl;
        std::cout << " 3 - User set 3" << std::endl;
        return false;
    }

    // "User Set Default Selector"
    unsigned int base_addr = 0x00060060;
    if (!writeEnumValue(base_addr, SerialRefPtr, userSetNum, log, debug))
    {
        log->LogError("writeEnumValue error");
        return false;
    }

    return true;
}

