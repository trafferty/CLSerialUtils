/*

    Utility functions for use with the Basler Binary Protocol, used to communicate with Basler
    CameraLink cameras using a framegrabber's clser***.lib 

 * Author: trafferty
 * 
 * Created on Feb 02, 2022
*/

#include "BBP_utils.h"
#include "utils.h"

// all this from the Basler Binary Protocol doc
//  https://www.baslerweb.com/en/sales-support/downloads/software-downloads/basler-binary-protocol-library/

std::vector<unsigned char> createReadDataFrame( bool doBCC, unsigned char dataLen, int base_addr, unsigned char offset)
{
    std::vector<unsigned char> frame;

    // BFS field (always 0x01)
    frame.push_back(0x01);

    unsigned char FTF = 0;
    // readFrame
    FTF += (1 << 3);
        
    if (doBCC)
        FTF += (1 << 2);
        
    if (base_addr >= 2^16)
        FTF += (1);
        
    frame.push_back(FTF);
    frame.push_back(dataLen);
    
    if (base_addr >= 2^16)
    {
        frame.push_back( ((base_addr) & 0xFF) + offset );
        frame.push_back( (base_addr >> 8) & 0xFF);
        frame.push_back( (base_addr >> 16) & 0xFF);
        frame.push_back( (base_addr >> 24) & 0xFF);
    }
    else
    {
        frame.push_back( ((base_addr) & 0xFF) + offset );
        frame.push_back( (base_addr >> 8) & 0xFF);
    }
    
    if (doBCC)
    {
        std::vector<unsigned char> bytes;
        for(int i = 1; i < frame.size(); i++)
            bytes.push_back(frame[i]);
        
        unsigned char BCC =  calc_xor_sum( bytes);
        frame.push_back(BCC);
    }

    // BFE field (always 0x03)
    frame.push_back(0x03);

    return frame;
}

std::vector<unsigned char> createWriteDataFrame( bool doBCC, unsigned char dataLen, int base_addr, unsigned char offset, int data)
{
    std::vector<unsigned char> frame;

    // BFS field (always 0x01)
    frame.push_back(0x01);

    unsigned char FTF = 0;
    // write Frame
    FTF += (0 << 3);   //no-op
        
    if (doBCC)
        FTF += (1 << 2);
        
    if (base_addr >= 2^16)
        FTF += (1);
        
    frame.push_back(FTF);
    frame.push_back(dataLen);
    
    if (base_addr >= 2^16)
    {
        frame.push_back( ((base_addr) & 0xFF) + offset );
        frame.push_back( (base_addr >> 8) & 0xFF);
        frame.push_back( (base_addr >> 16) & 0xFF);
        frame.push_back( (base_addr >> 24) & 0xFF);
    }
    else
    {
        frame.push_back( ((base_addr) & 0xFF) + offset );
        frame.push_back( (base_addr >> 8) & 0xFF);
    }
    
    // add data:
    frame.push_back( ((data) & 0xFF) );
    frame.push_back( (data >> 8) & 0xFF);
    frame.push_back( (data >> 16) & 0xFF);
    frame.push_back( (data >> 24) & 0xFF);
    
    if (doBCC)
    {
        std::vector<unsigned char> bytes;
        for(int i = 1; i < frame.size(); i++)
            bytes.push_back(frame[i]);
        
        unsigned char BCC =  calc_xor_sum( bytes);
        frame.push_back(BCC);
    }

    // BFE field (always 0x03)
    frame.push_back(0x03);

    return frame;
}

std::string TapGeometry_valueToStr(int tapGeometryValue)
{
    std::string str;
    switch (tapGeometryValue)
    {
        case 0:  str = "CLGeometry1X_1Y"; break;
        case 1:  str = "CLGeometry1X2_1Y"; break;
        case 6:  str = "CLGeometry1X_2YE"; break;
        case 7:  str = "CLGeometry1X3_1Y"; break;
        case 9:  str = "CLGeometry1X4_1Y"; break;
        case 11: str = "CLGeometry1X6_1Y"; break;
        case 12: str = "CLGeometry1X8_1Y"; break;
        case 14: str = "CLGeometry1X10_1Y"; break;
        case 16: str = "CLGeometry1X2"; break;
        case 18: str = "CLGeometry1X4"; break;
        case 20: str = "CLGeometry1X8"; break;
        case 21: str = "CLGeometry1X10"; break;
        default: str = "unknown tap geometry value";
    }
    return str;
}

std::string TriggerMode_valueToStr(int triggerModeValue)
{
    std::string str;
    switch (triggerModeValue)
    {
        case 0:  str = "OFF"; break;
        case 1:  str = "ON"; break;
        default: str = "unknown trigger mode value";
    }
    return str;
}

std::string ExposureMode_valueToStr(int exposureModeValue)
{
    std::string str;
    switch (exposureModeValue)
    {
        case 0:  str = "OFF"; break;
        case 1:  str = "TIMED"; break;
        case 2:  str = "TRIGGER WIDTH"; break;
        default: str = "unknown exposure mode value";
    }
    return str;
}

std::string DeviceScanType_valueToStr(int value)
{
    std::string str;
    switch (value)
    {
        case 0:  str = "Areascan"; break;
        case 1:  str = "Linescan"; break;
        default: str = "unknown device scan type value";
    }
    return str;
}

int TapGeometry_strToValue(std::string tapGeometryStr)
{
    int tapGeometryValue = 0;
    if ((tapGeometryStr.find("CLGeometry1X_1Y") != std::string::npos))
        tapGeometryValue = 0;
    else if ((tapGeometryStr.find("CLGeometry1X2_1Y") != std::string::npos))
        tapGeometryValue = 1;
    else if ((tapGeometryStr.find("CLGeometry1X_2YE") != std::string::npos))
        tapGeometryValue = 6;
    else if ((tapGeometryStr.find("CLGeometry1X3_1Y") != std::string::npos))
        tapGeometryValue = 7;
    else if ((tapGeometryStr.find("CLGeometry1X4_1Y") != std::string::npos))
        tapGeometryValue = 9;
    else if ((tapGeometryStr.find("CLGeometry1X6_1Y") != std::string::npos))
        tapGeometryValue = 11;
    else if ((tapGeometryStr.find("CLGeometry1X8_1Y") != std::string::npos))
        tapGeometryValue = 12;
    else if ((tapGeometryStr.find("CLGeometry1X10_1Y") != std::string::npos))
        tapGeometryValue = 14;
    else if ((tapGeometryStr.find("CLGeometry1X2") != std::string::npos))
        tapGeometryValue = 16;
    else if ((tapGeometryStr.find("CLGeometry1X4") != std::string::npos))
        tapGeometryValue = 18;
    else if ((tapGeometryStr.find("CLGeometry1X8") != std::string::npos))
        tapGeometryValue = 20;
    else if ((tapGeometryStr.find("CLGeometry1X10") != std::string::npos))
        tapGeometryValue = 21;
    return tapGeometryValue;
}
