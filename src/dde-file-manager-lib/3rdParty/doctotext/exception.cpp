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

#include "exception.h"

#include <list>

using namespace doctotext;

struct Exception::Implementation
{
	std::list<std::string> m_errors;
};

Exception::Exception() throw()
{
	try
	{
		impl = NULL;
		impl = new Implementation;
	}
	catch (std::bad_alloc& ba)
	{
		if (impl)
		{
			delete impl;
		}
		throw;
	}
}

Exception::Exception(const std::string &first_error_message) throw()
{
	try
	{
		impl = NULL;
		impl = new Implementation;
		impl->m_errors.push_back(first_error_message);
	}
	catch (std::bad_alloc& ba)
	{
		if (impl)
		{
			delete impl;
		}
		throw;
	}
}

Exception::Exception(const Exception &ex) throw()
{
	try
	{
		impl = NULL;
		impl = new Implementation;
		*impl = *ex.impl;
	}
	catch (std::bad_alloc& ba)
	{
		if (impl)
		{
			delete impl;
		}
		throw;
	}
}

Exception::~Exception() throw()
{
	if (impl)
	{
		delete impl;
	}
}

Exception& Exception::operator = (const Exception& ex) throw()
{
	*impl = *ex.impl;
	return *this;
}


std::string Exception::getBacktrace()
{
	std::string backtrace;
	for (std::list<std::string>::iterator it = impl->m_errors.begin(); it != impl->m_errors.end(); ++it)
	{
		backtrace += (*it) + "\n";
	}
	return backtrace;
}

void Exception::appendError(const std::string &error_message)
{
	impl->m_errors.push_back(error_message);
}

std::list<std::string>::iterator Exception::getErrorIterator() const
{
	return impl->m_errors.begin();
}

size_t Exception::getErrorCount() const
{
	return impl->m_errors.size();
}
