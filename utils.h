
#include "Logger.h"
#include "CNT_JSON.h"

const int invalid_value = -999999;

enum Reg_Type_t
{
    REG_TYPE_STRING_VALUE = 0,
    REG_TYPE_ENUM_VALUE
};

typedef struct
{
    std::string   name;
    unsigned char dataLen; 
    unsigned int  base_addr;
    unsigned char offset;
    int           value;
} Reg_Enum_set_t;

typedef struct
{
    std::string   name;
    unsigned char dataLen; 
    unsigned int  base_addr;
    unsigned char offset;
    std::string   value;
} Reg_String_set_t;

bool validateFields(cJSON* reg_config, bool checkValue);

std::string byteToStrH(unsigned char inByte);
std::string byteToStrHB(unsigned char inByte);

unsigned char calc_xor_sum( std::vector<unsigned char> bytes);

int dataFieldsToInt(std::vector<unsigned char> bytes);
std::string dataFieldsToStr(std::vector<unsigned char> bytes);
std::vector<unsigned char> intToDataField(int data);


bool convertJSONtoEnumStruct(cJSON* reg_config, Reg_Enum_set_t& reg_set, std::shared_ptr<Logger> log);
bool convertJSONtoStringStruct(cJSON* reg_config, Reg_String_set_t& reg_set, std::shared_ptr<Logger> log);

void printRegValue(std::string reg_name, int value, std::shared_ptr<Logger> log);

bool readBoolValue(unsigned int base_addr, void* SerialRefPtr, bool& boolValue, std::shared_ptr<Logger> log, bool debug);
bool readEnumValue(unsigned int base_addr, void* SerialRefPtr, int& enumValue, std::shared_ptr<Logger> log, bool debug);
bool readInfoValue(unsigned int base_addr, void* SerialRefPtr, int& infoValue, std::shared_ptr<Logger> log, bool debug);
bool readStringValue(unsigned int base_addr, void* SerialRefPtr, string& enumValue, std::shared_ptr<Logger> log, bool debug);

bool writeEnumValue(unsigned int base_addr, void* SerialRefPtr, int enumValue, std::shared_ptr<Logger> log, bool debug);
bool writeBoolValue(unsigned int base_addr, void* SerialRefPtr, bool enumValue, std::shared_ptr<Logger> log, bool debug);
bool writeCommandReg(unsigned int base_addr, void* SerialRefPtr, std::shared_ptr<Logger> log, bool debug);

bool userSetSelect(int userSetNum, void* SerialRefPtr, std::shared_ptr<Logger> log, bool debug);
bool userSetLoad(int userSetNum, bool doSelect, void* SerialRefPtr, std::shared_ptr<Logger> log, bool debug);
bool userSetSave(int userSetNum, bool doSelect, void* SerialRefPtr, std::shared_ptr<Logger> log, bool debug);
bool userSetGetCurrent(int& userSetNum, void* SerialRefPtr, std::shared_ptr<Logger> log, bool debug);
bool userSetDefaultSelect(int userSetNum, void* SerialRefPtr, std::shared_ptr<Logger> log, bool debug);
bool userSetGetDefault(int& userSetNum, void* SerialRefPtr, std::shared_ptr<Logger> log, bool debug);

