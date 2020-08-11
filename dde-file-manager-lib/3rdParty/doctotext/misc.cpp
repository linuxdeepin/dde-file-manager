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

#include "misc.h"

#include "link.h"
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <time.h>

std::string ustring_to_string(const UString& s)
{
	const UChar* src = s.data();
	int length = s.length();
	std::string r;
	int index = 0;
	while (index < length)
	{
		unsigned int ch = src[index++].unicode();
		if (utf16_unichar_has_4_bytes(ch) && index < length)
			ch = (ch << 16) + src[index++].unicode();
		if (ch != 0)
			r += unichar_to_utf8(ch);
	}
	return r;
}

std::string unichar_to_utf8(unsigned int unichar)
{
	//unichars which have only two bytes are represented by 1, 2 or 3 bytes in utf8.
	char out[4];
	if (unichar < 0x80)
	{
		out[0] = (char)unichar;
		return std::string(out, 1);
	}
	else if (unichar < 0x800)
	{
		out[0] = 0xC0 | (unichar >> 6);
		out[1] = 0x80 | (unichar & 0x0000003F);
		return std::string(out, 2);
	}
	else if (unichar < 0x10000)
	{
		out[0] = 0xE0 | (unichar >> 12);
		out[1] = 0x80 | ((unichar & 0x00000FFF) >> 6);
		out[2] = 0x80 | (unichar & 0x0000003F);
		return std::string(out, 3);
	}
	else
	{
		//4-bytes length unichars (code points from 0x10000 to 0x10FFFF) have 4-bytes length characters equivalent in utf8
		unsigned short second_byte = unichar & 0x000003FF;
		unichar = (unichar & 0x03FF0000) >> 6;
		unichar = (unichar | second_byte) + 0x10000;
		out[0] = 0xF0 | (unichar >> 18);
		out[1] = 0x80 | ((unichar & 0x0003FFFF) >> 12);
		out[2] = 0x80 | ((unichar & 0x00000FFF) >> 6);
		out[3] = 0x80 | (unichar & 0x0000003F);
		return std::string(out, 4);
	}
	return "";
}

int prepareTable(std::vector<svector> &p_table)
{
	if(p_table.empty())
		return 0;
	for(int i = 0 ; i < p_table.size() ; i ++)
	{
		if(p_table.at(i).empty())
			continue;
		for(int j = 0 ; j < p_table.at(i).size() ; j ++)
		{
			if(p_table.at(i).at(j).length() < 2)
				continue;
			int k = p_table.at(i).at(j).find('\n', p_table.at(i).at(j).length() - 2);
			if(k >= 0)
				p_table.at(i).at(j).replace(k, 1, "");
		}
	}
	return 1;
}

int prepareList(std::vector<std::string> &p_list)
{
	if(p_list.empty())
		return 0;
	for(int i = 0 ; i < p_list.size() ; i ++)
	{
		if(p_list.at(i).empty())
			continue;
		if(p_list.at(i).length() < 2)
			continue;
		int k = p_list.at(i).find('\n', p_list.at(i).length() - 2);
		if(k >= 0)
			p_list.at(i).replace(k, 1, "");
	}
	return 1;
}

std::string formatTable(std::vector<svector>& mcols, const FormattingStyle& options)
{
	std::string table_out;

	prepareTable(mcols);

	for(int i = 0 ; i < mcols.size() ; i ++)
	{
		for(int j = 0 ; j < mcols.at(i).size() ; j ++)
		{
			table_out += mcols.at(i).at(j);
			switch(options.table_style)
			{
				case TABLE_STYLE_TABLE_LOOK:
					if(j + 1 == mcols.at(i).size())
					{
						table_out += "\n";
						break;
					}
					table_out += "\t";
					break;
				case TABLE_STYLE_ONE_ROW:
					table_out += " ";
					break;
				case TABLE_STYLE_ONE_COL:
					table_out += "\n";
					break;
				default:
					table_out += "\n";
					break;
			}
		}
	}
	return table_out;
}

std::string formatUrl(const std::string& mlink_url, const std::string &mlink_text, const FormattingStyle& options)
{
	std::string u_url;
	int a = options.url_style;
	switch(a)
	{
		case URL_STYLE_TEXT_ONLY:
			u_url = mlink_text;
			break;
		case URL_STYLE_EXTENDED:
			if (mlink_url.length() > 0)
			{
				u_url += "<";
				u_url += mlink_url;
				u_url += ">";
			}
			u_url += mlink_text;
			break;
		case URL_STYLE_UNDERSCORED:
			if (mlink_text.length() > 0)
			{
				u_url += "_";
				for (size_t i = 0; i < mlink_text.length(); ++i)
				{
					if (mlink_text[i] == ' ')
						u_url += '_';
					else
						u_url += mlink_text[i];
				}
				u_url += "_";
			}
			break;
		default:
			if (mlink_url.length() > 0)
			{
				u_url += "<";
				u_url += mlink_url;
				u_url += ">";
			}
			u_url += mlink_text;
			break;
	}
	return u_url;
}

std::string formatList(std::vector<std::string>& mlist, const FormattingStyle& options)
{
	std::string list_out;
	const char* prefix = options.list_style.getPrefix();
	prepareList(mlist);

	for(int i = 0 ; i < mlist.size() ; i++)
	{
		list_out = list_out + prefix;
		list_out = list_out + mlist.at(i);
		list_out = list_out + "\n";
	}
	return list_out;
}

std::string formatNumberedList(std::vector<std::string>& mlist)
{
	std::string list_out;
	prepareList(mlist);
	char count[100];
	for (int i = 0 ; i < mlist.size() ; i++)
	{
		sprintf(count, "%d. ", i + 1);
		list_out = list_out + count;
		list_out = list_out + mlist.at(i);
		list_out = list_out + "\n";
	}
	return list_out;
}

static bool string_to_int(const std::string& s, int& i)
{
	std::istringstream ss(s);
	ss >> i;
	return ss && ss.eof();
}

std::string uint_to_string(unsigned int value)
{
	std::ostringstream os;
	os << value;
	return os.str();
}

bool string_to_date(const std::string& s, tm& date)
{
	// %Y-%m-%dT%H:%M:%S
	if (s.length() >= 19 &&
		string_to_int(s.substr(0, 4), date.tm_year) && s[4] == '-' &&
		string_to_int(s.substr(5, 2), date.tm_mon) && s[7] == '-' &&
		string_to_int(s.substr(8, 2), date.tm_mday) && s[10] == 'T' &&
		string_to_int(s.substr(11, 2), date.tm_hour) && s[13] == ':' &&
		string_to_int(s.substr(14, 2), date.tm_min) && s[16] == ':' &&
		string_to_int(s.substr(17, 2), date.tm_sec))
	{
		date.tm_year -= 1900;
		date.tm_mon--;
		return true;
	}
	// %Y%m%d;%H%M%S
	if (s.length() >= 15 &&
		string_to_int(s.substr(0, 4), date.tm_year) &&
		string_to_int(s.substr(4, 2), date.tm_mon) &&
		string_to_int(s.substr(6, 2), date.tm_mday) && s[8] == ';' &&
		string_to_int(s.substr(9, 2), date.tm_hour) &&
		string_to_int(s.substr(11, 2), date.tm_min) &&
		string_to_int(s.substr(13, 2), date.tm_sec))
	{
		date.tm_year -= 1900;
		date.tm_mon--;
		return true;
	}
	date.tm_year = 2001 - 1900;
	return false;
}

std::string date_to_string(const tm& date)
{
	if (date.tm_year == 0)
		return "";
	char buf[64];
	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &date);
	return buf;
}

/*
 UTF8:
 Code points from 0x0 to 0x7F:			0xxxxxxx (1 byte)
 Code points from 0x80 to 0x7FF:		110xxxxx 10xxxxxx (2 bytes)
 Code points from 0x800 to 0xFFFF:		1110xxxx 10xxxxxx 10xxxxxx (3 bytes)
 Code points from 0x10000 to 0x10FFFF:	11110xxx 10xxxxxx 10xxxxxx 10xxxxxx (4 bytes)

 UTF16:
 Code points from 0x0 to 0xFFFF:		xxxxxxxx xxxxxxxx (2 bytes)
 Code points from 0x10000 to 0x10FFFF:	110110xx xxxxxxxx 110111xx xxxxxxxx (4 bytes)

 In UTF16, we subtract 0x10000 from any code point beetwen 0x10000 to 0x10FFFF, so
 we need only 20 bits to encode position (4-bytes length utf16 characters).
*/

UString utf8_to_ustring(const std::string& src)
{
	UString res;
	const char* str = src.data();
	size_t str_size = src.length();
	unsigned int ch;
	unsigned char tmp;
	size_t index = 0;
	res = "";
	while (index < str_size)
	{
		tmp = str[index++];
		if (tmp < 0x80)
			res += UString(tmp);
		else
		{
			ch = tmp & 0x1F;
			int len_char = 1;
			while (index < str_size)
			{
				tmp = str[index++];
				if ((tmp & 0xC0) == 0x80)
				{
					ch = ch << 6;
					ch = ch | (tmp & 0x3F);
					++len_char;
				}
				else
				{
					if (len_char < 4)
					{
						res += UString((UChar)((unsigned short)ch));
					}
					else
					{
						//beetwen 10000H and 10FFFFH
						unsigned short first = 0xD800, second = 0xDC00;
						ch = ch & 0x001FFFFF;
						ch -= 0x10000;
						res += UString((UChar)(first | ((ch & 0x000FFC00) >> 10)));
						res += UString(((UChar)(second | (ch & 0x000003FF))));
					}
					--index;
					break;
				}
			}
		}
	}
	return res;
}

/*Lets look at this data:
0x00 to 0x7F: 0xxxxxxx (1 byte)
0x80 to 0x7FF: 110xxxxx 10xxxxxx (2 bytes)
0x800 to 0xFFFF: 1110xxxx 10xxxxxx 10xxxxxx (3 bytes)
0x10000 to 0x1FFFFF: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx (4 bytes)
0x2000000 to 0x3FFFFFF: 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx (5 bytes) -> no possible conversion to UTF16
0x40000000 to 0x7FFFFFFF: 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx (6 bytes) -> no possible conversion to UTF16
Invalid characters in UTF8 are: 11111110 and 11111111.
What can we do with that? We can use those values to "mark" special places. We can use them to substitute our links.
But we have to be sure that "text" is already encoded in UTF8.
*/
void insertSpecialLinkBlockIntoText(std::string& text, const Link &link)
{
	std::string substitute_string(strlen(link.getLinkText()), 0xFF);
	text += substitute_string;
}

void decodeSpecialLinkBlocks(std::string& text, std::vector<Link>& links, std::ostream &log_stream)
{
	size_t search_position = 0;
	for (std::vector<Link>::iterator it = links.begin(); it != links.end(); ++it)
	{
		size_t link_text_size = strlen((*it).getLinkText());
		if (link_text_size > 0)
		{
			std::string substitute_string(link_text_size, 0xFF);
			search_position = text.find(substitute_string, search_position);
			if (search_position != std::string::npos)	//should never happen, because we have inserted such a string before
			{
				for (size_t i = 0; i < link_text_size && i < text.length() - search_position; ++i)
					text[search_position + i] = (*it).getLinkText()[i];
				(*it).setLinkTextPosition(search_position);
			}
			else
			{
				//If you can see this message, then check last changes.
				log_stream << "Warning: output text is corrupted: cannot recover links\n";
			}
		}
		else
			(*it).setLinkTextPosition(0);	//link has no output in final text?
	}
}

std::string int_to_str(int i)
{
	std::ostringstream s;
	s << i;
	return s.str();
}

int str_to_int(const std::string& s)
{
	std::istringstream ss(s);
	int i;
	ss >> i;
	return i;
}
