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

#include "link.h"

using namespace doctotext;

struct Link::Implementation
{
	std::string m_link_url;
	std::string m_link_text;
	size_t m_link_text_position;
};

Link::Link()
{
	impl = NULL;
	try
	{
		impl = new Implementation;
		impl->m_link_text_position = 0;
	}
	catch (std::bad_alloc& ba)
	{
		if (impl)
			delete impl;
		impl = NULL;
	}
}

Link::Link(const std::string& link_url, const std::string& link_text, size_t link_text_position)
{
	impl = NULL;
	try
	{
		impl = new Implementation;
		impl->m_link_text_position = link_text_position;
		impl->m_link_url = link_url;
		impl->m_link_text = link_text;
	}
	catch (std::bad_alloc& ba)
	{
		if (impl)
			delete impl;
		impl = NULL;
	}
}

Link::Link(const Link& link)
{
	impl = NULL;
	try
	{
		impl = new Implementation;
		*impl = *link.impl;
	}
	catch (std::bad_alloc& ba)
	{
		if (impl)
			delete impl;
		impl = NULL;
	}
}

Link::~Link()
{
	if (impl)
		delete impl;
}

Link& Link::operator = (const Link& link)
{
	*impl = *link.impl;
	return *this;
}

void Link::setLinkUrl(const std::string& link_url)
{
	impl->m_link_url = link_url;
}

void Link::setLinkText(const std::string &link_text)
{
	impl->m_link_text = link_text;
}

void Link::setLinkTextPosition(size_t link_text_position)
{
	impl->m_link_text_position = link_text_position;
}

const char* Link::getLinkUrl() const
{
	return impl->m_link_url.c_str();
}

const char* Link::getLinkText() const
{
	return impl->m_link_text.c_str();
}

size_t Link::getLinkTextPosition() const
{
	return impl->m_link_text_position;
}
