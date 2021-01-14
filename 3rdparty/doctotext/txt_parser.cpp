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

#include "txt_parser.h"

#include "charsetdetect.h"
#include "data_stream.h"
#include "exception.h"
#include "htmlcxx/html/CharsetConverter.h"
#include <iostream>
#include <string.h>

using namespace doctotext;

struct TXTParser::Implementation
{
	bool m_error;
	std::string m_file_name;
	bool m_verbose_logging;
	std::ostream* m_log_stream;
	DataStream* m_data_stream;
};

TXTParser::TXTParser(const std::string& file_name)
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

TXTParser::TXTParser(const char* buffer, size_t size)
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
				delete impl->m_data_stream;
			delete impl;
		}
		throw;
	}
}

TXTParser::~TXTParser()
{
	if (impl)
	{
		if (impl->m_data_stream)
			delete impl->m_data_stream;
		delete impl;
	}
}

void TXTParser::setVerboseLogging(bool verbose)
{
	impl->m_verbose_logging = verbose;
}

void TXTParser::setLogStream(std::ostream& log_stream)
{
	impl->m_log_stream = &log_stream;
}

std::string TXTParser::plainText()
{
	std::string text;
	csd_t charset_detector = NULL;
	htmlcxx::CharsetConverter* converter = NULL;
	try
	{
		std::string encoding;
		if (!impl->m_data_stream->open())
			throw Exception("Could not open file: " + impl->m_file_name);
		std::string content;
		size_t file_size = impl->m_data_stream->size();
		content.resize(file_size);
		if (!impl->m_data_stream->read(&content[0], 1, file_size))
			throw Exception("Could not read from file: " + impl->m_file_name);
		impl->m_data_stream->close();
		charset_detector = csd_open();
		if (charset_detector == (csd_t)-1)
		{
			charset_detector = NULL;
			*impl->m_log_stream << "Warning: Could not create charset detector\n";
			encoding = "UTF-8";
		}
		else
		{
			csd_consider(charset_detector, content.c_str(), content.length());
			const char* res = csd_close(charset_detector);
			charset_detector = NULL;
			if (res != NULL)
			{
				encoding = std::string(res);
				if (impl->m_verbose_logging)
					*impl->m_log_stream << "Estimated encoding: " + encoding + "\n";
			}
			else
			{
				encoding = "ASCII";
				if (impl->m_verbose_logging)
					*impl->m_log_stream << "Could not detect encoding. Document is assumed to be encoded in ASCII\n";
			}
		}
		if (encoding != "utf-8" && encoding != "UTF-8")
		{
			try
			{
				converter = new htmlcxx::CharsetConverter(encoding, "UTF-8");
			}
			catch (htmlcxx::CharsetConverter::Exception& ex)
			{
				*impl->m_log_stream << "Warning: Cant convert text to UTF-8 from " + encoding;
				if (converter)
					delete converter;
				converter = NULL;
			}
		}
		if (converter)
		{
			text = converter->convert(content);
			delete converter;
			converter = NULL;
			return text;
		}
		return content;
	}
	catch (std::bad_alloc& ba)
	{
		if (converter)
			delete converter;
		converter = NULL;
		if (charset_detector)
			csd_close(charset_detector);
		charset_detector = NULL;
		*impl->m_log_stream << "Error parsing file : bad_alloc\n";
		impl->m_error = true;
	}
	catch (Exception& ex)
	{
		if (converter)
			delete converter;
		converter = NULL;
		if (charset_detector)
			csd_close(charset_detector);
		charset_detector = NULL;
		*impl->m_log_stream << "Error parsing file. Backtrace:\n" + ex.getBacktrace();
		impl->m_error = true;
	}
	impl->m_data_stream->close();
	return text;
}

bool TXTParser::error()
{
	return impl->m_error;
}
