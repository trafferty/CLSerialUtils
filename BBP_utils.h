/*

    Utility functions for use with the Basler Binary Protocol, used to communicate with Basler
    CameraLink cameras using a framegrabber's clser***.lib 
*/

#include <bitset>
#include <sstream>
#include <vector>
#include <iostream>
#include <iomanip>

#ifndef _BBP_UTILS_H_
#define _BBP_UTILS_H_

std::string byteToStrH(unsigned char inByte);
std::string byteToStrHB(unsigned char inByte);

unsigned char calc_xor_sum( std::vector<unsigned char> bytes);
std::vector<unsigned char> createReadDataFrame( bool doBCC, unsigned char dataLen, int addr, unsigned char offset);
std::vector<unsigned char> createWriteDataFrame( bool doBCC, unsigned char dataLen, int addr, unsigned char offset, int data);

int dataFieldsToInt(std::vector<unsigned char> bytes);
std::vector<unsigned char> intToDataField(int data);

std::string TapGeometry_valueToStr(int tapGeometryValue);
int TapGeometry_strToValue(std::string tapGeometryStr);

#endif //_BBP_UTILS_H_
