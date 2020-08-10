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

#include "xml_fixer.h"

#include <algorithm>
#include <libxml/parserInternals.h> // xmlParserMaxDepth
#include <list>
#include <sstream>
#include <stack>

struct XmlName
{
	std::string nspace;
	std::string name;

	XmlName()
	{
	}

	XmlName(const std::string& _nspace, const std::string _name)
		: nspace(_nspace), name(_name)
	{
	}

	bool operator==(const XmlName& right)
	{
		return nspace == right.nspace && name == right.name;
	}

	std::string fullName() const
	{
		std::string full_name;
		if (nspace != "")
			full_name += nspace + ":";
		full_name += name;
		return full_name;
	}
};

struct Attr
{
	XmlName name;
	std::string value;

	Attr()
	{
	}

	Attr(const XmlName& _name, const std::string& _value)
		: name(_name), value(_value)
	{
	}

	std::string toText() const
	{
		return name.fullName() + "=\"" + value + "\"";
	}

	/**
		Required for std::find.
	**/
	bool operator==(const Attr& rhs)
	{
		return name == rhs.name;
	}
};

struct Tag
{
	enum Type { HEADER, OPENING, CLOSING, OPENING_AND_CLOSING };
	Type type;
	XmlName name;
	std::list<Attr> attrs;

	Tag()
	{
	}

	Tag(Type _type, const XmlName& _name)
		: type(_type), name(_name)
	{
	}

	std::string toText() const
	{
		std::string text = "<";
		if (type == CLOSING)
			text += '/';
		else if (type == HEADER)
			text += '?';
		text += name.fullName();
		for (std::list<Attr>::const_iterator i = attrs.begin(); i != attrs.end(); i++)
			text += " " + (*i).toText();
		if (type == OPENING_AND_CLOSING)
			text += '/';
		else if (type == HEADER)
			text += '?';
		text += ">";
		return text;
	}
};

struct DocToTextXmlFixer::Implementation
{
	std::istringstream xml;

	bool parseChar(char* ch)
	{
		int res = xml.get();
		if (res == EOF)
			return false;
		*ch = res;
		return true;
	}

	bool parseAssertChar(char assert_ch)
	{
		char ch;
		if (!parseChar(&ch))
			return false;
		if (ch == assert_ch)
			return true;
		xml.seekg(-1, std::ios_base::cur);
		return false;
	}

	bool parseAlpha(char* ch)
	{
		if (!parseChar(ch))
			return false;
		if (isalpha(*ch))
			return true;
		xml.seekg(-1, std::ios_base::cur);
		return false;
	}

	bool parseAlnumMinus(char* ch)
	{
		if (!parseChar(ch))
			return false;
		if (isalnum(*ch) || *ch == '-')
			return true;
		xml.seekg(-1, std::ios_base::cur);
		return false;
	}

	bool parseName(std::string* s)
	{
		char ch;
		if (!parseAlpha(&ch))
			return false;
		*s = ch;
		while (parseAlnumMinus(&ch))
			*s += ch;
		return true;
	}

	bool parseLiteral(std::string* s)
	{
		std::streampos pos = xml.tellg();
		if (!parseAssertChar('"'))
		{
			return false;
		}
		char ch;
		*s = "";
		for (;;)
		{
			if (!parseChar(&ch))
			{
				xml.seekg(pos);
				return false;
			}
			else if (ch == '>')
			{
				xml.seekg(pos);
				return false;
			}
			else if (ch == '"')
				return true;
			*s += ch;
		};
	}

	bool parseAttr(Attr* attr)
	{
		std::streampos pos = xml.tellg();
		if (!parseName(&attr->name.name))
			return false;
		char ch;
		if (!parseChar(&ch))
		{
			xml.seekg(pos);
			return false;
		}
		if (ch == ':')
		{
			attr->name.nspace = attr->name.name;
			if (!parseName(&attr->name.name))
			{
				xml.seekg(pos);
				return false;
			}
			if (!parseChar(&ch))
			{
				xml.seekg(pos);
				return false;
			}
		}
		if (ch != '=')
		{
			xml.seekg(pos);
			return false;
		}
		if (!parseLiteral(&attr->value))
		{
			xml.seekg(pos);
			return false;
		}
		return true;
	}

	bool parseTag(Tag* tag)
	{
		if (!parseAssertChar('<'))
			return false;
		std::streampos pos = xml.tellg();
		tag->type = Tag::OPENING;
		if (parseAssertChar('/'))
			tag->type = Tag::CLOSING;
		else if (parseAssertChar('?'))
			tag->type = Tag::HEADER;
		if (!parseName(&tag->name.name))
		{
			xml.seekg(pos);
			return false;
		}
		if (parseAssertChar(':'))
		{
			tag->name.nspace = tag->name.name;
			if (!parseName(&tag->name.name))
			{
				xml.seekg(pos);
				return false;
			}
		}
		if (tag->type == Tag::HEADER || tag->type == Tag::OPENING)
		{
			while (parseAssertChar(' '))
			{
				Attr attr;
				if (!parseAttr(&attr))
				{
					xml.seekg(pos);
					return false;
				}
				// we do not use std::set because original order of attributes is important
				if (std::find(tag->attrs.begin(), tag->attrs.end(), attr) == tag->attrs.end())
					tag->attrs.push_back(attr);
			}
		}
		if (parseAssertChar('/'))
			tag->type = Tag::OPENING_AND_CLOSING;
		else
			parseAssertChar('?');
		if (!parseAssertChar('>'))
		{
			xml.seekg(pos);
			return false;
		}
		return true;
	}

	bool parseUtf8Char(std::string* s)
	{
		std::streampos pos = xml.tellg();
		char ch;
		if (!parseChar(&ch))
			return false;
		int len;
		if ((ch & 0xFE) == 0xFC) // 1111110x
			len = 5;
		else if ((ch & 0xFC) == 0xF8) // 111110xx
			len = 4;
		else if ((ch & 0xF8) == 0xF0) // 11110xxx
			len = 3;
		else if ((ch & 0xF0) == 0xE0) // 1110xxxx
			len = 2;
		else if ((ch & 0xE0) == 0xC0) // 110xxxxx
			len = 1;
		else
		{
			xml.seekg(pos);
			return false;
		}
		*s = ch;
		for (int i = 0; i < len; i++)
		{
			if (!parseChar(&ch))
			{
				xml.seekg(pos);
				return false;
			}
			if ((ch & 0xC0) != 0x80) // 10xxxxxx
			{
				xml.seekg(pos);
				return false;
			}
			*s += ch;
		}
		return true;
	}

	bool parseAssertString(const std::string s)
	{
		std::streampos pos = xml.tellg();
		for (size_t i = 0; i < s.length(); i++)
			if (!parseAssertChar(s[i]))
			{
				xml.seekg(pos);
				return false;
			}
		return true;
	}

	bool parseEntity(std::string* s)
	{
		const std::string entities[] = { "&quot;", "&amp;", "&apos;", "&lt;", "&gt;" };
		for (int i = 0; i < sizeof(entities) / sizeof(entities[0]); i++)
			if (parseAssertString(entities[i]))
			{
				*s = entities[i];
				return true;
			}
		return false;
	}
};

DocToTextXmlFixer::DocToTextXmlFixer()
{
	Impl = new Implementation();
}

DocToTextXmlFixer::~DocToTextXmlFixer()
{
	delete Impl;
}

std::string DocToTextXmlFixer::fix(const std::string& xml) const
{
	Impl->xml.str(xml);
	std::stack<XmlName> open_tags;
	std::string fixed_xml;
	for (;;)
	{
		Tag tag;
		std::string entity;
		std::string utf8_char;
		char ch;
		if (Impl->parseTag(&tag))
		{
			if (tag.type == Tag::OPENING_AND_CLOSING && open_tags.empty() && Impl->xml.peek() != EOF)
			{
				// some text after root opening and closing tag!
				open_tags.push(tag.name);
				tag.type = Tag::OPENING;
			}
			else if (tag.type == Tag::CLOSING)
			{
				if (!open_tags.empty() && open_tags.top() == tag.name)
					open_tags.pop();
				else
				{
					if (open_tags.empty() && Impl->xml.peek() != EOF)
					{
						// some text after root closing tag!
						continue;
					}
					fixed_xml += Tag(Tag::OPENING, tag.name).toText();
				}
			}
			else if (tag.type == Tag::OPENING)
			{
				if (open_tags.size() == xmlParserMaxDepth)
				{
					// We are aproaching libxml security limit of tags depth.
					// Something is probably wrong, especially when we are
					// processing corrupted file.
					// Important question: what should we do now, skip
					// new tag or close some tags?
					// Current algorithm preserves as many tags as possible,
					// so we are simply closing the last tag.
					fixed_xml += Tag(Tag::CLOSING, open_tags.top()).toText();
					open_tags.pop();
				}
				open_tags.push(tag.name);
			}
			fixed_xml += tag.toText();
		}
		else if (Impl->parseEntity(&entity))
			fixed_xml += entity;
		else if (Impl->parseUtf8Char(&utf8_char))
			fixed_xml += utf8_char;
		else if (Impl->parseChar(&ch))
		{
			if (isascii(ch) && ch != '<' && ch != '&')
				fixed_xml += ch;
		}
		else
			break;
	}
	while (!open_tags.empty())
	{
		const XmlName& tag = open_tags.top();
		fixed_xml += Tag(Tag::CLOSING, tag).toText();
		open_tags.pop();
	}
	return fixed_xml;
}
