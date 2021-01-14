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

#include "odfxml_parser.h"

#include <fstream>
#include <iostream>
#include <libxml/xmlreader.h>
#include "metadata.h"
#include "misc.h"
#include "xml_stream.h"

class ODFXMLParser::CommandHandlersSet
{
	public:
		static void onODFBody(CommonXMLDocumentParser& parser, XmlStream& xml_stream, XmlParseMode mode,
							  const FormattingStyle& options, const DocToTextUnzip* zipfile, std::string& text,
							  bool& children_processed, std::string& level_suffix, bool first_on_level,
							  std::vector<Link>& links)
		{
			#warning TODO: Unfortunately, in CommonXMLDocumentParser we are not checking full names for xml tags.\
			Thats a problem, since we can have table:body, office:body etc. What if more xml tags are not handled correctly?
			if (xml_stream.fullName() != "office:body")
				return;
			if (parser.verbose())
				parser.getLogStream() << "ODF_BODY Command\n";
			//we are inside body, we can disable adding text nodes
			parser.disableText(false);
		}

		static void onODFObject(CommonXMLDocumentParser& parser, XmlStream& xml_stream, XmlParseMode mode,
								const FormattingStyle& options, const DocToTextUnzip* zipfile, std::string& text,
								bool& children_processed, std::string& level_suffix, bool first_on_level,
								std::vector<Link>& links)
		{
			if (parser.verbose())
				parser.getLogStream() << "ODF_OBJECT Command\n";
			xml_stream.levelDown();
			parser.disableText(true);
			text += parser.parseXmlData(xml_stream, mode, options, zipfile, links);
			parser.disableText(false);
			xml_stream.levelUp();
		}

		static void onODFBinaryData(CommonXMLDocumentParser& parser, XmlStream& xml_stream, XmlParseMode mode,
									const FormattingStyle& options, const DocToTextUnzip* zipfile, std::string& text,
									bool& children_processed, std::string& level_suffix, bool first_on_level,
									std::vector<Link>& links)
		{
			if (parser.verbose())
				parser.getLogStream() << "ODF_BINARY_DATA Command\n";
			children_processed = true;
		}
};

struct ODFXMLParser::ExtendedImplementation
{
	const char* m_buffer;
	size_t m_buffer_size;
	std::string m_file_name;
	ODFXMLParser* m_interf;
};

ODFXMLParser::ODFXMLParser(const std::string& file_name)
{
	extended_impl = NULL;
	try
	{
		extended_impl = new ExtendedImplementation();
		extended_impl->m_file_name = file_name;
		extended_impl->m_buffer = NULL;
		extended_impl->m_buffer_size = 0;
		extended_impl->m_interf = this;
		registerODFOOXMLCommandHandler("body", &CommandHandlersSet::onODFBody);
		registerODFOOXMLCommandHandler("object", &CommandHandlersSet::onODFObject);
		registerODFOOXMLCommandHandler("binary-data", &CommandHandlersSet::onODFBinaryData);
	}
	catch (std::bad_alloc& ba)
	{
		if (extended_impl)
			delete extended_impl;
		cleanUp();
		throw;
	}
}

ODFXMLParser::ODFXMLParser(const char *buffer, size_t size)
{
	extended_impl = NULL;
	try
	{
		extended_impl = new ExtendedImplementation();
		extended_impl->m_file_name = "Memory buffer";
		extended_impl->m_buffer = buffer;
		extended_impl->m_buffer_size = size;
		extended_impl->m_interf = this;
		registerODFOOXMLCommandHandler("body", &CommandHandlersSet::onODFBody);
		registerODFOOXMLCommandHandler("object", &CommandHandlersSet::onODFObject);
		registerODFOOXMLCommandHandler("binary-data", &CommandHandlersSet::onODFBinaryData);
	}
	catch (std::bad_alloc& ba)
	{
		if (extended_impl)
			delete extended_impl;
		cleanUp();
		throw;
	}
}

ODFXMLParser::~ODFXMLParser()
{
	if (extended_impl)
		delete extended_impl;
}

bool ODFXMLParser::isODFXML()
{
	std::string xml_content;
	setError(false);
	if (extended_impl->m_buffer_size > 0)
		xml_content = std::string(extended_impl->m_buffer, extended_impl->m_buffer_size);
	else
	{
		std::ifstream file(extended_impl->m_file_name.c_str(), std::ios_base::in|std::ios_base::binary);
		if (!file.is_open())
		{
			setError(true);
			getLogStream() << "Error opening file: " + extended_impl->m_file_name + "\n";
			return false;
		}
		xml_content = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		file.close();
	}
	if (xml_content.find("office:document") == std::string::npos)
		return false;
	return true;
}

std::string ODFXMLParser::plainText(XmlParseMode mode, FormattingStyle& formatting_style)
{
	std::string text;
	std::string xml_content;
	setError(false);
	if (extended_impl->m_buffer_size > 0)
		xml_content = std::string(extended_impl->m_buffer, extended_impl->m_buffer_size);
	else
	{
		std::ifstream file(extended_impl->m_file_name.c_str(), std::ios_base::in|std::ios_base::binary);
		if (!file.is_open())
		{
			setError(true);
			getLogStream() << "Error opening file: " + extended_impl->m_file_name + "\n";
			return "";
		}
		xml_content = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		file.close();
	}

	//according to the ODF specification, we must skip blank nodes. Otherwise output from flat xml will be messed up.
	setXmlOptions(XML_PARSE_NOBLANKS);
	//in the beggining of xml stream, there are some options which we do not want to parse
	disableText(true);
	if (!extractText(xml_content, mode, formatting_style, NULL, text, getInnerLinks()))
	{
		getLogStream() << "Error parsing Flat XML file\n";
		setError(true);
		return "";
	}
	decodeSpecialLinkBlocks(text, getInnerLinks(), getLogStream());
	return text;
}

Metadata ODFXMLParser::metaData()
{
	Metadata metadata;
	setError(false);

	std::string xml_content;
	if (extended_impl->m_buffer_size > 0)
		xml_content = std::string(extended_impl->m_buffer, extended_impl->m_buffer_size);
	else
	{
		std::ifstream file(extended_impl->m_file_name.c_str(), std::ios_base::in|std::ios_base::binary);
		if (!file.is_open())
		{
			setError(true);
			getLogStream() << "Error opening file: " + extended_impl->m_file_name + "\n";
			return metadata;
		}
		xml_content = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		file.close();
	}
	if (!parseODFMetadata(xml_content, metadata))
	{
		getLogStream() << "Error parsing Flat XML file\n";
		setError(true);
	}
	if (metadata.pageCount() == -1)
	{
		// If we are processing ODP use slide count as page count
		// If we are processing ODG extract page count the same way
		if (xml_content.find("<office:presentation") != std::string::npos ||
			xml_content.find("<office:drawing") != std::string::npos)
		{
			int page_count = 0;
			std::string page_str = "<draw:page ";
			for (size_t pos = xml_content.find(page_str); pos != std::string::npos;
					pos = xml_content.find(page_str, pos + page_str.length()))
				page_count++;
			metadata.setPageCount(page_count);
		}
	}
	return metadata;
}
