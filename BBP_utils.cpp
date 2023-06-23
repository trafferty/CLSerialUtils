/*

    Utility functions for use with the Basler Binary Protocol, used to communicate with Basler
    CameraLink cameras using a framegrabber's clser***.lib 

 * Author: trafferty
 * 
 * Created on Feb 02, 2022
*/

#include "BBP_utils.h"

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

std::vector<unsigned char> createReadDataFrame( bool doBCC, unsigned char dataLen, int addr, unsigned char offset)
{
    std::vector<unsigned char> frame;

    // BFS field (always 0x01)
    frame.push_back(0x01);

    unsigned char FTF = 0;
    // readFrame
    FTF += (1 << 3);
        
    if (doBCC)
        FTF += (1 << 2);
        
    if (addr >= 2^16)
        FTF += (1);
        
    frame.push_back(FTF);
    frame.push_back(dataLen);
    
    if (addr >= 2^16)
    {
        frame.push_back( ((addr) & 0xFF) + offset );
        frame.push_back( (addr >> 8) & 0xFF);
        frame.push_back( (addr >> 16) & 0xFF);
        frame.push_back( (addr >> 24) & 0xFF);
    }
    else
    {
        frame.push_back( ((addr) & 0xFF) + offset );
        frame.push_back( (addr >> 8) & 0xFF);
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

std::vector<unsigned char> createWriteDataFrame( bool doBCC, unsigned char dataLen, int addr, unsigned char offset, int data)
{
    std::vector<unsigned char> frame;

    // BFS field (always 0x01)
    frame.push_back(0x01);

    unsigned char FTF = 0;
    // write Frame
    FTF += (0 << 3);   //no-op
        
    if (doBCC)
        FTF += (1 << 2);
        
    if (addr >= 2^16)
        FTF += (1);
        
    frame.push_back(FTF);
    frame.push_back(dataLen);
    
    if (addr >= 2^16)
    {
        frame.push_back( ((addr) & 0xFF) + offset );
        frame.push_back( (addr >> 8) & 0xFF);
        frame.push_back( (addr >> 16) & 0xFF);
        frame.push_back( (addr >> 24) & 0xFF);
    }
    else
    {
        frame.push_back( ((addr) & 0xFF) + offset );
        frame.push_back( (addr >> 8) & 0xFF);
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


int dataFieldsToInt(std::vector<unsigned char> bytes)
{
    int data = bytes[0];
    data += (bytes[1] << 8);
    data += (bytes[2] << 16);
    data += (bytes[3] << 24);
    return data;
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
