
#include "Logger.h"
#include "CNT_JSON.h"

const int invalid_value = -999999;

enum Reg_Type_t
{
    REG_TYPE_STRING_VALUE = 0,
    REG_TYPE_ENUM_VALUE = 0
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

bool convertJSONtoEnumStruct(cJSON* reg_config, Reg_Enum_set_t& reg_set, std::shared_ptr<Logger> log);
bool convertJSONtoStringStruct(cJSON* reg_config, Reg_String_set_t& reg_set, std::shared_ptr<Logger> log);

void printRegValue(std::string reg_name, int value, std::shared_ptr<Logger> log);

