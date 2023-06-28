/*

    Utility functions for use with the Basler Binary Protocol, used to communicate with Basler
    CameraLink cameras using a framegrabber's clser***.lib 

 * Author: trafferty
 * 
 * Created on Feb 02, 2022
*/

#include <bitset>
#include <sstream>
#include <vector>
#include <iostream>
#include <iomanip>

#ifndef _BBP_UTILS_H_
#define _BBP_UTILS_H_


// all this from the Basler Binary Protocol doc
//  https://www.baslerweb.com/en/sales-support/downloads/software-downloads/basler-binary-protocol-library/
std::vector<unsigned char> createReadDataFrame( bool doBCC, unsigned char dataLen, int base_addr, unsigned char offset);
std::vector<unsigned char> createWriteDataFrame( bool doBCC, unsigned char dataLen, int base_addr, unsigned char offset, int data);

std::string TapGeometry_valueToStr(int value);
std::string TriggerMode_valueToStr(int value);
std::string ExposureMode_valueToStr(int value);
std::string DeviceScanType_valueToStr(int value);
int TapGeometry_strToValue(std::string tapGeometryStr);

#endif //_BBP_UTILS_H_
