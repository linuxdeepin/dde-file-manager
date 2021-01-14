/****************************************************************************
**
** DocToText - Converts DOC, XLS, XLSB, PPT, RTF, ODF (ODT, ODS, ODP),
**             OOXML (DOCX, XLSX, PPTX), iWork (PAGES, NUMBERS, KEYNOTE),
**             ODFXML (FODP, FODS, FODT), PDF, EML and HTML documents to plain text.
**             Extracts metadata and annotations.
**
** Copyright (c) 2006-2013, SILVERCODERS(R)
** http://silvercoders.com
**
** Project homepage: http://silvercoders.com/en/products/doctotext
**
** This program may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file COPYING.GPL included in the
** packaging of this file.
**
** Please remember that any attempt to workaround the GNU General Public
** License using wrappers, pipes, client/server protocols, and so on
** is considered as license violation. If your program, published on license
** other than GNU General Public License version 2, calls some part of this
** code directly or indirectly, you have to buy commercial license.
** If you do not like our point of view, simply do not use the product.
**
** Licensees holding valid commercial license for this product
** may use this file in accordance with the license published by
** SILVERCODERS and appearing in the file COPYING.COM
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
**
*****************************************************************************/

#include "pdf_parser.h"

#include "data_stream.h"
#include "exception.h"
#include <iostream>
#include <list>
#include <map>
#include "metadata.h"
#include "misc.h"
#include <new>
#include <set>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <zlib.h>

//some functions specific for this parser

//converts hex char (2 bytes) to single char. Examples:
//0x41 will be converted to single 'A'
//0x30 will be converted to '0'
static unsigned char hex_char_to_single_char(const char* hex_char)
{
	//first four bits
	unsigned char res = 0;
	if (hex_char[0] >= 'A')
		res = (hex_char[0] - 'A' + 10) << 4;
	else
		res = (hex_char[0] - '0') << 4;
	if (hex_char[1] >= 'A')
		return res | (hex_char[1] - 'A' + 10);
	return res | (hex_char[1] - '0');
}

//just like above, but in reverse order
static std::string char_to_hex_char(const char ch)
{
	char res[2];
	res[0] = ((ch & 0xF0) >> 4);
	if (res[0] < 10)
		res[0] += '0';
	else
		res[0] += ('A' - 10);
	res[1] = ch & 0x0F;
	if (res[1] < 10)
		res[1] += '0';
	else
		res[1] += ('A' - 10);
	return std::string(res, 2);
}

static bool hex_char_is_valid(const char hex_char)
{
	return (hex_char >= '0' && hex_char <= '9') || (hex_char >= 'A' && hex_char <= 'F');
}

static void normalize_hex_char(char& ch)
{
	if (ch >= 'a' && ch <= 'f')
		ch -= ('a' - 'A');
}

static std::string unicode_codepoint_to_utf8(unsigned int codepoint)
{
	char out[4];
	if (codepoint < 0x80)
	{
		out[0] = (char)codepoint;
		return std::string(out, 1);
	}
	else if (codepoint < 0x800)
	{
		out[0] = 0xC0 | (codepoint >> 6);
		out[1] = 0x80 | (codepoint & 0x0000003F);
		return std::string(out, 2);
	}
	else if (codepoint < 0x10000)
	{
		out[0] = 0xE0 | (codepoint >> 12);
		out[1] = 0x80 | ((codepoint & 0x00000FFF) >> 6);
		out[2] = 0x80 | (codepoint & 0x0000003F);
		return std::string(out, 3);
	}
	else
	{
		out[0] = 0xF0 | (codepoint >> 18);
		out[1] = 0x80 | ((codepoint & 0x0003FFFF) >> 12);
		out[2] = 0x80 | ((codepoint & 0x00000FFF) >> 6);
		out[3] = 0x80 | (codepoint & 0x0000003F);
		return std::string(out, 4);
	}
	return "";
}

static std::string utf8_codepoint_to_utf8(unsigned int utf8_codepoint)
{
	char out[4];
	if (utf8_codepoint < 0x100)
	{
		out[0] = utf8_codepoint;
		return std::string(out, 1);
	}
	else if (utf8_codepoint < 0x10000)
	{
		out[0] = utf8_codepoint >> 8;
		out[1] = (utf8_codepoint & 0x000000FF);
		return std::string(out, 2);
	}
	else if (utf8_codepoint < 0x1000000)
	{
		out[0] = utf8_codepoint >> 16;
		out[1] = (utf8_codepoint & 0x0000FF00) >> 8;
		out[2] = (utf8_codepoint & 0x000000FF);
		return std::string(out, 3);
	}
	else
	{
		out[0] = utf8_codepoint >> 24;
		out[1] = (utf8_codepoint & 0x00FF0000) >> 16;
		out[2] = (utf8_codepoint & 0x0000FF00) >> 8;
		out[3] = (utf8_codepoint & 0x000000FF);
		return std::string(out, 4);
	}
	return "";
}

static void uint_to_hex_string(unsigned int value, std::string& output)
{
	char buffer[20];
	sprintf(buffer, "%x", value);
	for (int i = 0; i < 20 && buffer[i] != 0; ++i)
	{
		if (buffer[i] >= 'a')
			buffer[i] = buffer[i] - ('a' - 'A');
	}
	if (strlen(buffer) % 2 == 1)
		output += '0';
	output += buffer;
}

static unsigned int hex_string_to_uint(const char* hex_number, size_t size)
{
	unsigned int val = 0;
	for (int i = 0; i < size; ++i)
	{
		val = val << 4;
		if (hex_number[i] <= '9')
			val = val | (hex_number[i] - '0');
		else
			val = val | (hex_number[i] - 'A' + 10);
	}
	return val;
}

static std::string utf16be_to_utf8(std::string& utf16be)
{
	unsigned int utf16 = 0;
	if (utf16be.length() % 4 != 0)
	{
		for (int i = 0; i < 4 - utf16be.length() % 4; ++i)
			utf16be += '0';
	}
	std::string ret;
	for (unsigned int index = 0; index < utf16be.length(); index += 4)
	{
		if (utf16be[index] == 'D' && utf16be[index + 1] == '8' && index + 8 <= utf16be.length())
		{
			utf16 = hex_string_to_uint(&utf16be[index], 8);
			index += 4;
		}
		else
			utf16 = hex_string_to_uint(&utf16be[index], 4);
		ret += unichar_to_utf8(utf16);
	}
	return ret;
}

//increments "hex string". Example:
//0FCB will be converted to 0FCC
static void increment_hex_string(std::string& hex_string)
{
	if (hex_string.length() == 0)
		hex_string = "01";
	else
	{
		int index = hex_string.length() - 1;
		while (true)
		{
			if (hex_string[index] < '9' || hex_string[index] < 'F')
			{
				++hex_string[index];
				return;
			}
			if (hex_string[index] == '9')
			{
				hex_string[index] = 'A';
				return;
			}
			if (hex_string[index] == 'F')
			{
				hex_string[index] = '0';
				--index;
			}
			if (index < 0)
			{
				hex_string = "0001" + hex_string;
				return;
			}
		}
	}
}

void parsePDFDate(tm& date, const std::string& str_date)
{
	if (str_date.length() < 14)
		return;
	std::string year = str_date.substr(0, 4);
	date.tm_year = strtol(year.c_str(), NULL, 10);
	std::string month = str_date.substr(4, 2);
	date.tm_mon = strtol(month.c_str(), NULL, 10);
	std::string day = str_date.substr(6, 2);
	date.tm_mday = strtol(day.c_str(), NULL, 10);
	std::string hour = str_date.substr(8, 2);
	date.tm_hour = strtol(hour.c_str(), NULL, 10);
	std::string minute = str_date.substr(10, 2);
	date.tm_min = strtol(minute.c_str(), NULL, 10);
	std::string second = str_date.substr(12, 2);
	date.tm_sec = strtol(second.c_str(), NULL, 10);
	date.tm_year -= 1900;
	--date.tm_mon;
}

bool last_is_new_line(const std::string& str)
{
	return str.length() > 0 && str[str.length() - 1] == '\n';
}

//predefined encodings:
const static unsigned int PdfDocEncodingUtf8[256] =
{
	0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
	0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x17, 0x17,
	0xcb98, 0xcb87, 0xcb86, 0xcb99, 0xcb9d, 0xcb9b, 0xcb9a, 0xcb9c,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
	0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
	0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
	0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
	0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
	0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
	0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x0,
	0xe280a2, 0xe280a0, 0xe280a1, 0xe280a6, 0xe28094, 0xe28093, 0xc692, 0xe28184,
	0xe280b9, 0xe280ba, 0xe28892, 0xe280b0, 0xe2809e, 0xe2809c, 0xe2809d, 0xe28098,
	0xe28099, 0xe2809a, 0xe284a2, 0xefac81, 0xefac82, 0xc581, 0xc592, 0xc5a0,
	0xc5b8, 0xc5bd, 0xc4b1, 0xc582, 0xc593, 0xc5a1, 0xc5be, 0x0,
	0xe282ac, 0xc2a1, 0xc2a2, 0xc2a3, 0xc2a4, 0xc2a5, 0xc2a6, 0xc2a7,
	0xc2a8, 0xc2a9, 0xc2aa, 0xc2ab, 0xc2ac, 0x0, 0xc2ae, 0xc2af,
	0xc2b0, 0xc2b1, 0xc2b2, 0xc2b3, 0xc2b4, 0xc2b5, 0xc2b6, 0xc2b7,
	0xc2b8, 0xc2b9, 0xc2ba, 0xc2bb, 0xc2bc, 0xc2bd, 0xc2be, 0xc2bf,
	0xc380, 0xc381, 0xc382, 0xc383, 0xc384, 0xc385, 0xc386, 0xc387,
	0xc388, 0xc389, 0xc38a, 0xc38b, 0xc38c, 0xc38d, 0xc38e, 0xc38f,
	0xc390, 0xc391, 0xc392, 0xc393, 0xc394, 0xc395, 0xc396, 0xc397,
	0xc398, 0xc399, 0xc39a, 0xc39b, 0xc39c, 0xc39d, 0xc39e, 0xc39f,
	0xc3a0, 0xc3a1, 0xc3a2, 0xc3a3, 0xc3a4, 0xc3a5, 0xc3a6, 0xc3a7,
	0xc3a8, 0xc3a9, 0xc3aa, 0xc3ab, 0xc3ac, 0xc3ad, 0xc3ae, 0xc3af,
	0xc3b0, 0xc3b1, 0xc3b2, 0xc3b3, 0xc3b4, 0xc3b5, 0xc3b6, 0xc3b7,
	0xc3b8, 0xc3b9, 0xc3ba, 0xc3bb, 0xc3bc, 0xc3bd, 0xc3be, 0xc3bf
};

const static unsigned int WinAnsiEncodingUtf8[256] =
{
	0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
	0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
	0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
	0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
	0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
	0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
	0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
	0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
	0xe282ac, 0x0, 0xe2809a, 0xc692, 0xe2809e, 0xe280a6, 0xe280a0, 0xe280a1,
	0xcb86, 0xe280b0, 0xc5a0, 0xe280b9, 0xc592, 0x0, 0xc5bd, 0x0,
	0x0, 0xe28098, 0xe28099, 0xe2809c, 0xe2809d, 0xe280a2, 0xe28093, 0xe28094,
	0xcb9c, 0xe284a2, 0xc5a1, 0xe280ba, 0xc593, 0x0, 0xc5be, 0xc5b8,
	0xc2a0, 0xc2a1, 0xc2a2, 0xc2a3, 0xc2a4, 0xc2a5, 0xc2a6, 0xc2a7,
	0xc2a8, 0xc2a9, 0xc2aa, 0xc2ab, 0xc2ac, 0xc2ad, 0xc2ae, 0xc2af,
	0xc2b0, 0xc2b1, 0xc2b2, 0xc2b3, 0xc2b4, 0xc2b5, 0xc2b6, 0xc2b7,
	0xc2b8, 0xc2b9, 0xc2ba, 0xc2bb, 0xc2bc, 0xc2bd, 0xc2be, 0xc2bf,
	0xc380, 0xc381, 0xc382, 0xc383, 0xc384, 0xc385, 0xc386, 0xc387,
	0xc388, 0xc389, 0xc38a, 0xc38b, 0xc38c, 0xc38d, 0xc38e, 0xc38f,
	0xc390, 0xc391, 0xc392, 0xc393, 0xc394, 0xc395, 0xc396, 0xc397,
	0xc398, 0xc399, 0xc39a, 0xc39b, 0xc39c, 0xc39d, 0xc39e, 0xc39f,
	0xc3a0, 0xc3a1, 0xc3a2, 0xc3a3, 0xc3a4, 0xc3a5, 0xc3a6, 0xc3a7,
	0xc3a8, 0xc3a9, 0xc3aa, 0xc3ab, 0xc3ac, 0xc3ad, 0xc3ae, 0xc3af,
	0xc3b0, 0xc3b1, 0xc3b2, 0xc3b3, 0xc3b4, 0xc3b5, 0xc3b6, 0xc3b7,
	0xc3b8, 0xc3b9, 0xc3ba, 0xc3bb, 0xc3bc, 0xc3bd, 0xc3be, 0xc3bf
};

const static unsigned int MacRomanEncodingUtf8[256] =
{
	0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
	0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
	0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
	0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
	0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
	0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
	0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
	0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
	0xc384, 0xc385, 0xc387, 0xc389, 0xc391, 0xc396, 0xc39c, 0xc3a1,
	0xc3a0, 0xc3a2, 0xc3a4, 0xc3a3, 0xc3a5, 0xc3a7, 0xc3a9, 0xc3a8,
	0xc3aa, 0xc3ab, 0xc3ad, 0xc3ac, 0xc3ae, 0xc3af, 0xc3b1, 0xc3b3,
	0xc3b2, 0xc3b4, 0xc3b6, 0xc3b5, 0xc3ba, 0xc3b9, 0xc3bb, 0xc3bc,
	0xe280a0, 0xc2b0, 0xc2a2, 0xc2a3, 0xc2a7, 0xe280a2, 0xc2b6, 0xc39f,
	0xc2ae, 0xc2a9, 0xe284a2, 0xc2b4, 0xc2a8, 0xe289a0, 0xc386, 0xc398,
	0xe2889e, 0xc2b1, 0xe289a4, 0xe289a5, 0xc2a5, 0xc2b5, 0xe28882, 0xe28891,
	0xe2888f, 0xcf80, 0xe288ab, 0xc2aa, 0xc2ba, 0xcea9, 0xc3a6, 0xc3b8,
	0xc2bf, 0xc2a1, 0xc2ac, 0xe2889a, 0xc692, 0xe28988, 0xe28886, 0xc2ab,
	0xc2bb, 0xe280a6, 0xc2a0, 0xc380, 0xc383, 0xc395, 0xc592, 0xc593,
	0xe28093, 0xe28094, 0xe2809c, 0xe2809d, 0xe28098, 0xe28099, 0xc3b7, 0xe2978a,
	0xc3bf, 0xc5b8, 0xe28184, 0xe282ac, 0xe280b9, 0xe280ba, 0xefac81, 0xefac82,
	0xe280a1, 0xc2b7, 0xe2809a, 0xe2809e, 0xe280b0, 0xc382, 0xc38a, 0xc381,
	0xc38b, 0xc388, 0xc38d, 0xc38e, 0xc38f, 0xc38c, 0xc393, 0xc394,
	0xefa3bf, 0xc392, 0xc39a, 0xc39b, 0xc399, 0xc4b1, 0xcb86, 0xcb9c,
	0xc2af, 0xcb98, 0xcb99, 0xcb9a, 0xc2b8, 0xcb9d, 0xcb9b, 0xcb87
};

const static unsigned int MacExpertEncodingUtf8[256] =
{
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x20, 0xef9ca1, 0xef9bb8, 0xef9ea2, 0xef9ca4, 0xef9ba4, 0xef9ca6, 0xef9eb4,
	0xe281bd, 0xe281be, 0xe280a5, 0xe280a4, 0x2c, 0x2d, 0x2e, 0xe28184,
	0xef9cb0, 0xef9cb1, 0xef9cb2, 0xef9cb3, 0xef9cb4, 0xef9cb5, 0xef9cb6, 0xef9cb7,
	0xef9cb8, 0xef9cb9, 0x3a, 0x3b, 0x0, 0xef9b9e, 0x0, 0xef9cbf,
	0x0, 0x0, 0x0, 0x0, 0xef9fb0, 0x0, 0x0, 0xc2bc,
	0xc2bd, 0xc2be, 0xe2859b, 0xe2859c, 0xe2859d, 0xe2859e, 0xe28593, 0xe28594,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xefac80, 0xefac81,
	0xefac82, 0xefac83, 0xefac84, 0xe2828d, 0x0, 0xe2828e, 0xef9bb6, 0xef9ba5,
	0xef9da0, 0xef9da1, 0xef9da2, 0xef9da3, 0xef9da4, 0xef9da5, 0xef9da6, 0xef9da7,
	0xef9da8, 0xef9da9, 0xef9daa, 0xef9dab, 0xef9dac, 0xef9dad, 0xef9dae, 0xef9daf,
	0xef9db0, 0xef9db1, 0xef9db2, 0xef9db3, 0xef9db4, 0xef9db5, 0xef9db6, 0xef9db7,
	0xef9db8, 0xef9db9, 0xef9dba, 0xe282a1, 0xef9b9c, 0xef9b9d, 0xef9bbe, 0x0,
	0x0, 0xef9ba9, 0xef9ba0, 0x0, 0x0, 0x0, 0x0, 0xef9fa1,
	0xef9fa0, 0xef9fa2, 0xef9fa4, 0xef9fa3, 0xef9fa5, 0xef9fa7, 0xef9fa9, 0xef9fa8,
	0xef9faa, 0xef9fab, 0xef9fad, 0xef9fac, 0xef9fae, 0xef9faf, 0xef9fb1, 0xef9fb3,
	0xef9fb2, 0xef9fb4, 0xef9fb6, 0xef9fb5, 0xef9fba, 0xef9fb9, 0xef9fbb, 0xef9fbc,
	0x0, 0xe281b8, 0xe28284, 0xe28283, 0xe28286, 0xe28288, 0xe28287, 0xef9bbd,
	0x0, 0xef9b9f, 0xe28282, 0x0, 0xef9ea8, 0x0, 0xef9bb5, 0xef9bb0,
	0xe28285, 0x0, 0xef9ba1, 0xef9ba7, 0xef9fbd, 0x0, 0xef9ba3, 0x0,
	0x0, 0xef9fbe, 0x0, 0xe28289, 0xe28280, 0xef9bbf, 0xef9fa6, 0xef9fb8,
	0xef9ebf, 0xe28281, 0xef9bb9, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0xef9eb8, 0x0, 0x0, 0x0, 0x0, 0x0, 0xef9bba,
	0xe28092, 0xef9ba6, 0x0, 0x0, 0x0, 0x0, 0xef9ea1, 0x0,
	0xef9fbf, 0x0, 0xc2b9, 0xc2b2, 0xc2b3, 0xe281b4, 0xe281b5, 0xe281b6,
	0xe281b7, 0xe281b9, 0xe281b0, 0x0, 0xef9bac, 0xef9bb1, 0xef9bb3, 0x0,
	0x0, 0xef9bad, 0xef9bb2, 0xef9bab, 0x0, 0x0, 0x0, 0x0,
	0x0, 0xef9bae, 0xef9bbb, 0xef9bb4, 0xef9eaf, 0xef9baa, 0xe281bf, 0xef9baf,
	0xef9ba2, 0xef9ba8, 0xef9bb7, 0xef9bbc, 0x0, 0x0, 0x0, 0x0
};

const static unsigned int StandardEncodingUtf8[256] =
{
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0xe28099,
	0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
	0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
	0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
	0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
	0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
	0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0xc2a1, 0xc2a2, 0xc2a3, 0xe28184, 0xc2a5, 0xc692, 0xc2a7,
	0xc2a4, 0x27, 0xe2809c, 0xc2ab, 0xe280b9, 0xe280ba, 0xefac81, 0xefac82,
	0x0, 0xe28093, 0xe280a0, 0xe280a1, 0xc2b7, 0x0, 0xc2b6, 0xe280a2,
	0xe2809a, 0xe2809e, 0xe2809d, 0xc2bb, 0xe280a6, 0xe280b0, 0x0, 0xc2bf,
	0x0, 0x60, 0xc2b4, 0xcb86, 0xcb9c, 0xc2af, 0xcb98, 0xcb99,
	0xc2a8, 0x0, 0xcb9a, 0xc2b8, 0xcb9d, 0xcb9b, 0xcb87, 0xe28094,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0xc386, 0x0, 0xc2aa, 0x0, 0x0, 0x0, 0x0, 0xc581,
	0xc398, 0xc592, 0xc2ba, 0x0, 0x0, 0x0, 0x0, 0x0,
	0xc3a6, 0x0, 0x0, 0x0, 0xc4b1, 0x0, 0x0, 0xc582,
	0xc3b8, 0xc593, 0xc39f, 0x0, 0x0, 0x0, 0x0, 0x0
};

const static unsigned int SymbolEncodingUtf8[256] =
{
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x20, 0x21, 0xe28880, 0x23, 0xe28883, 0x25, 0x26, 0xe2888b,
	0x28, 0x29, 0xe28897, 0x2b, 0x2c, 0xe28892, 0x2e, 0x2f,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
	0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
	0xe28985, 0xce91, 0xce92, 0xcea7, 0xce94, 0xce95, 0xcea6, 0xce93,
	0xce97, 0xce99, 0xcf91, 0xce9a, 0xce9b, 0xce9c, 0xce9d, 0xce9f,
	0xcea0, 0xce98, 0xcea1, 0xcea3, 0xcea4, 0xcea5, 0xcf82, 0xcea9,
	0xce9e, 0xcea8, 0xce96, 0x5b, 0xe288b4, 0x5d, 0xe28aa5, 0x5f,
	0xefa3a5, 0xceb1, 0xceb2, 0xcf87, 0xceb4, 0xceb5, 0xcf86, 0xceb3,
	0xceb7, 0xceb9, 0xcf95, 0xceba, 0xcebb, 0xcebc, 0xcebd, 0xcebf,
	0xcf80, 0xceb8, 0xcf81, 0xcf83, 0xcf84, 0xcf85, 0xcf96, 0xcf89,
	0xcebe, 0xcf88, 0xceb6, 0x7b, 0x7c, 0x7d, 0xe288bc, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0xe282ac, 0xcf92, 0xe280b2, 0xe289a4, 0xe28184, 0xe2889e, 0xc692, 0xe299a3,
	0xe299a6, 0xe299a5, 0xe299a0, 0xe28694, 0xe28690, 0xe28691, 0xe28692, 0xe28693,
	0xc2b0, 0xc2b1, 0xe280b3, 0xe289a5, 0xc397, 0xe2889d, 0xe28882, 0xe280a2,
	0xc3b7, 0xe289a0, 0xe289a1, 0xe28988, 0xe280a6, 0xefa3a6, 0xefa3a7, 0xe286b5,
	0xe284b5, 0xe28491, 0xe2849c, 0xe28498, 0xe28a97, 0xe28a95, 0xe28885, 0xe288a9,
	0xe288aa, 0xe28a83, 0xe28a87, 0xe28a84, 0xe28a82, 0xe28a86, 0xe28888, 0xe28889,
	0xe288a0, 0xe28887, 0xef9b9a, 0xef9b99, 0xef9b9b, 0xe2888f, 0xe2889a, 0xe28b85,
	0xc2ac, 0xe288a7, 0xe288a8, 0xe28794, 0xe28790, 0xe28791, 0xe28792, 0xe28793,
	0xe2978a, 0xe28ca9, 0xefa3a8, 0xefa3a9, 0xefa3aa, 0xe28891, 0xefa3ab, 0xefa3ac,
	0xefa3ad, 0xefa3ae, 0xefa3af, 0xefa3b0, 0xefa3b1, 0xefa3b2, 0xefa3b3, 0xefa3b4,
	0x0, 0xe28caa, 0xe288ab, 0xe28ca0, 0xefa3b5, 0xe28ca1, 0xefa3b6, 0xefa3b7,
	0xefa3b8, 0xefa3b9, 0xefa3ba, 0xefa3bb, 0xefa3bc, 0xefa3bd, 0xefa3be, 0x0
};

const static unsigned int ZapfDingbatsEncodingUtf8[256] =
{
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x20, 0xe29c81, 0xe29c82, 0xe29c83, 0xe29c84, 0xe2988e, 0xe29c86, 0xe29c87,
	0xe29c88, 0xe29c89, 0xe2989b, 0xe2989e, 0xe29c8c, 0xe29c8d, 0xe29c8e, 0xe29c8f,
	0xe29c90, 0xe29c91, 0xe29c92, 0xe29c93, 0xe29c94, 0xe29c95, 0xe29c96, 0xe29c97,
	0xe29c98, 0xe29c99, 0xe29c9a, 0xe29c9b, 0xe29c9c, 0xe29c9d, 0xe29c9e, 0xe29c9f,
	0xe29ca0, 0xe29ca1, 0xe29ca2, 0xe29ca3, 0xe29ca4, 0xe29ca5, 0xe29ca6, 0xe29ca7,
	0xe29885, 0xe29ca9, 0xe29caa, 0xe29cab, 0xe29cac, 0xe29cad, 0xe29cae, 0xe29caf,
	0xe29cb0, 0xe29cb1, 0xe29cb2, 0xe29cb3, 0xe29cb4, 0xe29cb5, 0xe29cb6, 0xe29cb7,
	0xe29cb8, 0xe29cb9, 0xe29cba, 0xe29cbb, 0xe29cbc, 0xe29cbd, 0xe29cbe, 0xe29cbf,
	0xe29d80, 0xe29d81, 0xe29d82, 0xe29d83, 0xe29d84, 0xe29d85, 0xe29d86, 0xe29d87,
	0xe29d88, 0xe29d89, 0xe29d8a, 0xe29d8b, 0xe2978f, 0xe29d8d, 0xe296a0, 0xe29d8f,
	0xe29d90, 0xe29d91, 0xe29d92, 0xe296b2, 0xe296bc, 0xe29786, 0xe29d96, 0xe29797,
	0xe29d98, 0xe29d99, 0xe29d9a, 0xe29d9b, 0xe29d9c, 0xe29d9d, 0xe29d9e, 0x0,
	0xefa397, 0xefa398, 0xefa399, 0xefa39a, 0xefa39b, 0xefa39c, 0xefa39d, 0xefa39e,
	0xefa39f, 0xefa3a0, 0xefa3a1, 0xefa3a2, 0xefa3a3, 0xefa3a4, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0xe29da1, 0xe29da2, 0xe29da3, 0xe29da4, 0xe29da5, 0xe29da6, 0xe29da7,
	0xe299a3, 0xe299a6, 0xe299a5, 0xe299a0, 0xe291a0, 0xe291a1, 0xe291a2, 0xe291a3,
	0xe291a4, 0xe291a5, 0xe291a6, 0xe291a7, 0xe291a8, 0xe291a9, 0xe29db6, 0xe29db7,
	0xe29db8, 0xe29db9, 0xe29dba, 0xe29dbb, 0xe29dbc, 0xe29dbd, 0xe29dbe, 0xe29dbf,
	0xe29e80, 0xe29e81, 0xe29e82, 0xe29e83, 0xe29e84, 0xe29e85, 0xe29e86, 0xe29e87,
	0xe29e88, 0xe29e89, 0xe29e8a, 0xe29e8b, 0xe29e8c, 0xe29e8d, 0xe29e8e, 0xe29e8f,
	0xe29e90, 0xe29e91, 0xe29e92, 0xe29e93, 0xe29e94, 0xe28692, 0xe28694, 0xe28695,
	0xe29e98, 0xe29e99, 0xe29e9a, 0xe29e9b, 0xe29e9c, 0xe29e9d, 0xe29e9e, 0xe29e9f,
	0xe29ea0, 0xe29ea1, 0xe29ea2, 0xe29ea3, 0xe29ea4, 0xe29ea5, 0xe29ea6, 0xe29ea7,
	0xe29ea8, 0xe29ea9, 0xe29eaa, 0xe29eab, 0xe29eac, 0xe29ead, 0xe29eae, 0xe29eaf,
	0x0, 0xe29eb1, 0xe29eb2, 0xe29eb3, 0xe29eb4, 0xe29eb5, 0xe29eb6, 0xe29eb7,
	0xe29eb8, 0xe29eb9, 0xe29eba, 0xe29ebb, 0xe29ebc, 0xe29ebd, 0xe29ebe, 0x0
};

struct PDFParser::Implementation
{
	class PredefinedSimpleEncodings : public std::map<std::string, unsigned int*>
	{
		public:
			PredefinedSimpleEncodings()
			{
				insert(std::pair<std::string, unsigned int*>("MacRomanEncoding", (unsigned int*)MacRomanEncodingUtf8));
				insert(std::pair<std::string, unsigned int*>("WinAnsiEncoding", (unsigned int*)WinAnsiEncodingUtf8));
				insert(std::pair<std::string, unsigned int*>("MacExpertEncoding", (unsigned int*)MacExpertEncodingUtf8));
				insert(std::pair<std::string, unsigned int*>("StandardEncoding", (unsigned int*)StandardEncodingUtf8));
				insert(std::pair<std::string, unsigned int*>("SymbolEncoding",(unsigned int*)SymbolEncodingUtf8));
				insert(std::pair<std::string, unsigned int*>("ZapfDingbatsEncoding", (unsigned int*)ZapfDingbatsEncodingUtf8));
				insert(std::pair<std::string, unsigned int*>("PdfDocEncoding", (unsigned int*)PdfDocEncodingUtf8));
			}
	};

	static PredefinedSimpleEncodings m_pdf_predefined_simple_encodings;

	class CIDToUnicode : public std::map<std::string, std::string>
	{
		public:
			CIDToUnicode()
			{
				insert(std::pair<std::string, std::string>("GB-EUC-H", "Adobe-GB1-UCS2"));
				insert(std::pair<std::string, std::string>("GB-EUC-V", "Adobe-GB1-UCS2"));
				insert(std::pair<std::string, std::string>("GBpc-EUC-H", "Adobe-GB1-UCS2"));
				insert(std::pair<std::string, std::string>("GBpc-EUC-V", "Adobe-GB1-UCS2"));
				insert(std::pair<std::string, std::string>("GBK-EUC-H", "Adobe-GB1-UCS2"));
				insert(std::pair<std::string, std::string>("GBK-EUC-V", "Adobe-GB1-UCS2"));
				insert(std::pair<std::string, std::string>("GBK2K-H", "Adobe-GB1-UCS2"));
				insert(std::pair<std::string, std::string>("GBK2K-V", "Adobe-GB1-UCS2"));
				insert(std::pair<std::string, std::string>("UniGB-UCS2-H", "Adobe-GB1-UCS2"));
				insert(std::pair<std::string, std::string>("UniGB-UCS2-V", "Adobe-GB1-UCS2"));
				insert(std::pair<std::string, std::string>("UniGB-UTF16-H", "Adobe-GB1-UCS2"));
				insert(std::pair<std::string, std::string>("UniGB-UTF16-V", "Adobe-GB1-UCS2"));
				insert(std::pair<std::string, std::string>("B5pc-H", "Adobe-CNS1-UCS2"));
				insert(std::pair<std::string, std::string>("B5pc-V", "Adobe-CNS1-UCS2"));
				insert(std::pair<std::string, std::string>("HKscs-B5-H", "Adobe-CNS1-UCS2"));
				insert(std::pair<std::string, std::string>("HKscs-B5-V", "Adobe-CNS1-UCS2"));
				insert(std::pair<std::string, std::string>("ETen-B5-H", "Adobe-CNS1-UCS2"));
				insert(std::pair<std::string, std::string>("ETen-B5-V", "Adobe-CNS1-UCS2"));
				insert(std::pair<std::string, std::string>("ETenms-B5-H", "Adobe-CNS1-UCS2"));
				insert(std::pair<std::string, std::string>("ETenms-B5-V", "Adobe-CNS1-UCS2"));
				insert(std::pair<std::string, std::string>("CNS-EUC-H", "Adobe-CNS1-UCS2"));
				insert(std::pair<std::string, std::string>("CNS-EUC-V", "Adobe-CNS1-UCS2"));
				insert(std::pair<std::string, std::string>("UniCNS-UCS2-H", "Adobe-CNS1-UCS2"));
				insert(std::pair<std::string, std::string>("UniCNS-UCS2-V", "Adobe-CNS1-UCS2"));
				insert(std::pair<std::string, std::string>("UniCNS-UTF16-H", "Adobe-CNS1-UCS2"));
				insert(std::pair<std::string, std::string>("UniCNS-UTF16-V", "Adobe-CNS1-UCS2"));
				insert(std::pair<std::string, std::string>("83pv-RKSJ-H", "Adobe-Japan1-UCS2"));
				insert(std::pair<std::string, std::string>("90ms-RKSJ-H", "Adobe-Japan1-UCS2"));
				insert(std::pair<std::string, std::string>("90ms-RKSJ-V", "Adobe-Japan1-UCS2"));
				insert(std::pair<std::string, std::string>("90msp-RKSJ-H", "Adobe-Japan1-UCS2"));
				insert(std::pair<std::string, std::string>("90msp-RKSJ-V", "Adobe-Japan1-UCS2"));
				insert(std::pair<std::string, std::string>("90pv-RKSJ-H", "Adobe-Japan1-UCS2"));
				insert(std::pair<std::string, std::string>("Add-RKSJ-H", "Adobe-Japan1-UCS2"));
				insert(std::pair<std::string, std::string>("Add-RKSJ-V", "Adobe-Japan1-UCS2"));
				insert(std::pair<std::string, std::string>("EUC-H", "Adobe-Japan1-UCS2"));
				insert(std::pair<std::string, std::string>("EUC-V", "Adobe-Japan1-UCS2"));
				insert(std::pair<std::string, std::string>("Ext-RKSJ-H", "Adobe-Japan1-UCS2"));
				insert(std::pair<std::string, std::string>("Ext-RKSJ-V", "Adobe-Japan1-UCS2"));
				insert(std::pair<std::string, std::string>("H", "Adobe-Japan1-UCS2"));
				insert(std::pair<std::string, std::string>("V", "Adobe-Japan1-UCS2"));
				insert(std::pair<std::string, std::string>("UniJIS-UCS2-H", "Adobe-Japan1-UCS2"));
				insert(std::pair<std::string, std::string>("UniJIS-UCS2-V", "Adobe-Japan1-UCS2"));
				insert(std::pair<std::string, std::string>("UniJIS-UCS2-HW-H", "Adobe-Japan1-UCS2"));
				insert(std::pair<std::string, std::string>("UniJIS-UCS2-HW-V", "Adobe-Japan1-UCS2"));
				insert(std::pair<std::string, std::string>("UniJIS-UTF16-H", "Adobe-Japan1-UCS2"));
				insert(std::pair<std::string, std::string>("UniJIS-UTF16-V", "Adobe-Japan1-UCS2"));
				insert(std::pair<std::string, std::string>("KSC-EUC-H", "Adobe-Korea1-UCS2"));
				insert(std::pair<std::string, std::string>("KSC-EUC-V", "Adobe-Korea1-UCS2"));
				insert(std::pair<std::string, std::string>("KSCms-UHC-H", "Adobe-Korea1-UCS2"));
				insert(std::pair<std::string, std::string>("KSCms-UHC-V", "Adobe-Korea1-UCS2"));
				insert(std::pair<std::string, std::string>("KSCms-UHC-HW-H", "Adobe-Korea1-UCS2"));
				insert(std::pair<std::string, std::string>("KSCms-UHC-HW-V", "Adobe-Korea1-UCS2"));
				insert(std::pair<std::string, std::string>("KSCpc-EUC-H", "Adobe-Korea1-UCS2"));
				insert(std::pair<std::string, std::string>("UniKS-UCS2-H", "Adobe-Korea1-UCS2"));
				insert(std::pair<std::string, std::string>("UniKS-UCS2-V", "Adobe-Korea1-UCS2"));
				insert(std::pair<std::string, std::string>("UniKS-UTF16-H", "Adobe-Korea1-UCS2"));
				insert(std::pair<std::string, std::string>("UniKS-UTF16-V", "Adobe-Korea1-UCS2"));
			}
	};

	static CIDToUnicode m_pdf_cid_to_unicode;

	class CharacterNames : public std::map<std::string, unsigned int>
	{
		public:
			CharacterNames()
			{
				#warning TODO: List is incomplete. Can we find something better? Full list in PDFMiner counts about... 2000 names
				insert(std::pair<std::string, unsigned int>(".notdef", 0x0));
				insert(std::pair<std::string, unsigned int>("A", 0x41));
				insert(std::pair<std::string, unsigned int>("AE", 0xC386));
				insert(std::pair<std::string, unsigned int>("Aacute", 0xC381));
				insert(std::pair<std::string, unsigned int>("Acircumflex", 0xC382));
				insert(std::pair<std::string, unsigned int>("Adieresis", 0xC384));
				insert(std::pair<std::string, unsigned int>("Agrave", 0xC380));
				insert(std::pair<std::string, unsigned int>("Aring", 0xC385));
				insert(std::pair<std::string, unsigned int>("Aogonek", 0xC484));
				insert(std::pair<std::string, unsigned int>("Atilde", 0xC383));
				insert(std::pair<std::string, unsigned int>("B", 0x42));
				insert(std::pair<std::string, unsigned int>("C", 0x43));
				insert(std::pair<std::string, unsigned int>("Cacute", 0xC486));
				insert(std::pair<std::string, unsigned int>("Ccedilla", 0xC387));
				insert(std::pair<std::string, unsigned int>("D", 0x44));
				insert(std::pair<std::string, unsigned int>("E", 0x45));
				insert(std::pair<std::string, unsigned int>("Eacute", 0xC389));
				insert(std::pair<std::string, unsigned int>("Ecircumflex", 0xC38A));
				insert(std::pair<std::string, unsigned int>("Edieresis", 0xC38B));
				insert(std::pair<std::string, unsigned int>("Egrave", 0xC388));
				insert(std::pair<std::string, unsigned int>("Eogonek", 0xC498));
				insert(std::pair<std::string, unsigned int>("Eth", 0xC390));
				insert(std::pair<std::string, unsigned int>("Euro", 0xE282AC));
				insert(std::pair<std::string, unsigned int>("F", 0x46));
				insert(std::pair<std::string, unsigned int>("G", 0x47));
				insert(std::pair<std::string, unsigned int>("H", 0x48));
				insert(std::pair<std::string, unsigned int>("I", 0x49));
				insert(std::pair<std::string, unsigned int>("Iacute", 0xC38D));
				insert(std::pair<std::string, unsigned int>("Icircumflex", 0xC38E));
				insert(std::pair<std::string, unsigned int>("Idiereses", 0xC38F));
				insert(std::pair<std::string, unsigned int>("Igrave", 0xC38C));
				insert(std::pair<std::string, unsigned int>("J", 0x4A));
				insert(std::pair<std::string, unsigned int>("K", 0x4B));
				insert(std::pair<std::string, unsigned int>("L", 0x4C));
				insert(std::pair<std::string, unsigned int>("Lslash", 0xC581));
				insert(std::pair<std::string, unsigned int>("M", 0x4D));
				insert(std::pair<std::string, unsigned int>("N", 0x4E));
				insert(std::pair<std::string, unsigned int>("Nacute", 0xC583));
				insert(std::pair<std::string, unsigned int>("Ntilde", 0xC391));
				insert(std::pair<std::string, unsigned int>("O", 0x4F));
				insert(std::pair<std::string, unsigned int>("OE", 0xC592));
				insert(std::pair<std::string, unsigned int>("Oacute", 0xC393));
				insert(std::pair<std::string, unsigned int>("Ocircumflex", 0xC394));
				insert(std::pair<std::string, unsigned int>("Odieresis", 0xC396));
				insert(std::pair<std::string, unsigned int>("Ograve", 0xC392));
				insert(std::pair<std::string, unsigned int>("Oslash", 0xC398));
				insert(std::pair<std::string, unsigned int>("Otilde", 0xC395));
				insert(std::pair<std::string, unsigned int>("P", 0x50));
				insert(std::pair<std::string, unsigned int>("Q", 0x51));
				insert(std::pair<std::string, unsigned int>("R", 0x52));
				insert(std::pair<std::string, unsigned int>("S", 0x53));
				insert(std::pair<std::string, unsigned int>("Sacute", 0xC59A));
				insert(std::pair<std::string, unsigned int>("Scaron", 0xC5A0));
				insert(std::pair<std::string, unsigned int>("T", 0x54));
				insert(std::pair<std::string, unsigned int>("Thorn", 0xC3BE));
				insert(std::pair<std::string, unsigned int>("U", 0x55));
				insert(std::pair<std::string, unsigned int>("Uacute", 0xC39A));
				insert(std::pair<std::string, unsigned int>("Ucircumflex", 0xC39B));
				insert(std::pair<std::string, unsigned int>("Udieresis", 0xC39C));
				insert(std::pair<std::string, unsigned int>("Ugrave", 0xC399));
				insert(std::pair<std::string, unsigned int>("V", 0x56));
				insert(std::pair<std::string, unsigned int>("W", 0x57));
				insert(std::pair<std::string, unsigned int>("X", 0x58));
				insert(std::pair<std::string, unsigned int>("Y", 0x59));
				insert(std::pair<std::string, unsigned int>("Yacute", 0xC39D));
				insert(std::pair<std::string, unsigned int>("Ydieresis", 0xC5B8));
				insert(std::pair<std::string, unsigned int>("Z", 0x5A));
				insert(std::pair<std::string, unsigned int>("Zacute", 0xC5B9));
				insert(std::pair<std::string, unsigned int>("Zcaron", 0xC5BD));
				insert(std::pair<std::string, unsigned int>("Zdot", 0xC5BB));	//Im not sure about this one
				insert(std::pair<std::string, unsigned int>("a", 0x61));
				insert(std::pair<std::string, unsigned int>("aacute", 0xC3A1));
				insert(std::pair<std::string, unsigned int>("acircumflex", 0xC3A2));
				insert(std::pair<std::string, unsigned int>("acute", 0xC2B4));
				insert(std::pair<std::string, unsigned int>("adieresis", 0xC3A4));
				insert(std::pair<std::string, unsigned int>("ae", 0xC3A6));
				insert(std::pair<std::string, unsigned int>("agrave", 0xC3A0));
				insert(std::pair<std::string, unsigned int>("ampersand", 0x26));
				insert(std::pair<std::string, unsigned int>("aogonek", 0xC485));
				insert(std::pair<std::string, unsigned int>("aring", 0xC3A5));
				insert(std::pair<std::string, unsigned int>("asciicircum", 0xCB86));
				insert(std::pair<std::string, unsigned int>("asciitilde", 0xCB9C));
				insert(std::pair<std::string, unsigned int>("asterisk", 0x2A));
				insert(std::pair<std::string, unsigned int>("at", 0x40));
				insert(std::pair<std::string, unsigned int>("atilde", 0xC3A3));
				insert(std::pair<std::string, unsigned int>("b", 0x62));
				insert(std::pair<std::string, unsigned int>("backslash", 0x5C));
				insert(std::pair<std::string, unsigned int>("bar", 0x7C));
				insert(std::pair<std::string, unsigned int>("braceleft", 0x7B));
				insert(std::pair<std::string, unsigned int>("braceright", 0x7D));
				insert(std::pair<std::string, unsigned int>("bracketleft", 0x5B));
				insert(std::pair<std::string, unsigned int>("bracketright", 0x5D));
				insert(std::pair<std::string, unsigned int>("breve", 0xCB98));
				insert(std::pair<std::string, unsigned int>("brokenbar", 0xC2A6));
				insert(std::pair<std::string, unsigned int>("bullet", 0xE280A2));
				insert(std::pair<std::string, unsigned int>("c", 0x63));
				insert(std::pair<std::string, unsigned int>("caron", 0xCB87));
				insert(std::pair<std::string, unsigned int>("ccedilla", 0xC3A7));
				insert(std::pair<std::string, unsigned int>("cedilla", 0xC2B8));
				insert(std::pair<std::string, unsigned int>("cent", 0xC2A2));
				insert(std::pair<std::string, unsigned int>("circumflex", 0x5E));
				insert(std::pair<std::string, unsigned int>("cacute", 0xC487));
				insert(std::pair<std::string, unsigned int>("colon", 0x3A));
				insert(std::pair<std::string, unsigned int>("comma", 0x2C));
				insert(std::pair<std::string, unsigned int>("copyright", 0xC2A9));
				insert(std::pair<std::string, unsigned int>("currency", 0xC2A4));
				insert(std::pair<std::string, unsigned int>("d", 0x64));
				insert(std::pair<std::string, unsigned int>("dagger", 0xE280A0));
				insert(std::pair<std::string, unsigned int>("daggerdbl", 0xE280A1));
				insert(std::pair<std::string, unsigned int>("degree", 0xC2B0));
				insert(std::pair<std::string, unsigned int>("dieresis", 0xC2A8));
				insert(std::pair<std::string, unsigned int>("divide", 0xC3B7));
				insert(std::pair<std::string, unsigned int>("dollar", 0x24));
				insert(std::pair<std::string, unsigned int>("dotaccent", 0xCB99));
				insert(std::pair<std::string, unsigned int>("dotlessi", 0xC4B1));
				insert(std::pair<std::string, unsigned int>("e", 0x65));
				insert(std::pair<std::string, unsigned int>("eacute", 0xC3A9));
				insert(std::pair<std::string, unsigned int>("ecircumflex", 0xC3AA));
				insert(std::pair<std::string, unsigned int>("edieresis", 0xC3AB));
				insert(std::pair<std::string, unsigned int>("eogonek", 0xC499));
				insert(std::pair<std::string, unsigned int>("egrave", 0xC3A8));
				insert(std::pair<std::string, unsigned int>("eight", 0x38));
				insert(std::pair<std::string, unsigned int>("ellipsis", 0xE280A6));
				insert(std::pair<std::string, unsigned int>("emdash", 0xE28094));
				insert(std::pair<std::string, unsigned int>("endash", 0xE28093));
				insert(std::pair<std::string, unsigned int>("equal", 0x3D));
				insert(std::pair<std::string, unsigned int>("eth", 0xC3B0));
				insert(std::pair<std::string, unsigned int>("exclam", 0x21));
				insert(std::pair<std::string, unsigned int>("exclamdown", 0xC2A1));
				insert(std::pair<std::string, unsigned int>("f", 0x66));
				insert(std::pair<std::string, unsigned int>("fi", 0xEFAC81));
				insert(std::pair<std::string, unsigned int>("five", 0x35));
				insert(std::pair<std::string, unsigned int>("fl", 0xEFAC82));
				insert(std::pair<std::string, unsigned int>("florin", 0xC692));
				insert(std::pair<std::string, unsigned int>("four", 0x34));
				insert(std::pair<std::string, unsigned int>("fraction", 0xE281A4));
				insert(std::pair<std::string, unsigned int>("g", 0x67));
				insert(std::pair<std::string, unsigned int>("germandbls", 0xC39F));
				insert(std::pair<std::string, unsigned int>("grave", 0x60));
				insert(std::pair<std::string, unsigned int>("greater", 0x3E));
				insert(std::pair<std::string, unsigned int>("guillemotleft", 0xC2AB));
				insert(std::pair<std::string, unsigned int>("guillemotright", 0xC2BB));
				insert(std::pair<std::string, unsigned int>("guilsinglleft", 0xE280B9));
				insert(std::pair<std::string, unsigned int>("guilsinglright", 0xE280BA));
				insert(std::pair<std::string, unsigned int>("h", 0x68));
				insert(std::pair<std::string, unsigned int>("hungarumlaut", 0xCB9D));
				insert(std::pair<std::string, unsigned int>("hyphen", 0x2D));
				insert(std::pair<std::string, unsigned int>("i", 0x69));
				insert(std::pair<std::string, unsigned int>("iacute", 0xC3AD));
				insert(std::pair<std::string, unsigned int>("icircumflex", 0xC3AE));
				insert(std::pair<std::string, unsigned int>("idieresis", 0xC3AF));
				insert(std::pair<std::string, unsigned int>("igrave", 0xC3AC));
				insert(std::pair<std::string, unsigned int>("j", 0x6A));
				insert(std::pair<std::string, unsigned int>("k", 0x6B));
				insert(std::pair<std::string, unsigned int>("l", 0x6C));
				insert(std::pair<std::string, unsigned int>("less", 0x3C));
				insert(std::pair<std::string, unsigned int>("logicalnot", 0xC2AC));
				insert(std::pair<std::string, unsigned int>("lslash", 0xC582));
				insert(std::pair<std::string, unsigned int>("m", 0x6D));
				insert(std::pair<std::string, unsigned int>("macron", 0xC2AF));
				insert(std::pair<std::string, unsigned int>("minus", 0xE28892));
				insert(std::pair<std::string, unsigned int>("mu", 0xC2B5));
				insert(std::pair<std::string, unsigned int>("multiply", 0xC397));
				insert(std::pair<std::string, unsigned int>("n", 0x6E));
				insert(std::pair<std::string, unsigned int>("nine", 0x39));
				insert(std::pair<std::string, unsigned int>("nacute", 0xC584));
				insert(std::pair<std::string, unsigned int>("ntilde", 0xC3B1));
				insert(std::pair<std::string, unsigned int>("numbersign", 0x23));
				insert(std::pair<std::string, unsigned int>("o", 0x6F));
				insert(std::pair<std::string, unsigned int>("oacute", 0xC3B3));
				insert(std::pair<std::string, unsigned int>("ocircumflex", 0xC3B4));
				insert(std::pair<std::string, unsigned int>("odieresis", 0xC3B6));
				insert(std::pair<std::string, unsigned int>("oe", 0xC593));
				insert(std::pair<std::string, unsigned int>("ogonek", 0xCB9B));
				insert(std::pair<std::string, unsigned int>("ograve", 0xC3B2));
				insert(std::pair<std::string, unsigned int>("one", 0x31));
				insert(std::pair<std::string, unsigned int>("onehalf", 0xC2BD));
				insert(std::pair<std::string, unsigned int>("onequarter", 0xC2BC));
				insert(std::pair<std::string, unsigned int>("onesuperior", 0xC2B9));
				insert(std::pair<std::string, unsigned int>("ordfeminine", 0xC2AA));
				insert(std::pair<std::string, unsigned int>("ordmasculine", 0xC2BA));
				insert(std::pair<std::string, unsigned int>("oslash", 0xC3B8));
				insert(std::pair<std::string, unsigned int>("otilde", 0xC3B5));
				insert(std::pair<std::string, unsigned int>("p", 0x70));
				insert(std::pair<std::string, unsigned int>("paragraph", 0xC2B6));
				insert(std::pair<std::string, unsigned int>("parenleft", 0x28));
				insert(std::pair<std::string, unsigned int>("parenright", 0x29));
				insert(std::pair<std::string, unsigned int>("percent", 0x25));
				insert(std::pair<std::string, unsigned int>("period", 0x2E));
				insert(std::pair<std::string, unsigned int>("periodcentered", 0xC2B7));
				insert(std::pair<std::string, unsigned int>("perthousand", 0xE280B0));
				insert(std::pair<std::string, unsigned int>("plus", 0x2B));
				insert(std::pair<std::string, unsigned int>("plusminus", 0xC2B1));
				insert(std::pair<std::string, unsigned int>("q", 0x71));
				insert(std::pair<std::string, unsigned int>("question", 0x3F));
				insert(std::pair<std::string, unsigned int>("questiondown", 0xC2BF));
				insert(std::pair<std::string, unsigned int>("quotedbl", 0x22));
				insert(std::pair<std::string, unsigned int>("quotedblbase", 0xE2809E));
				insert(std::pair<std::string, unsigned int>("quotedblleft", 0xE2809C));
				insert(std::pair<std::string, unsigned int>("quotedblright", 0xE2809D));
				insert(std::pair<std::string, unsigned int>("quoteleft", 0xE28098));
				insert(std::pair<std::string, unsigned int>("quoteright", 0xE28099));
				insert(std::pair<std::string, unsigned int>("quotesinglbase", 0xE2809A));
				insert(std::pair<std::string, unsigned int>("quotesingle", 0x27));
				insert(std::pair<std::string, unsigned int>("r", 0x72));
				insert(std::pair<std::string, unsigned int>("registered", 0xC2AE));
				insert(std::pair<std::string, unsigned int>("rign", 0xCB9A));
				insert(std::pair<std::string, unsigned int>("s", 0x73));
				insert(std::pair<std::string, unsigned int>("sacute", 0xC59B));
				insert(std::pair<std::string, unsigned int>("scaron", 0xC5A1));
				insert(std::pair<std::string, unsigned int>("section", 0xC2A7));
				insert(std::pair<std::string, unsigned int>("semicolon", 0x3B));
				insert(std::pair<std::string, unsigned int>("seven", 0x37));
				insert(std::pair<std::string, unsigned int>("six", 0x36));
				insert(std::pair<std::string, unsigned int>("slash", 0x2F));
				insert(std::pair<std::string, unsigned int>("space", 0x20));
				insert(std::pair<std::string, unsigned int>("sterling", 0xC2A3));
				insert(std::pair<std::string, unsigned int>("t", 0x74));
				insert(std::pair<std::string, unsigned int>("thorn", 0xC39E));
				insert(std::pair<std::string, unsigned int>("three", 0x33));
				insert(std::pair<std::string, unsigned int>("threequarters", 0xC2BE));
				insert(std::pair<std::string, unsigned int>("threesuperior", 0xC2B3));
				insert(std::pair<std::string, unsigned int>("tilde", 0x7E));
				insert(std::pair<std::string, unsigned int>("trademark", 0xE284A2));
				insert(std::pair<std::string, unsigned int>("two", 0x32));
				insert(std::pair<std::string, unsigned int>("twosuperior", 0xC2B2));
				insert(std::pair<std::string, unsigned int>("u", 0x75));
				insert(std::pair<std::string, unsigned int>("uacute", 0xC3BA));
				insert(std::pair<std::string, unsigned int>("ucircumflex", 0xC3BB));
				insert(std::pair<std::string, unsigned int>("udieresis", 0xC3BC));
				insert(std::pair<std::string, unsigned int>("ugrave", 0xC3B9));
				insert(std::pair<std::string, unsigned int>("underscore", 0x5F));
				insert(std::pair<std::string, unsigned int>("v", 0x76));
				insert(std::pair<std::string, unsigned int>("w", 0x77));
				insert(std::pair<std::string, unsigned int>("x", 0x78));
				insert(std::pair<std::string, unsigned int>("y", 0x79));
				insert(std::pair<std::string, unsigned int>("yacute", 0xC3BD));
				insert(std::pair<std::string, unsigned int>("ydieresis", 0xC3BF));
				insert(std::pair<std::string, unsigned int>("yen", 0xC2A5));
				insert(std::pair<std::string, unsigned int>("z", 0x7A));
				insert(std::pair<std::string, unsigned int>("zacute", 0xC5BA));
				insert(std::pair<std::string, unsigned int>("zcaron", 0xC5BE));
				insert(std::pair<std::string, unsigned int>("zdot", 0xC5BC));	//not sure about this
				insert(std::pair<std::string, unsigned int>("zero", 0x30));
			}
	};

	static CharacterNames m_pdf_character_names;

	class PDFReader
	{
		public:
			enum PDFObjectTypes
			{
				dictionary,
				boolean,
				int_numeric,
				float_numeric,
				array,
				string,
				name,
				stream,
				null,
				//indirect object has structure:
				//A B obj
				//[object data of any kind (can be array, number, dictionary etc.)]
				//endobj
				//
				//where A is an index of this object, B is generation number
				indirect_object,
				//reference is a combination of two numbers and 'R' character like:
				//A B R.
				//A is an index of the object, B is generation number
				reference_call,
				operator_obj,
				unknown_obj
			};

			enum CompressionTypes
			{
				ascii_hex,
				lzw,
				rle,
				ascii_85,
				flat,
				unknown_compression
			};

			enum OperatorTypes
			{
				Tj, //直接类型内容的标识
				TJ, //cid内容标识
				Td, //Td 和 Tm 决定是否换行
				TD,
				T_star,
				Tm, //Td 和 Tm 决定是否换行
				double_quote,
				quote,
				TL,
				BT, //文本块开始标识
				ET, //文本块结束标识
				Tf, //字体名称标识
				TZ,
				cm,
				q,
				Q,
				Tc,
				Tw,
				Ts,
				#warning TODO: Add support for operator "Do".
				Do, //存储在 resource 中的 cid 内容标识
				usecmap,
				begincidrange,
				endcidrange,
				begincidchar,
				endcidchar,
				beginnotdefrange,
				endnotdefrange,
				beginnotdefchar,
				endnotdefchar,
				beginbfrange,
				endbfrange,
				beginbfchar,
				endbfchar,
				unknown_operator
			};

		private:
			class CompressionCodes : public std::map<std::string, CompressionTypes>
			{
				public:
					CompressionCodes()
					{
						insert(std::pair<std::string, CompressionTypes>("ASCIIHexDecode", ascii_hex));
						insert(std::pair<std::string, CompressionTypes>("LZWDecode", lzw));
						insert(std::pair<std::string, CompressionTypes>("RunLengthDecode", rle));
						insert(std::pair<std::string, CompressionTypes>("ASCII85Decode", ascii_85));
						insert(std::pair<std::string, CompressionTypes>("FlateDecode", flat));
					}
			};

			class OperatorCodes : public std::map<std::string, OperatorTypes>
			{
				public:
					OperatorCodes()
					{
						insert(std::pair<std::string, OperatorTypes>("Tj", Tj));
						insert(std::pair<std::string, OperatorTypes>("TJ", TJ));
						insert(std::pair<std::string, OperatorTypes>("Td", Td));
						insert(std::pair<std::string, OperatorTypes>("TD", TD));
						insert(std::pair<std::string, OperatorTypes>("T*", T_star));
						insert(std::pair<std::string, OperatorTypes>("Tm", Tm));
						insert(std::pair<std::string, OperatorTypes>("\"", double_quote));
						insert(std::pair<std::string, OperatorTypes>("\'", quote));
						insert(std::pair<std::string, OperatorTypes>("TL", TL));
						insert(std::pair<std::string, OperatorTypes>("BT", BT));
						insert(std::pair<std::string, OperatorTypes>("ET", ET));
						insert(std::pair<std::string, OperatorTypes>("Tf", Tf));
						insert(std::pair<std::string, OperatorTypes>("Do", Do));
						insert(std::pair<std::string, OperatorTypes>("TZ", TZ));
						insert(std::pair<std::string, OperatorTypes>("cm", cm));
						insert(std::pair<std::string, OperatorTypes>("q", q));
						insert(std::pair<std::string, OperatorTypes>("Q", Q));
						insert(std::pair<std::string, OperatorTypes>("Ts", Ts));
						insert(std::pair<std::string, OperatorTypes>("Tw", Tw));
						insert(std::pair<std::string, OperatorTypes>("Tc", Tc));
						insert(std::pair<std::string, OperatorTypes>("usecmap", usecmap));
						insert(std::pair<std::string, OperatorTypes>("begincidrange", begincidrange));
						insert(std::pair<std::string, OperatorTypes>("endcidrange", endcidrange));
						insert(std::pair<std::string, OperatorTypes>("begincidchar", begincidchar));
						insert(std::pair<std::string, OperatorTypes>("endcidchar", endcidchar));
						insert(std::pair<std::string, OperatorTypes>("beginnotdefrange", beginnotdefrange));
						insert(std::pair<std::string, OperatorTypes>("endnotdefrange", endnotdefrange));
						insert(std::pair<std::string, OperatorTypes>("beginnotdefchar", beginnotdefchar));
						insert(std::pair<std::string, OperatorTypes>("endnotdefchar", endnotdefchar));
						insert(std::pair<std::string, OperatorTypes>("beginbfrange", beginbfrange));
						insert(std::pair<std::string, OperatorTypes>("endbfrange", endbfrange));
						insert(std::pair<std::string, OperatorTypes>("beginbfchar", beginbfchar));
						insert(std::pair<std::string, OperatorTypes>("endbfchar", endbfchar));
					}
			};

			static CompressionCodes m_compression_codes;
			static OperatorCodes m_operator_codes;

		public:
			static CompressionTypes getCompressionCode(const std::string& compression_name)
			{
				CompressionCodes::const_iterator it = m_compression_codes.find(compression_name);
				if (it == m_compression_codes.end())
					return unknown_compression;
				return it->second;
			}

			static OperatorTypes getOperatorCode(const std::string& operator_name)
			{
				OperatorCodes::const_iterator it = m_operator_codes.find(operator_name);
				if (it == m_operator_codes.end())
					return unknown_operator;
				return it->second;
			}

		public:
			class ReferenceInfo;
			class PDFObject;
			class PDFDictionary;
			class PDFBoolean;
			class PDFNumericInteger;
			class PDFNumericFloat;
			class PDFArray;
			class PDFString;
			class PDFName;
			class PDFStream;
			class PDFNull;
			class PDFIndirectObject;
			class PDFReferenceCall;

			class PDFObject
			{
				public:
					virtual ~PDFObject()
					{
					}

					virtual PDFObjectTypes getType() = 0;

					virtual PDFString* getString()
					{
						return NULL;
					}

					bool isString()
					{
						return getType() == string;
					}

					virtual PDFDictionary* getDictionary()
					{
						return NULL;
					}

					bool isDictionary()
					{
						return getType() == dictionary;
					}

					virtual PDFBoolean* getBoolean()
					{
						return NULL;
					}

					bool isBoolean()
					{
						return getType() == boolean;
					}

					virtual PDFNumericFloat* getNumericFloat()
					{
						return NULL;
					}

					bool isNumericFloat()
					{
						return getType() == float_numeric;
					}

					virtual PDFNumericInteger* getNumericInteger()
					{
						return NULL;
					}

					bool isNumericInteger()
					{
						return getType() == int_numeric;
					}

					virtual PDFArray* getArray()
					{
						return NULL;
					}

					bool isArray()
					{
						return getType() == array;
					}

					virtual PDFName* getName()
					{
						return NULL;
					}

					bool isName()
					{
						return getType() == name;
					}

					virtual PDFStream* getStream()
					{
						return NULL;
					}

					bool isStream()
					{
						return getType() == stream;
					}

					virtual PDFNull* getNull()
					{
						return NULL;
					}

					bool isNull()
					{
						return getType() == null;
					}

					virtual PDFIndirectObject* getIndirectObject()
					{
						return NULL;
					}

					virtual bool isIndirectObject()
					{
						return false;
					}

					virtual PDFReferenceCall* getReferenceCall()
					{
						return NULL;
					}

					virtual bool isReferenceCall()
					{
						return false;
					}
			};

			class PDFDictionary : public PDFObject
			{
				public:
					std::map<std::string, PDFObject*> m_objects;

					~PDFDictionary()
					{
						clearDictionary();
					}

					void clearDictionary()
					{
						std::map<std::string, PDFObject*>::iterator it;
						for (it = m_objects.begin(); it != m_objects.end(); ++it)
							delete it->second;
						m_objects.clear();
					}

					PDFObject* operator [](const std::string& key)
					{
						std::map<std::string, PDFObject*>::iterator it = m_objects.find(key);
						if (it == m_objects.end())
							return NULL;
						return it->second;
					}

					PDFDictionary* getObjAsDictionary(const std::string& key)
					{
						std::map<std::string, PDFObject*>::iterator it = m_objects.find(key);
						if (it == m_objects.end())
						{
							return NULL;
						}
						return it->second->getDictionary();
					}

					PDFBoolean* getObjAsBoolean(const std::string& key)
					{
						std::map<std::string, PDFObject*>::iterator it = m_objects.find(key);
						if (it == m_objects.end())
						{
							return NULL;
						}
						return it->second->getBoolean();
					}

					bool getValAsBoolean(const std::string& key, bool def)
					{
						std::map<std::string, PDFObject*>::iterator it = m_objects.find(key);
						if (it == m_objects.end())
							return def;
						PDFBoolean* val = it->second->getBoolean();
						if (val)
							return val->m_value;
						return def;
					}

					PDFNumericInteger* getObjAsNumericInteger(const std::string& key)
					{
						std::map<std::string, PDFObject*>::iterator it = m_objects.find(key);
						if (it == m_objects.end())
							return NULL;
						return it->second->getNumericInteger();
					}

					PDFNumericFloat* getObjAsNumericFloat(const std::string& key)
					{
						std::map<std::string, PDFObject*>::iterator it = m_objects.find(key);
						if (it == m_objects.end())
							return NULL;
						return it->second->getNumericFloat();
					}

					double getValAsDouble(const std::string& key, double def)
					{
						std::map<std::string, PDFObject*>::iterator it = m_objects.find(key);
						if (it == m_objects.end())
							return def;
						PDFNumericFloat* float_val = it->second->getNumericFloat();
						if (float_val)
							return float_val->m_value;
						PDFNumericInteger* int_val = it->second->getNumericInteger();
						if (int_val)
							return (double)int_val->m_value;
						return def;
					}

					long getValAsInteger(const std::string& key, long def)
					{
						std::map<std::string, PDFObject*>::iterator it = m_objects.find(key);
						if (it == m_objects.end())
							return def;
						PDFNumericInteger* int_val = it->second->getNumericInteger();
						if (int_val)
							return int_val->m_value;
						return def;
					}

					PDFArray* getObjAsArray(const std::string& key)
					{
						std::map<std::string, PDFObject*>::iterator it = m_objects.find(key);
						if (it == m_objects.end())
							return NULL;
						return it->second->getArray();
					}

					PDFString* getObjAsString(const std::string& key)
					{
						std::map<std::string, PDFObject*>::iterator it = m_objects.find(key);
						if (it == m_objects.end())
							return NULL;
						return it->second->getString();
					}

					PDFName* getObjAsName(const std::string& key)
					{
						std::map<std::string, PDFObject*>::iterator it = m_objects.find(key);
						if (it == m_objects.end())
							return NULL;
						return it->second->getName();
					}

					std::string getValAsString(const std::string& key, const std::string& def)
					{
						std::map<std::string, PDFObject*>::iterator it = m_objects.find(key);
						if (it == m_objects.end())
							return def;
						PDFString* str_obj = it->second->getString();
						if (str_obj)
						{
							str_obj->ConvertToLiteral();
							return str_obj->m_value;
						}
						PDFName* name_obj = it->second->getName();
						if (name_obj)
						{
							return name_obj->m_value;
						}
						return def;
					}

					PDFNull* getObjAsNull(const std::string& key)
					{
						std::map<std::string, PDFObject*>::iterator it = m_objects.find(key);
						if (it == m_objects.end())
							return NULL;
						return it->second->getNull();
					}

					PDFReferenceCall* getObjAsReferenceCall(const std::string& key)
					{
						std::map<std::string, PDFObject*>::iterator it = m_objects.find(key);
						if (it == m_objects.end())
						{
							return NULL;
						}
						return it->second->getReferenceCall();
					}

					PDFStream* getObjAsStream(const std::string& key)
					{
						std::map<std::string, PDFObject*>::iterator it = m_objects.find(key);
						if (it == m_objects.end())
						{
							return NULL;
						}
						return it->second->getStream();
					}

					PDFObjectTypes getType()
					{
						return dictionary;
					}

					PDFDictionary* getDictionary()
					{
						return this;
					}
			};

			class PDFBoolean : public PDFObject
			{
				public:
					bool m_value;

					PDFBoolean()
					{
						m_value == false;
					}

					bool& operator ()()
					{
						return m_value;
					}

					PDFObjectTypes getType()
					{
						return boolean;
					}

					PDFBoolean* getBoolean()
					{
						return this;
					}
			};

			class PDFNumericInteger : public PDFObject
			{
				public:
					long m_value;

					PDFNumericInteger()
					{
						m_value = 0;
					}

					long& operator()()
					{
						return m_value;
					}

					PDFObjectTypes getType()
					{
						return int_numeric;
					}

					PDFNumericInteger* getNumericInteger()
					{
						return this;
					}
			};

			class PDFNumericFloat : public PDFObject
			{
				public:
					double m_value;

					PDFNumericFloat()
					{
						m_value = 0.0;
					}

					double& operator()()
					{
						return m_value;
					}

					PDFObjectTypes getType()
					{
						return float_numeric;
					}

					PDFNumericFloat* getNumericFloat()
					{
						return this;
					}
			};

			class PDFArray : public PDFObject
			{
				public:
					std::vector<PDFObject*> m_objects;

					~PDFArray()
					{
						for (size_t i = 0; i < m_objects.size(); ++i)
							delete m_objects[i];
						m_objects.clear();
					}

					PDFObject* operator [](unsigned long index)
					{
						return m_objects[index];
					}

					PDFDictionary* getObjAsDictionary(unsigned long index)
					{
						return m_objects[index]->getDictionary();
					}

					PDFBoolean* getObjAsBoolean(unsigned long index)
					{
						return m_objects[index]->getBoolean();
					}

					bool getValAsBoolean(unsigned long index, bool def)
					{
						PDFObject* obj = m_objects[index];
						PDFBoolean* val = obj->getBoolean();
						if (val)
							return val->m_value;
						return def;
					}

					PDFNumericInteger* getObjAsNumericInteger(unsigned long index)
					{
						return m_objects[index]->getNumericInteger();
					}

					PDFNumericFloat* getObjAsNumericFloat(unsigned long index)
					{
						return m_objects[index]->getNumericFloat();
					}

					PDFArray* getObjAsArray(unsigned long index)
					{
						return m_objects[index]->getArray();
					}

					double getValAsDouble(unsigned long index, double def)
					{
						PDFObject* obj = m_objects[index];
						PDFNumericFloat* float_val = obj->getNumericFloat();
						if (float_val)
							return float_val->m_value;
						PDFNumericInteger* int_val = obj->getNumericInteger();
						if (int_val)
							return (double)int_val->m_value;
						return def;
					}

					long getValAsInteger(unsigned long index, long def)
					{
						PDFObject* obj = m_objects[index];
						PDFNumericInteger* int_val = obj->getNumericInteger();
						if (int_val)
							return int_val->m_value;
						return def;
					}

					PDFStream* getObjAsStream(unsigned long index)
					{
						return m_objects[index]->getStream();
					}

					PDFString* getObjAsString(unsigned long index)
					{
						return m_objects[index]->getString();
					}

					PDFName* getObjAsName(unsigned long index)
					{
						return m_objects[index]->getName();
					}

					std::string getValAsString(unsigned long index, const std::string& def)
					{
						PDFObject* obj = m_objects[index];
						PDFString* str_obj = obj->getString();
						if (str_obj)
						{
							str_obj->ConvertToLiteral();
							return str_obj->m_value;
						}
						PDFName* name_obj = obj->getName();
						if (name_obj)
							return name_obj->m_value;
						return def;
					}

					PDFNull* getObjAsNull(unsigned long index)
					{
						return m_objects[index]->getNull();
					}

					PDFReferenceCall* getObjAsReferenceCall(unsigned long index)
					{
						return m_objects[index]->getReferenceCall();
					}

					PDFObjectTypes getType()
					{
						return array;
					}

					PDFArray* getArray()
					{
						return this;
					}

					size_t Size()
					{
						return m_objects.size();
					}
			};

			class PDFString : public PDFObject
			{
				public:
					bool m_is_hex;
					std::string m_value;

					PDFString()
					{
						m_is_hex = false;
					}

					std::string& operator()()
					{
						return m_value;
					}

					bool IsHex()
					{
						return m_is_hex;
					}

					bool IsLiteral()
					{
						return !m_is_hex;
					}

					void ConvertToHex()
					{
						if (!m_is_hex)
						{
							m_is_hex = true;
							std::string tmp;
							tmp.reserve(m_value.length() * 2);
							for (size_t i = 0; i < m_value.length(); ++i)
								tmp += char_to_hex_char(m_value[i]);
							m_value = tmp;
						}
					}

					void ConvertToLiteral()
					{
						if (m_is_hex)
						{
							m_is_hex = false;
							std::string tmp;
							tmp.reserve(m_value.length() / 2);
							for (size_t i = 0; i < m_value.length(); i += 2)
								tmp += hex_char_to_single_char(&m_value[i]);
							m_value = tmp;
						}
					}

					PDFObjectTypes getType()
					{
						return string;
					}

					PDFString* getString()
					{
						return this;
					}
			};

			class PDFName : public PDFObject
			{
				public:
					std::string m_value;

					std::string& operator ()()
					{
						return m_value;
					}

					PDFObjectTypes getType()
					{
						return name;
					}

					PDFName* getName()
					{
						return this;
					}
			};

			class PDFStream : public PDFObject
			{
				private:
					typedef std::vector<unsigned char> lzw_item;

					class Predictior
					{
						public:
							size_t m_predictor;
							size_t m_colors;
							size_t m_bpc;
							size_t m_columns;
							size_t m_early_change;
							bool m_next_byte_is_predictor;
							int m_current_predictor;
							int m_current_row_index;
							int m_bpp;
							std::vector<char> m_previos;

							Predictior(PDFDictionary& decode_params)
							{
								try
								{
									m_predictor = 1;
									PDFNumericInteger* pred_numeric = decode_params.getObjAsNumericInteger("Predictor");
									if (pred_numeric)
										m_predictor = (*pred_numeric)();

									m_colors = 1;
									PDFNumericInteger* color_numeric = decode_params.getObjAsNumericInteger("Colors");
									if (color_numeric)
										m_colors = (*color_numeric)();

									m_bpc = 8;
									PDFNumericInteger* bpc_numeric = decode_params.getObjAsNumericInteger("BitsPerComponent");
									if (bpc_numeric)
										m_colors = (*bpc_numeric)();

									m_columns = 1;
									PDFNumericInteger* columns_numeric = decode_params.getObjAsNumericInteger("Columns");
									if (columns_numeric)
										m_columns = (*columns_numeric)();

									m_early_change = 1;
									PDFNumericInteger* early_change_numeric = decode_params.getObjAsNumericInteger("EarlyChange");
									if (early_change_numeric)
										m_early_change = (*early_change_numeric)();
								}
								catch (Exception& ex)
								{
									ex.appendError("Error while creating predictor handler for decoding");
									throw;
								}

								if (m_predictor >= 10)
								{
									m_next_byte_is_predictor = true;
									m_current_predictor = -1;
								}
								else
								{
									m_next_byte_is_predictor = false;
									m_current_predictor = m_predictor;
								}
								m_current_row_index = 0;
								m_bpp = (m_bpc * m_colors) >> 3;
								m_previos.resize((m_columns * m_colors * m_bpc) >> 3, 0);
							}

							void decode(unsigned char* src, unsigned int src_len, std::vector<unsigned char>& dest)
							{
								if (m_predictor == 1)
								{
									for (size_t i = 0; i < src_len; ++i)
										dest.push_back(src[i]);
									return;
								}
								size_t read_index = 0;
								while (read_index < src_len)
								{
									char ch = src[read_index++];
									if (m_next_byte_is_predictor)
									{
										m_current_predictor = ch + 10;
										m_next_byte_is_predictor = false;
									}
									else
									{
										switch (m_current_predictor)
										{
											case 2:
											{
												if (m_bpc == 8)
												{
													int tmp = 0;
													if (m_current_row_index - m_bpp >= 0)
														tmp = m_previos[m_current_row_index - m_bpp];
													m_previos[m_current_row_index++] = ch + tmp;
													break;
												}
												throw Exception("Predictor cannot decode data, unsupported bpc value: " + m_bpc);
											}
											case 10:
											{
												m_previos[m_current_row_index++] = ch;
												break;
											}
											case 11:
											{
												int tmp = 0;
												if (m_current_row_index - m_bpp >= 0)
													tmp = m_previos[m_current_row_index - m_bpp];
												m_previos[m_current_row_index++] = ch + tmp;
												break;
											}
											case 12:
											{
												m_previos[m_current_row_index++] += ch;
												break;
											}
											case 13:
											{
												int tmp = 0;
												if (m_current_row_index - m_bpp >= 0)
													tmp = m_previos[m_current_row_index - m_bpp];
												m_previos[m_current_row_index++] = ((tmp + m_previos[m_current_row_index]) >> 1) + ch;
												break;
											}
											case 14:
											case 15:
												throw Exception("Predictor cannot decode data, unsupported predictor value: " + uint_to_string(m_current_predictor));
										}
									}

									if (m_current_row_index >= m_previos.size())
									{
										m_current_row_index = 0;
										m_next_byte_is_predictor = m_current_predictor >= 10;
										for (size_t i = 0; i < m_previos.size(); ++i)
											dest.push_back(m_previos[i]);
									}
								}
							}
					};

					struct CompressedObjectInfo
					{
						size_t m_offset;
						size_t m_index;
					};

				public:
					//iterates throw all elements in stream data. This kind of object can exist without PDFStream.
					//PDFStream only provides data for iterator.
					class PDFStreamIterator
					{
						private:
							struct Pointer
							{
								char* m_buffer;
								size_t m_element_size;
								size_t m_buffer_size;
								PDFObjectTypes m_type;
							};
							std::vector<Pointer> m_pointers_stack;
							size_t m_depth;

						public:
							void init(char* buffer, size_t buffer_size)
							{
								Pointer ptr;
								ptr.m_buffer = buffer;
								ptr.m_buffer_size = buffer_size;
								ptr.m_element_size = buffer_size;
								ptr.m_type = array;	//treat stream like a big array
								m_depth = 0;
								m_pointers_stack.push_back(ptr);
							}

							void seek(size_t offset)
							{
								Pointer* ptr = &m_pointers_stack[m_depth];
								if (offset > ptr->m_buffer_size)
								{
									throw Exception(std::string("PDF Stream iterator: Cant seek. Offset: ") + uint_to_string(offset) +
													std::string(", size of buffer: ") + uint_to_string(ptr->m_buffer_size));
								}
								ptr->m_buffer_size -= offset;
								ptr->m_element_size = 0;
								ptr->m_type = null;
								ptr->m_buffer += offset;
							}

							void backToRoot()
							{
								char* buffer = m_pointers_stack[0].m_buffer;
								size_t buffer_size = m_pointers_stack[0].m_buffer_size;
								m_pointers_stack.clear();
								init(buffer, buffer_size);
							}

							bool canDown()
							{
								PDFObjectTypes type = m_pointers_stack[m_depth].m_type;
								return type == array || type == dictionary;
							}

							void levelDown()
							{
								if (!canDown())
									throw Exception("PDF Stream iterator: cant go down. Type of object is not dictionary nor array");
								Pointer* prev_ptr = &m_pointers_stack[m_depth];
								++m_depth;
								Pointer ptr;
								ptr.m_buffer = prev_ptr->m_buffer;
								if (prev_ptr->m_type == array)
								{
									ptr.m_buffer += 1;
									ptr.m_buffer_size = prev_ptr->m_element_size - 2;
								}
								else if (prev_ptr->m_type == dictionary)
								{
									ptr.m_buffer += 2;
									ptr.m_buffer_size = prev_ptr->m_element_size - 4;
								}
								ptr.m_element_size = 0;
								ptr.m_type = null;
								m_pointers_stack.push_back(ptr);
							}

							void getNextElement()
							{
								Pointer* ptr = &m_pointers_stack[m_depth];
								ptr->m_buffer += ptr->m_element_size;
								ptr->m_buffer_size -= ptr->m_element_size;
								ptr->m_element_size = 0;
								ptr->m_type = null;

								if (ptr->m_buffer_size == 0)
								{
									ptr->m_type = null;
									return;
								}
								while (ptr->m_buffer_size > 0)
								{
									switch (ptr->m_buffer[0])
									{
										case '/':
										{
											++ptr->m_element_size;
											ptr->m_type = name;
											readName(*ptr);
											return;
										}
										case '<':
										{
											++ptr->m_element_size;
											if (ptr->m_buffer_size > ptr->m_element_size)
											{
												if (ptr->m_buffer[1] == '<')
												{
													ptr->m_type = dictionary;
													++ptr->m_element_size;
													readDictionary(*ptr);
												}
												else
												{
													ptr->m_type = string;
													readHexString(*ptr);
												}
											}
											else
												ptr->m_type = null;
											return;
										}
										case '[':
										{
											++ptr->m_element_size;
											ptr->m_type = array;
											readArray(*ptr);
											return;
										}
										case '(':
										{
											++ptr->m_element_size;
											ptr->m_type = string;
											readLiteralString(*ptr);
											return;
										}
										case '+':
										case '-':
										case '.':
										{
											++ptr->m_element_size;
											if (ptr->m_buffer[0] == '.')
												ptr->m_type = float_numeric;
											else
												ptr->m_type = int_numeric;
											readNumeric(*ptr);
											return;
										}
										case '%':
										{
											//this is some comment we are not interested in.
											++ptr->m_buffer;
											--ptr->m_buffer_size;
											while (ptr->m_buffer_size > 0)
											{
												if (ptr->m_buffer[0] != 13 && ptr->m_buffer[0] != 10)
												{
													++ptr->m_buffer;
													--ptr->m_buffer_size;
												}
												else
													break;
											}
											break;
										}
										case '0':
										case '1':
										case '2':
										case '3':
										case '4':
										case '5':
										case '6':
										case '7':
										case '8':
										case '9':	//numeric or reference
										{
											size_t s = 0;
											size_t spaces = 0;
											//check if this object is numeric or reference
											while (s < ptr->m_buffer_size)
											{
												char ch = ptr->m_buffer[s++];
												switch (ch)
												{
													case ' ':
													{
														++spaces;
														if (spaces > 2)
														{
															ptr->m_type = int_numeric;
															readNumeric(*ptr);
															return;
														}
														break;
													}
													case 'R':
													{
														if (spaces == 2)
														{
															ptr->m_type = reference_call;
															readReferenceCall(*ptr);
															return;
														}
														ptr->m_type = int_numeric;
														readNumeric(*ptr);
														return;
													}
													case '0':
													case '1':
													case '2':
													case '3':
													case '4':
													case '5':
													case '6':
													case '7':
													case '8':
													case '9':
													{
														break;
													}
													default:
													{
														ptr->m_type = int_numeric;
														readNumeric(*ptr);
														return;
													}
												}
											}
											ptr->m_type = int_numeric;
											readNumeric(*ptr);
											return;
										}
										case 0:
										case 9:
										case 10:
										case 12:
										case 13:
										case 32:
										{
											break;
										}
										case 'f':
										{
											if (ptr->m_buffer_size >= 5 && memcmp(ptr->m_buffer, "false", 5) == 0)
											{
												ptr->m_type = boolean;
												ptr->m_element_size = 5;
												return;
											}
											ptr->m_type = operator_obj;
											readOperator(*ptr);
											return;
										}
										case 't':
										{
											if (ptr->m_buffer_size >= 4 && memcmp(ptr->m_buffer, "true", 4) == 0)
											{
												ptr->m_type = boolean;
												ptr->m_element_size = 4;
												return;
											}
											ptr->m_type = operator_obj;
											readOperator(*ptr);
											return;
										}
										case 'n':
										{
											if (ptr->m_buffer_size >= 4 && memcmp(ptr->m_buffer, "null", 4) == 0)
											{
												ptr->m_type = null;
												ptr->m_element_size = 4;
												return;
											}
											ptr->m_type = operator_obj;
											readOperator(*ptr);
											return;
										}
										default:
										{
											++ptr->m_element_size;
											ptr->m_type = operator_obj;
											readOperator(*ptr);
											return;
										}
									}
									++ptr->m_buffer;
									--ptr->m_buffer_size;
								}
								ptr->m_type = null;
							}

							bool canUp()
							{
								return m_depth > 0;
							}

							void levelUp()
							{
								if (!canUp())
									throw Exception("PDF Stream iterator: cant go up. Already in root element");
								--m_depth;
								m_pointers_stack.pop_back();
							}

							bool hasNext()
							{
								Pointer* ptr = &m_pointers_stack[m_depth];
								return ptr->m_buffer_size - ptr->m_element_size > 0;
							}

							const char* getData()
							{
								return m_pointers_stack[m_depth].m_buffer;
							}

							size_t getDataLength()
							{
								return m_pointers_stack[m_depth].m_element_size;
							}

							PDFObjectTypes getType()
							{
								return m_pointers_stack[m_depth].m_type;
							}

							std::string toPlainText()
							{
								Pointer* ptr = &m_pointers_stack[m_depth];
								return std::string(ptr->m_buffer, ptr->m_element_size);
							}

							void toHexString(std::string& val)
							{
								val.clear();
								Pointer* ptr = &m_pointers_stack[m_depth];
								if (ptr->m_type != string)
									throw Exception("PDF Stream iterator: Cannot convert to hex string. Type of object: " + ptr->m_type);
								if (ptr->m_element_size == 0)
								{
									val = "00";
									return;
								}
								//already hex
								if (ptr->m_buffer[0] == '<')
								{
									for (int i = 1; i < ptr->m_element_size - 1; ++i)	//skip < and >
									{
										if (hex_char_is_valid(ptr->m_buffer[i]))
											val += ptr->m_buffer[i];
									}
									if (val.length() % 2 == 1)
										val += '0';
								}
								//convert from literal to hex
								else
								{
									for (int i = 1; i < ptr->m_element_size - 1; ++i)	//skip ( and )
									{
										if (ptr->m_buffer[i] == '\\')
										{
											++i;
											if (i < ptr->m_element_size - 1)
											{
												switch (ptr->m_buffer[i])
												{
													case 'n':
													case 10:
													{
														val += "0A";
														break;
													}
													case 't':
													case 9:
													{
														val += "09";
														break;
													}
													case 'f':
													case 12:
													{
														val += "0C";
														break;
													}
													case 8:
													case 'b':
													{
														val += "08";
														break;
													}
													case 'r':
													case 13:
													{
														val += "0D";
														break;
													}
													case '\\':
													{
														val += "5C";
														break;
													}
													case '(':
													{
														val += "28";
														break;
													}
													case ')':
													{
														val += "29";
														break;
													}
													case '0':
													case '1':
													case '2':
													case '3':
													case '4':
													case '5':
													case '6':
													case '7':
													case '8':
													case '9':
													{
														if (i < ptr->m_element_size - 3)
														{
															char octal[3];
															octal[0] = ptr->m_buffer[i];
															octal[1] = ptr->m_buffer[i + 1];
															octal[2] = ptr->m_buffer[i + 2];
															i += 2;
															char res = ((octal[0] - '0') << 6);
															res = res | ((octal[1] - '0') << 3);
															res = res | (octal[2] - '0');
															val += char_to_hex_char(res);
														}
														break;
													}
												}
											}
										}
										else
											val += char_to_hex_char(ptr->m_buffer[i]);
									}
								}
							}

							double toDouble()
							{
								Pointer* ptr = &m_pointers_stack[m_depth];
								if (ptr->m_type != int_numeric && ptr->m_type != float_numeric)
									throw Exception("PDF Stream iterator: Error while converting to double. Current type is: " + ptr->m_type);
								return strtod(ptr->m_buffer, NULL);
							}

							long toLong()
							{
								Pointer* ptr = &m_pointers_stack[m_depth];
								if (ptr->m_type != int_numeric)
									throw Exception("PDF Stream iterator: Error while converting to long. Current type is: " + ptr->m_type);
								return strtol(ptr->m_buffer, NULL, 10);
							}

						private:
							void readOperator(Pointer& ptr)
							{
								while (ptr.m_element_size < ptr.m_buffer_size)
								{
									switch (ptr.m_buffer[ptr.m_element_size])
									{
										case 0:
										case 9:
										case 10:
										case 13:
										case 32:
										case '[':
										case '{':
										case '<':
										case '(':
										case '%':
										{
											return;
										}
									}
									++ptr.m_element_size;
								}
							}

							void readName(Pointer& ptr)
							{
								while (ptr.m_element_size < ptr.m_buffer_size)
								{
									switch (ptr.m_buffer[ptr.m_element_size])
									{
										case 0:
										case 9:
										case 10:
										case 12:
										case 13:
										case 32:
										case '(':
										case ')':
										case '<':
										case '>':
										case '[':
										case ']':
										case '/':
										case '%':
										case '{':
										case '}':
										{
											return;
										}
									}
									++ptr.m_element_size;
								}
							}

							void readDictionary(Pointer& ptr)
							{
								char ch = 0, prev;
								int count = 0;
								int parentheses = 0;
								bool inside_comment = false;
								while (ptr.m_element_size < ptr.m_buffer_size)
								{
									prev = ch;
									ch = ptr.m_buffer[ptr.m_element_size++];
									switch (ch)
									{
										case '<':
										{
											if (parentheses == 0 && !inside_comment)
												++count;
											break;
										}
										case '>':
										{
											if (parentheses == 0 && !inside_comment)
											{
												--count;
												if (count == -2)	//we have reached closing '>>'
													return;
											}
											break;
										}
										case '%':
										{
											if (parentheses == 0)
												inside_comment = true;
											break;
										}
										case 10:
										case 13:
										{
											inside_comment = false;
											break;
										}
										case '(':
										{
											if (!inside_comment && (parentheses == 0 || prev != '\\'))
												++parentheses;
											break;
										}
										case ')':
										{
											if (!inside_comment && prev != '\\' && parentheses > 0)
												--parentheses;
											break;
										}
										case '\\':
										{
											if (prev == '\\')
											{
												prev = 0;
												ch = 0;
											}
											break;
										}
									}
								}
								throw Exception("PDF Stream iterator: invalid dictionary, could not found '>>'");
							}

							void readHexString(Pointer& ptr)
							{
								char ch;
								while (ptr.m_element_size < ptr.m_buffer_size)
								{
									ch = ptr.m_buffer[ptr.m_element_size++];
									if (ch == '>')
										return;
									if (ch >= 'a' && ch <= 'f')
									{
										ptr.m_buffer[ptr.m_element_size - 1] -= ('a' - 'A');
										ch -= ('a' - 'A');
									}
								}
								throw Exception("PDF Stream iterator: invalid hex string, could not found '>' character");
							}

							//literral 内容
							void readLiteralString(Pointer& ptr)
							{
								char ch = 0, prev;
								int count = 0;
								while (ptr.m_element_size < ptr.m_buffer_size)
								{
									prev = ch;
									ch = ptr.m_buffer[ptr.m_element_size++];
									switch (ch)
									{
										case '(':
										{
											if (prev != '\\')
											{
												++count;
											}
											break;
										}
										case ')':
										{
											if (prev != '\\')
											{
												--count;
											}
											if (count == -1)	//we have reached closing ')'
											{
												return;
											}
											break;
										}
										case '\\':
										{
											if (prev == '\\')
											{
												prev = 0;
												ch = 0;
											}
											break;
										}
									}
								}
								throw Exception("PDF Stream iterator: invalid literal string, could not found ')' character");
							}

							void readNumeric(Pointer& ptr)
							{
								while (ptr.m_element_size < ptr.m_buffer_size)
								{
									switch (ptr.m_buffer[ptr.m_element_size])
									{
										case '.':
										{
											ptr.m_type = float_numeric;
											break;
										}
										case '0':
										case '1':
										case '2':
										case '3':
										case '4':
										case '5':
										case '6':
										case '7':
										case '8':
										case '9':
										{
											break;
										}
										default:
										{
											return;
										}
									}
									++ptr.m_element_size;
								}
							}

							void readReferenceCall(Pointer& ptr)
							{
								while (ptr.m_element_size < ptr.m_buffer_size)
								{
									if (ptr.m_buffer[ptr.m_element_size++] == 'R')
										return;
								}
								throw Exception("PDF Stream iterator: invalid referece, could not found 'R' character");
							}

							void readArray(Pointer& ptr)
							{
								char ch = 0, prev;
								int count = 0;
								int parentheses = 0;
								bool inside_comment = false;
								while (ptr.m_element_size < ptr.m_buffer_size)
								{
									prev = ch;
									ch = ptr.m_buffer[ptr.m_element_size++];
									switch (ch)
									{
										case '[':
										{
											if (parentheses == 0 && !inside_comment)
												++count;
											break;
										}
										case ']':
										{
											if (parentheses == 0 && !inside_comment)
											{
												--count;
												if (count == -1)	//we have reached closing ']'
													return;
											}
											break;
										}
										case '%':
										{
											if (parentheses == 0)
												inside_comment = true;
											break;
										}
										case 10:
										case 13:
										{
											inside_comment = false;
											break;
										}
										case '(':
										{
											if (!inside_comment && prev != '\\')
												++parentheses;
											break;
										}
										case ')':
										{
											if (!inside_comment && prev != '\\' && parentheses > 0)
												--parentheses;
											break;
										}
										case '\\':
										{
											if (prev == '\\')
											{
												prev = 0;
												ch = 0;
											}
											break;
										}
									}
								}
								throw Exception("PDF Stream iterator: invalid array");
							}
					};

					PDFDictionary* m_dictionary;
					char* m_stream_data_buffer;			//buffer for decoded stream
					size_t m_stream_data_buffer_len;
					size_t m_position;
					size_t m_size;
					bool m_is_in_external_file;									//are the data for this stream outside of this file?
					bool m_is_obj_stream;										//is this stream a collection of compressed object?
					bool m_is_decoded;											//is this stream decoded?
					bool m_loaded_compressed_objects;							//if this stream has compressed objects, are they loaded?

				private:
					std::vector<CompressedObjectInfo> m_compressed_objects;
					PDFStreamIterator m_stream_iterator;
					PDFReader* m_reader;

				public:
					PDFStream(PDFReader& reader, PDFDictionary& dictionary)
					{
						m_is_in_external_file = false;
						m_loaded_compressed_objects = false;
						m_is_decoded = true;
						m_is_obj_stream = false;
						m_stream_data_buffer = NULL;
						m_stream_data_buffer_len = 0;
						m_dictionary = &dictionary;
						m_position = 0;
						m_size = 0;
						m_reader = &reader;
					}

					~PDFStream()
					{
						if (m_stream_data_buffer)
							delete[] m_stream_data_buffer;
						if (m_dictionary)
							delete m_dictionary;
					}

					PDFObjectTypes getType()
					{
						return stream;
					}

					PDFStream* getStream()
					{
						return this;
					}

					PDFObject* getCompressedObject(size_t index)
					{
						try
						{
							load();
							if (!m_is_obj_stream)
								throw Exception("Stream does not contain any compressed objects");
							if (!m_loaded_compressed_objects)
							{
								PDFNumericInteger* num_obj_count = m_dictionary->getObjAsNumericInteger("N");
								if (!num_obj_count)
									throw Exception("Object stream must contain \"N\" entry in its dictionary");
								PDFNumericInteger* offset_for_first_obj = m_dictionary->getObjAsNumericInteger("First");
								if (!offset_for_first_obj)
									throw Exception("Object stream must contain \"First\" entry in its dictionary");
								size_t first_offset = (*offset_for_first_obj)();
								size_t compressed_objects_count = (*num_obj_count)();
								m_stream_iterator.backToRoot();
								m_stream_iterator.levelDown();
								for (size_t i = 0; i < compressed_objects_count; ++i)
								{
									CompressedObjectInfo obj_info;
									m_stream_iterator.getNextElement();
									obj_info.m_index = m_stream_iterator.toLong();
									m_stream_iterator.getNextElement();
									obj_info.m_offset = m_stream_iterator.toLong() + first_offset;
									m_compressed_objects.push_back(obj_info);
								}
								m_loaded_compressed_objects = true;
							}
							m_stream_iterator.backToRoot();
							m_stream_iterator.levelDown();
							if (index >= m_compressed_objects.size())
								throw Exception("Specified compressed object does not exist in the stream");
							try
							{
								m_stream_iterator.seek(m_compressed_objects[index].m_offset);
							}
							catch (Exception& ex)
							{
								ex.appendError("Cant seek to the compressed object, offset: " + uint_to_string(m_compressed_objects[index].m_offset));
								throw;
							}
							return createNewObjectFromStream();
						}
						catch (Exception& ex)
						{
							ex.appendError("Error while reading compressed object number " + uint_to_string(index) + " in the stream");
							throw;
						}
					}

					PDFStreamIterator& getIterator()
					{
						load();
						return m_stream_iterator;
					}

				private:
					static void ascii_85_decode(std::vector<unsigned char>& src, std::vector<unsigned char>& dest)
					{
						const unsigned long powers_85[5] = { 85 * 85 * 85 * 85, 85 * 85 * 85, 85 * 85, 85, 1 };
						int count = 0;
						unsigned int tuple = 0;
						size_t index_read = 0;
						size_t len = src.size();
						dest.clear();

						try
						{
							while (index_read < len)
							{
								unsigned char ch = src[index_read++];
								switch (ch)
								{
									case 'z':
										if (count != 0)
											throw Exception("Count parameter is not equal to zero");
										dest.push_back(0);
										dest.push_back(0);
										dest.push_back(0);
										dest.push_back(0);
										break;
									case '~':
										if(index_read < len && src[index_read] != '>')
											throw Exception("Invalid end of compressed stream");
										return;
									case '\n':
									case '\r':
									case '\t':
									case ' ':
									case '\0':
									case '\f':
									case '\b':
									case 0177:
										break;
									default:
										if (ch < '!' || ch > 'u')
											throw Exception("Invalid character");
										tuple += (ch - '!') * powers_85[count++];
										if (count == 5)
										{
											dest.push_back(tuple >> 24);
											dest.push_back((tuple & 0x00FF0000) >> 16);
											dest.push_back((tuple & 0x0000FF00) >> 8);
											dest.push_back(tuple & 0x000000FF);
											count = 0;
											tuple = 0;
										}
										break;
								}
							}
							if (count > 0)
							{
								tuple += powers_85[--count];
								int offset = 24;
								unsigned int mask = 0xFF000000;
								while (count > 0)
								{
									dest.push_back((tuple && mask) >> offset);
									offset -= 8;
									mask = mask >> 8;
									--count;
								}
							}
						}
						catch (Exception& ex)
						{
							ex.appendError("Error while decoding a stream using ascii-85 algorithm");
							throw;
						}
					}

					static void lzw_decode(std::vector<unsigned char>& src, std::vector<unsigned char>& dest, PDFDictionary* decode_params)
					{
						const unsigned short masks[4] = { 0x01FF, 0x03FF, 0x07FF, 0x0FFF };
						unsigned int mask = 0;
						unsigned int code_len = 9;
						unsigned char ch = src[0];
						unsigned int buffer_size = 0;
						unsigned int old = 0;
						unsigned int code = 0;
						unsigned int buffer = 0;
						unsigned int read_index = 0;
						Predictior* predictor = NULL;
						try
						{
							if (decode_params)
								predictor = new Predictior(*decode_params);
							size_t len = src.size();
							dest.clear();
							dest.reserve(len);
							lzw_item item;
							std::vector<lzw_item> items_table;
							items_table.reserve(4096);
							lzw_item data;
							for (int i = 0; i < 256; ++i)
							{
								item.clear();
								item.push_back((unsigned char)i);
								items_table.push_back(item);
							}
							item.clear();
							items_table.push_back(item);

							while (read_index < len)
							{
								while (buffer_size <= 16 && read_index < len)
								{
									buffer <<= 8;
									buffer |= (unsigned int)src[read_index];
									buffer_size += 8;
									++read_index;
								}
								while (buffer_size >= code_len)
								{
									code = (buffer >> (buffer_size - code_len)) & masks[mask];
									buffer_size -= code_len;

									if (code == 0x0100)
									{
										mask = 0;
										code_len = 9;
										items_table.clear();
										for (int i = 0; i < 256; ++i)
										{
											item.clear();
											item.push_back((unsigned char)i);
											items_table.push_back(item);
										}
										item.clear();
										items_table.push_back(item);
									}
									else if (code == 0x0101)
									{
										if (predictor)
											delete predictor;
										predictor = NULL;
										return;
									}
									else
									{
										if (code >= items_table.size())
										{
											if (old >= items_table.size())
											{
												if (predictor)
													delete predictor;
												predictor = NULL;
												throw Exception("Index of old and current code are bigger the size of table");
											}
											data = items_table[old];
											data.push_back(ch);
										}
										else
											data = items_table[code];
										if (predictor)
										{
											try
											{
												predictor->decode(&data[0], data.size(), dest);
											}
											catch (Exception& ex)
											{
												delete predictor;
												predictor = NULL;
												ex.appendError("Error while decoding data by predictor");
												throw;
											}
										}
										else
										{
											for (size_t i = 0; i < data.size(); ++i)
												dest.push_back(data[i]);
										}
										ch = data[0];
										if (old < items_table.size())
											data = items_table[old];
										data.push_back(ch);
										items_table.push_back(data);
										data.pop_back();
										old = code;
										switch ((int)items_table.size())
										{
											case 511:
											case 1023:
											case 2047:
												++code_len;
												++mask;
										}
									}
								}
							}
							if (predictor)
								delete predictor;
							predictor = NULL;
						}
						catch (std::bad_alloc& ba)
						{
							if (predictor)
								delete predictor;
							predictor = NULL;
							throw Exception("Bad alloc");
						}
						catch (Exception& ex)
						{
							if (predictor)
								delete predictor;
							predictor = NULL;
							ex.appendError("Error while decoding a stream using lzw algorithm");
							throw;
						}
					}

					static void ascii_hex_decode(std::vector<unsigned char>& src, std::vector<unsigned char>& dest)
					{
						char hex_char[2];
						int got = 0;
						size_t len = src.size();
						size_t read_index = 0;
						dest.clear();
						try
						{
							while (read_index != len)
							{
								char ch = src[read_index++];
								if (ch >= 'a' && ch <= 'f')
									ch -= ('a' - 'A');
								if (ch < '0' || (ch > '9' && ch < 'A') || ch > 'F')
									continue;
								hex_char[got++] = ch;
								if (got == 2)
								{
									got = 0;
									dest.push_back(hex_char_to_single_char(hex_char));
								}
							}
							if (got == 1)
							{
								hex_char[1] = '0';
								dest.push_back(hex_char_to_single_char(hex_char));
							}
						}
						catch (Exception& ex)
						{
							ex.appendError("Error while decoding a stream using ascii-hex algorithm");
							throw;
						}
					}

					static void flat_decode(std::vector<unsigned char>& src, std::vector<unsigned char>& dest, PDFDictionary* decode_params)
					{
						z_stream stream;
						unsigned char buffer[4096];
						Predictior* predictor = NULL;
						try
						{
							dest.clear();
							if (decode_params)
								predictor = new Predictior(*decode_params);
							stream.zalloc = Z_NULL;
							stream.zfree = Z_NULL;
							stream.opaque = Z_NULL;
							if (inflateInit(&stream) != Z_OK)
							{
								if (predictor)
									delete predictor;
								predictor = NULL;
								throw Exception("inflateInit has failed");
							}
							int err;
							int written;
							stream.avail_in = src.size();
							stream.next_in  = &src[0];
							do
							{
								stream.avail_out = 4096;
								stream.next_out  = (Bytef*)buffer;
								switch ((err = inflate(&stream, Z_NO_FLUSH)))
								{
									case Z_NEED_DICT:
									case Z_DATA_ERROR:
									case Z_MEM_ERROR:
									{
										#warning TODO: Should I ignore this error and continue? One of the files I had was corrupted, but most data was readable.
										(void)inflateEnd(&stream);
										if (predictor)
											delete predictor;
										predictor = NULL;
										return;
									}
								}
								written = 4096 - stream.avail_out;
								if (predictor)
								{
									try
									{
										predictor->decode(buffer, written, dest);
									}
									catch (Exception& ex)
									{
										delete predictor;
										predictor = NULL;
										ex.appendError("Error while decoding data by predictor");
										throw;
									}
								}
								else
								{
									for (size_t i = 0; i < written; ++i)
										dest.push_back(buffer[i]);
								}
							}
							while (stream.avail_out == 0);
							if (predictor)
								delete predictor;
							predictor = NULL;
						}
						catch (std::bad_alloc& ba)
						{
							if (predictor)
								delete predictor;
							predictor = NULL;
							throw Exception("Bad alloc");
						}
						catch (Exception& ex)
						{
							if (predictor)
								delete predictor;
							predictor = NULL;
							ex.appendError("Error while decoding a stream using flat algorithm");
							throw;
						}
					}

					static void run_length_decode(std::vector<unsigned char>& src, std::vector<unsigned char>& dest)
					{
						int code_len = 0;
						dest.clear();
						size_t read_index = 0;
						size_t len = src.size();
						try
						{
							while (read_index != len)
							{
								char ch = src[read_index++];
								if (!code_len)
									code_len = ch;
								else if (code_len == 128)
									break;
								else if (code_len <= 127)
								{
									dest.push_back(ch);
									--code_len;
								}
								else if (code_len >= 129)
								{
									code_len = 257 - code_len;
									while (code_len--)
										dest.push_back(ch);
								}
							}
						}
						catch (Exception& ex)
						{
							ex.appendError("Error while decoding a stream using RLE algorithm");
							throw;
						}
					}

					void decode()
					{
						if (m_is_decoded)
							return;
						if (m_is_in_external_file)
							throw Exception("Cannot read stream data: data is inside external file, which is not supported yet");

						std::vector<PDFName*> filters;
						std::vector<PDFDictionary*> filter_options;
						try
						{
							PDFObject* filter_entry = (*m_dictionary)["Filter"];
							PDFObject* decode_params_entry = (*m_dictionary)["DecodeParms"];
							if (decode_params_entry)
							{
								if (decode_params_entry->isArray())
								{
									PDFArray* array_decode_params = decode_params_entry->getArray();
									for (size_t i = 0; i < array_decode_params->Size(); ++i)
										filter_options.push_back(array_decode_params->getObjAsDictionary(i));
								}
								else
									filter_options.push_back(decode_params_entry->getDictionary());
							}
							else
								filter_options.push_back(NULL);

							if (filter_entry->isArray())
							{
								PDFArray* array_filters = filter_entry->getArray();
								for (size_t i = 0; i < array_filters->Size(); ++i)
									filters.push_back(array_filters->getObjAsName(i));
							}
							else
								filters.push_back(filter_entry->getName());
						}
						catch (Exception& ex)
						{
							ex.appendError("Cannot decode stream: Cant load Filter and DecodeParms");
							throw;
						}

						if (filters.size() != filter_options.size())
							throw Exception("Cannot decode stream: number of filters does not match the number of decoding parameters");

						std::vector<unsigned char> stream_first_content(m_size);
						std::vector<unsigned char> stream_second_content;
						stream_second_content.reserve(m_size);
						size_t current_pos = m_reader->tell();
						if (!m_reader->seek(m_position, SEEK_SET))
							throw Exception("Cannot decode stream: cant seek to the position of the stream");
						if (!m_reader->read((char*)&stream_first_content[0], 1, m_size))
							throw Exception("Cannot decode stream: cant read encoded data");
						try
						{
							for (size_t i = 0; i < filters.size(); ++i)
							{
								switch (PDFReader::getCompressionCode((*filters[i])()))
								{
									case ascii_85:
									{
										if (i % 2 == 0)
											ascii_85_decode(stream_first_content, stream_second_content);
										else
											ascii_85_decode(stream_second_content, stream_first_content);
										break;
									}
									case lzw:
									{
										if (i % 2 == 0)
											lzw_decode(stream_first_content, stream_second_content, filter_options[i]);
										else
											lzw_decode(stream_second_content, stream_first_content, filter_options[i]);
										break;
									}
									case ascii_hex:
									{
										if (i % 2 == 0)
											ascii_hex_decode(stream_first_content, stream_second_content);
										else
											ascii_hex_decode(stream_second_content, stream_first_content);
										break;
									}
									case flat:
									{
										if (i % 2 == 0)
											flat_decode(stream_first_content, stream_second_content, filter_options[i]);
										else
											flat_decode(stream_second_content, stream_first_content, filter_options[i]);
										break;
									}
									case rle:
									{
										if (i % 2 == 0)
											run_length_decode(stream_first_content, stream_second_content);
										else
											run_length_decode(stream_second_content, stream_first_content);
										break;
									}
									default:
									{
										throw Exception("Cannot decode stream: Filter " + (*filters[i])() + " is not supported");
									}
								}
							}
							if (filters.size() % 2 == 1)
							{
								m_stream_data_buffer = new char[stream_second_content.size() + 2];
								m_stream_data_buffer[0] = '[';
								m_stream_data_buffer_len = stream_second_content.size() + 2;
								memcpy(m_stream_data_buffer + 1, &stream_second_content[0], stream_second_content.size());
								m_stream_data_buffer[m_stream_data_buffer_len - 1] = ']';
								m_stream_iterator.init(m_stream_data_buffer, m_stream_data_buffer_len);
							}
							else
							{
								m_stream_data_buffer = new char[stream_first_content.size() + 2];
								m_stream_data_buffer[0] = '[';
								m_stream_data_buffer_len = stream_first_content.size() + 2;
								memcpy(m_stream_data_buffer + 1, &stream_first_content[0], stream_first_content.size());
								m_stream_data_buffer[m_stream_data_buffer_len - 1] = ']';
								m_stream_iterator.init(m_stream_data_buffer, m_stream_data_buffer_len);
							}
							if (!m_reader->seek(current_pos, SEEK_SET))
								throw Exception("Cannot decode stream: Cannot go back to the previous location of the file, which is " + uint_to_string(current_pos));
							m_is_decoded = true;
						}
						catch (Exception& ex)
						{
							ex.appendError("Error while decoding stream");
							throw;
						}
					}

					void load()
					{
						try
						{
							if (m_stream_data_buffer)
								return;
							if (m_is_in_external_file)
								throw Exception("Cannot read stream data: data is inside external file, which is not supported yet");
							if (!m_is_decoded)
								decode();
							else
							{
								m_stream_data_buffer = new char[m_size + 2];
								m_stream_data_buffer[0] = '[';					//make array
								m_stream_data_buffer_len = m_size + 2;
								size_t current_pos = m_reader->tell();
								if (!m_reader->seek(m_position, SEEK_SET))
									throw Exception("Cannot load stream data: cant seek to the beggining of the stream, position: " + uint_to_string(m_position));
								if (!m_reader->read(m_stream_data_buffer + 1, 1, m_size))
									throw Exception("Cannot read stream data, size to read: " + uint_to_string(m_size) + " from position " + uint_to_string(m_position));
								m_stream_data_buffer[m_stream_data_buffer_len - 1] = ']';
								if (!m_reader->seek(current_pos, SEEK_SET))
									throw Exception("Cannot go back to the previous position after loading stream data. Previous location: " + uint_to_string(current_pos));
								m_stream_iterator.init(m_stream_data_buffer, m_stream_data_buffer_len);
							}
						}
						catch (Exception& ex)
						{
							ex.appendError("Error while loading stream data at offset " + uint_to_string(m_position) + " and size " + uint_to_string(m_size));
							throw;
						}
					}

					PDFObject* createNewObjectFromStream()
					{
						m_stream_iterator.getNextElement();
						PDFObject* obj = NULL;
						try
						{
							switch (m_stream_iterator.getType())
							{
								case array:
								{
									obj = new PDFArray;
									m_stream_iterator.levelDown();
									while (m_stream_iterator.hasNext())
										((PDFArray*)(obj))->m_objects.push_back(createNewObjectFromStream());
									m_stream_iterator.levelUp();
									return obj;
								}
								case boolean:
								{
									obj = new PDFBoolean;
									if (m_stream_iterator.getData()[0] == 't')
										((PDFBoolean*)(obj))->m_value = true;
									else
										((PDFBoolean*)(obj))->m_value = false;
									return obj;
								}
								case dictionary:
								{
									obj = new PDFDictionary;
									m_stream_iterator.levelDown();
									while (m_stream_iterator.hasNext())
									{
										m_stream_iterator.getNextElement();
										if (m_stream_iterator.getType() != name)
											throw Exception("Error while reading dictionary in stream: key is not a name");
										std::string name = std::string(m_stream_iterator.getData() + 1, m_stream_iterator.getDataLength() - 1);
										((PDFDictionary*)obj)->m_objects[name] = createNewObjectFromStream();
									}
									m_stream_iterator.levelUp();
									return obj;
								}
								case int_numeric:
								{
									obj = new PDFNumericInteger;
									((PDFNumericInteger*)obj)->m_value = strtol(m_stream_iterator.getData(), NULL, 10);
									return obj;
								}
								case float_numeric:
								{
									obj = new PDFNumericFloat;
									((PDFNumericFloat*)obj)->m_value = strtod(m_stream_iterator.getData(), NULL);
									return obj;
								}
								case string:
								{
									obj = new PDFString;
									((PDFString*)obj)->m_value = std::string(m_stream_iterator.getData() + 1, m_stream_iterator.getDataLength() - 2);
									if (m_stream_iterator.getData()[0] == '(')
									{
										((PDFString*)obj)->m_is_hex = false;
									}
									else
									{
										((PDFString*)obj)->m_is_hex = true;
									}
									return obj;
								}
								case name:
								{
									obj = new PDFName;
									((PDFName*)obj)->m_value = std::string(m_stream_iterator.getData() + 1, m_stream_iterator.getDataLength() - 1);
									return obj;
								}
								case null:
								{
									obj = new PDFNull;
									return obj;
								}
								case reference_call:
								{
									obj = new PDFReferenceCall(*m_reader);
									char* ptr_begin = (char*)m_stream_iterator.getData();
									char* ptr_end = ptr_begin;
									((PDFReferenceCall*)obj)->m_index = strtol(ptr_begin, &ptr_end, 10);
									((PDFReferenceCall*)obj)->m_generation = strtol(ptr_end, NULL, 10);
									return obj;
								}
								default:
								{
									throw Exception("Cannot create object from stream: Unsupported type " + uint_to_string(m_stream_iterator.getType()) + ", value: " + m_stream_iterator.toPlainText());
								}
							}
						}
						catch (std::bad_alloc& ba)
						{
							if (obj)
								delete obj;
							obj = NULL;
							throw Exception("Bad alloc");
						}
						catch (Exception& ex)
						{
							if (obj)
								delete obj;
							obj = NULL;
							ex.appendError("Error while creating object from stream");
							throw;
						}
					}
			};

			class PDFNull : public PDFObject
			{
				public:
					PDFObjectTypes getType()
					{
						return null;
					}

					PDFNull* getNull()
					{
						return this;
					}
			};

			class PDFIndirectObject : public PDFObject
			{
				public:
					size_t m_generation;
					size_t m_index;
					PDFObject* m_object;
					PDFReader* m_reader;
					ReferenceInfo* m_owner;

				private:
					void loadObject()
					{
						if (!m_object)
						{
							try
							{
								m_object = m_reader->readIndirectObject(m_index);
							}
							catch (Exception& ex)
							{
								ex.appendError("Indirect object: Error while loading object with index " + uint_to_string(m_index)
											   + " and generation number " + uint_to_string(m_generation));
								throw;
							}
						}
					}

				public:
					PDFIndirectObject(PDFReader& reader, ReferenceInfo* owner)
					{
						m_generation = 0;
						m_index = 0;
						m_object = NULL;
						m_reader = &reader;
						m_owner = owner;
					}

					~PDFIndirectObject()
					{
						if (m_object)
							delete m_object;
					}

					PDFObjectTypes getType()
					{
						if (!m_object)
						{
							loadObject();
						}
						return m_object->getType();
					}

					PDFString* getString()
					{
						if (!m_object)
						{
							loadObject();
						}
						return m_object->getString();
					}

					PDFDictionary* getDictionary()
					{
						if (!m_object)
						{
							loadObject();
						}
						return m_object->getDictionary();
					}

					PDFBoolean* getBoolean()
					{
						if (!m_object)
						{
							loadObject();
						}
						return m_object->getBoolean();
					}

					PDFNumericFloat* getNumericFloat()
					{
						if (!m_object)
						{
							loadObject();
						}
						return m_object->getNumericFloat();
					}

					PDFNumericInteger* getNumericInteger()
					{
						if (!m_object)
						{
							loadObject();
						}
						return m_object->getNumericInteger();
					}

					PDFArray* getArray()
					{
						if (!m_object)
						{
							loadObject();
						}
						return m_object->getArray();
					}

					PDFName* getName()
					{
						if (!m_object)
						{
							loadObject();
						}
						return m_object->getName();
					}

					PDFNull* getNull()
					{
						if (!m_object)
						{
							loadObject();
						}
						return m_object->getNull();
					}

					PDFStream* getStream()
					{
						if (!m_object)
						{
							loadObject();
						}
						return m_object->getStream();
					}

					PDFIndirectObject* getIndirectObject()
					{
						return this;
					}

					bool isIndirectObject()
					{
						return true;
					}
			};

			class PDFReferenceCall : public PDFObject
			{
				//例如:xxx 0 R
				public:
					size_t m_index;       //对象号:xxx
					size_t m_generation;  //产生号:一般为0
					PDFObject* m_object;
					PDFReader* m_reader;

				private:
					void loadObject()
					{
						if (!m_object)
						{
							try
							{
								m_object = m_reader->readIndirectObject(m_index);
							}
							catch (Exception& ex)
							{
								ex.appendError("Call to reference: Error while loading object with index " + uint_to_string(m_index)
											   + " and generation number " + uint_to_string(m_generation));
								throw;
							}
						}
					}

				public:
					PDFReferenceCall(PDFReader& reader)
					{
						m_index = 0;
						m_generation = 0;
						m_object = NULL;
						m_reader = &reader;
					}

					PDFObjectTypes getType()
					{
						if (!m_object)
						{
							loadObject();
						}
						return m_object->getType();
					}

					PDFString* getString()
					{
						if (!m_object)
						{
							loadObject();
						}
						return m_object->getString();
					}

					PDFDictionary* getDictionary()
					{
						if (!m_object)
						{
							loadObject();
						}
						return m_object->getDictionary();
					}

					PDFBoolean* getBoolean()
					{
						if (!m_object)
						{
							loadObject();
						}
						return m_object->getBoolean();
					}

					PDFNumericFloat* getNumericFloat()
					{
						if (!m_object)
						{
							loadObject();
						}
						return m_object->getNumericFloat();
					}

					PDFNumericInteger* getNumericInteger()
					{
						if (!m_object)
						{
							loadObject();
						}
						return m_object->getNumericInteger();
					}

					PDFArray* getArray()
					{
						if (!m_object)
						{
							loadObject();
						}
						return m_object->getArray();
					}

					PDFName* getName()
					{
						if (!m_object)
						{
							loadObject();
						}
						return m_object->getName();
					}

					PDFNull* getNull()
					{
						if (!m_object)
						{
							loadObject();
						}
						return m_object->getNull();
					}

					PDFStream* getStream()
					{
						if (!m_object)
						{
							loadObject();
						}
						return m_object->getStream();
					}

					PDFReferenceCall* getReferenceCall()
					{
						return this;
					}

					bool isReferenceCall()
					{
						return true;
					}
			};

			class ReferenceInfo
			{
				public:
					enum ReferenceType
					{
						free,
						in_use,
						compressed
					};

					ReferenceType m_type;
					size_t m_generation;
					size_t m_offset;
					bool m_read;
					PDFIndirectObject* m_object;

					ReferenceInfo()
					{
						m_type = free;
						m_generation = 0;
						m_offset = 0;
						m_object = NULL;
						m_read = false;
					}

					ReferenceInfo& operator = (const ReferenceInfo& info)
					{
						m_read = info.m_read;
						m_offset = info.m_offset;
						m_generation = info.m_generation;
						m_type = info.m_type;
						m_object = info.m_object;
						if (m_object)
							m_object->m_owner = this;
						return *this;
					}

					ReferenceInfo(const ReferenceInfo& info)
					{
						m_read = info.m_read;
						m_offset = info.m_offset;
						m_generation = info.m_generation;
						m_type = info.m_type;
						m_object = info.m_object;
						if (m_object)
							m_object->m_owner = this;
					}

					~ReferenceInfo()
					{
						if (m_object && m_object->m_owner == this)
							delete m_object;
					}
			};

		private:
			DataStream* m_data_stream;
			std::vector<ReferenceInfo> m_references;
			PDFDictionary m_trailer_dict;
			bool m_got_root;
			bool m_got_info;
			PDFReferenceCall* m_root_ref;
			PDFReferenceCall* m_info_ref;

		public:
			PDFDictionary* m_root_dictionary;
			PDFDictionary* m_info;
			PDFStream* m_metadata;

		public:
			PDFReader(DataStream* data_stream)
				: m_data_stream(data_stream), m_root_dictionary(NULL), m_info(NULL), m_metadata(NULL), m_got_info(false), m_got_root(false)
			{
				m_root_ref = new PDFReferenceCall(*this);
				m_info_ref = new PDFReferenceCall(*this);

				//获取所有的交叉引用表
				readReferenceData();
				if (m_got_info)
				{
					m_info = m_info_ref->getDictionary();
				}

				//得到 Root 信息，从树形结构的根开始解析
				if (m_got_root)
				{
					m_root_dictionary = m_root_ref->getDictionary();
				}

				if (!m_root_dictionary)
				{
					throw Exception("Root dictionary is missing!");
				}

				//获取属性的对象
				m_metadata = m_root_dictionary->getObjAsStream("Metadata");
			}

			~PDFReader()
			{
				delete m_root_ref;
				delete m_info_ref;
			}

			void readLine(std::string& line)
			{
				int ch;
				line.clear();
				while (true)
				{
					ch = m_data_stream->getc();
					switch (ch)
					{
						case 13:
						{
							ch = m_data_stream->getc();
							if (ch != 10)
							{
								m_data_stream->unGetc(ch);
							}
							return;
						}
						case '%':
						case 10:
						{
							return;
						}
						case EOF:
						{
							throw Exception("PDF Reader: Cant read line, current content: " + line);
						}
						default:
						{
							line.push_back(ch);
						}
					}
				}
			}

			void skipComment()
			{
				int ch;
				while (true)
				{
					ch = m_data_stream->getc();
					switch (ch)
					{
						case 13:
						{
							ch = m_data_stream->getc();
							if (ch != 10)
								m_data_stream->unGetc(ch);
							return;
						}
						case 10:
						{
							return;
						}
						case EOF:
						{
							throw Exception("PDF Reader: Error while skipping comment: EOF");
						}
					}
				}
			}

			void skipKeyword(const std::string& keyword)
			{
				size_t found = 0;
				size_t len = keyword.length();
				while (true)
				{
					int ch = m_data_stream->getc();
					if (ch == EOF)
						throw Exception("Unexpected end of buffer during skipping keyword: " + keyword);
					if (keyword[found] == ch)
					{
						++found;
						if (found == len)
							return;
					}
					else
						found = 0;
				}
			}

			//获取 name 对象： 以 / 为标识
			void readName(PDFName& name)
			{
				char ch;

				//确认对象类型
				//go to the name. Character (/) marks beggining of the name
				while (true)
				{
					if (!m_data_stream->read(&ch, 1, 1))
					{
						throw Exception("PDF Reader: Unexpected end of buffer during reading PDF name");
					}
					if (ch == '/')
					{
						break;
					}
				}
				name.m_value.clear();
				while (true)
				{
					ch = m_data_stream->getc();
					if (ch == EOF)
					{
						throw Exception("PDF Reader: Unexpected end of buffer during reading PDF name. Current content: " + name());
					}
					switch (ch)
					{
						case 0:
						case 9:
						case 10:
						case 12:
						case 13:
						case 32:
						case '(':
						case ')':
						case '<':
						case '>':
						case '[':
						case ']':
						case '/':
						case '%':
						case '{':
						case '}':
						{
							//不是 name 对象的合法字符，this character is not part of the name
							m_data_stream->unGetc(ch);
							return;
						}
						case '#':
						{
							//#标识这个字符是16进制表示的字符，例如: /A#42 = AB
							char hex_char[2];
							if (!m_data_stream->read(hex_char, 1, 2))
							{
								throw Exception("PDF Reader: Unexpected end of buffer during reading PDF name. Current content: " + name());
							}
							name.m_value += hex_char_to_single_char(hex_char);
							break;
						}
						default:
						{
							name.m_value += ch;
						}
					}
				}
			}

			//获取 string 对象
			void readString(PDFString& string)
			{
				char ch;
				string.m_is_hex = false;
				string.m_value.clear();

				//确认对象类型
				//search for string content
				while (true)
				{
					if (!m_data_stream->read(&ch, 1, 1))
					{
						throw Exception("PDF Reader: Unexpected end of buffer during reading PDF string");
					}
					if (ch == '(')	//literal
					{
						//字面类型值: ()
						break;
					}
					if (ch == '<')	//hex
					{
						//十六进值类型: <>
						string.m_is_hex = true;
						break;
					}
				}
				if (string.m_is_hex)
				{
					//转换为字面值
					char hex_char[2];
					unsigned int got = 0;
					while (true)
					{
						if (!m_data_stream->read(&ch, 1, 1))
						{
							throw Exception("PDF Reader: Unexpected end of buffer during reading PDF hex string. Current value: " + string.m_value);
						}
						if (ch == '>')
						{
							if (got == 1)
							{
								hex_char[1] = '0';
								string.m_value += hex_char_to_single_char(hex_char);
							}
							return;
						}
						normalize_hex_char(ch);
						if (!hex_char_is_valid(ch))
						{
							continue;
						}
						hex_char[got++] = ch;
						if (got == 2)
						{
							got = 0;
							string.m_value += hex_char_to_single_char(hex_char);
						}
					}
				}
				else
				{
					int parentheses_depth = 0;
					while (true)
					{
						if (!m_data_stream->read(&ch, 1, 1))
						{
							throw Exception("PDF Reader: Unexpected end of buffer during reading PDF literal string. Current value: " + string.m_value);
						}
						switch (ch)
						{
							case '\\':
							{
								if (!m_data_stream->read(&ch, 1, 1))
								{
									throw Exception("PDF Reader: Unexpected end of buffer during reading PDF literal string. Current value: " + string.m_value);
								}
								switch (ch)
								{
									case 10:
									case 'n':
									{
										string.m_value += '\n';
										break;
									}
									case 'r':
									{
										string.m_value += '\r';
										break;
									}
									case 't':
									{
										string.m_value += '\t';
										break;
									}
									case 'b':
									{
										string.m_value += '\b';
										break;
									}
									case 'f':
									{
										string.m_value += '\f';
										break;
									}
									case '(':
									{
										string.m_value += '(';
										break;
									}
									case ')':
									{
										string.m_value += ')';
										break;
									}
									case 13:
									{
										ch = m_data_stream->getc();
										if (ch == EOF)
										{
											throw Exception("PDF Reader: Unexpected end of buffer during reading PDF literal string. Current value: " + string.m_value);
										}
										if (ch != 10)
										{
											m_data_stream->unGetc(ch);
										}
										break;
									}
									case '0':
									case '1':
									case '2':
									case '3':
									case '4':
									case '5':
									case '6':
									case '7':
									case '8':
									case '9':
									{
										//八进制的数据: 例如 A\66 = AB
										char octal[3];
										octal[0] = ch;
										if (!m_data_stream->read(octal + 1, 1, 2))
										{
											throw Exception("PDF Reader: Unexpected end of buffer during reading PDF literal string (octal number). Current value: " + string.m_value);
										}
										char res = ((octal[0] - '0') << 6);
										res = res | ((octal[1] - '0') << 3);
										res = res | (octal[2] - '0');
										string.m_value += res;
										break;
									}
									case '\\':
									{
										string.m_value += '\\';
										break;
									}
								}
								break;
							}
							case 10:
							{
								//换行,在linux 下面出现字符 10 是一次回车换行
								string.m_value += '\n';
								break;
							}
							case 13:
							{
								//回车,在windows 下面出现字符 13 10 连续在一起时才是一次回车换行
								ch = m_data_stream->getc();
								if (ch == EOF)
								{
									throw Exception("PDF Reader: Unexpected end of buffer during reading PDF literal string. Current value: " + string.m_value);
								}
								if (ch != 10)
								{
									m_data_stream->unGetc(ch);
								}
								string.m_value += '\n';
								break;
							}
							case '(':
							{
								//处理嵌套 (())
								++parentheses_depth;
								string.m_value += '(';
								break;
							}
							case ')':
							{
								if (parentheses_depth == 0)
								{
									//到结尾的右括号就结束解析
									return;
								}
								--parentheses_depth;
								string.m_value += ')';
								break;
							}
							default:
							{
								string.m_value += ch;
							}
						}
					}
				}
			}

			//获取 boolean 对象: true 和 false
			void readBoolean(PDFBoolean& boolean)
			{
				char buffer[4];
				char ch = m_data_stream->getc();
				if (ch == EOF)
				{
					throw Exception("PDF Reader: Unexpected end of buffer during reading a boolean");
				}
				if (ch == 't')
				{
					boolean.m_value = true;
					//read rest of the string (true)
					if (!m_data_stream->read(buffer, 1, 3))
					{
						throw Exception("PDF Reader: Unexpected end of buffer during reading a boolean");
					}
				}
				else	//false
				{
					boolean.m_value = false;
					//read rest of the string (false)
					if (!m_data_stream->read(buffer, 1, 4))
					{
						throw Exception("PDF Reader: Unexpected end of buffer during reading a boolean");
					}
				}
			}

			//获取 array 对象，可以是任何对象，包含 array 对象
			void readArray(PDFArray& array)
			{
				char ch;

				//确认对象类型
				while (true)
				{
					if (!m_data_stream->read(&ch, 1, 1))
					{
						throw Exception("PDF Reader: Unexpected end of buffer during reading an array");
					}
					if (ch == '[')
					{
						break;
					}
				}
				while (true)
				{
					ch = m_data_stream->getc();
					if (ch == EOF)
					{
						throw Exception("PDF Reader: Unexpected end of buffer during reading an array");
					}
					if (ch == ']')
					{
						return;
					}
					PDFObject* value_object = NULL;
					try
					{
						switch (ch)
						{
							case '/':	//value is a name
							{
								value_object = new PDFName;
								m_data_stream->unGetc(ch);
								readName(*(PDFName*)value_object);
								array.m_objects.push_back(value_object);
								break;
							}
							case '<':	//value is a hexadecimal string or dictionary
							{
								ch = m_data_stream->getc();
								if (ch == EOF)
								{
									throw Exception("PDF Reader: Unexpected end of buffer during reading an array");
								}
								if (ch == '<')	//dictionary
								{
									value_object = new PDFDictionary;
									m_data_stream->unGetc('<');
									m_data_stream->unGetc(ch);
									readDictionary(*(PDFDictionary*)value_object);
									array.m_objects.push_back(value_object);
								}
								else	//hexadecimal string
								{
									value_object = new PDFString;
									m_data_stream->unGetc('<');
									m_data_stream->unGetc(ch);
									readString(*(PDFString*)value_object);
									array.m_objects.push_back(value_object);
								}
								break;
							}
							case '(':	//value is a literal string
							{
								value_object = new PDFString;
								m_data_stream->unGetc(ch);
								readString(*(PDFString*)value_object);
								array.m_objects.push_back(value_object);
								break;
							}
							case '%':
							{
								skipComment();
								break;
							}
							case 'f':
							case 't':	//value is a boolean
							{
								value_object = new PDFBoolean;
								m_data_stream->unGetc(ch);
								readBoolean(*(PDFBoolean*)value_object);
								array.m_objects.push_back(value_object);
								break;
							}
							case '[':	//value is an array
							{
								//嵌套 array 对象
								value_object = new PDFArray;
								m_data_stream->unGetc(ch);
								readArray(*(PDFArray*)value_object);
								array.m_objects.push_back(value_object);
								break;
							}
							case 'n':	//value is a null
							{
								value_object = new PDFNull;
								m_data_stream->unGetc(ch);
								readNull(*(PDFNull*)value_object);
								array.m_objects.push_back(value_object);
								break;
							}
							case '+':
							case '-':
							case '.':	//value is a numeric
							{
								m_data_stream->unGetc(ch);
								value_object = readNumeric();
								array.m_objects.push_back(value_object);
								break;
							}
							case '0':
							case '1':
							case '2':
							case '3':
							case '4':
							case '5':
							case '6':
							case '7':
							case '8':
							case '9':	//value is an indirect reference or number
							{
								int to_seek_backward = 1;
								int spaces = 0;
								bool is_reference = false;
								while (true)
								{
									//number -> just a number
									//indirect reference: two numbers and 'R' character with spaces
									ch = m_data_stream->getc();
									++to_seek_backward;
									if (ch == EOF)
									{
										throw Exception("PDF Reader: Unexpected end of buffer during reading an array");
									}
									if (ch == ' ')
									{
										++spaces;
										if (spaces > 2)	//indirect reference contain only two spaces
										{
											break;
										}
									}
									else if (ch == 'R' && spaces == 2)	//indirect reference
									{
										//indirect reference 间接对象: 例如 1 0 R
										is_reference = true;
										break;
									}
									else if (ch < '0' || ch > '9')	//something beyond a digit (and this is not a space)
									{
										break;
									}
								}
								if (!m_data_stream->seek(-to_seek_backward, SEEK_CUR))
								{
									throw Exception("PDF Reader: Cant rewind cursor during reading an array");
								}
								if (is_reference)
								{
									value_object = new PDFReferenceCall(*this);
									readIndirectReference(*(PDFReferenceCall*)value_object);
								}
								else
								{
									value_object = readNumeric();
								}
								array.m_objects.push_back(value_object);
								break;
							}
						}
					}
					catch (std::bad_alloc& ba)
					{
						if (value_object)
							delete value_object;
						value_object = NULL;
						throw Exception("Bad alloc");
					}
					catch (Exception& ex)
					{
						if (value_object)
							delete value_object;
						value_object = NULL;
						ex.appendError("PDF Reader: Error reading an array");
						throw;
					}
				}
			}

			//获取 numeric 对象： 123
			PDFObject* readNumeric()
			{
				bool negative = false;
				bool is_float = false;
				std::string number_str;
				while (true)
				{
					char ch = m_data_stream->getc();
					switch (ch)
					{
						case EOF:
						{
							throw Exception("PDF Reader: Unexpected end of buffer during reading a numeric value");
						}
						case '-':
						{
							negative = true;
							break;
						}
						case '+':
						{
							break;
						}
						case '.':
						{
							if (number_str.length() == 0)
							{
								number_str += "0.";
							}
							else
							{
								number_str += ".";
							}
							is_float = true;
							break;
						}
						case '0':
						case '1':
						case '2':
						case '3':
						case '4':
						case '5':
						case '6':
						case '7':
						case '8':
						case '9':
						{
							number_str += ch;
							break;
						}
						default:
						{
							m_data_stream->unGetc(ch);
							char* begin = (char*)number_str.c_str();
							char* end = begin;
							PDFObject* object;
							if (is_float)
							{
								double value = strtod(begin, &end);
								if (value == 0.0 && begin == end)
								{
									throw Exception("Error while converting number " + number_str + " to double");
								}
								if (negative)
								{
									value = -value;
								}
								object = new PDFNumericFloat;
								((PDFNumericFloat*)object)->m_value = value;
							}
							else
							{
								long value = strtol(begin, &end, 10);
								if (value == 0 && begin == end)
								{
									throw Exception("Error while converting number " + number_str + " to long");
								}
								if (negative)
								{
									value = -value;
								}
								object = new PDFNumericInteger;
								((PDFNumericInteger*)object)->m_value = value;
							}
							return object;
						}
					}
				}
			}

			//获取 null 对象： null
			void readNull(PDFNull& null)
			{
				char buffor[4];
				if (!m_data_stream->read(buffor, 1, 4) || memcmp(buffor, "null", 4) != 0)
				{
					throw Exception("PDF Reader: Invalid null value");
				}
			}

			void readStream(PDFStream& stream)
			{
				try
				{
					char ch;
					PDFDictionary* stream_dict = stream.m_dictionary;
					PDFNumericInteger* len = stream_dict->getObjAsNumericInteger("Length");
					if (!len)
						throw Exception("PDF Reader: Error while reading a stream: missing \"Length\" entry");
					stream.m_size = (*len)();
					//check if stream is encoded.
					if (stream_dict->getObjAsName("Filter") || stream_dict->getObjAsArray("Filter"))
						stream.m_is_decoded = false;
					if (stream_dict->getObjAsNumericInteger("N"))
						stream.m_is_obj_stream = true;

					skipKeyword("stream");
					//skip EOL
					ch = m_data_stream->getc();
					if (ch == 13)
						ch = m_data_stream->getc();
					if (ch != 10)
						throw Exception("PDF Reader: Error while reading a stream: invalid beggining of the stream");
					stream.m_position = m_data_stream->tell();
					//Stream data can be included in external file.
					if ((*stream_dict)["F"])
					{
						#warning TODO: Add support for reading from external files
						stream.m_is_in_external_file = true;
					}
					else
					{
						if (!m_data_stream->seek(stream.m_size, SEEK_CUR))
							throw Exception("PDF Reader: Error while reading a stream: cant skip stream data");
					}
					skipKeyword("endstream");
				}
				catch (Exception& ex)
				{
					ex.appendError("PDF Reader: Error while reading stream");
					throw;
				}
			}

			//获取间接对象: 例如 3 0 R
			void readIndirectReference(PDFReferenceCall& reference)
			{
				char ch;
				reference.m_generation = 0;
				reference.m_index = 0;
				std::string text;
				text.reserve(25);
				int stage = 0;	//0 = reading reference index, 1 = reading reference generation, 2 = reading 'R' character
				while (true)
				{
					if (!m_data_stream->read(&ch, 1, 1))
						throw Exception("PDF Reader: Unexpected end of buffer during reading a call to reference");
					switch (ch)
					{
						case '0':
						case '1':
						case '2':
						case '3':
						case '4':
						case '5':
						case '6':
						case '7':
						case '8':
						case '9':
						{
							if (stage < 2)
								text += ch;
							break;
						}
						case 'R':
						{
							if (stage == 2)
								return;
							break;
						}
						default:
						{
							char* begin = (char*)text.c_str();
							char* end = begin;
							if (stage == 0 && text.length() > 0)
							{
								reference.m_index = strtol(begin, &end, 10);
								if (reference.m_index == 0 || end == begin)
									throw Exception("PDF Reader: Error while reading \"" + text + "\" as indirect reference.");
								text.clear();
								++stage;
							}
							else if (stage == 1 && text.length() > 0)
							{
								reference.m_generation = strtol(begin, &end, 10);
								if (reference.m_index == 0 || end == begin)
									throw Exception("PDF Reader: Error while reading \"" + text + "\" as indirect reference.");
								text.clear();
								++stage;
							}
						}
					}
				}
			}

			//读取字典信息： name 对象和任意对象组成
			void readDictionary(PDFDictionary& dictionary)
			{
				int ch = 0, prev_ch = 0;
				bool reading_key = false;
				bool reading_value = false;
				PDFName key_name;

				//找到字典的标识: << 跳过，search for dictionary
				while (true)
				{
					prev_ch = ch;
					ch = m_data_stream->getc();
					if (ch == EOF)
					{
						throw Exception("PDF Reader: Unexpected end of buffer during reading a dictionary");
					}
					if (prev_ch == '<' && ch == '<')
					{
						reading_key = true;
						break;
					}
				}

				//解析字典信息
				while (true)
				{
					prev_ch = ch;
					ch = m_data_stream->getc();
					if (ch == EOF)
					{
						throw Exception("PDF Reader: Unexpected end of buffer during reading a dictionary");
					}
					if (ch == '>' && prev_ch == '>')
					{
						//字典的结尾标识
						return;
					}

					if (ch == '%')
					{
						//跳过注释信息
						skipComment();
						break;
					}
					else if (reading_key && ch == '/')
					{
						//获取name 对象: key_name 可以标识字典中值作用的关键字
						m_data_stream->unGetc(ch);
						key_name.m_value.clear();
						try
						{
							readName(key_name);
						}
						catch (Exception& ex)
						{
							ex.appendError("Error while reading dictionary name");
							throw;
						}
						reading_value = true;
						reading_key = false;
					}
					else if (reading_value)
					{
						//获取值，可以是任意类型的 pdf 对象
						PDFObject* value_object = NULL;
						try
						{
							switch (ch)
							{
								case '/':	//value is a name
								{
									//值是 name 对象，

									value_object = new PDFName;
									m_data_stream->unGetc(ch);
									readName(*(PDFName*)value_object);
									reading_value = false;
									reading_key = true;
									dictionary.m_objects[key_name.m_value] = value_object;
									break;
								}
								case '<':	//value is a hexadecimal string or dictionary
								{
									//值是十六进制的 string 或者 dictionary 对象

									ch = m_data_stream->getc();
									if (ch == EOF)
									{
										throw Exception("PDF Reader: Unexpected end of buffer during reading a dictionary");
									}
									if (ch == '<')	//dictionary
									{
										//可以确定是 dictionary 对象: <<>>

										value_object = new PDFDictionary;
										m_data_stream->unGetc('<');
										m_data_stream->unGetc(ch);

										//嵌套的 dictionary 对象, 递归调用
										readDictionary(*(PDFDictionary*)value_object);
										reading_value = false;
										reading_key = true;
										dictionary.m_objects[key_name.m_value] = value_object;
									}
									else	//hexadecimal string
									{
										//可以确定是十六进制的 string  对象: <>

										value_object = new PDFString;
										m_data_stream->unGetc('<');
										m_data_stream->unGetc(ch);
										readString(*(PDFString*)value_object);
										reading_value = false;
										reading_key = true;
										dictionary.m_objects[key_name.m_value] = value_object;
									}
									break;
								}
								case '(':	//value is a literal string
								{
									//字面意义的 string 对象

									value_object = new PDFString;
									m_data_stream->unGetc(ch);
									readString(*(PDFString*)value_object);
									reading_value = false;
									reading_key = true;
									dictionary.m_objects[key_name.m_value] = value_object;
									break;
								}
								case 'f':
								case 't':	//value is a boolean
								{
									value_object = new PDFBoolean;
									m_data_stream->unGetc(ch);
									readBoolean(*(PDFBoolean*)value_object);
									reading_value = false;
									reading_key = true;
									dictionary.m_objects[key_name.m_value] = value_object;
									break;
								}
								case '[':	//value is an array
								{
									value_object = new PDFArray;
									m_data_stream->unGetc(ch);
									readArray(*(PDFArray*)value_object);
									reading_value = false;
									reading_key = true;
									dictionary.m_objects[key_name.m_value] = value_object;
									break;
								}
								case 'n':	//value is a null
								{
									value_object = new PDFNull;
									m_data_stream->unGetc(ch);
									readNull(*(PDFNull*)value_object);
									reading_value = false;
									reading_key = true;
									dictionary.m_objects[key_name.m_value] = value_object;
									break;
								}
								case '+':
								case '-':
								case '.':	//value is a numeric
								{
									m_data_stream->unGetc(ch);
									value_object = readNumeric();
									reading_value = false;
									reading_key = true;
									dictionary.m_objects[key_name.m_value] = value_object;
									break;
								}
								case '0':
								case '1':
								case '2':
								case '3':
								case '4':
								case '5':
								case '6':
								case '7':
								case '8':
								case '9':	//value is an indirect reference or number
								{
									int seek_backward = 1;
									int spaces = 0;
									bool is_reference = false;
									while (true)
									{
										//number -> just a number
										//indirect reference: two integers and 'R' character with spaces
										ch = m_data_stream->getc();
										++seek_backward;
										if (ch == EOF)
										{
											throw Exception("PDF Reader: Unexpected end of buffer during reading a dictionary");
										}
										if (ch == ' ')
										{
											++spaces;
											if (spaces > 2)	//indirect reference contain only two spaces
											{
												break;
											}
										}
										else if (ch == 'R' && spaces == 2)	//indirect reference
										{
											//indirect reference 间接对象: 例如 1 0 R
											is_reference = true;
											break;
										}
										else if (ch < '0' || ch > '9')	//something beyond a digit (and this is not space)
										{
											break;
										}
									}
									if (!m_data_stream->seek(-seek_backward, SEEK_CUR))
									{
										throw Exception("PDF Reader: Cant rewind cursor after reading a dictionary");
									}
									if (is_reference)
									{
										value_object = new PDFReferenceCall(*this);
										readIndirectReference(*(PDFReferenceCall*)value_object);
									}
									else
									{
										value_object = readNumeric();
									}
									reading_value = false;
									reading_key = true;
									dictionary.m_objects[key_name.m_value] = value_object;
									break;
								}
							}
						}
						catch (std::bad_alloc& ba)
						{
							if (value_object)
								delete value_object;
							value_object = NULL;
							throw Exception("Bad alloc");
						}
						catch (Exception& ex)
						{
							if (value_object)
								delete value_object;
							value_object = NULL;
							ex.appendError("PDF Reader: Cant read dictionary, error while reading entry: " + key_name());
							throw;
						}
					}
				}
			}

			//间接对象: xx 0 R
			PDFObject* readIndirectObject(size_t index)
			{
				if (index >= m_references.size())
				{
					throw Exception("PDF Reader: Cannot read indirect object. Size of the table: " + uint_to_string(m_references.size())
									+ ", index: " + uint_to_string(index));
				}
				ReferenceInfo* reference_info = &m_references[index];
				try
				{
					if (reference_info->m_object)
					{
						if (reference_info->m_object->m_object)
						{
							return reference_info->m_object->m_object;
						}
					}
					else
					{
						reference_info->m_object = new PDFIndirectObject(*this, reference_info);
						reference_info->m_object->m_generation = reference_info->m_generation;
						reference_info->m_object->m_index = index;
					}

					//分三类交叉引用表信息，free(不使用), compressed(压缩类型), in_use(使用中)
					switch (reference_info->m_type)
					{
						case ReferenceInfo::free:
						{
							//不使用的直接废弃
							reference_info->m_object->m_object = new PDFNull;
							return reference_info->m_object->m_object;
						}
						case ReferenceInfo::compressed:	//in use, but compressed
						{

							//object is compressed in another stream, m_offset is an index here.
							//对象是在另外一个压缩流中，m_offset 是压缩流的对象号索引值
							if (reference_info->m_offset >= m_references.size())
							{
								throw Exception("PDF Reader: Cannot read compressed object. Size of the table: "
												+ uint_to_string(m_references.size()) + ", index: "
												+ uint_to_string(reference_info->m_offset));
							}

							ReferenceInfo* object_stream_reference = &m_references[reference_info->m_offset];
							if (!object_stream_reference->m_object)
							{
								object_stream_reference->m_object = new PDFIndirectObject(*this, object_stream_reference);
								object_stream_reference->m_object->m_index = reference_info->m_offset;
								object_stream_reference->m_object->m_generation = object_stream_reference->m_generation;
							}
							PDFStream* object_stream = NULL;
							try
							{
								if (!object_stream_reference->m_object->m_object)
								{
									object_stream_reference->m_object->m_object = readIndirectObject(object_stream_reference->m_object->m_index);
								}
								object_stream = object_stream_reference->m_object->getStream();
								if (!object_stream)
								{
									throw Exception("Object stream does not exist");
								}
							}
							catch (Exception& ex)
							{
								ex.appendError("PDF Reader: Cannot read stream with compressed objects");
								throw;
							}

							//generation is an index in compressed object.
							try
							{
								reference_info->m_object->m_object = object_stream->getCompressedObject(reference_info->m_generation);
							}
							catch (Exception& ex)
							{
								ex.appendError("PDF Reader: Cant load object from object stream with index " + uint_to_string(reference_info->m_offset));
								throw;
							}
							return reference_info->m_object->m_object;
						}
						case ReferenceInfo::in_use:
						{
							size_t current_position = m_data_stream->tell();
							if (!seek(reference_info->m_offset, SEEK_SET))
								throw Exception("PDF Reader: Cant seek to the begginig of the indirect object, position: "
												+ reference_info->m_offset);
							try
							{
								skipKeyword("obj");
							}
							catch (Exception& ex)
							{
								ex.appendError("PDF Reader: Cant find \"obj\"a keyword during reading indirect object");
								throw;
							}
							PDFObject* value_object = NULL;
							try
							{
								while (true)
								{
									char ch = m_data_stream->getc();
									if (ch == EOF)
										throw Exception("PDF Reader: Unexpected end of buffer during reading indirect object");
									switch (ch)
									{
										case 'e':	//endobj
										{
											char buffer[5];
											if (!m_data_stream->read(buffer, 1, 5))
												throw Exception("PDF Reader: Unexpected end of buffer during reading indirect object, error while reading endobj keyword");
											if (memcmp(buffer, "ndobj", 5) != 0)
												throw Exception("PDF Reader: Invalid keyword for end of the object");
											if (!reference_info->m_object->m_object)
												reference_info->m_object->m_object = new PDFNull;
											if (!m_data_stream->seek(current_position, SEEK_SET))
												throw Exception("Cant seek to the prevoius positon after reading an object");
											return reference_info->m_object->m_object;
										}
										case 's':	//stream
										{
											value_object = reference_info->m_object->m_object;
											reference_info->m_object->m_object = NULL;
											if (!value_object || !value_object->isDictionary())
												throw Exception("PDF Reader: Error while reading indirect object: Stream cannot exist without dictionary");
											reference_info->m_object->m_object = new PDFStream(*this, *value_object->getDictionary());
											value_object = NULL;
											m_data_stream->unGetc(ch);
											readStream(*((PDFStream*)reference_info->m_object->m_object));
											break;
										}
										case '/':	//name
										{
											if (reference_info->m_object->m_object)
												throw Exception("PDF Reader: Indirect object can contain only one object inside");
											value_object = new PDFName;
											m_data_stream->unGetc(ch);
											readName(*(PDFName*)value_object);
											reference_info->m_object->m_object = value_object;
											value_object = NULL;
											break;
										}
										case '<':	//hexadecimal string or dictionary
										{
											if (reference_info->m_object->m_object)
												throw Exception("PDF Reader: Indirect object can contain only one object inside");
											ch = m_data_stream->getc();
											if (ch == EOF)
												throw Exception("PDF Reader: Unexpected end of buffer during reading indirect object");
											if (ch == '<')	//dictionary
											{
												value_object = new PDFDictionary;
												m_data_stream->unGetc('<');
												m_data_stream->unGetc(ch);
												readDictionary(*(PDFDictionary*)value_object);
												reference_info->m_object->m_object = value_object;
												value_object = NULL;
											}
											else	//hexadecimal string
											{
												value_object = new PDFString;
												m_data_stream->unGetc('<');
												m_data_stream->unGetc(ch);
												readString(*(PDFString*)value_object);
												reference_info->m_object->m_object = value_object;
												value_object = NULL;
											}
											break;
										}
										case '(':	//value is a literal string
										{
											if (reference_info->m_object->m_object)
												throw Exception("PDF Reader: Indirect object can contain only one object inside");
											value_object = new PDFString;
											m_data_stream->unGetc(ch);
											readString(*(PDFString*)value_object);
											reference_info->m_object->m_object = value_object;
											value_object = NULL;
											break;
										}
										case 'f':
										case 't':	//value is a boolean
										{
											if (reference_info->m_object->m_object)
												throw Exception("PDF Reader: Indirect object can contain only one object inside");
											value_object = new PDFBoolean;
											m_data_stream->unGetc(ch);
											readBoolean(*(PDFBoolean*)value_object);
											reference_info->m_object->m_object = value_object;
											value_object = NULL;
											break;
										}
										case '[':	//value is an array
										{
											if (reference_info->m_object->m_object)
												throw Exception("PDF Reader: Indirect object can contain only one object inside");
											value_object = new PDFArray;
											m_data_stream->unGetc(ch);
											readArray(*(PDFArray*)value_object);
											reference_info->m_object->m_object = value_object;
											value_object = NULL;
											break;
										}
										case 'n':	//value is a null
										{
											if (reference_info->m_object->m_object)
												throw Exception("PDF Reader: Indirect object can contain only one object inside");
											value_object = new PDFNull;
											m_data_stream->unGetc(ch);
											readNull(*(PDFNull*)value_object);
											reference_info->m_object->m_object = value_object;
											value_object = NULL;
											break;
										}
										case '%':
										{
											skipComment();
											break;
										}
										case '+':
										case '-':
										case '.':
										case '0':
										case '1':
										case '2':
										case '3':
										case '4':
										case '5':
										case '6':
										case '7':
										case '8':
										case '9':	//value is a numeric
										{
											if (reference_info->m_object->m_object)
												throw Exception("PDF Reader: Indirect object can contain only one object inside");
											m_data_stream->unGetc(ch);
											value_object = readNumeric();
											reference_info->m_object->m_object = value_object;
											value_object = NULL;
											break;
										}
									}
								}
							}
							catch (std::bad_alloc& ba)
							{
								if (value_object)
									delete value_object;
								value_object = NULL;
								throw Exception("Bad alloc");
							}
							catch (Exception& ex)
							{
								if (value_object)
									delete value_object;
								value_object = NULL;
								ex.appendError("PDF Reader: Cant read indirect object.");
								throw;
							}
						}
						default:
						{
							throw Exception("PDF Reader: invalid reference type: " + reference_info->m_type);
						}
					}
				}
				catch (Exception& ex)
				{
					ex.appendError("PDF Reader: Error while reading indirect object");
					throw;
				}
			}

			inline bool seek(int offset, int whence)
			{
				 return m_data_stream->seek(offset, whence);
			}

			inline bool read(char* buffer, size_t elements_len, size_t elements_count)
			{
				return m_data_stream->read(buffer, elements_len, elements_count);
			}

			inline size_t tell()
			{
				return m_data_stream->tell();
			}

		private:
			//获取所有的交叉引用表
			void readReferenceData()
			{
				try
				{
					/*
					 * 解析 startxref 的值，找到交叉引用表的偏移量
					 * 这个地方是取文件尾开始的后25个字节，这个有写pdf文件尾部会有一些其他的数据
					 * 会造成解析失败，应该改为从文件尾开始查找关键字 startxref 开始
					 */
					char start_xref_buffer[25];
					size_t xref_data_position;
					if (!m_data_stream->seek(-25, SEEK_END))
					{
						throw Exception("PDF Reader: Cant read xref data: Cant seek to the position of startxref");
					}
					if (!m_data_stream->read(start_xref_buffer, 1, 25))
					{
						throw Exception("PDF Reader: Cant read xref data: Cant read the position of startxref");
					}
					int index = 0;

					while (start_xref_buffer[index] > '9' || start_xref_buffer[index] < '0')
					{
						++index;
						if (index == 25)
						{
							throw Exception("PDF Reader: Cant read xref data: Cant read the position of startxref");
						}
					}

					//得到交叉引用表的偏移量
					bool backward_compatibility = false;
					std::set<size_t> start_xref_positions;
					xref_data_position = strtol(start_xref_buffer + index, NULL, 10);
					start_xref_positions.insert(xref_data_position);

                    //解析交叉引用表
					while (true)
					{
						if (!m_data_stream->seek(xref_data_position, SEEK_SET))
						{
							throw Exception("PDF Reader: Cant seek to the cross reference data");
						}

						char ch = m_data_stream->getc();
						if (ch == EOF)
						{
							throw Exception("PDF Reader: Unexpected end of buffer");
						}

						if (ch == 'x')	//xref line
						{
							//有 xref 关键字的是明文存储的交叉引用表
							//xref table
							std::string line;
							readLine(line);
							if (line.length() < 3 || line.substr(0, 3) != "ref")
							{
								throw Exception("PDF Reader: Invalid xref keyword");
							}

							//获取交叉引用表
							readXrefTable();

							m_trailer_dict.clearDictionary();

							//读取 trailer dictionary 信息，从这里我们可以获取 Root 的对象号
							readDictionary(m_trailer_dict);

							//Root 对象可能出现多次，但是值是一定的
							if (!m_got_root && m_trailer_dict.getObjAsReferenceCall("Root"))
							{
								m_got_root = true;
								*m_root_ref = *m_trailer_dict.getObjAsReferenceCall("Root");
							}

							//Root 对象可能出现多次，但是值是一定的
							if (!m_got_info && m_trailer_dict.getObjAsReferenceCall("Info"))
							{
								m_got_info = true;
								*m_info_ref = *m_trailer_dict.getObjAsReferenceCall("Info");
							}

							//压缩类型的交叉引用表的偏移量标识
							PDFNumericInteger* xref_stm = m_trailer_dict.getObjAsNumericInteger("XRefStm");
							if (xref_stm)
							{
								xref_data_position = (*xref_stm)();
								backward_compatibility = true;
							}
							else
							{
								//明文类型的交叉引用表的偏移量标识
								PDFNumericInteger* prev = m_trailer_dict.getObjAsNumericInteger("Prev");
								if (prev)
								{
									xref_data_position = (*prev)();
								}
								else
								{
									return;	//no more cross reference data
								}
							}
							if (start_xref_positions.find(xref_data_position) != start_xref_positions.end())
							{
								return;
							}
							start_xref_positions.insert(xref_data_position);
						}
						else
						{
							//没有 xref 关键字的是压缩类型存储的交叉引用表
							//xref stream
							m_data_stream->unGetc(ch);
							PDFNumericInteger* num_index = readNumeric()->getNumericInteger();
							if (!num_index)
							{
								throw Exception("PDF Reader: Invalid XRef stream");
							}
							size_t index = (*num_index)();
							delete num_index;
							//initialize obj:
							if (m_references.size() < index + 1)
							{
								m_references.resize(index + 1);
							}
							m_references[index].m_type = ReferenceInfo::in_use;
							m_references[index].m_offset = xref_data_position;
							m_references[index].m_read = true;
							PDFStream* xref_stream = readIndirectObject(index)->getStream();
							if (!xref_stream)
							{
								throw Exception("PDF Reader: Invalid XRef stream");
							}
							//压缩类型的交叉引用表
							readXRefStream(*xref_stream);

							if (!m_got_root && xref_stream->m_dictionary->getObjAsReferenceCall("Root"))
							{
								m_got_root = true;
								*m_root_ref = *xref_stream->m_dictionary->getObjAsReferenceCall("Root");
							}
							if (!m_got_info && xref_stream->m_dictionary->getObjAsReferenceCall("Info"))
							{
								m_got_info = true;
								*m_info_ref = *xref_stream->m_dictionary->getObjAsReferenceCall("Info");
							}

							//获取另外部分的压缩类型的交叉引用表的偏移量
							PDFNumericInteger* prev = NULL;
							if (backward_compatibility)
							{
								prev = m_trailer_dict.getObjAsNumericInteger("Prev");
							}
							else
							{
								prev = xref_stream->m_dictionary->getObjAsNumericInteger("Prev");
							}

							if (prev)
							{
								xref_data_position = (*prev)();
							}
							else
							{
								return;	//no more cross reference data
							}

							if (start_xref_positions.find(xref_data_position) != start_xref_positions.end())
							{
								return;
							}
							start_xref_positions.insert(xref_data_position);
						}
					}
				}
				catch (Exception& ex)
				{
					ex.appendError("PDF Reader: Error while reading reference data");
					throw;
				}
			}

			//获取明文存储的交叉引用表
			void readXrefTable()
			{
				try
				{
					std::string line;
					line.reserve(256);
					char* ptr_start = NULL;
					char* ptr_end = NULL;
					ReferenceInfo* reference;
					do
					{
						readLine(line);
						ptr_start = (char*)line.c_str();
						ptr_end = ptr_start;

						//判断结尾关键字 trailer
						if (ptr_start[0] != 't')	//trailer
						{
							//获取起始对象号
							size_t start = strtol(ptr_start, &ptr_end, 10);
							if (start == 0 && ptr_start == ptr_end)
							{
								throw Exception("PDF Reader: Error while coverting \"" + line + "\" to reference info");
							}
							ptr_start = ptr_end;

							//获取对象个数
							size_t count = strtol(ptr_start, &ptr_end, 10);
							if (count == 0 && ptr_start == ptr_end)
							{
								throw Exception("PDF Reader: Error while coverting \"" + line + "\" to reference info");
							}

							if (start + count > m_references.size())
							{
								m_references.resize(start + count);
							}
							for (size_t i = 0; i < count; ++i)
							{
								readLine(line);
								if (line.length() < 18)
								{
									throw Exception("PDF Reader: Line in cross reference table is too short: " + line);
								}
								reference = &m_references[start + i];
								if (!reference->m_read)
								{
									//获取偏移量
									ptr_start = (char*)line.c_str();
									ptr_end = ptr_start;
									reference->m_offset = strtol(ptr_start, &ptr_end, 10);
									if (reference->m_offset == 0 && ptr_start == ptr_end)
									{
										throw Exception("PDF Reader: Error while coverting \"" + line + "\" to reference info");
									}

									//获取产生号
									ptr_start = (char*)line.c_str() + 11;
									ptr_end = ptr_start;
									reference->m_generation = strtol(ptr_start, &ptr_end, 10);
									if (reference->m_generation == 0 && ptr_start == ptr_end)
									{
										throw Exception("PDF Reader: Error while coverting \"" + line + "\" to reference info");
									}

									//获取状态标识，f 标识未使用，n 标识使用中
									if (line[17] == 'f')
									{
										reference->m_type = ReferenceInfo::free;
									}
									else
									{
										reference->m_type = ReferenceInfo::in_use;
									}

									reference->m_read = true;
								}
							}
						}
					}
					while (ptr_start[0] != 't');	//trailer
				}
				catch (Exception& ex)
				{
					ex.appendError("PDF Reader: Error while reading XRef table");
					throw;
				}
			}

			//解析压缩类型的交叉引用表
			void readXRefStream(PDFStream& stream)
			{
				try
				{
					size_t entries_count = 0;
					std::vector<int> start_positions;
					std::vector<int> sizes;
					size_t w_sizes[3];

					/*
					 * 交叉引用表中的对象个数，这里一次可能存储不完，所以后面可能会有个Prev关键字，
					 * 也是记录一个压缩的交叉引用表，两个的对象总数就是 Size 的值
					 */
					PDFNumericInteger* num_size = stream.m_dictionary->getObjAsNumericInteger("Size");
					if (!num_size)
					{
						throw Exception("PDF Reader: No Size entry in XRef stream");
					}
					size_t size = (*num_size)();

					/* Index:交叉引用流中的对象数组，存储 Pdf_pairnum 结构信息
					 *      形式如 [a1 a2 a3 a4 ...] ，
					 *       每两个数字为一组，
					 *       例如: a1 a2
					 *       a1为首对象号，a2为该组的行数（即对象个数）
					 *       等价于明文的交叉引用表 xref 每组开始行的两个数字
					 */
					PDFArray* index_array = stream.m_dictionary->getObjAsArray("Index");
					if (index_array)
					{
						for (size_t i = 0; i < index_array->Size(); ++i)
						{
							PDFNumericInteger* element = index_array->getObjAsNumericInteger(i);
							if (element)
							{
								if (start_positions.size() == sizes.size())
								{
									start_positions.push_back((*element)());
								}
								else
								{
									sizes.push_back((*element)());
									entries_count += (*element)();
								}
							}
						}
					}
					else
					{
						start_positions.push_back(0);
						sizes.push_back(size);
					}

					if (sizes.size() != start_positions.size())
					{
						throw Exception("PDF Reader: \"Index\" entry in cross reference stream is invalid");
					}

					/*
					 * W 标识读取交叉引用表时，三个域长度格式，
					 * 例如 [1 2 1]，读取的时候，1字节 2字节 1字节
					 *     [1 3 2], 读取的时候，1字节 3字节 2字节
					 */
					PDFArray* w_array = stream.m_dictionary->getObjAsArray("W");
					if (!w_array || w_array->Size() != 3)
					{
						throw Exception("PDF Reader: Reference stream hasnt got \"W\" entry or this entry is invalid");
					}

					for (int i = 0; i < 3; ++i)
					{
						PDFNumericInteger* element = w_array->getObjAsNumericInteger(i);
						if (!element)
						{
							throw Exception("PDF Reader: \"W\" entry is invalid");
						}
						w_sizes[i] = (*element)();
					}

					ReferenceInfo* reference;
					PDFStream::PDFStreamIterator iterator = stream.getIterator();
					iterator.backToRoot();
					const unsigned char* data = (const unsigned char*)iterator.getData() + 1;	//skip '[' character
					size_t record_size = w_sizes[0] + w_sizes[1] + w_sizes[2];
					if (iterator.getDataLength() - 2 < record_size * entries_count)
					{
						throw Exception("PDF Reader: XRef stream is too short, does not contain all declared entries");
					}
					size_t read_index = 0;
					for (size_t i = 0; i < sizes.size(); ++i)
					{
						size_t elements_count = sizes[i];
						size_t element_start = start_positions[i];
						if (element_start + elements_count > m_references.size())
						{
							m_references.resize(element_start + elements_count);
						}
						for (size_t j = 0; j < elements_count; ++j)
						{
							reference = &m_references[element_start + j];
							if (reference->m_read)
							{
								read_index += record_size;
								continue;
							}
							reference->m_type = ReferenceInfo::in_use;		//default value
							for (int k = 0; k < w_sizes[0]; ++k)
							{
								if (data[read_index] > 2)
								{
									//invalid objct, mark as free
									reference->m_type = ReferenceInfo::free;
									++read_index;
								}
								else
								{
									reference->m_type = (ReferenceInfo::ReferenceType)(data[read_index++]);	//0 -> free, 1 -> in_ise, 2 -> compressed
								}
							}
							reference->m_offset = 0;
							for (int k = 0; k < w_sizes[1]; ++k)
							{
								reference->m_offset = reference->m_offset << 8;
								reference->m_offset += data[read_index++];
							}
							reference->m_generation = 0;
							for (int k = 0; k < w_sizes[2]; ++k)
							{
								reference->m_generation = reference->m_generation << 8;
								reference->m_generation += data[read_index++];
							}
							reference->m_read = true;
						}
					}
				}
				catch (Exception& ex)
				{
					ex.appendError("PDF Reader: Error while reading XRef stream");
					throw;
				}
			}
	};

	struct PDFContent
	{
		//object responsible for fast character mapping, implemented as a tree.
		/*
		 *	Simple description:
		 *	First, read about CMaps in PDF reference (beginbfchar, begincidchar etc.)
		 *	For simplicity, I convert everything to the "range":
		 *
		 *	1 beginbfchar
		 *	<01> <20>
		 *	endbfchar
		 *
		 *	The code above I treat like:
		 *	1 beginbfrange
		 *	<01> <01> <20>
		 *	endbfrange
		 *	It simplifies the problem.
		 *
		 *	I have created this CMap structure to face main problem: iterating through thousands of lines
		 *	(beginbfrange, begincidrange etc.) for each character will slow down parsing file. So I have
		 *	decided to use some kind of tree algorithm. All ranges (beginbfrange, begincidrange) are
		 *	converted to the tree. Each node can have up to 16 childs (one for '0', one for '1', etc.).
		 *	So, consider an example:
		 *
		 *	beginbfrange
		 *	<8140> <8200> <10234>
		 *	<9220> <925F> <189AA>
		 *	endbfrange
		 *
		 *	I repeat, all "bfchar" are treated like "bfrange", the same is for "cid" (for simplicity)
		 *
		 *	As the result of the example above, the tree will look like:
		 *
		 *	root
		 *	|	\
		 *	8	 9
		 *	|\	 |
		 *	1 2  2
		 *	| |	 |\
		 *	4 0	 2 5
		 *	| |	 | |
		 *	0 0	 0 F
		 *	| |	 | |
		 *	d d	 d d
		 *
		 *	(where 'd' is a pointer to the NodeData)
		 *
		 *	Lets take some number: for example we have to convert number 8155 to the Unicode.
		 *
		 *	First digit is '8'. We look at the root and we can see that slot 8 is allocated. Then, we jump
		 *	to '8' node. We check if this node has 'NodeData'. It hasnt, so we should look further.
		 *	Next digit is 1. Node '8' has a child at index '1'. So, jump deeper to the next node.
		 *	Unfortunately, this node also has NULL pointer to the NodeData. We need to look deeper. Next digit is '5'.
		 *	But current node doesnt have slot '5'. We look on the left. There is a '4', so go to the '4'. Next digit
		 *	in the stream (number 8155) is '5', but this time we dont look for 5, because we have descended from the
		 *	path before (on the left). So we look for the child with the biggest index. It is '0' (only '0' remains). So we
		 *	jump to the '0'. This time we have a pointer to the NodeData. Using this data, we can check how to convert
		 *	8155 to the Unicode/CID. Parameters:
		 *	m_utf8 -> "\xF0\x90\x88\xB4"	(0x10234 in UTF8)
		 *	m_first_codepoint -> 0x10234
		 *	m_is_not_def -> false
		 *	m_min_range -> 0x8140
		 *	m_max_range -> 0x8200
		 *	0x8155 is a number in a proper range (min and max above). So we are in correct place. 0x8155 is different
		 *	than m_min_range, so we wont check m_utf8. We will use m_first_codepoint:
		 *	result = m_first_codepoint + (0x8155 - m_min_range)
		 *	result is a unichar already. We are done. We go back to the root. And now we can get next number from the stream.
		 *
		 *	Summing, we have fast enough method to obtain Unicode/CID values from the character code.
		 *
		 *	Note that in the tree above there are 4 pointers to the NodeData, but only two instances of them exist in fact.
		*/
		struct CMap
		{
			struct NodeData
			{
				std::string m_utf8;
				unsigned int m_max_range;
				unsigned int m_min_range;
				unsigned int m_first_codepoint;
				bool m_is_not_def;

				NodeData()
				{
					m_is_not_def = false;
					m_first_codepoint = 0;
				}
			};

			struct Node
			{
				Node** m_childs;
				NodeData* m_node_data;

				Node()
				{
					m_childs = NULL;
					m_node_data = NULL;
				}

				~Node()
				{
					if (m_childs)
					{
						for (int i = 0; i < 16; ++i)
							if (m_childs[i])
								delete m_childs[i];
						delete[] m_childs;
					}
				}
			};

			enum SearchState
			{
				equal,
				less,
				more
			};

			Node m_root;
			std::list<NodeData> m_node_datas;
			CMap* m_parent;
			bool m_ready;

			CMap()
			{
				m_parent = NULL;
				m_ready = false;
			}

			~CMap()
			{
				if (m_parent)
					delete m_parent;
			}

			void getCidString(const char* str, size_t len, std::string& cid_string)
			{
				Node* current_node = &m_root;
				unsigned int codepoint = 0;
				size_t codepoint_len = 0;
				SearchState state = equal;

				for (size_t i = 0; i < len; ++i)
				{
					int index = str[i];
					if (index <= '9')
						index -= '0';
					else
						index -= ('A' - 10);
					codepoint = codepoint << 4;
					codepoint += index;
					++codepoint_len;

					if (!current_node->m_childs)
					{
						if (m_parent)
							m_parent->getCidString(str + i + 1 - codepoint_len, codepoint_len, cid_string);
						current_node = &m_root;
						codepoint = 0;
						codepoint_len = 0;
						state = equal;
						continue;
					}
					switch (state)
					{
						case equal:
						{
							if (current_node->m_childs[index])
							{
								current_node = current_node->m_childs[index];
							}
							else
							{
								int left_index = index - 1;
								while (left_index >= 0)
								{
									if (current_node->m_childs[left_index])
									{
										current_node = current_node->m_childs[left_index];
										state = less;
										break;
									}
									--left_index;
								}
								if (state == equal)
								{
									int right_index = index + 1;
									while (right_index <= 15)
									{
										if (current_node->m_childs[right_index])
										{
											current_node = current_node->m_childs[right_index];
											state = more;
											break;
										}
										++right_index;
									}
								}
								if (state == equal)
								{
									if (m_parent)
										m_parent->getCidString(str + i + 1 - codepoint_len, codepoint_len, cid_string);
									current_node = &m_root;
									codepoint = 0;
									codepoint_len = 0;
									continue;
								}
							}
							break;
						}
						case less:
						{
							int left_index = 15;
							while (left_index >= 0)
							{
								if (current_node->m_childs[left_index])
								{
									current_node = current_node->m_childs[left_index];
									break;
								}
								--left_index;
							}
							if (left_index == -1)
							{
								if (m_parent)
									m_parent->getCidString(str + i + 1 - codepoint_len, codepoint_len, cid_string);
								current_node = &m_root;
								codepoint = 0;
								state = equal;
								codepoint_len = 0;
								continue;
							}
							break;
						}
						case more:
						{
							int right_index = 0;
							while (right_index <= 15)
							{
								if (current_node->m_childs[right_index])
								{
									current_node = current_node->m_childs[right_index];
									break;
								}
								++right_index;
							}
							if (right_index == 16)
							{
								if (m_parent)
									m_parent->getCidString(str + i + 1 - codepoint_len, codepoint_len, cid_string);
								current_node = &m_root;
								codepoint = 0;
								state = equal;
								codepoint_len = 0;
								continue;
							}
							break;
						}
					}

					if (current_node->m_node_data)
					{
						NodeData* data = current_node->m_node_data;
						if (codepoint <= data->m_max_range && codepoint >= data->m_min_range)
						{
							unsigned int res_code = data->m_first_codepoint;
							if (codepoint != data->m_min_range && !data->m_is_not_def)
								res_code += (codepoint - data->m_min_range);
							if (res_code <= 0xFF)
								cid_string += "00";	//each CIDs length must be 4.
							uint_to_hex_string(res_code, cid_string);
						}
						else if (m_parent)
							m_parent->getCidString(str + i + 1 - codepoint_len, codepoint_len, cid_string);
						state = equal;
						current_node = &m_root;
						codepoint = 0;
						codepoint_len = 0;
					}
				}
			}

			void addCodeRange(std::string& min, std::string& max, unsigned int first_code_point, const std::string& utf8, bool is_not_def)
			{
				Node* current_node = &m_root;
				m_node_datas.push_back(NodeData());
				NodeData* data = &(*m_node_datas.rbegin());

				data->m_first_codepoint = first_code_point;
				data->m_utf8 = utf8;
				data->m_is_not_def = is_not_def;

				unsigned int min_codepoint = 0;
				for (size_t i = 0; i < min.length(); ++i)
				{
					int index = min[i];
					if (index <= '9')
						index -= '0';
					else
						index -= ('A' - 10);
					min_codepoint = min_codepoint << 4;
					min_codepoint += index;
					if (!current_node->m_childs)
					{
						current_node->m_childs = new Node*[16];
						for (int i = 0; i < 16; ++i)
							current_node->m_childs[i] = NULL;
					}
					if (!current_node->m_childs[index])
						current_node->m_childs[index] = new Node;
					current_node = current_node->m_childs[index];
				}
				data->m_min_range = min_codepoint;
				current_node->m_node_data = data;

				if (min != max)
				{
					current_node = &m_root;
					unsigned int max_codepoint = 0;
					for (size_t i = 0; i < max.length(); ++i)
					{
						int index = max[i];
						if (index <= '9')
							index -= '0';
						else
							index -= ('A' - 10);
						max_codepoint = max_codepoint << 4;
						max_codepoint += index;
						if (!current_node->m_childs)
						{
							current_node->m_childs = new Node*[16];
							for (int i = 0; i < 16; ++i)
								current_node->m_childs[i] = NULL;
						}
						if (!current_node->m_childs[index])
							current_node->m_childs[index] = new Node;
						current_node = current_node->m_childs[index];
					}
					data->m_max_range = max_codepoint;
					current_node->m_node_data = data;
				}
				else
					data->m_max_range = min_codepoint;
			}

			bool parseNextCID(char* str, size_t str_len, unsigned int& cid_len, std::string& output, unsigned int& cid)
			{
				Node* current_node = &m_root;
				SearchState state = equal;

				cid_len = 0;
				if (str_len == 0)
					return true;
				for (size_t i = 0; i < str_len; ++i)
				{
					int index = str[i];
					if (index <= '9')
						index -= '0';
					else
						index -= ('A' - 10);
					cid = cid << 4;
					cid += index;
					++cid_len;

					if (!current_node->m_childs)
						return false;
					switch (state)
					{
						case equal:
						{
							if (current_node->m_childs[index])
								current_node = current_node->m_childs[index];
							else
							{
								int left_index = index - 1;
								while (left_index >= 0)
								{
									if (current_node->m_childs[left_index])
									{
										current_node = current_node->m_childs[left_index];
										state = less;
										break;
									}
									--left_index;
								}
								if (state == equal)
								{
									int right_index = index + 1;
									while (right_index <= 15)
									{
										if (current_node->m_childs[right_index])
										{
											current_node = current_node->m_childs[right_index];
											state = more;
											break;
										}
										++right_index;
									}
								}
								if (state == equal)
									return false;
							}
							break;
						}
						case less:
						{
							int left_index = 15;
							while (left_index >= 0)
							{
								if (current_node->m_childs[left_index])
								{
									current_node = current_node->m_childs[left_index];
									break;
								}
								--left_index;
							}
							if (left_index == -1)
								return false;
							break;
						}
						case more:
						{
							int right_index = 0;
							while (right_index <= 15)
							{
								if (current_node->m_childs[right_index])
								{
									current_node = current_node->m_childs[right_index];
									break;
								}
								++right_index;
							}
							if (right_index == 16)
								return false;
						}
					}
					if (current_node->m_node_data)
					{
						NodeData* data = current_node->m_node_data;
						if (cid <= data->m_max_range && cid >= data->m_min_range)
						{
							if (cid != data->m_min_range && !data->m_is_not_def)
								output += unicode_codepoint_to_utf8(data->m_first_codepoint + (cid - data->m_min_range));
							else
								output += data->m_utf8;
							return true;
						}
						else
							return false;
					}
				}
				return false;
			}
		};

		struct FontMetrics
		{
			enum FontWeight
			{
				medium,
				bold,
				roman
			};

			std::string m_font_name;
			std::string m_font_family;
			unsigned int m_first_char;
			unsigned int m_last_char;
			double m_descent;
			double m_font_bbox[4];
			FontWeight m_font_weight;
			double m_cap_height;
			unsigned int m_flags;
			double m_x_height;
			double m_italic_angle;
			double m_ascent;
			std::vector<unsigned int> m_widths;
			unsigned int m_missing_width;
			unsigned int m_leading;
			double m_vscale, m_hscale;
			double m_font_matrix[6];

			FontMetrics()
			{
				m_font_matrix[0] = 0.001;
				m_font_matrix[1] = 0.0;
				m_font_matrix[2] = 0.0;
				m_font_matrix[3] = 0.001;
				m_font_matrix[4] = 0.0;
				m_font_matrix[5] = 0.0;
				m_widths.reserve(256);
				m_descent = 0.0;
				m_first_char = 0;
				m_last_char = 0;
				m_font_bbox[0] = 0.0;
				m_font_bbox[1] = 0.0;
				m_font_bbox[2] = 0.0;
				m_font_bbox[3] = 0.0;
				m_font_weight = medium;
				m_cap_height = 0.0;
				m_flags = 0;
				m_x_height = 0.0;
				m_italic_angle = 0.0;
				m_ascent = 0.0;
				m_missing_width = 0;
				m_leading = 0;
				m_vscale = m_hscale = 0.001;
			}
		};

		/* Font metrics for the Adobe core 14 fonts.

		Font metrics are used to compute the boundary of each character
		written with a proportional font.

		The following data were extracted from the AFM files:

		  http://www.ctan.org/tex-archive/fonts/adobe/afm/

		*/

		//  BEGIN Verbatim copy of the license part
		//
		//
		// Adobe Core 35 AFM Files with 229 Glyph Entries - ReadMe
		//
		// This file and the 35 PostScript(R) AFM files it accompanies may be
		// used, copied, and distributed for any purpose and without charge,
		// with or without modification, provided that all copyright notices
		// are retained; that the AFM files are not distributed without this
		// file; that all modifications to this file or any of the AFM files
		// are prominently noted in the modified file(s); and that this
		// paragraph is not modified. Adobe Systems has no responsibility or
		// obligation to support the use of the AFM files.
		//
		//
		//  END Verbatim copy of the license part

		class FontMetricsMap : public std::map<std::string, FontMetrics>
		{
			public:
				FontMetricsMap()
				{
					FontMetrics* font_metrics = &((*this)["Courier-Oblique"]);
					font_metrics->m_font_name = "Courier-Oblique";
					font_metrics->m_descent = -194.0;
					font_metrics->m_font_bbox[0] = -49.0;
					font_metrics->m_font_bbox[1] = -249.0;
					font_metrics->m_font_bbox[2] = 749.0;
					font_metrics->m_font_bbox[3] = 803.0;
					font_metrics->m_font_weight = FontMetrics::medium;
					font_metrics->m_cap_height = 572.0;
					font_metrics->m_font_family = "Courier";
					font_metrics->m_flags = 64;
					font_metrics->m_x_height = 434.0;
					font_metrics->m_italic_angle = -11.0;
					font_metrics->m_ascent = 627.0;
					font_metrics->m_first_char = 0;
					font_metrics->m_last_char = 0;
					font_metrics->m_missing_width = 600;

					font_metrics = &((*this)["Times-BoldItalic"]);
					font_metrics->m_font_name = "Times-BoldItalic";
					font_metrics->m_descent = -217.0;
					font_metrics->m_font_bbox[0] = -200.0;
					font_metrics->m_font_bbox[1] = -218.0;
					font_metrics->m_font_bbox[2] = 996.0;
					font_metrics->m_font_bbox[3] = 921.0;
					font_metrics->m_font_weight = FontMetrics::bold;
					font_metrics->m_cap_height = 669.0;
					font_metrics->m_font_family = "Times";
					font_metrics->m_flags = 0;
					font_metrics->m_x_height = 462.0;
					font_metrics->m_italic_angle = -15.0;
					font_metrics->m_ascent = 683.0;
					font_metrics->m_first_char = 0;
					font_metrics->m_last_char = 255;
					for (size_t i = 0; i < 32; ++i)
						font_metrics->m_widths.push_back(0);
					unsigned int times_bold_italic_widths[] = {250, 389, 555, 500, 500, 833, 778, 333, 333, 333, 500, 570, 250, 333,
															 250, 278, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 333, 333,
															 570, 570, 570, 500, 832, 667, 667, 667, 722, 667, 667, 722, 778, 389,
															 500, 667, 611, 889, 722, 722, 611, 722, 667, 556, 611, 722, 667, 889,
															 667, 611, 611, 333, 278, 333, 570, 500, 333, 500, 500, 444, 500, 444,
															 333, 500, 556, 278, 278, 500, 278, 778, 556, 500, 500, 500, 389, 389,
															 278, 556, 444, 667, 500, 444, 389, 348, 220, 348, 570, 0, 0, 0, 0, 0,
															 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
															 0, 0, 0, 0, 0, 0, 389, 500, 500, 167, 500, 500, 500, 500, 278, 500,
															 500, 333, 333, 556, 556, 0, 500, 500, 500, 250, 0, 500, 350, 333, 500,
															 500, 500, 1000, 1000, 0, 500, 0, 333, 333, 333, 333, 333, 333, 333,
															 333, 0, 333, 333, 0, 333, 333, 333, 1000, 0, 0, 0, 0, 0, 0, 0, 0, 0,
															 0, 0, 0, 0, 0, 0, 0, 944, 0, 266, 0, 0, 0, 0, 611, 722, 944, 300, 0,
															 0, 0, 0, 0, 722, 0, 0, 0, 278, 0, 0, 278, 500, 722, 500, 0, 0, 0};
					font_metrics->m_widths.insert(font_metrics->m_widths.end(), times_bold_italic_widths, times_bold_italic_widths + 224);

					font_metrics = &((*this)["Helvetica-Bold"]);
					font_metrics->m_font_name = "Helvetica-Bold";
					font_metrics->m_descent = -207.0;
					font_metrics->m_font_bbox[0] = -170.0;
					font_metrics->m_font_bbox[1] = -228.0;
					font_metrics->m_font_bbox[2] = 1003.0;
					font_metrics->m_font_bbox[3] = 962.0;
					font_metrics->m_font_weight = FontMetrics::bold;
					font_metrics->m_cap_height = 718.0;
					font_metrics->m_font_family = "Helvetica";
					font_metrics->m_flags = 0;
					font_metrics->m_x_height = 532.0;
					font_metrics->m_italic_angle = 0.0;
					font_metrics->m_ascent = 718.0;
					font_metrics->m_first_char = 0;
					font_metrics->m_last_char = 255;
					for (size_t i = 0; i < 32; ++i)
						font_metrics->m_widths.push_back(0);
					unsigned int helvetica_bold_widths[] = {278, 333, 474, 556, 556, 889, 722, 278, 333, 333, 389, 584, 278, 333,
															278, 278, 556, 556, 556, 556, 556, 556, 556, 556, 556, 556, 333, 333,
															584, 584, 584, 611, 975, 722, 722, 722, 722, 667, 611, 778, 722, 278,
															556, 722, 611, 833, 722, 778, 667, 778, 722, 667, 611, 722, 667, 944,
															667, 667, 611, 333, 278, 333, 584, 556, 278, 556, 611, 556, 611, 556,
															333, 611, 611, 278, 278, 556, 278, 889, 611, 611, 611, 611, 389, 556,
															333, 611, 556, 778, 556, 556, 500, 389, 280, 389, 584, 0, 0, 0, 0, 0,
															0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
															0, 0, 0, 0, 0, 0, 333, 556, 556, 167, 556, 556, 556, 556, 238, 500,
															556, 333, 333, 611, 611, 0, 556, 556, 556, 278, 0, 556, 350, 278, 500,
															500, 556, 1000, 1000, 0, 611, 0, 333, 333, 333, 333, 333, 333, 333,
															333, 0, 333, 333, 0, 333, 333, 333, 1000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
															0, 0, 0, 0, 0, 0, 1000, 0, 370, 0, 0, 0, 0, 611, 778, 1000, 365, 0, 0,
															0, 0, 0, 889, 0, 0, 0, 278, 0, 0, 278, 611, 944, 611, 0, 0, 0, 0};
					font_metrics->m_widths.insert(font_metrics->m_widths.end(), helvetica_bold_widths, helvetica_bold_widths + 224);

					font_metrics = &((*this)["Courier"]);
					font_metrics->m_font_name = "Courier";
					font_metrics->m_descent = -194.0;
					font_metrics->m_font_bbox[0] = -6.0;
					font_metrics->m_font_bbox[1] = -249.0;
					font_metrics->m_font_bbox[2] = 639.0;
					font_metrics->m_font_bbox[3] = 803.0;
					font_metrics->m_font_weight = FontMetrics::medium;
					font_metrics->m_cap_height = 572.0;
					font_metrics->m_font_family = "Courier";
					font_metrics->m_flags = 64;
					font_metrics->m_x_height = 434.0;
					font_metrics->m_italic_angle = 0.0;
					font_metrics->m_ascent = 627.0;
					font_metrics->m_first_char = 0;
					font_metrics->m_last_char = 0;
					font_metrics->m_missing_width = 600;

					font_metrics = &((*this)["Courier-BoldOblique"]);
					font_metrics->m_font_name = "Courier-BoldOblique";
					font_metrics->m_descent = -194.0;
					font_metrics->m_font_bbox[0] = -49.0;
					font_metrics->m_font_bbox[1] = -249.0;
					font_metrics->m_font_bbox[2] = 758.0;
					font_metrics->m_font_bbox[3] = 811.0;
					font_metrics->m_font_weight = FontMetrics::bold;
					font_metrics->m_cap_height = 572.0;
					font_metrics->m_font_family = "Courier";
					font_metrics->m_flags = 64;
					font_metrics->m_x_height = 434.0;
					font_metrics->m_italic_angle = -11.0;
					font_metrics->m_ascent = 627.0;
					font_metrics->m_first_char = 0;
					font_metrics->m_last_char = 0;
					font_metrics->m_missing_width = 600;

					font_metrics = &((*this)["Times-Bold"]);
					font_metrics->m_font_name = "Times-Bold";
					font_metrics->m_descent = -217.0;
					font_metrics->m_font_bbox[0] = -168.0;
					font_metrics->m_font_bbox[1] = -218.0;
					font_metrics->m_font_bbox[2] = 1000.0;
					font_metrics->m_font_bbox[3] = 935.0;
					font_metrics->m_font_weight = FontMetrics::bold;
					font_metrics->m_cap_height = 676.0;
					font_metrics->m_font_family = "Times";
					font_metrics->m_flags = 0;
					font_metrics->m_x_height = 461.0;
					font_metrics->m_italic_angle = 0.0;
					font_metrics->m_ascent = 683.0;
					font_metrics->m_first_char = 0;
					font_metrics->m_last_char = 255;
					for (size_t i = 0; i < 32; ++i)
						font_metrics->m_widths.push_back(0);
					unsigned int times_bold_widths[] = {250, 333, 555, 500, 500, 1000, 833, 333, 333, 333, 500, 570, 250, 333, 250,
														278, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 333, 333, 570, 570,
														570, 500, 930, 722, 667, 722, 722, 667, 611, 778, 778, 389, 500, 778, 667,
														944, 722, 778, 611, 778, 722, 556, 667, 722, 722, 1000, 722, 722, 667, 333,
														278, 333, 581, 500, 333, 500, 556, 444, 556, 444, 333, 500, 556, 278, 333,
														556, 278, 833, 556, 500, 556, 556, 444, 389, 333, 556, 500, 722, 500, 500,
														444, 394, 220, 394, 520, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
														0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 333, 500, 500, 167, 500,
														500, 500, 500, 278, 500, 500, 333, 333, 556, 556, 0, 500, 500, 500, 250, 0,
														540, 350, 333, 500, 500, 500, 1000, 1000, 0, 500, 0, 333, 333, 333, 333,
														333, 333, 333, 333, 0, 333, 333, 0, 333, 333, 333, 1000, 0, 0, 0, 0, 0, 0, 0,
														0, 0, 0, 0, 0, 0, 0, 0, 0, 1000, 0, 300, 0, 0, 0, 0, 667, 778, 1000, 330, 0,
														0, 0, 0, 0, 722, 0, 0, 0, 278, 0, 0, 278, 500, 722, 556, 0, 0, 0, 0};
					font_metrics->m_widths.insert(font_metrics->m_widths.end(), times_bold_widths, times_bold_widths + 224);

					font_metrics = &((*this)["Symbol"]);
					font_metrics->m_font_name = "Symbol";
					font_metrics->m_descent = 0.0;
					font_metrics->m_font_bbox[0] = -180.0;
					font_metrics->m_font_bbox[1] = -293.0;
					font_metrics->m_font_bbox[2] = 1090.0;
					font_metrics->m_font_bbox[3] = 1010.0;
					font_metrics->m_font_weight = FontMetrics::medium;
					font_metrics->m_cap_height = 676.0;
					font_metrics->m_font_family = "Symbol";
					font_metrics->m_flags = 0;
					font_metrics->m_x_height = 0.0;
					font_metrics->m_italic_angle = 0.0;
					font_metrics->m_ascent = 0.0;
					font_metrics->m_first_char = 0;
					font_metrics->m_last_char = 255;
					for (size_t i = 0; i < 32; ++i)
						font_metrics->m_widths.push_back(0);
					unsigned int symbol_widths[] = {250, 333, 713, 500, 549, 833, 778, 439, 333, 333, 500, 549, 250, 549, 250, 278, 500,
													500, 500, 500, 500, 500, 500, 500, 500, 500, 278, 278, 549, 549, 549, 444, 549, 722,
													667, 722, 612, 611, 763, 603, 722, 333, 631, 722, 686, 889, 722, 722, 768, 741, 556,
													592, 611, 690, 439, 768, 645, 795, 611, 333, 863, 333, 658, 500, 500, 631, 549, 549,
													494, 439, 521, 411, 603, 329, 603, 549, 549, 576, 521, 549, 549, 521, 549, 603, 439,
													576, 713, 686, 493, 686, 494, 480, 200, 480, 549, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
													0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 750, 620, 247, 549,
													167, 713, 500, 753, 753, 753, 753, 1042, 987, 603, 987, 603, 400, 549, 411, 549, 549,
													713, 494, 460, 549, 549, 549, 549, 1000, 603, 1000, 658, 823, 686, 795, 987, 768, 768,
													823, 768, 768, 713, 713, 713, 713, 713, 713, 713, 768, 713, 790, 790, 890, 823, 549,
													250, 713, 603, 603, 1042, 987, 603, 987, 603, 494, 329, 790, 790, 786, 713, 384,
													384, 384, 384, 384, 384, 494, 494, 494, 494, 329, 274, 0, 686, 686, 686, 384, 384,
													384, 384, 384, 384, 494, 494, 494, 0};
					font_metrics->m_widths.insert(font_metrics->m_widths.end(), symbol_widths, symbol_widths + 224);

					font_metrics = &((*this)["Helvetica"]);
					font_metrics->m_font_name = "Helvetica";
					font_metrics->m_descent = -207.0;
					font_metrics->m_font_bbox[0] = -166.0;
					font_metrics->m_font_bbox[1] = -225.0;
					font_metrics->m_font_bbox[2] = 1000.0;
					font_metrics->m_font_bbox[3] = 931.0;
					font_metrics->m_font_weight = FontMetrics::medium;
					font_metrics->m_cap_height = 718.0;
					font_metrics->m_font_family = "Helvetica";
					font_metrics->m_flags = 0;
					font_metrics->m_x_height = 523.0;
					font_metrics->m_italic_angle = 0.0;
					font_metrics->m_ascent = 718.0;
					font_metrics->m_first_char = 0;
					font_metrics->m_last_char = 255;
					for (size_t i = 0; i < 32; ++i)
						font_metrics->m_widths.push_back(0);
					unsigned int helvetica_widths[] = {278, 278, 355, 556, 556, 889, 667, 222, 333, 333, 389, 584, 278, 333, 278, 278,
													   556, 556, 556, 556, 556, 556, 556, 556, 556, 556, 278, 278, 584, 584, 584, 556,
													   1015, 667, 667, 722, 722, 667, 611, 778, 722, 278, 500, 667, 556, 833, 722, 778,
													   667, 778, 722, 667, 611, 722, 667, 944, 667, 667, 611, 278, 278, 278, 469, 556,
													   222, 556, 556, 500, 556, 556, 278, 556, 556, 222, 222, 500, 222, 833, 556, 556,
													   556, 556, 333, 500, 278, 556, 500, 722, 500, 500, 500, 334, 260, 334, 584, 0, 0,
													   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
													   0, 0, 0, 0, 0, 333, 556, 556, 167, 556, 556, 556, 556, 191, 333, 556, 333, 333,
													   500, 500, 0, 556, 556, 556, 278, 0, 537, 350, 222, 333, 333, 556, 1000, 1000, 0,
													   611, 0, 333, 333, 333, 333, 333, 333, 333, 333, 0, 333, 333, 0, 333, 333, 333,
													   1000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1000, 0, 370, 0, 0, 0, 0,
													   556, 778, 1000, 365, 0, 0, 0, 0, 0, 889, 0, 0, 0, 278, 0, 0, 222, 611, 944, 611,
													   0, 0, 0, 0};
					font_metrics->m_widths.insert(font_metrics->m_widths.end(), helvetica_widths, helvetica_widths + 224);

					font_metrics = &((*this)["Helvetica-BoldOblique"]);
					font_metrics->m_font_name = "Helvetica-BoldOblique";
					font_metrics->m_descent = -207.0;
					font_metrics->m_font_bbox[0] = -175.0;
					font_metrics->m_font_bbox[1] = -228.0;
					font_metrics->m_font_bbox[2] = 1114.0;
					font_metrics->m_font_bbox[3] = 962.0;
					font_metrics->m_font_weight = FontMetrics::bold;
					font_metrics->m_cap_height = 718.0;
					font_metrics->m_font_family = "Helvetica";
					font_metrics->m_flags = 0;
					font_metrics->m_x_height = 532.0;
					font_metrics->m_italic_angle = -12.0;
					font_metrics->m_ascent = 718.0;
					font_metrics->m_first_char = 0;
					font_metrics->m_last_char = 255;
					for (size_t i = 0; i < 32; ++i)
						font_metrics->m_widths.push_back(0);
					unsigned int helvetica_bold_oblique_widths[] = {278, 333, 474, 556, 556, 889, 722, 278, 333, 333, 389, 584, 278, 333,
																  278, 278, 556, 556, 556, 556, 556, 556, 556, 556, 556, 556, 333, 333,
																  584, 584, 584, 611, 975, 722, 722, 722, 722, 667, 611, 778, 722, 278,
																  556, 722, 611, 833, 722, 778, 667, 778, 722, 667, 611, 722, 667, 944,
																  667, 667, 611, 333, 278, 333, 584, 556, 278, 556, 611, 556, 611, 556,
																  333, 611, 611, 278, 278, 556, 278, 889, 611, 611, 611, 611, 389, 556,
																  333, 611, 556, 778, 556, 556, 500, 389, 280, 389, 584, 0, 0, 0, 0, 0,
																  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																  0, 0, 0, 0, 0, 0, 333, 556, 556, 167, 556, 556, 556, 556, 238, 500,
																  556, 333, 333, 611, 611, 0, 556, 556, 556, 278, 0, 556, 350, 278, 500,
																  500, 556, 1000, 1000, 0, 611, 0, 333, 333, 333, 333, 333, 333, 333, 333,
																  0, 333, 333, 0, 333, 333, 333, 1000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																  0, 0, 0, 0, 1000, 0, 370, 0, 0, 0, 0, 611, 778, 1000, 365, 0, 0, 0, 0,
																  0, 889, 0, 0, 0, 278, 0, 0, 278, 611, 944, 611, 0, 0, 0, 0};
					font_metrics->m_widths.insert(font_metrics->m_widths.end(), helvetica_bold_oblique_widths, helvetica_bold_oblique_widths + 224);

					font_metrics = &((*this)["ZapfDingbats"]);
					font_metrics->m_font_name = "ZapfDingbats";
					font_metrics->m_descent = 0.0;
					font_metrics->m_font_bbox[0] = -1.0;
					font_metrics->m_font_bbox[1] = -143.0;
					font_metrics->m_font_bbox[2] = 981.0;
					font_metrics->m_font_bbox[3] = 820.0;
					font_metrics->m_font_weight = FontMetrics::medium;
					font_metrics->m_cap_height = 718.0;
					font_metrics->m_font_family = "ITC";
					font_metrics->m_flags = 0;
					font_metrics->m_x_height = 0.0;
					font_metrics->m_italic_angle = 0.0;
					font_metrics->m_ascent = 0.0;
					font_metrics->m_first_char = 0;
					font_metrics->m_last_char = 255;
					for (size_t i = 0; i < 32; ++i)
						font_metrics->m_widths.push_back(0);
					unsigned int zapf_dingbats_widths[] = {278, 974, 961, 974, 980, 719, 789, 790, 791, 690, 960, 939, 549, 855, 911, 933,
														 911, 945, 974, 755, 846, 762, 761, 571, 677, 763, 760, 759, 754, 494, 552, 537,
														 577, 692, 786, 788, 788, 790, 793, 794, 816, 823, 789, 841, 823, 833, 816, 831,
														 923, 744, 723, 749, 790, 792, 695, 776, 768, 792, 759, 707, 708, 682, 701, 826,
														 815, 789, 789, 707, 687, 696, 689, 786, 787, 713, 791, 785, 791, 873, 761, 762,
														 762, 759, 759, 892, 892, 788, 784, 438, 138, 277, 415, 392, 392, 668, 668, 0,
														 390, 390, 317, 317, 276, 276, 509, 509, 410, 410, 234, 234, 334, 334, 0, 0, 0,
														 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 732, 544, 544, 910, 667, 760, 760,
														 776, 595, 694, 626, 788, 788, 788, 788, 788, 788, 788, 788, 788, 788, 788, 788,
														 788, 788, 788, 788, 788, 788, 788, 788, 788, 788, 788, 788, 788, 788, 788, 788,
														 788, 788, 788, 788, 788, 788, 788, 788, 788, 788, 788, 788, 894, 838, 1016, 458,
														 748, 924, 748, 918, 927, 928, 928, 834, 873, 828, 924, 924, 917, 930, 931, 463,
														 883, 836, 836, 867, 867, 696, 696, 874, 0, 874, 760, 946, 771, 865, 771, 888, 967,
														 888, 831, 873, 927, 970, 918, 0};
					font_metrics->m_widths.insert(font_metrics->m_widths.end(), zapf_dingbats_widths, zapf_dingbats_widths + 224);

					font_metrics = &((*this)["Courier-Bold"]);
					font_metrics->m_font_name = "Courier-Bold";
					font_metrics->m_descent = -194.0;
					font_metrics->m_font_bbox[0] = -88.0;
					font_metrics->m_font_bbox[1] = -249.0;
					font_metrics->m_font_bbox[2] = 697.0;
					font_metrics->m_font_bbox[3] = 811.0;
					font_metrics->m_font_weight = FontMetrics::bold;
					font_metrics->m_cap_height = 572.0;
					font_metrics->m_font_family = "Courier";
					font_metrics->m_flags = 64;
					font_metrics->m_x_height = 434.0;
					font_metrics->m_italic_angle = 0.0;
					font_metrics->m_ascent = 627.0;
					font_metrics->m_first_char = 0;
					font_metrics->m_last_char = 0;
					font_metrics->m_missing_width = 600;

					font_metrics = &((*this)["Times-Italic"]);
					font_metrics->m_font_name = "Times-Italic";
					font_metrics->m_descent = -217.0;
					font_metrics->m_font_bbox[0] = -169.0;
					font_metrics->m_font_bbox[1] = -217.0;
					font_metrics->m_font_bbox[2] = 1010.0;
					font_metrics->m_font_bbox[3] = 883.0;
					font_metrics->m_font_weight = FontMetrics::medium;
					font_metrics->m_cap_height = 653.0;
					font_metrics->m_font_family = "Times";
					font_metrics->m_flags = 0;
					font_metrics->m_x_height = 441.0;
					font_metrics->m_italic_angle = -15.5;
					font_metrics->m_ascent = 683.0;
					font_metrics->m_first_char = 0;
					font_metrics->m_last_char = 255;
					for (size_t i = 0; i < 32; ++i)
						font_metrics->m_widths.push_back(0);
					unsigned int times_italic_widths[] = {250, 333, 420, 500, 500, 833, 778, 333, 333, 333, 500, 675, 250, 333, 250,
														  278, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 333, 333, 675, 675,
														  675, 500, 920, 611, 611, 667, 722, 611, 611, 722, 722, 333, 444, 667, 556,
														  833, 667, 722, 611, 722, 611, 500, 556, 722, 611, 833, 611, 556, 556, 389,
														  278, 389, 422, 500, 333, 500, 500, 444, 500, 444, 278, 500, 500, 278, 278,
														  444, 278, 722, 500, 500, 500, 500, 389, 389, 278, 500, 444, 667, 444, 444,
														  389, 400, 275, 400, 541, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
														  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 389, 500, 500, 167, 500,
														  500, 500, 500, 214, 556, 500, 333, 333, 500, 500, 0, 500, 500, 500, 250, 0,
														  523, 350, 333, 556, 556, 500, 889, 1000, 0, 500, 0, 333, 333, 333, 333, 333,
														  333, 333, 333, 0, 333, 333, 0, 333, 333, 333, 889, 0, 0, 0, 0, 0, 0, 0, 0,
														  0, 0, 0, 0, 0, 0, 0, 0, 889, 0, 276, 0, 0, 0, 0, 556, 722, 944, 310, 0, 0,
														  0, 0, 0, 667, 0, 0, 0, 278, 0, 0, 278, 500, 667, 500, 0, 0, 0, 0};
					font_metrics->m_widths.insert(font_metrics->m_widths.end(), times_italic_widths, times_italic_widths + 224);

					font_metrics = &((*this)["Times-Roman"]);
					font_metrics->m_font_name = "Times-Roman";
					font_metrics->m_descent = -217.0;
					font_metrics->m_font_bbox[0] = -168.0;
					font_metrics->m_font_bbox[1] = -218.0;
					font_metrics->m_font_bbox[2] = 1000.0;
					font_metrics->m_font_bbox[3] = 898.0;
					font_metrics->m_font_weight = FontMetrics::roman;
					font_metrics->m_cap_height = 662.0;
					font_metrics->m_font_family = "Times";
					font_metrics->m_flags = 0;
					font_metrics->m_x_height = 450.0;
					font_metrics->m_italic_angle = 0.0;
					font_metrics->m_ascent = 683.0;
					font_metrics->m_first_char = 0;
					font_metrics->m_last_char = 255;
					for (size_t i = 0; i < 32; ++i)
						font_metrics->m_widths.push_back(0);
					unsigned int times_roman_widths[] = {250, 333, 408, 500, 500, 833, 778, 333, 333, 333, 500, 564, 250, 333, 250, 278,
													   500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 278, 278, 564, 564, 564, 444,
													   921, 722, 667, 667, 722, 611, 556, 722, 722, 333, 389, 722, 611, 889, 722, 722,
													   556, 722, 667, 556, 611, 722, 722, 944, 722, 722, 611, 333, 278, 333, 469, 500,
													   333, 444, 500, 444, 500, 444, 333, 500, 500, 278, 278, 500, 278, 778, 500, 500,
													   500, 500, 333, 389, 278, 500, 500, 722, 500, 500, 444, 480, 200, 480, 541, 0, 0,
													   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
													   0, 0, 0, 0, 0, 333, 500, 500, 167, 500, 500, 500, 500, 180, 444, 500, 333, 333,
													   556, 556, 0, 500, 500, 500, 250, 0, 453, 350, 333, 444, 444, 500, 1000, 1000, 444,
													   0, 333, 333, 333, 333, 333, 333, 333, 333, 0, 333, 333, 0, 333, 333, 333, 1000,
													   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 889, 0, 276, 0, 0, 0, 0, 611, 722,
													   889, 310, 0, 0, 0, 0, 0, 667, 0, 0, 0, 278, 0, 0, 278, 500, 722, 500, 0, 0, 0, 0};
					font_metrics->m_widths.insert(font_metrics->m_widths.end(), times_roman_widths, times_roman_widths + 224);

					font_metrics = &((*this)["Helvetica-Oblique"]);
					font_metrics->m_font_name = "Helvetica-Oblique";
					font_metrics->m_descent = -207.0;
					font_metrics->m_font_bbox[0] = -171.0;
					font_metrics->m_font_bbox[1] = -225.0;
					font_metrics->m_font_bbox[2] = 1116.0;
					font_metrics->m_font_bbox[3] = 931.0;
					font_metrics->m_font_weight = FontMetrics::medium;
					font_metrics->m_cap_height = 718.0;
					font_metrics->m_font_family = "Helvetica";
					font_metrics->m_flags = 0;
					font_metrics->m_x_height = 523.0;
					font_metrics->m_italic_angle = -12.0;
					font_metrics->m_ascent = 718.0;
					font_metrics->m_first_char = 0;
					font_metrics->m_last_char = 255;
					for (size_t i = 0; i < 32; ++i)
						font_metrics->m_widths.push_back(0);
					unsigned int helvetica_oblique_widths[] = {278, 278, 355, 556, 556, 889, 667, 222, 333, 333, 389, 584, 278, 333, 278,
															   278, 556, 556, 556, 556, 556, 556, 556, 556, 556, 556, 278, 278, 584, 584,
															   584, 556, 1015, 667, 667, 722, 722, 667, 611, 778, 722, 278, 500, 667, 556,
															   833, 722, 778, 667, 778, 722, 667, 611, 722, 667, 944, 667, 667, 611, 278,
															   278, 278, 469, 556, 222, 556, 556, 500, 556, 556, 278, 556, 556, 222, 222,
															   500, 222, 833, 556, 556, 556, 556, 333, 500, 278, 556, 500, 722, 500, 500,
															   500, 334, 260, 334, 584, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
															   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 333, 556, 556, 167, 556,
															   556, 556, 556, 191, 333, 556, 333, 333, 500, 500, 0, 556, 556, 556, 278, 0,
															   537, 350, 222, 333, 333, 556, 1000, 1000, 0, 611, 0, 333, 333, 333, 333, 333,
															   333, 333, 333, 0, 333, 333, 0, 333, 333, 333, 1000, 0, 0, 0, 0, 0, 0, 0, 0,
															   0, 0, 0, 0, 0, 0, 0, 0, 1000, 0, 370, 0, 0, 0, 0, 556, 778, 1000, 365, 0, 0,
															   0, 0, 0, 889, 0, 0, 0, 278, 0, 0, 222, 611, 944, 611, 0, 0, 0, 0};
					font_metrics->m_widths.insert(font_metrics->m_widths.end(), helvetica_oblique_widths, helvetica_oblique_widths + 224);
				}
		};

		static FontMetricsMap pdf_font_metrics_map;

		struct Font
		{
			PDFReader::PDFDictionary* m_font_dictionary;
			std::string m_font_encoding;
			bool m_predefined_simple_encoding;
			bool m_predefined_cmap;
			CMap m_cmap;
			CMap m_to_cid_cmap;
			unsigned int* m_simple_encoding_table;
			bool m_own_simple_encoding_table;
			std::string m_font_type;
			std::string m_base_font;
			FontMetrics m_font_metrics;
			bool m_multibyte;
			PDFReader::PDFDictionary* m_font_descriptor;
			char* m_cid_begin;
			size_t m_cid_len;

			Font()
			{
				m_multibyte = false;
				m_font_encoding = "unknown";
				m_font_dictionary = NULL;
				m_predefined_cmap = false;
				m_predefined_simple_encoding = false;
				m_simple_encoding_table = NULL;
				m_own_simple_encoding_table = false;
				m_font_descriptor = NULL;
				m_cid_begin = NULL;
				m_cid_len = 0;
			}

			~Font()
			{
				if (m_own_simple_encoding_table && m_simple_encoding_table)
					delete[] m_simple_encoding_table;
			}

			double getWidth(unsigned int cid)
			{
				if (cid < m_font_metrics.m_first_char)
					return m_font_metrics.m_missing_width * m_font_metrics.m_hscale;
				cid -= m_font_metrics.m_first_char;
				if (cid >= m_font_metrics.m_widths.size())
					return m_font_metrics.m_missing_width * m_font_metrics.m_hscale;
				return m_font_metrics.m_widths[cid] * m_font_metrics.m_hscale;
			}

			double getHeight()
			{
				double height = m_font_metrics.m_font_bbox[3] == m_font_metrics.m_font_bbox[1] ?
								m_font_metrics.m_ascent - m_font_metrics.m_descent : m_font_metrics.m_font_bbox[3] - m_font_metrics.m_font_bbox[1];
				return height * m_font_metrics.m_vscale;
			}

			double getDescent()
			{
				return m_font_metrics.m_descent * m_font_metrics.m_vscale;
			}

			unsigned int getNextCIDandDecode(std::string& output)
			{
				unsigned int cid = 0;
				bool parsed_cid = false;
				if (m_cmap.m_ready)
				{
					unsigned int cid_len;
					parsed_cid = m_cmap.parseNextCID(m_cid_begin, m_cid_len, cid_len, output, cid);
					if (parsed_cid)
					{
						m_cid_begin += cid_len;
						m_cid_len -= cid_len;
					}
				}
				if (m_predefined_simple_encoding && !parsed_cid)
				{
					cid = hex_char_to_single_char(m_cid_begin);
					output += utf8_codepoint_to_utf8(m_simple_encoding_table[cid]);
					m_cid_begin += 2;
					m_cid_len -= 2;
					parsed_cid = true;
				}
				if (!parsed_cid)
				{
					m_cid_len -= 2;
					m_cid_begin += 2;
				}
				return cid;
			}

			bool hasNextCid()
			{
				return m_cid_len > 0;
			}

			void setCidString(std::string& cid_str)
			{
				m_cid_begin = (char*)cid_str.c_str();
				m_cid_len = cid_str.length();
			}

			void convertToCidString(std::string& str)
			{
				if (m_predefined_cmap)
				{
					std::string cid_string;
					m_to_cid_cmap.getCidString(str.c_str(), str.length(), cid_string);
					str = cid_string;
				}
			}
		};

		struct TJArrayElement
		{
			bool m_is_number;
			std::string m_text;
			double m_value;
		};

		struct PageText
		{
			struct TextState
			{
				double m_ctm[6];
				double m_matrix[6];
				double m_line_matrix[2];
				double m_font_size;
				double m_scaling;
				double m_leading;
				double m_rise;
				double m_word_space;
				double m_char_space;

				TextState()
				{
					reset();
				}

				void reset()
				{
					m_ctm[0] = 1.0;
					m_ctm[1] = 0.0;
					m_ctm[2] = 0.0;
					m_ctm[3] = 1.0;
					m_ctm[4] = 0.0;
					m_ctm[5] = 0.0;
					m_font_size = 0.0;
					m_scaling = 100.0;
					m_leading = 0.0;
					m_matrix[0] = 1.0;
					m_matrix[1] = 0.0;
					m_matrix[2] = 0.0;
					m_matrix[3] = 1.0;
					m_matrix[4] = 0.0;
					m_matrix[5] = 0.0;
					m_line_matrix[0] = 0.0;
					m_line_matrix[1] = 0.0;
					m_word_space = 0.0;
					m_char_space = 0.0;
					m_rise = 0.0;
				}
			};

			struct TextElement
			{
				std::string m_text;
				double m_x, m_y, m_width, m_height;
				double m_space_size;

				TextElement(double x, double y, double w, double h, double space_size, const std::string& text)
				{
					#warning TODO: We have position and size for each string. We can use those values to improve parser
					m_x = correctSize(x);
					m_y = correctSize(y);
					m_text = text;
					m_width = correctSize(w);
					m_height = correctSize(h);
					m_space_size = space_size;
				}

				double correctSize(double value)
				{
					//file may be corrupted, we should set some maximum values.
					#warning TODO: Check MediaBox entry (defines page area)
					if (value < 0)
						value = 0.0;
					if (value > 5000)
						value = 5000;
					return value;
				}

				bool operator == (const TextElement& compared) const
				{
					return compared.m_y == m_y && compared.m_x == m_x;
				}

				bool operator < (const TextElement& compared) const
				{
					if (abs(m_y - compared.m_y) > 4.0)	//tolerace
					{
						return m_y > compared.m_y;
					}
					return m_x < compared.m_x;
				}

				bool operator > (const TextElement& compared) const
				{
					if (abs(m_y - compared.m_y) > 4.0) //tolerace
					{
						return m_y < compared.m_y;
					}
					return m_x > compared.m_x;
				}
			};

			Font* m_font;
			std::list<TextState> m_text_states;
			TextState m_current_state;
			std::multiset<TextElement> m_text_elements;

			void reset()
			{
				m_font = NULL;
				m_text_states.clear();
				m_current_state.reset();
			}

			void pushState()
			{
				m_text_states.push_back(m_current_state);
			}

			void popState()
			{
				if (m_text_states.size() > 0)
				{
					m_current_state = m_text_states.back();
					m_text_states.pop_back();
				}
			}

			/*
			 * 文本矩阵：Tm
			 * 6个参数：
			 * [X1 0 0 X2 X3 X4]
			 * X1：字体宽度
			 * X2：字体高度
			 * X3：水平位置
			 * X4：垂直位置
			 *
			 * 例： 1 0 0 1 368.35 758.28 Tm
			 *
			 */
			void executeTm(std::vector<double>& args)
			{
				for (int i = 0; i < 6; ++i)
				{
					m_current_state.m_matrix[i] = args[i];
				}
				m_current_state.m_line_matrix[0] = 0.0;
				m_current_state.m_line_matrix[1] = 0.0;
			}

			void executeTs(std::vector<double>& args)
			{
				m_current_state.m_rise = args[0];
			}

			void executeTc(std::vector<double>& args)
			{
				m_current_state.m_char_space = args[0];
			}

			void executeTw(std::vector<double>& args)
			{
				m_current_state.m_word_space = args[0];
			}

			void executeTd(std::vector<double>& args)
			{
				m_current_state.m_matrix[4] += args[0] * m_current_state.m_matrix[0] + args[1] * m_current_state.m_matrix[2];
				m_current_state.m_matrix[5] += args[0] * m_current_state.m_matrix[1] + args[1] * m_current_state.m_matrix[3];
				m_current_state.m_line_matrix[0] = 0.0;
				m_current_state.m_line_matrix[1] = 0.0;
			}

			void executeTD(std::vector<double>& args)
			{
				executeTd(args);
				m_current_state.m_leading = args[1];
			}

			void executeTstar()
			{
				m_current_state.m_matrix[4] += m_current_state.m_matrix[2] * m_current_state.m_leading;
				m_current_state.m_matrix[5] += m_current_state.m_matrix[3] * m_current_state.m_leading;
				m_current_state.m_line_matrix[0] = 0.0;
				m_current_state.m_line_matrix[1] = 0.0;
			}

			void executeTf(std::vector<double>& args, Font& font)
			{
				m_current_state.m_font_size = args[0];
				m_font = &font;
			}

			void executeTL(std::vector<double>& args)
			{
				m_current_state.m_leading = -args[0];
			}

			void executeTZ(std::vector<double>& args)
			{
				m_current_state.m_scaling = args[0];
			}

			void executeQuote(std::string& str)
			{
				executeTstar();
				executeTj(str);
			}

			void executeDoubleQuote(std::string& str, std::vector<double>& args)
			{
				executeTw(args);
				args[0] = args[1];
				args.pop_back();
				executeTc(args);
				executeTj(str);
			}

			void executeCm(std::vector<double>& args)
			{
				double tmp[6];
				for (int i = 0; i < 6; ++i)
					tmp[i] = m_current_state.m_ctm[i];
				m_current_state.m_ctm[0] = tmp[0] * args[0] + tmp[2] * args[1];
				m_current_state.m_ctm[1] = tmp[1] * args[0] + tmp[3] * args[1];
				m_current_state.m_ctm[2] = tmp[0] * args[2] + tmp[2] * args[3];
				m_current_state.m_ctm[3] = tmp[1] * args[2] + tmp[3] * args[3];
				m_current_state.m_ctm[4] += tmp[0] * args[4] + tmp[2] * args[5];
				m_current_state.m_ctm[5] += tmp[1] * args[4] + tmp[3] * args[5];
			}

			/*
			 * 文本对象
			 * BT 文本开始
			 * 	字体信息
			 * 	位置信息
			 * 	。。。
			 * 	格式信息
			 * ET 文本结束
			 */
			void executeBT()
			{
				m_current_state.m_matrix[0] = 1.0;
				m_current_state.m_matrix[1] = 0.0;
				m_current_state.m_matrix[2] = 0.0;
				m_current_state.m_matrix[3] = 1.0;
				m_current_state.m_matrix[4] = 0.0;
				m_current_state.m_matrix[5] = 0.0;
				m_current_state.m_line_matrix[0] = 0.0;
				m_current_state.m_line_matrix[1] = 0.0;
			}

			//内容是 cid 编码信息
			void executeTJ(std::vector<TJArrayElement>& tj_array)
			{
				if (!m_font)
				{
					return;
				}
				double tmp_matrix[6], cid_matrix[6];
				cid_matrix[0] = tmp_matrix[0] = m_current_state.m_ctm[0] * m_current_state.m_matrix[0] + m_current_state.m_ctm[2] * m_current_state.m_matrix[1];
				cid_matrix[1] = tmp_matrix[1] = m_current_state.m_ctm[1] * m_current_state.m_matrix[0] + m_current_state.m_ctm[3] * m_current_state.m_matrix[1];
				cid_matrix[2] = tmp_matrix[2] = m_current_state.m_ctm[0] * m_current_state.m_matrix[2] + m_current_state.m_ctm[2] * m_current_state.m_matrix[3];
				cid_matrix[3] = tmp_matrix[3] = m_current_state.m_ctm[1] * m_current_state.m_matrix[2] + m_current_state.m_ctm[3] * m_current_state.m_matrix[3];
				cid_matrix[4] = tmp_matrix[4] = m_current_state.m_ctm[4] + m_current_state.m_ctm[0] * m_current_state.m_matrix[4] + m_current_state.m_ctm[2] * m_current_state.m_matrix[5];
				cid_matrix[5] = tmp_matrix[5] = m_current_state.m_ctm[5] + m_current_state.m_ctm[1] * m_current_state.m_matrix[4] + m_current_state.m_ctm[3] * m_current_state.m_matrix[5];
				double scale = m_current_state.m_scaling / 100.0;
				double x_scale = (m_current_state.m_font_size * scale) / 1000.0;
				double char_space = m_current_state.m_char_space * scale;
				double word_space = m_font->m_multibyte ? 0 : m_current_state.m_word_space * scale;

				bool add_charspace = false;
				double str_width = 0.0, str_height = 0.0;
				double x_pos = 0.0, y_pos = 0.0;
				std::string output;
				bool first = true, last = false;
				double space_size = 1.5;	//default space size

				for (size_t i = 0; i < tj_array.size(); ++i)
				{
					if (tj_array[i].m_is_number)
					{
						double distance = (-tj_array[i].m_value * x_scale);
						m_current_state.m_line_matrix[0] += distance;
						if (distance >= space_size)
						{
							output += ' ';
						}
						add_charspace = true;
					}
					else
					{
						m_font->convertToCidString(tj_array[i].m_text);
						m_font->setCidString(tj_array[i].m_text);
						while (m_font->hasNextCid())
						{
							unsigned int cid = m_font->getNextCIDandDecode(output);
							if (i == tj_array.size() - 1 && !m_font->hasNextCid())
							{
								last = true;
							}
							if (add_charspace)
							{
								m_current_state.m_line_matrix[0] += char_space;
							}

							//update matrix for cid
							cid_matrix[4] = tmp_matrix[4] + tmp_matrix[0] * m_current_state.m_line_matrix[0] + tmp_matrix[2] * m_current_state.m_line_matrix[1];
							cid_matrix[5] = tmp_matrix[5] + tmp_matrix[1] * m_current_state.m_line_matrix[0] + tmp_matrix[3] * m_current_state.m_line_matrix[1];

							//get character size
							double cid_width = m_font->getWidth(cid);
							double advance = cid_width * scale * m_current_state.m_font_size;

							//calculate bounding box
							double tmp_y = m_current_state.m_rise + m_current_state.m_font_size * m_font->getDescent();
							double text_height = m_current_state.m_font_size * m_font->getHeight();
							double x0 = cid_matrix[4] + cid_matrix[2] * tmp_y;
							double y0 = cid_matrix[5] + cid_matrix[3] * tmp_y;
							double x1 = cid_matrix[4] + cid_matrix[2] * (tmp_y + text_height) + cid_matrix[0] * advance;
							double y1 = cid_matrix[5] + cid_matrix[3] * (tmp_y + text_height) + cid_matrix[1] * advance;
							if (first)
							{
								x_pos = x0 < x1 ? x0 : x1;
								y_pos = y0 < y1 ? y0 : y1;
								first = false;
							}
							if (last)
							{
								str_width = x0 > x1 ? x0 - x_pos : x1 - x_pos;
							}
							if (abs(y1 - y0) > str_height)
							{
								str_height = abs(y1 - y0);
							}
							if (y_pos > y1)
							{
								y_pos = y1;
							}
							if (y_pos > y0)
							{
								y_pos = y0;
							}

							m_current_state.m_line_matrix[0] += advance;
							if (output.length() > 0 && output[output.length() - 1] == ' ')
							{
								space_size = abs(x0 - x1) > 1.0 ? abs(x0 - x1) : 1.0;
								m_current_state.m_line_matrix[0] += word_space;
							}
							add_charspace = true;
						}
					}
				}
				TextElement new_element(x_pos, y_pos, str_width, str_height, space_size, output);
				m_text_elements.insert(new_element);
			}

			//显示字符串
			void executeTj(std::string& str)
			{
				std::vector<TJArrayElement> tj_array;
				tj_array.push_back(TJArrayElement());
				tj_array[0].m_is_number = false;
				tj_array[0].m_text = str;
				executeTJ(tj_array);
				if (!m_font)
				{
					return;
				}
			}

			void getText(std::string& output)
			{
				#warning TODO: For now we are sorting strings using their x and y positions. Maybe we should implement better algorithms.
				std::multiset<TextElement>::iterator it = m_text_elements.begin();
				bool first = true;
				double x_end, y, x_begin;
				while (it != m_text_elements.end())
				{
					//some minimum values for new line and space. Calculated experimentally
					double new_line_size = (*it).m_height * 0.75 < 4.0 ? 4.0 : (*it).m_height * 0.75;
					double space_size = (*it).m_space_size < 1.0 ? 1.0 : (*it).m_space_size;
					space_size *= 2.0;	//decrease number of spaces
					if (!first)
					{
						double dx = (*it).m_x - x_end;
						double dy = y - ((*it).m_y + (*it).m_height / 2);
						if (dy >= new_line_size)
						{
							while (dy >= new_line_size)
							{
								output += '\n';
								dy -= new_line_size;
							}
						}
						else if ((*it).m_x < x_begin)	//force new line
						{
							output += '\n';
						}
						else if (dx >= space_size)
						{
							while (dx >= space_size)
							{
								dx -= space_size;
								output += ' ';
							}
						}
					}
					output += (*it).m_text;
					first = false;
					x_begin = (*it).m_x;
					x_end = x_begin + (*it).m_width;
					y = (*it).m_y + (*it).m_height / 2;
					++it;
				}
			}

			PageText()
			{
				m_font = NULL;
			}
		};

		typedef std::map<std::string, Font*> FontsByNames;
		std::vector<FontsByNames> m_fonts_for_pages;
		std::map<unsigned int, Font*> m_fonts_by_indexes;
		std::vector<Font*> m_fonts;
		std::vector<PDFReader::PDFDictionary*> m_pages;
		std::vector<PDFReader::PDFDictionary*> m_pages_resources;

		~PDFContent()
		{
			for (size_t i = 0; i < m_fonts.size(); ++i)
				delete m_fonts[i];
			m_fonts.clear();
		}
	};

	bool m_error;
	DataStream* m_data_stream;
	bool m_verbose_logging;
	std::ostream* m_log_stream;
	PDFContent m_pdf_content;

	//获取字体信息（CMap）或者是 Do 的 cid 内容信息(现在没有实现Do)
	void parseFonts()
	{
		/*
		 * page 的字体信息，多个也的字体名称可能相同，但是字体名称对应的对象号有可能不同，
		 * 所以每个 page 有自己单独的字体集合
		 */
		for (size_t i = 0; i < m_pdf_content.m_pages_resources.size(); ++i)
		{
			m_pdf_content.m_fonts_for_pages.push_back(PDFContent::FontsByNames());
			PDFReader::PDFDictionary* res_dictionary = m_pdf_content.m_pages_resources[i];
			if (!res_dictionary)
			{
				continue;
			}

			//获取字体的 dicctionary 信息：包含了 CMap 信息或者是 cid 内容信息(Do 的情况下)
			PDFReader::PDFDictionary* fonts_dictionary = res_dictionary->getObjAsDictionary("Font");
            //page 的字体集合
			PDFContent::FontsByNames* fonts_for_page = &m_pdf_content.m_fonts_for_pages[i];
			if (fonts_dictionary)
			{
				std::map<std::string, PDFReader::PDFObject*>::iterator it = fonts_dictionary->m_objects.begin();
				std::map<std::string, PDFReader::PDFObject*>::iterator end = fonts_dictionary->m_objects.end();
				while (it != end)
				{
					//字体名称
					std::string font_code = it->first;

					//根据字体名称，获取字体对象的信息: /Font <<>>
					PDFReader::PDFDictionary* font_dictionary = fonts_dictionary->getObjAsDictionary(font_code);
					if (font_dictionary)
					{
						PDFContent::Font* font = NULL;

						//标识是一个新字体对象
						bool is_new_font = false;

						//确保不创建相同的字体对象到字体集合中
						//make sure we wont create the same instance of Font twice.
						try
						{

							//这里根据字体名称获取到间接对象:  例: /Font <</F1 3 0 R>> 中根据 F1 得到间接对象 3 0 R
							PDFReader::PDFReferenceCall* font_dictionary_ref = fonts_dictionary->getObjAsReferenceCall(font_code);
							if (font_dictionary_ref)
							{
								//查找字体对象是否已经存在字体集合中，find方法：找不到返回的迭代器的 end
								if (m_pdf_content.m_fonts_by_indexes.find(font_dictionary_ref->m_index) == m_pdf_content.m_fonts_by_indexes.end())
								{
									//字体对象不在字体 map 中
									is_new_font = true;
									font = new PDFContent::Font;
									font->m_font_dictionary = font_dictionary;
									(*fonts_for_page)[font_code] = font;

									//以对象号为索引值生成字体对象集合
									m_pdf_content.m_fonts_by_indexes[font_dictionary_ref->m_index] = font;
									m_pdf_content.m_fonts.push_back(font);

//									std::cout << "font name is1:" + font_code << std::endl;
//									std::cout << "m_index is1:" << font_dictionary_ref->m_index << std::endl;
								}
								else
								{
//									std::cout << "font name is2:" + font_code << std::endl;
									(*fonts_for_page)[font_code] = m_pdf_content.m_fonts_by_indexes[font_dictionary_ref->m_index];
								}
							}
							else
							{
                                //根据 Font 中的字体名称找不到对应的对象, 例: /Font <</F1 3 0 R /F3>> 有这种情况？
//								std::cout << "font name is3:" + font_code << std::endl;
								is_new_font = true;
								font = new PDFContent::Font;
								font->m_font_dictionary = font_dictionary;
								(*fonts_for_page)[font_code] = font;
								m_pdf_content.m_fonts.push_back(font);
							}
						}
						catch (std::bad_alloc& ba)
						{
							if (font)
							{
								delete font;
							}
							font = NULL;
							throw Exception("Bad alloc");
						}

						if (is_new_font)
						{
							//解析字体获取 CMap
							getFontEncoding(*font);
							getFontInfo(*font);
						}
					}
					++it;
				}
			}
		}
	}

	void getFontWidths(PDFContent::Font& font)
	{
		if (font.m_font_metrics.m_first_char > font.m_font_metrics.m_last_char)
			font.m_font_metrics.m_last_char = font.m_font_metrics.m_first_char;	//throw an exception?
		PDFReader::PDFArray* widths = font.m_font_dictionary->getObjAsArray("Widths");
		if (widths)
		{
			for (size_t i = 0; i < widths->Size(); ++i)
				font.m_font_metrics.m_widths.push_back(widths->getValAsInteger(i, 0));
		}
	}

	void loadFontDescriptor(PDFContent::Font& font)
	{
		if (font.m_font_descriptor)
		{
			font.m_font_metrics.m_font_name = font.m_font_descriptor->getValAsString("FontName", "unknown");
			font.m_font_metrics.m_flags = font.m_font_descriptor->getValAsInteger("Flags", 0);
			font.m_font_metrics.m_ascent = font.m_font_descriptor->getValAsDouble("Ascent", 0.0);
			font.m_font_metrics.m_descent = font.m_font_descriptor->getValAsDouble("Descent", 0.0);
			font.m_font_metrics.m_italic_angle = font.m_font_descriptor->getValAsDouble("ItalicAngle", 0.0);
			font.m_font_metrics.m_x_height = font.m_font_descriptor->getValAsDouble("XHeight", 0.0);
			font.m_font_metrics.m_missing_width = font.m_font_descriptor->getValAsInteger("MissingWidth", 0);
			font.m_font_metrics.m_leading = font.m_font_descriptor->getValAsDouble("Leading", 0.0);
			font.m_font_metrics.m_cap_height = font.m_font_descriptor->getValAsDouble("CapHeight", 0.0);
			PDFReader::PDFArray* pdf_bbox = font.m_font_descriptor->getObjAsArray("FontBBox");
			if (!pdf_bbox)
				pdf_bbox = font.m_font_dictionary->getObjAsArray("FontBBox");
			if (pdf_bbox && pdf_bbox->Size() == 4)
			{
				for (size_t i = 0; i < 4; ++i)
					font.m_font_metrics.m_font_bbox[i] = pdf_bbox->getValAsDouble(i, font.m_font_metrics.m_font_bbox[i]);
			}
		}
	}

	void getFontInfo(PDFContent::Font& font)
	{
		font.m_font_type = font.m_font_dictionary->getValAsString("Subtype", "Type1");

		if (font.m_font_type != "TrueType" && font.m_font_type != "Type0" && font.m_font_type != "Type3" && font.m_font_type != "Type1" && font.m_font_type != "MMType1")
			font.m_font_type = "Type1";

		font.m_font_descriptor = font.m_font_dictionary->getObjAsDictionary("FontDescriptor");
		font.m_base_font = font.m_font_dictionary->getValAsString("BaseFont", "unknown");

		if (font.m_font_type == "Type0")
		{
			font.m_multibyte = true;
			PDFReader::PDFArray* descendant_fonts = font.m_font_dictionary->getObjAsArray("DescendantFonts");
			if (descendant_fonts && descendant_fonts->Size() > 0)	//according to the documentation, only one value is allowed
			{
				PDFReader::PDFDictionary* descendant_font_dictionary = descendant_fonts->getObjAsDictionary(0);
				if (descendant_font_dictionary)
				{
					font.m_font_descriptor = descendant_font_dictionary->getObjAsDictionary("FontDescriptor");
					font.m_font_metrics.m_missing_width = descendant_font_dictionary->getValAsInteger("DW", 1000);
					PDFReader::PDFArray* widths_array = descendant_font_dictionary->getObjAsArray("W");
					if (widths_array)
					{
						unsigned int got_values = 0, first_value, to_range;
						for (size_t i = 0; i < widths_array->Size(); ++i)
						{
							PDFReader::PDFObject* obj = (*widths_array)[i];
							if (obj->isArray() && got_values == 1) //INDEX [VAL1 Val2 ... VALN]
							{
								PDFReader::PDFArray* subwidth_array = obj->getArray();
								if (first_value > font.m_font_metrics.m_widths.size())
									font.m_font_metrics.m_widths.resize(first_value, font.m_font_metrics.m_missing_width);
								for (size_t j = 0; j < subwidth_array->Size(); ++j)
								{
									font.m_font_metrics.m_widths.push_back(subwidth_array->getValAsInteger(j, 0));
								}
								got_values = 0;
							}
							else if (obj->isNumericInteger())
							{
								++got_values;
								if (got_values == 1)
									first_value = obj->getNumericInteger()->m_value;
								else if (got_values == 2)
									to_range = obj->getNumericInteger()->m_value;
								else if (got_values == 3) // [FROM TO VAL]
								{
									got_values = 0;
									if (to_range < first_value)
										to_range = first_value;	//throw an exception?
									if (to_range >= font.m_font_metrics.m_widths.size())
										font.m_font_metrics.m_widths.resize(to_range + 1, font.m_font_metrics.m_missing_width);
									unsigned int calculated_value = obj->getNumericInteger()->m_value;
									for (size_t j = first_value; j <= to_range; ++j)
										font.m_font_metrics.m_widths[j] = calculated_value;
								}
							}
						}
					}
					loadFontDescriptor(font);
					#warning TODO: Those fonts can be vertical. PDF parser should support that feature
				}
			}
		}
		else if (font.m_font_type == "Type3")
		{
			font.m_font_metrics.m_first_char = font.m_font_dictionary->getValAsInteger("FirstChar", 0);
			font.m_font_metrics.m_last_char = font.m_font_dictionary->getValAsInteger("LastChar", 0);
			if (!font.m_font_descriptor)
			{
				PDFReader::PDFArray* pdf_bbox = font.m_font_dictionary->getObjAsArray("FontBBox");
				if (pdf_bbox && pdf_bbox->Size() == 4)
				{
					for (size_t i = 0; i < 4; ++i)
						font.m_font_metrics.m_font_bbox[i] = pdf_bbox->getValAsDouble(i, font.m_font_metrics.m_font_bbox[i]);
				}
			}
			else
				loadFontDescriptor(font);
			font.m_font_metrics.m_ascent = font.m_font_metrics.m_font_bbox[3];
			font.m_font_metrics.m_descent = font.m_font_metrics.m_font_bbox[1];
			PDFReader::PDFArray* pdf_font_matrix = font.m_font_dictionary->getObjAsArray("FontMatrix");
			if (pdf_font_matrix && pdf_font_matrix->Size() == 6)
			{
				for (size_t i = 0; i < 6; ++i)
					font.m_font_metrics.m_font_matrix[i] = pdf_font_matrix->getValAsDouble(i, font.m_font_metrics.m_font_matrix[i]);
			}
			font.m_font_metrics.m_vscale = font.m_font_metrics.m_font_matrix[1] + font.m_font_metrics.m_font_matrix[3];
			font.m_font_metrics.m_hscale = font.m_font_metrics.m_font_matrix[0] + font.m_font_metrics.m_font_matrix[2];
		}
		else
		{
			if (PDFContent::pdf_font_metrics_map.find(font.m_base_font) != PDFContent::pdf_font_metrics_map.end())
				font.m_font_metrics = PDFContent::pdf_font_metrics_map[font.m_base_font];
			else
			{
				font.m_font_metrics.m_first_char = font.m_font_dictionary->getValAsInteger("FirstChar", 0);
				font.m_font_metrics.m_last_char = font.m_font_dictionary->getValAsInteger("LastChar", 255);
				getFontWidths(font);
				loadFontDescriptor(font);
			}
		}
	}

	//获取 CMap 信息
	void getFontEncoding(PDFContent::Font& font)
	{
		//Check if this font contain "ToUnicode" stream.
		PDFReader::PDFStream* to_unicode_stream = font.m_font_dictionary->getObjAsStream("ToUnicode");
		if (to_unicode_stream)
		{
			parseCMap(to_unicode_stream->getIterator(), font.m_cmap);
		}
		//check if "Encoding" is defined. It can be a name...
		PDFReader::PDFName* encoding_name = font.m_font_dictionary->getObjAsName("Encoding");
		if (encoding_name)
		{
			font.m_font_encoding = (*encoding_name)();
			PredefinedSimpleEncodings::const_iterator it = m_pdf_predefined_simple_encodings.find(font.m_font_encoding);
			if (it != m_pdf_predefined_simple_encodings.end())
			{
				font.m_predefined_simple_encoding = true;
				font.m_simple_encoding_table = (unsigned int*)it->second;
			}
			//In that case, Encoding may be something more "complicated" like 90ms-RKSJ-H
			else
			{
				CIDToUnicode::const_iterator it = m_pdf_cid_to_unicode.find(font.m_font_encoding);
				if (it != m_pdf_cid_to_unicode.end())
				{
					font.m_predefined_cmap = true;
					parsePredefinedCMap(font, it->second);
				}
			}
		}
		//or dictionary
		PDFReader::PDFDictionary* encoding_dict = font.m_font_dictionary->getObjAsDictionary("Encoding");
		if (encoding_dict)
		{
			font.m_predefined_simple_encoding = true;
			font.m_own_simple_encoding_table = true;
			font.m_simple_encoding_table = new unsigned int[256];
			PDFReader::PDFName* base_encoding_name = encoding_dict->getObjAsName("BaseEncoding");
			const unsigned int* source_table;
			if (base_encoding_name)
			{
				PredefinedSimpleEncodings::const_iterator it = m_pdf_predefined_simple_encodings.find((*base_encoding_name)());
				if (it != m_pdf_predefined_simple_encodings.end())
					source_table = it->second;
				else
					source_table = StandardEncodingUtf8;
			}
			else
				source_table = StandardEncodingUtf8;
			for (int j = 0; j < 256; ++j)
				font.m_simple_encoding_table[j] = source_table[j];
			size_t replacements = 0;
			PDFReader::PDFArray* differences = encoding_dict->getObjAsArray("Differences");
			if (differences)
			{
				for (size_t j = 0; j < differences->Size(); ++j)
				{
					PDFReader::PDFNumericInteger* index = differences->getObjAsNumericInteger(j);
					if (index)
					{
						replacements = (*index)();
						if (replacements > 255)
							replacements = 0;
					}
					else
					{
						PDFReader::PDFName* char_name = differences->getObjAsName(j);
						if (char_name)
						{
							CharacterNames::const_iterator it = m_pdf_character_names.find((*char_name)());
							if (it != m_pdf_character_names.end())
							{
								font.m_simple_encoding_table[replacements] = it->second;
								++replacements;
								if (replacements > 255)
									replacements = 0;
							}
						}
					}
				}
			}
		}
	}

	void parsePredefinedCMap(PDFContent::Font& font, const std::string& cid_to_unicode_cmap)
	{
		try
		{
			#ifdef WIN32
			std::string cmap_to_cid_file_name = "resources\\" + font.m_font_encoding;
			#else
			std::string cmap_to_cid_file_name = "resources/" + font.m_font_encoding;
			#endif
			bool next_cmap_exist = true;
			PDFContent::CMap* current_cmap = &font.m_to_cid_cmap;
			while (next_cmap_exist)
			{
				PDFReader::PDFStream::PDFStreamIterator to_cid_stream_iterator;
				FileStream file_stream(cmap_to_cid_file_name);
				if (!file_stream.open())
					throw Exception("Cannot open file: " + cmap_to_cid_file_name);
				std::vector<char> buffer(file_stream.size() + 2);
				if (!file_stream.read(&buffer[1], 1, buffer.size() - 2))
					throw Exception("Cannot read from file: " + cmap_to_cid_file_name);
				file_stream.close();

				std::string last_name;
				std::string min, max;
				unsigned int codepoint;
				bool is_not_def = false;
				bool in_cid_range = false;
				bool in_cid_char = false;
				bool reading_min = false;
				bool reading_max = false;
				buffer[0] = '[';
				buffer[buffer.size() - 1] = ']';
				to_cid_stream_iterator.init(&buffer[0], buffer.size());
				to_cid_stream_iterator.levelDown();
				next_cmap_exist = false;

				while (to_cid_stream_iterator.hasNext())
				{
					to_cid_stream_iterator.getNextElement();
					switch (to_cid_stream_iterator.getType())
					{
						case PDFReader::name:
						{
							last_name = std::string(to_cid_stream_iterator.getData() + 1, to_cid_stream_iterator.getDataLength() - 1);
							break;
						}
						case PDFReader::string:
						{
							if (reading_min)
							{
								to_cid_stream_iterator.toHexString(min);
								reading_min = false;
								if (in_cid_range)
									reading_max = true;
							}
							else if (reading_max)
							{
								to_cid_stream_iterator.toHexString(max);
								reading_max = false;
							}
							break;
						}
						case PDFReader::int_numeric:
						{
							codepoint = to_cid_stream_iterator.toLong();
							if (in_cid_range)
							{
								reading_min = true;
								current_cmap->addCodeRange(min, max, codepoint, "", is_not_def);
							}
							else if (in_cid_char)
							{
								reading_min = true;
								current_cmap->addCodeRange(min, min, codepoint, "", is_not_def);
							}
							break;
						}
						case PDFReader::operator_obj:
						{
							std::string pdf_operator = std::string(to_cid_stream_iterator.getData(), to_cid_stream_iterator.getDataLength());
							switch (PDFReader::getOperatorCode(pdf_operator))
							{
								case PDFReader::usecmap:
								{
									#ifdef WIN32
									cmap_to_cid_file_name = "resources\\" + last_name;
									#else
									cmap_to_cid_file_name = "resources/" + last_name;
									#endif
									next_cmap_exist = true;
									break;
								}
								case PDFReader::begincidrange:
								{
									reading_min = true;
									in_cid_range = true;
									break;
								}
								case PDFReader::endcidrange:
								{
									reading_min = false;
									in_cid_range = false;
									break;
								}
								case PDFReader::begincidchar:
								{
									reading_min = true;
									in_cid_char = true;
									break;
								}
								case PDFReader::endcidchar:
								{
									reading_min = false;
									in_cid_char = false;
									break;
								}
								case PDFReader::beginnotdefrange:
								{
									reading_min = true;
									is_not_def = true;
									in_cid_range = true;
									break;
								}
								case PDFReader::endnotdefrange:
								{
									reading_min = false;
									is_not_def = false;
									in_cid_range = false;
									break;
								}
								case PDFReader::beginnotdefchar:
								{
									reading_min = true;
									is_not_def = true;
									in_cid_char = true;
									break;
								}
								case PDFReader::endnotdefchar:
								{
									reading_min = false;
									is_not_def = false;
									in_cid_char = false;
									break;
								}
							}
							break;
						}
					}
				}
				if (next_cmap_exist)
				{
					current_cmap->m_ready = true;
					current_cmap->m_parent = new PDFContent::CMap;
					current_cmap = current_cmap->m_parent;
				}
			}

			PDFReader::PDFStream::PDFStreamIterator to_unicode_stream_iterator;
			#ifdef WIN32
			FileStream file_stream("resources\\" + cid_to_unicode_cmap);
			#else
			FileStream file_stream("resources/" + cid_to_unicode_cmap);
			#endif
			if (!file_stream.open())
				throw Exception("Cannot open file: " + cid_to_unicode_cmap);
			std::vector<char> buffer(file_stream.size() + 2);
			if (!file_stream.read(&buffer[1], 1, buffer.size() - 2))
				throw Exception("Cannot read from file: " + cmap_to_cid_file_name);
			file_stream.close();
			buffer[0] = '[';
			buffer[buffer.size() - 1] = ']';
			to_unicode_stream_iterator.init(&buffer[0], buffer.size());
			parseCMap(to_unicode_stream_iterator, font.m_cmap);
		}
		catch (Exception& ex)
		{
			ex.appendError("Error while parsing predefined CMap: " + font.m_font_encoding);
			throw;
		}
	}

	//解析 CMap 信息，生成一个字体数
	void parseCMap(PDFReader::PDFStream::PDFStreamIterator& iterator, PDFContent::CMap& cmap)
	{
		iterator.backToRoot();
		iterator.levelDown();

		bool in_bf_range = false;
		std::string min;
		std::string max;
		bool reading_min = true;
		bool reading_max = false;
		bool reading_range = false;

		bool in_bf_char = false;
		bool bf_char_next_first = true;
		std::string bf_char;
		std::string bf_code;

		bool in_not_def = false;

		while (iterator.hasNext())
		{
			iterator.getNextElement();
			switch (iterator.getType())
			{
				case PDFReader::string:
				{
					if (in_bf_range)
					{
						if (reading_min)
						{
							reading_min = false;
							reading_max = true;
							iterator.toHexString(min);
						}
						else if (reading_max)
						{
							reading_max = false;
							reading_range = true;
							iterator.toHexString(max);
						}
						else if (reading_range)
						{
							reading_range = false;
							reading_min = true;
							std::string range;
							iterator.toHexString(range);
							cmap.addCodeRange(min, max, hex_string_to_uint(range.c_str(), range.length()), utf16be_to_utf8(range), in_not_def);
						}
					}
					else if (in_bf_char)
					{
						if (bf_char_next_first)
						{
							bf_char_next_first = false;
							iterator.toHexString(bf_char);
						}
						else
						{
							bf_char_next_first = true;
							iterator.toHexString(bf_code);
							cmap.addCodeRange(bf_char, bf_char, 0, utf16be_to_utf8(bf_code), in_not_def);	//first code point doesnt matter here
						}
					}
					break;
				}
				case PDFReader::operator_obj:
				{
					std::string pdf_operator = iterator.toPlainText();
					switch (PDFReader::getOperatorCode(pdf_operator))
					{
						case PDFReader::beginbfrange:
						{
							in_bf_range = true;
							break;
						}
						case PDFReader::endbfrange:
						{
							in_bf_range = false;
							break;
						}
						case PDFReader::beginbfchar:
						{
							in_bf_char = true;
							break;
						}
						case PDFReader::endbfchar:
						{
							in_bf_char = false;
							break;
						}
						case PDFReader::beginnotdefrange:
						{
							in_bf_range = true;
							in_not_def = true;
							break;
						}
						case PDFReader::endnotdefrange:
						{
							in_bf_range = false;
							in_not_def = false;
							break;
						}
						case PDFReader::beginnotdefchar:
						{
							in_not_def = true;
							in_bf_char = true;
							break;
						}
						case PDFReader::endnotdefchar:
						{
							in_not_def = false;
							in_bf_char = false;
							break;
						}
					}
					break;
				}
				case PDFReader::array:
				{
					if (in_bf_range && reading_range)
					{
						reading_range = false;
						reading_min = true;
						iterator.levelDown();
						while (iterator.hasNext())
						{
							iterator.getNextElement();
							if (iterator.getType() == PDFReader::string)
							{
								std::string range;
								iterator.toHexString(range);
								cmap.addCodeRange(min, min, 0, utf16be_to_utf8(range), in_not_def);	//first code point doesnt matter here
								increment_hex_string(min);
							}
						}
						iterator.levelUp();
					}
					break;
				}
			}
		}
		cmap.m_ready = true;
	}

	//获取 Pages 树形结构
	void parsePagesTree(PDFReader& pdf_reader)
	{
		PDFReader::PDFDictionary* root_dictionary = pdf_reader.m_root_dictionary;
		PDFReader::PDFDictionary* pages_dictionary = root_dictionary->getObjAsDictionary("Pages");
		if (!pages_dictionary)
		{
			throw Exception("Root dictionary: missing Pages dictionary");
		}

		//解析各个 page 内容
		parsePageNode(*pages_dictionary, NULL);
	}

	//解析各个 page 内容
	void parsePageNode(PDFReader::PDFDictionary& page_node_dictionary, PDFReader::PDFDictionary* resources_dictionary)
	{
		// Resources 中包含了 cmap 字体信息
		//check resources
		PDFReader::PDFDictionary* overrided_resources = page_node_dictionary.getObjAsDictionary("Resources");
		if (overrided_resources)
		{
			resources_dictionary = overrided_resources;
		}

		PDFReader::PDFName* type = page_node_dictionary.getObjAsName("Type");
		if (!type)
		{
			throw Exception("Error while parsing page node: missing Type entry");
		}
		if ((*type)() == "Pages")	//another node with childs
		{
			// 从 Pages 获取的有能是其他的多个 Pages，这样就需要递归到树形结构的下一层，得到 Page
			PDFReader::PDFArray* array = page_node_dictionary.getObjAsArray("Kids");
			if (array)
			{
				for (size_t i = 0; i < array->Size(); ++i)
				{
					PDFReader::PDFDictionary* child = array->getObjAsDictionary(i);
					if (child)
					{
						parsePageNode(*child, resources_dictionary);
					}
				}
			}
		}
		else	//page object without any childs (so this is a page)
		{
			// page 对象是没有子节点的
			m_pdf_content.m_pages.push_back(&page_node_dictionary);
			m_pdf_content.m_pages_resources.push_back(resources_dictionary);
		}
	}

	//获取文本内容
	void parseText(std::string& text)
	{
		for (size_t i = 0; i < m_pdf_content.m_pages.size(); ++i)
		{
			try
			{
				//当前 page 的字体信息
				PDFContent::FontsByNames* fonts_for_page = &m_pdf_content.m_fonts_for_pages[i];
				std::vector<PDFReader::PDFStream*> contents;

				PDFReader::PDFDictionary* page_obj = m_pdf_content.m_pages[i];
				//cid内容标识关键字: Contents
				PDFReader::PDFArray* page_contents_array = page_obj->getObjAsArray("Contents");
				if (page_contents_array)
				{
					//是一个cid内容数组对象
					for (size_t j = 0; j < page_contents_array->Size(); ++j)
					{
						contents.push_back(page_contents_array->getObjAsStream(j));
					}
				}
				else
				{
					//直接就是cid内容信息
					contents.push_back(page_obj->getObjAsStream("Contents"));
				}

				//解析 cid 内容，获取cid编码信息
				PDFContent::PageText page_text;
				for (size_t j = 0; j < contents.size(); ++j)
				{
					PDFReader::PDFStream* page_content = contents[j];
					if (page_content)
					{
						PDFReader::PDFStream::PDFStreamIterator iterator = page_content->getIterator();
						std::string last_name;
						std::string last_string;
						bool in_text = false;
						std::vector<double> last_numbers;
						std::vector<PDFContent::TJArrayElement> tj_array;
						tj_array.reserve(100);

						iterator.backToRoot();
						iterator.levelDown();
						while (iterator.hasNext())
						{
							iterator.getNextElement();
							switch (iterator.getType())
							{
								case PDFReader::operator_obj:
								{
									std::string pdf_operator = iterator.toPlainText();
									switch (PDFReader::getOperatorCode(pdf_operator))
									{
										case PDFReader::ET:
										{
											in_text = false;
											break;
										}
										case PDFReader::Tm:
										{
											if (!in_text || last_numbers.size() < 6)
											{
												break;
											}
											page_text.executeTm(last_numbers);
											break;
										}
										case PDFReader::Td:
										{
											if (!in_text || last_numbers.size() < 2)
											{
												break;
											}
											page_text.executeTd(last_numbers);
											break;
										}
										case PDFReader::T_star:
										{
											if (!in_text)
											{
												break;
											}
											page_text.executeTstar();
											break;
										}
										case PDFReader::TD:
										{
											if (!in_text || last_numbers.size() < 2)
											{
												break;
											}
											page_text.executeTD(last_numbers);
											break;
										}
										case PDFReader::TJ:
										{
											if (!in_text)
											{
												break;
											}
											page_text.executeTJ(tj_array);
											tj_array.clear();
											break;
										}
										case PDFReader::Tj:
										{
											if (!in_text)
											{
												break;
											}
											page_text.executeTj(last_string);
											break;
										}
										case PDFReader::Tw:
										{
											if (!in_text || last_numbers.size() < 1)
											{
												break;
											}
											page_text.executeTw(last_numbers);
											break;
										}
										case PDFReader::Tc:
										{
											if (!in_text || last_numbers.size() < 1)
											{
												break;
											}
											page_text.executeTc(last_numbers);
											break;
										}
										case PDFReader::Ts:
										{
											if (!in_text || last_numbers.size() < 1)
											{
												break;
											}
											page_text.executeTs(last_numbers);
											break;
										}
										case PDFReader::quote:
										{
											if (!in_text)
											{
												break;
											}
											page_text.executeQuote(last_string);
											break;
										}
										case PDFReader::double_quote:
										{
											if (!in_text || last_numbers.size() < 2)
											{
												break;
											}
											page_text.executeDoubleQuote(last_string, last_numbers);
											break;
										}
										case PDFReader::Tf:
										{
											if (!in_text && last_numbers.size() < 1)
											{
												break;
											}
											if (fonts_for_page->find(last_name) != fonts_for_page->end())
											{
												page_text.executeTf(last_numbers, *(*fonts_for_page)[last_name]);
											}
											break;
										}
										case PDFReader::BT:
										{
											in_text = true;
											page_text.executeBT();
											break;
										}
										case PDFReader::TL:
										{
											if (last_numbers.size() < 1)
												break;
											page_text.executeTL(last_numbers);
											break;
										}
										case PDFReader::TZ:
										{
											if (last_numbers.size() < 1)
												break;
											page_text.executeTZ(last_numbers);
											break;
										}
										case PDFReader::cm:
										{
											if (last_numbers.size() < 6)
												break;
											page_text.executeCm(last_numbers);
											break;
										}
										case PDFReader::Q:
										{
											page_text.popState();
											break;
										}
										case PDFReader::q:
										{
											page_text.pushState();
											break;
										}
									}
									last_numbers.clear();
									break;
								}
								case PDFReader::string:
								{
									if (in_text)
									{
										iterator.toHexString(last_string);
									}
									break;
								}
								case PDFReader::array:
								{
									if (in_text)
									{
										tj_array.clear();
										iterator.levelDown();
										while (iterator.hasNext())
										{
											iterator.getNextElement();
											if (iterator.getType() == PDFReader::string)
											{
												tj_array.push_back(PDFContent::TJArrayElement());
												tj_array[tj_array.size() - 1].m_is_number = false;
												iterator.toHexString(tj_array[tj_array.size() - 1].m_text);
											}
											else if (iterator.getType() == PDFReader::int_numeric || iterator.getType() == PDFReader::float_numeric)
											{
												tj_array.push_back(PDFContent::TJArrayElement());
												tj_array[tj_array.size() - 1].m_is_number = true;
												tj_array[tj_array.size() - 1].m_value = iterator.toDouble();
											}
										}
										iterator.levelUp();
									}
									break;
								}
								case PDFReader::name:
								{
									if (in_text)
									{
										last_name = std::string(iterator.getData() + 1, iterator.getDataLength() - 1);
									}
									break;
								}
								case PDFReader::int_numeric:
								case PDFReader::float_numeric:
								{
									last_numbers.push_back(iterator.toDouble());
									break;
								}
							}
						}
						iterator.levelUp();
					}
				}
				page_text.getText(text);
				text += "\n\n";
			}
			catch (Exception& ex)
			{
				ex.appendError("Error while parsing page number: " + uint_to_string(i));
				throw;
			}
		}
	}

	void parseMetadata(PDFReader& pdf_reader, Metadata& metadata)
	{
		//according to PDF specification, we can extract: author, creation date and last modification date.
		//LastModifyBy is not possible. Other metadata information available in PDF are not supported in Metadata class.
		bool got_author = false;
		bool got_creation_date = false;
		bool got_modify_date = false;
		PDFReader::PDFDictionary* info = pdf_reader.m_info;
		if (info)
		{
			PDFReader::PDFString* author = info->getObjAsString("Author");
			if (author)
			{
				got_author = true;
				author->ConvertToLiteral();
				metadata.setAuthor(author->m_value);
			}
			PDFReader::PDFString* creation_date = info->getObjAsString("CreationDate");
			if (creation_date)
			{
				got_creation_date = true;
				creation_date->ConvertToLiteral();
				tm creation_date_tm;
				int offset = 0;
				std::string creation_date_str = (*creation_date)();
				while (creation_date_str.length() > offset && (creation_date_str[offset] < '0' || creation_date_str[offset] > '9'))
				{
					++offset;
				}
				creation_date_str.erase(0, offset);
				parsePDFDate(creation_date_tm, creation_date_str);
				metadata.setCreationDate(creation_date_tm);
			}
			PDFReader::PDFString* modify_date = info->getObjAsString("ModDate");
			if (modify_date)
			{
				got_modify_date = true;
				modify_date->ConvertToLiteral();
				tm modify_date_tm;
				int offset = 0;
				std::string mod_date_str = (*modify_date)();
				while (mod_date_str.length() > offset && (mod_date_str[offset] < '0' || mod_date_str[offset] > '9'))
				{
					++offset;
				}
				mod_date_str.erase(0, offset);
				parsePDFDate(modify_date_tm, mod_date_str);
				metadata.setLastModificationDate(modify_date_tm);
			}
		}

		if (!got_author || !got_creation_date || !got_modify_date)
		{
			PDFReader::PDFStream* metadata_stream = pdf_reader.m_metadata;
			if (metadata_stream)
			{
				size_t pos;
				std::string content = metadata_stream->getIterator().toPlainText();
				if (!got_author)
				{
					//possibilities I have found: Authour="name", Author='name', Author>name<.
					if ((pos = content.find("Author")) != std::string::npos)
					{
						pos += 7;
						std::string author;
						while (pos < content.length() && content[pos] != '\"' && content[pos] != '\'' && content[pos] != '<')
						{
							author += content[pos];
						}
						metadata.setAuthor(author);
					}
				}
				if (!got_creation_date)
				{
					size_t entry_len;
					if ((pos = content.find("CreationDate")) == std::string::npos)
					{
						pos = content.find("CreateDate");
						entry_len = 10;
					}
					else
					{
						entry_len = 12;
					}
					if (pos != std::string::npos)
					{
						pos += entry_len;
						std::string creation_date;
						while (pos < content.length() && content[pos] != '\"' && content[pos] != '\'' && content[pos] != '<')
						{
							creation_date += content[pos];
						}
						tm creation_date_tm;
						if (string_to_date(creation_date, creation_date_tm))
						{
							metadata.setCreationDate(creation_date_tm);
						}
					}
				}
				if (!got_modify_date)
				{
					size_t entry_len;
					if ((pos = content.find("ModifyDate")) == std::string::npos)
					{
						pos = content.find("ModDate");
						entry_len = 7;
					}
					else
					{
						entry_len = 10;
					}
					if (pos != std::string::npos)
					{
						pos += entry_len;
						std::string modify_date;
						while (pos < content.length() && content[pos] != '\"' && content[pos] != '\'' && content[pos] != '<')
						{
							modify_date += content[pos];
						}
						tm modify_date_tm;
						if (string_to_date(modify_date, modify_date_tm))
						{
							metadata.setLastModificationDate(modify_date_tm);
						}
					}
				}
			}
		}
		metadata.setPageCount(m_pdf_content.m_pages.size());
	}
};

PDFParser::Implementation::CIDToUnicode PDFParser::Implementation::m_pdf_cid_to_unicode;
PDFParser::Implementation::PDFContent::FontMetricsMap PDFParser::Implementation::PDFContent::pdf_font_metrics_map;
PDFParser::Implementation::CharacterNames PDFParser::Implementation::m_pdf_character_names;
PDFParser::Implementation::PredefinedSimpleEncodings PDFParser::Implementation::m_pdf_predefined_simple_encodings;
PDFParser::Implementation::PDFReader::CompressionCodes PDFParser::Implementation::PDFReader::m_compression_codes;
PDFParser::Implementation::PDFReader::OperatorCodes PDFParser::Implementation::PDFReader::m_operator_codes;

PDFParser::PDFParser(const std::string& file_name)
{
	impl = NULL;
	try
	{
		impl = new Implementation;
		impl->m_data_stream = NULL;
		impl->m_data_stream = new FileStream(file_name);
		impl->m_error = false;
		impl->m_verbose_logging = false;
		impl->m_log_stream = &std::cerr;
	}
	catch (std::bad_alloc& ba)
	{
		if (impl)
		{
			if (impl->m_data_stream)
				delete impl->m_data_stream;
			delete impl;
		}
		throw;
	}
}

PDFParser::PDFParser(const char* buffer, size_t size)
{
	impl = NULL;
	try
	{
		impl = new Implementation;
		impl->m_data_stream = NULL;
		impl->m_data_stream = new BufferStream(buffer, size);
		impl->m_error = false;
		impl->m_verbose_logging = false;
		impl->m_log_stream = &std::cerr;
	}
	catch (std::bad_alloc& ba)
	{
		if (impl)
		{
			if (impl->m_data_stream)
			{
				delete impl->m_data_stream;
			}
			delete impl;
		}
		throw;
	}
}

PDFParser::~PDFParser()
{
	if (impl)
	{
		if (impl->m_data_stream)
			delete impl->m_data_stream;
		delete impl;
	}
}

void PDFParser::setVerboseLogging(bool verbose)
{
	impl->m_verbose_logging = verbose;
}

void PDFParser::setLogStream(std::ostream& log_stream)
{
	impl->m_log_stream = &log_stream;
}

bool PDFParser::isPDF()
{
	char buffer[5];
	const char pdf[6] = "%PDF-";
	if (!impl->m_data_stream->open())
	{
		*impl->m_log_stream << "Cant open " + impl->m_data_stream->name() + " for reading\n";
		return false;
	}
	if (!impl->m_data_stream->read(buffer, 1, 5))
	{
		*impl->m_log_stream << "Cant read from file " + impl->m_data_stream->name() + "\n";
		impl->m_data_stream->close();
		return false;
	}
	impl->m_data_stream->close();
	if (memcmp(buffer, pdf, 5) != 0)
	{
		*impl->m_log_stream << "No PDF header found\n";
		return false;
	}
	return true;
}

void PDFParser::getLinks(std::vector<Link>& links)
{
	#warning TODO: Implement this functionality
}

std::string PDFParser::plainText(const FormattingStyle& formatting)
{
	std::string text;

	//打开文件获取文件原始内容
	if (!impl->m_data_stream->open())
	{
		*impl->m_log_stream << "Can`t open " + impl->m_data_stream->name() + " for reading\n";
		impl->m_error = true;
		return text;
	}

	try
	{
		Implementation::PDFReader pdf_reader(impl->m_data_stream);
		//获取 page 信息
		impl->parsePagesTree(pdf_reader);

		//获取字体信息
		impl->parseFonts();

		//获取文本内容
		impl->parseText(text);
	}
	catch (std::bad_alloc& ba)
	{
		*impl->m_log_stream << "Error parsing file : bad_alloc\n";
		impl->m_error = true;
		impl->m_data_stream->close();
		return text;
	}
	catch (Exception& ex)
	{
		*impl->m_log_stream << "Error parsing file. Backtrace:\n" + ex.getBacktrace();
		impl->m_error = true;
		impl->m_data_stream->close();
		return text;
	}

	impl->m_data_stream->close();
	return text;
}

bool PDFParser::error()
{
	return impl->m_error;
}

Metadata PDFParser::metaData()
{
	Metadata metadata;
	if (!impl->m_data_stream->open())
	{
		*impl->m_log_stream << "Cant open " + impl->m_data_stream->name() + " for reading\n";
		impl->m_error = true;
		return metadata;
	}
	try
	{
		Implementation::PDFReader pdf_reader(impl->m_data_stream);
		impl->parsePagesTree(pdf_reader);
		impl->parseMetadata(pdf_reader, metadata);
	}
	catch (std::bad_alloc& ba)
	{
		*impl->m_log_stream << "Error parsing file : bad_alloc\n";
		impl->m_error = true;
		impl->m_data_stream->close();
		return metadata;
	}
	catch (Exception& ex)
	{
		*impl->m_log_stream << "Error parsing file. Backtrace:\n" + ex.getBacktrace();
		impl->m_error = true;
		impl->m_data_stream->close();
		return metadata;
	}
	impl->m_data_stream->close();
	return metadata;
}
