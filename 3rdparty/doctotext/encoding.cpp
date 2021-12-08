/**
 * @brief   Encoding/decoding functions
 * @package encoding
 * @file    encoding.cpp
 * @author  dmryutov (dmryutov@gmail.com)
 * @date    22.08.2017 -- 29.10.2017
 */
#pragma once
#include <codecvt>
#include <iostream>
#include <sstream>
#include <locale>

#if defined(_WIN32) || defined(_WIN64)
    #define USING_STATIC_LIBICONV
#endif
#include <iconv.h>

#include "encoding.hpp"


namespace encoding {

std::string decode(const std::string& str, const std::string& fromCode,
                   const std::string& toCode)
{
    std::string result;
    try {
        iconv_t cnv = iconv_open(toCode.c_str(), fromCode.c_str());

        if (cnv == (iconv_t)(-1)) {
            iconv_close(cnv);
            return str;
        }
        char* outBuf;
        if ((outBuf = (char*) malloc(str.length()*2 + 1)) == NULL) {
            iconv_close(cnv);
            return str;
        }

        #if defined(_WIN32) || defined(_WIN64)
            const char* ip = (char*) str.c_str();
        #else
            char* ip = (char*) str.c_str();
        #endif

        char* op = outBuf;
        size_t iCount = str.length();
        size_t oCount = str.length() * 2;

        if (iconv(cnv, &ip, &iCount, &op, &oCount) != (size_t) - 1) {
            outBuf[str.length()*2 - oCount] = '\0';
            result = outBuf;
        }
        else {
            result = str;
        }

        free(outBuf);
        iconv_close(cnv);
    }
    catch (...) {
        result = str;
    }
    return result;
}

std::string htmlSpecialDecode(const std::string& code, int base) {
    std::string output;

    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::string bytes = converter.to_bytes(wchar_t(stoi(code, nullptr, base)));
    for (auto c : bytes) {
        int s = 0;
        std::stringstream ss;

        ss << 0 + (unsigned char)c;
        ss >> s;

        output.push_back(static_cast<unsigned char>(s));
    }
    return output;
}

}  // End namespace
