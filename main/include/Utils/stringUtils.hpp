#pragma once

#include "string"
#include "stdint.h"
#include <cstdarg>

using namespace std;

class stringUtils {

    public:
        static string newLineStr(const char* const = "");
        static bool containsSpace(const string& str);
        static bool caseInsEquals (const string& first, const string& second);
        static bool toUint32(const string& str, uint32_t &val);
        static bool toUint64(const string& str, uint64_t &val);
        static bool hexToUint64(const string& str, uint64_t &out);
        static void removeEndingPaddings(string& in);

        static string fromUint32(const uint32_t val);
        static string fromUint64(const uint64_t val);
        static string uint64ToHex(const uint64_t val);

        static string format(const char *fmt, ...);
        static string format(const char *fmt, va_list args);

        static size_t replace(std::string& src, const std::string& from, const std::string& to);

        static size_t countSymbols(const string& str, const char sym);
};