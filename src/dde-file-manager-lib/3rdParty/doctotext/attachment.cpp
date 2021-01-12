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

#include "attachment.h"

using namespace doctotext;

struct Attachment::Implementation
{
	std::string m_file_name;
	std::string m_binary_content;
	std::map<std::string, Variant> m_fields;
};

Attachment::Attachment()
{
	impl = NULL;
	try
	{
		impl = new Implementation;
	}
	catch (std::bad_alloc& ba)
	{
		if (impl)
			delete impl;
		impl = NULL;
		throw;
	}
}

Attachment::Attachment(const std::string &file_name)
{
	impl = NULL;
	try
	{
		impl = new Implementation;
		impl->m_file_name = file_name;
	}
	catch (std::bad_alloc& ba)
	{
		if (impl)
			delete impl;
		impl = NULL;
		throw;
	}
}

Attachment::Attachment(const Attachment &attachment)
{
	impl = NULL;
	try
	{
		impl = new Implementation;
		*impl = *attachment.impl;
	}
	catch (std::bad_alloc& ba)
	{
		if (impl)
			delete impl;
		impl = NULL;
		throw;
	}
}

Attachment& Attachment::operator = (const Attachment& attachment)
{
	*impl = *attachment.impl;
	return *this;
}

Attachment::~Attachment()
{
	if (impl)
		delete impl;
}

void Attachment::setFileName(const std::string& file_name)
{
	impl->m_file_name = file_name;
}

void Attachment::setBinaryContent(const std::string &binary_content)
{
	impl->m_binary_content = binary_content;
}

void Attachment::addField(const std::string &field_name, const Variant &field_value)
{
	impl->m_fields[field_name] = field_value;
}

const char* Attachment::filename() const
{
	return impl->m_file_name.c_str();
}

const char* Attachment::binaryContent() const
{
	return impl->m_binary_content.c_str();
}

size_t Attachment::binaryContentSize() const
{
	return impl->m_binary_content.length();
}

bool Attachment::hasField(const std::string& field_name) const
{
	return impl->m_fields.find(field_name) != impl->m_fields.end();
}

const Variant& Attachment::getField(const std::string& field_name) const
{
	return impl->m_fields[field_name];
}

const std::map<std::string, Variant>& Attachment::getFields() const
{
	return impl->m_fields;
}
