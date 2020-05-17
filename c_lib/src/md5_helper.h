#ifndef MD5_HELPER_H
#define MD5_HELPER_H

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string>
#include <cstring>

// MD5 from std::string
std::string md5(const std::string& dat);

// MD5 from c-string
std::string md5(const void* dat, size_t len);

// MD5 from filename
std::string md5file(const char* filename);

// MD5 from filename
std::string md5file(const std::string filename);

// MD5 from opened file
std::string md5file(FILE* file);

// Short MD5 from c-string
std::string md5sum6(std::string dat);

// Short MD5 from std::string
std::string md5sum6(const void* dat, size_t len);

#endif // end of MD5_HELPER_H
