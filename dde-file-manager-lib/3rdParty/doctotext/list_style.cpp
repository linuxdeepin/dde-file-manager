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

#include "formatting_style.h"

using namespace doctotext;

struct ListStyle::Implementation
{
	std::string m_list_leading_characters;
};

ListStyle::ListStyle()
{
	m_impl = new Implementation;
}

ListStyle::ListStyle(const ListStyle& style)
{
	m_impl = new Implementation;
	m_impl->m_list_leading_characters = std::string(style.getPrefix());
}

ListStyle& ListStyle::operator = (const ListStyle& style)
{
	m_impl->m_list_leading_characters = std::string(style.getPrefix());
	return *this;
}

ListStyle::~ListStyle()
{
	delete m_impl;
}

void ListStyle::setPrefix(const std::string &prefix)
{
	m_impl->m_list_leading_characters = prefix;
}

void ListStyle::setPrefix(const char *prefix)
{
	m_impl->m_list_leading_characters = std::string(prefix);
}

const char* ListStyle::getPrefix() const
{
	return m_impl->m_list_leading_characters.c_str();
}
