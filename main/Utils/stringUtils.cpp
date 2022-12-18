#include "Utils/stringUtils.hpp"
#include <algorithm>
#include <string>
#include <iostream>
#include "stdio.h"
#include <assert.h>
#include <vector>
#include "inttypes.h"
#include <algorithm>

#define UINT32_MAX_LENGHT 10
#define UINT64_MAX_LENGTH 20
using namespace std;

bool stringUtils::caseInsEquals (const string& first, const string& second) {
    if (first.length() != second.length()) {
        return false;
    }
    return equal(first.begin(), first.end(), second.begin(),[](char a, char b) {
            return tolower(a) == tolower(b);
    });
}
 bool stringUtils::containsSpace(const string& str) {

    for (auto ch : str) {
        if(0 != isspace(ch)) {
            return true;
        }
    }
    return false;
}

void stringUtils::removeEndingPaddings(string& in) {

    in.erase(std::remove_if(in.begin(), in.end(), [](int ch) {
        return ('\0' == ch);
    }), in.end());
}


bool stringUtils::toUint32(const string& str, uint32_t &out) {

    uint32_t value = 0;

    // More than once
    if (1 != sscanf(str.c_str(), "%u", &value)){
        return false;
    }
    
    string trimmed = str;
    trimmed.erase(std::remove_if(trimmed.begin(), trimmed.end(), [](int ch) {
        return std::isspace(ch);
    }), trimmed.end());

    if (trimmed.length() > UINT32_MAX_LENGHT){
        return false;
    }

    char testBuff[UINT32_MAX_LENGHT+1];
    sprintf(testBuff, "%u", value);

    // garbage check
    if(trimmed.compare(testBuff) != 0){
        return false;
    }

    out = value;
    return true;
}


bool stringUtils::toUint64(const string& str, uint64_t &out) {

    uint64_t value = 0;

    // More than once
    if (1 != sscanf(str.c_str(), "%llu", &value)){
        return false;
    }
    
    string trimmed = str;
    trimmed.erase(std::remove_if(trimmed.begin(), trimmed.end(), [](int ch) {
        return std::isspace(ch);
    }), trimmed.end());

    if (trimmed.length() > UINT64_MAX_LENGTH){
        return false;
    }

    char testBuff[UINT64_MAX_LENGTH+1];
    sprintf(testBuff, "%llu", value);

    // garbage check
    if(trimmed.compare(testBuff) != 0){
        return false;
    }

    out = value;
    return true;
}


string stringUtils::uint64ToHex(const uint64_t val) {

    uint32_t length = snprintf(NULL, 0, "0x%" PRIx64 "", val);
    char* buf = new char[length + 1];
    snprintf(buf, length + 1, "0x%" PRIx64 "", val);
    string str(buf);
    delete[] buf;
    return str;
}
bool stringUtils::hexToUint64(const string& str, uint64_t &out) {

    uint64_t value = 0;
    bool scanned = false;

    // 0x prefix
    if (1 == sscanf(str.c_str(), "0x%" PRIx64 "", &value)){
        scanned = true;
    }

    if (false == scanned) {
        // 0X prefix
        if (1 == sscanf(str.c_str(), "0X%" PRIx64 "", &value)){
            scanned = true;
        }
    }
    if (false == scanned) {
        // without prefix
        if (1 == sscanf(str.c_str(), "%" PRIx64 "", &value)){
            scanned = true;
        }
    }
    
    if (false == scanned){
        return false;
    }

    out = value;
    return true;
}

string stringUtils::fromUint32(const uint32_t val) {

    uint32_t length = snprintf(NULL, 0, "%u", val);
    char* buf = new char[length + 1];
    snprintf(buf, length + 1, "%u", val);
    string str(buf);
    delete[] buf;
    return str;
}

string stringUtils::fromUint64(const uint64_t val) {

    uint32_t length = snprintf(NULL, 0, "%llu", val);
    char* buf = new char[length + 1];
    snprintf(buf, length + 1, "%llu", val);
    string str(buf);
    delete[] buf;
    return str;
}

string stringUtils::format(const char *fmt, ...) {

    va_list args;
    va_start(args, fmt);
    std::vector<char> v;
    v.resize(1024, '\0');
    while (true) {
        
        va_list args2;
        va_copy(args2, args);
        int res = vsnprintf(v.data(), v.size(), fmt, args2);
        if ((res >= 0) && (res < static_cast<int>(v.size())))
        {
            va_end(args);
            va_end(args2);
            return string(v.data());
        }
        size_t size;
        if (res < 0) {
            size = v.size() * 2;
        }
        else{
            size = static_cast<size_t>(res) + 1;
        }
        v.clear();
        v.resize(size);
        va_end(args2);
    }
}

string stringUtils::format(const char *fmt, va_list args) {

    std::vector<char> vect;
    vect.resize(1024);
    while (true) {
        
        int res = vsnprintf(vect.data(), vect.size(), fmt, args);
        if ((res >= 0) && (res < static_cast<int>(vect.size())))
        {
            return string(vect.data());
        }
        size_t size;
        if (res < 0)
            size = vect.size() * 2;
        else
            size = static_cast<size_t>(res) + 1;
        vect.clear();
        vect.resize(size);
    }
}

string stringUtils::newLineStr(const char* const str) {

    return format("%s\r\n",str);
}

size_t stringUtils::countSymbols(const string& str, const char sym) {

    size_t ret = 0;
    for(char it : str){
        if (sym == it){
            ret ++;
        }
    }
    return ret;
}

size_t stringUtils::replace(std::string& src, const std::string& from, const std::string& to) {

    size_t ret {0};

    size_t index = 0;
    while (true) {
        /* Locate the substring to replace. */
        index = src.find(from, index);
        if (index == std::string::npos) {
            break;
        }

        /* Make the replacement. */
        src.replace(index, from.length(), to);
        ++ret;

        /* Advance index forward so the next iteration doesn't pick it up as well. */
        index += to.length();
    }
    return ret;
}