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

#include "variant.h"

#include "misc.h"
#include <string.h>

using namespace doctotext;

struct Variant::Implementation
{
	enum VariantType
	{
		string,
		number,
		date_time,
		null
	};

	std::string m_string_value;
	size_t m_number_value;
	tm m_date_time_value;
	VariantType m_variant_type;

	Implementation()
	{
		m_variant_type = null;
		m_number_value = 0;
		memset(&m_date_time_value, 0, sizeof(m_date_time_value));
	}
};

Variant::Variant()
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
		throw;
	}
}

Variant::Variant(const std::string& value)
{
	impl = NULL;
	try
	{
		impl = new Implementation;
		setValue(value);
	}
	catch (std::bad_alloc& ba)
	{
		if (impl)
			delete impl;
		throw;
	}
}

Variant::Variant(const tm& value)
{
	impl = NULL;
	try
	{
		impl = new Implementation;
		setValue(value);
	}
	catch (std::bad_alloc& ba)
	{
		if (impl)
			delete impl;
		throw;
	}
}

Variant::Variant(size_t value)
{
	impl = NULL;
	try
	{
		impl = new Implementation;
		setValue(value);
	}
	catch (std::bad_alloc& ba)
	{
		if (impl)
			delete impl;
		throw;
	}
}

Variant::Variant(const Variant& variant)
{
	impl = NULL;
	try
	{
		impl = new Implementation;
		*impl = *variant.impl;
	}
	catch (std::bad_alloc& ba)
	{
		if (impl)
			delete impl;
		throw;
	}
}

Variant& Variant::operator = (const Variant& variant)
{
	*impl = *variant.impl;
	return *this;
}

Variant::~Variant()
{
	if (impl)
		delete impl;
}

void Variant::setValue(size_t number)
{
	impl->m_variant_type = Implementation::number;
	impl->m_number_value = number;
	impl->m_string_value = uint_to_string(number);
}

void Variant::setValue(const tm& date_time)
{
	impl->m_variant_type = Implementation::date_time;
	impl->m_date_time_value = date_time;
	impl->m_string_value = date_to_string(impl->m_date_time_value);
}

void Variant::setValue(const std::string& str)
{
	impl->m_variant_type = Implementation::string;
	impl->m_string_value = str;
}

bool Variant::isEmpty() const
{
	return impl->m_variant_type == Implementation::null;
}

bool Variant::isString() const
{
	return impl->m_variant_type == Implementation::string;
}

bool Variant::isNumber() const
{
	return impl->m_variant_type == Implementation::number;
}

bool Variant::isDateTime() const
{
	return impl->m_variant_type == Implementation::date_time;
}

const tm& Variant::getDateTime() const
{
	return impl->m_date_time_value;
}

const char* Variant::getString() const
{
	return impl->m_string_value.c_str();
}

size_t Variant::getNumber() const
{
	return impl->m_number_value;
}
