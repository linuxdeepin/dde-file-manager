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

#include "odf_ooxml_parser.h"

#include "xml_fixer.h"
#include "doctotext_unzip.h"
#include <fstream>
#include <iostream>
#include <libxml/xmlreader.h>
#include <map>
#include "metadata.h"
#include "misc.h"
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include "xml_stream.h"

using namespace std;

const int CASESENSITIVITY = 1;

static string locate_main_file(const DocToTextUnzip& zipfile, std::ostream& log_stream)
{
	if (zipfile.exists("content.xml"))
		return "content.xml";
	if (zipfile.exists("word/document.xml"))
		return "word/document.xml";
	if (zipfile.exists("xl/workbook.xml"))
		return "xl/workbook.xml";
	if (zipfile.exists("ppt/presentation.xml"))
		return "ppt/presentation.xml";
	log_stream << "Error - no content.xml, no word/document.xml and no ppt/presentation.xml" << endl;
	return "";
}

class ODFOOXMLParser::CommandHandlersSet
{
	public:
		static void onOOXMLAttribute(CommonXMLDocumentParser& parser, XmlStream& xml_stream, XmlParseMode mode,
									 const FormattingStyle& options, const DocToTextUnzip* zipfile, std::string& text,
									 bool& children_processed, std::string& level_suffix, bool first_on_level,
									 std::vector<Link>& links)
		{
			if (parser.verbose())
				parser.getLogStream() << "OOXML_ATTR command.\n";
			children_processed = true;
		}

		static void onOOXMLCell(CommonXMLDocumentParser& parser, XmlStream& xml_stream, XmlParseMode mode,
								const FormattingStyle& options, const DocToTextUnzip* zipfile, std::string& text,
								bool& children_processed, std::string& level_suffix, bool first_on_level,
								std::vector<Link>& links)
		{
			if (parser.verbose())
				parser.getLogStream() << "OOXML_CELL command.\n";
			if (!first_on_level)
				text += "\t";
			if (xml_stream.attribute("t") == "s")
			{
				xml_stream.levelDown();
				int shared_string_index = str_to_int(parser.parseXmlData(xml_stream, mode, options, zipfile, links));
				xml_stream.levelUp();
				if (shared_string_index < parser.getSharedStrings().size())
				{
					text += parser.getSharedStrings()[shared_string_index].m_text;
					links.insert(links.begin(), parser.getSharedStrings()[shared_string_index].m_links.begin(), parser.getSharedStrings()[shared_string_index].m_links.end());
				}
			}
			else
			{
				xml_stream.levelDown();
				text += parser.parseXmlData(xml_stream, mode, options, zipfile, links);
				xml_stream.levelUp();
			}
			level_suffix = "\n";
			children_processed = true;
		}

		static void onOOXMLHeaderFooter(CommonXMLDocumentParser& parser, XmlStream& xml_stream, XmlParseMode mode,
										const FormattingStyle& options, const DocToTextUnzip* zipfile, std::string& text,
										bool& children_processed, std::string& level_suffix, bool first_on_level,
										std::vector<Link>& links)
		{
			if (parser.verbose())
				parser.getLogStream() << "OOXML_HEADERFOOTER command.\n";
			// Ignore headers and footers. They can contain some commands like font settings that can mess up output.
			#warning TODO: Better headers and footers support
			children_processed = true;
		}

		static void onOOXMLCommentReference(CommonXMLDocumentParser& parser, XmlStream& xml_stream, XmlParseMode mode,
											const FormattingStyle& options, const DocToTextUnzip* zipfile, std::string& text,
											bool& children_processed, std::string& level_suffix, bool first_on_level,
											std::vector<Link>& links)
		{
			if (parser.verbose())
				parser.getLogStream() << "OOXML_COMMENTREFERENCE command.\n";
			int comment_id = str_to_int(xml_stream.attribute("id"));
			if (parser.getComments().count(comment_id))
			{
				const Comment& c = parser.getComments()[comment_id];
				text += parser.formatComment(c.m_author, c.m_time, c.m_text);
				links.insert(links.end(), c.m_links.begin(), c.m_links.end());
			}
			else
				parser.getLogStream() << "Comment with id " << comment_id << " not found, skipping.\n";
		}

		static void onOOXMLBreak(CommonXMLDocumentParser& parser, XmlStream& xml_stream, XmlParseMode mode,
								 const FormattingStyle& options, const DocToTextUnzip* zipfile, std::string& text,
								 bool& children_processed, std::string& level_suffix, bool first_on_level,
								 std::vector<Link>& links)
		{
			if (parser.verbose())
				parser.getLogStream() << "OOXML_BREAK command.\n";
			text += "\n";
		}

		static void onOOXMLInstrtext(CommonXMLDocumentParser& parser, XmlStream& xml_stream, XmlParseMode mode,
									 const FormattingStyle& options, const DocToTextUnzip* zipfile, std::string& text,
									 bool& children_processed, std::string& level_suffix, bool first_on_level,
									 std::vector<Link>& links)
		{
			if (parser.verbose())
				parser.getLogStream() << "OOXML_INSTRTEXT command.\n";
			children_processed = true;
		}
};

struct ODFOOXMLParser::ExtendedImplementation
{
	const char* m_buffer;
	size_t m_buffer_size;
	std::string m_file_name;
	ODFOOXMLParser* m_interf;

	bool readOOXMLComments(const DocToTextUnzip& zipfile, XmlParseMode mode, FormattingStyle& options)
	{
		std::string content;
		if (!zipfile.read("word/comments.xml", &content))
		{
			m_interf->getLogStream() << "Error reading word/comments.xml" << endl;
			return false;
		}
		std::string xml;
		if (mode == FIX_XML)
		{
			DocToTextXmlFixer xml_fixer;
			xml = xml_fixer.fix(content);
		}
		else
			xml = content;
		XmlStream xml_stream(xml, m_interf->manageXmlParser(), m_interf->getXmlOptions());
		if (!xml_stream)
		{
			m_interf->getLogStream() << "Error parsing word/comments.xml" << endl;
			return false;
		}
		xml_stream.levelDown();
		while (xml_stream)
		{
			if (xml_stream.name() == "comment")
			{
				std::vector<Link> links;
				int id = str_to_int(xml_stream.attribute("id"));
				std::string author = xml_stream.attribute("author");
				std::string date = xml_stream.attribute("date");
				xml_stream.levelDown();
				std::string text = m_interf->parseXmlData(xml_stream, mode, options, &zipfile, links);
				xml_stream.levelUp();
				CommonXMLDocumentParser::Comment c(author, date, text, links);
				m_interf->getComments()[id] = c;
			}
			xml_stream.next();
		}
		return true;
	}

	void readStyles(const DocToTextUnzip& zipfile, XmlParseMode mode, FormattingStyle options)
	{
		std::string content;
		if (!zipfile.read("styles.xml", &content))
		{

			m_interf->getLogStream() << "Error reading styles.xml" << endl;
			return;
		}
		std::string xml;
		if (mode == FIX_XML)
		{
			DocToTextXmlFixer xml_fixer;
			xml = xml_fixer.fix(content);
		}
		else
			xml = content;
		XmlStream xml_stream(xml, m_interf->manageXmlParser(), m_interf->getXmlOptions());
		if (!xml_stream)
		{
			m_interf->getLogStream() << "Error parsing styles.xml" << endl;
			return;
		}
		//we dont need those links (we dont expect them in "styles". But parseXmlData require this
		std::vector<Link> links;
		m_interf->parseXmlData(xml_stream, mode, options, &zipfile, links);
	}
};

ODFOOXMLParser::ODFOOXMLParser(const string& file_name)
{
	extended_impl = NULL;
	try
	{
		extended_impl = new ExtendedImplementation();
		extended_impl->m_file_name = file_name;
		extended_impl->m_buffer = NULL;
		extended_impl->m_buffer_size = 0;
		extended_impl->m_interf = this;
		registerODFOOXMLCommandHandler("attrName", &CommandHandlersSet::onOOXMLAttribute);
		registerODFOOXMLCommandHandler("c", &CommandHandlersSet::onOOXMLCell);
		registerODFOOXMLCommandHandler("headerFooter", &CommandHandlersSet::onOOXMLHeaderFooter);
		registerODFOOXMLCommandHandler("commentReference", &CommandHandlersSet::onOOXMLCommentReference);
		registerODFOOXMLCommandHandler("br", &CommandHandlersSet::onOOXMLBreak);
		registerODFOOXMLCommandHandler("instrText", &CommandHandlersSet::onOOXMLInstrtext);
	}
	catch (std::bad_alloc& ba)
	{
		if (extended_impl)
			delete extended_impl;
		cleanUp();
		throw;
	}
}

ODFOOXMLParser::ODFOOXMLParser(const char *buffer, size_t size)
{
	extended_impl = NULL;
	try
	{
		extended_impl = new ExtendedImplementation();
		extended_impl->m_file_name = "Memory buffer";
		extended_impl->m_buffer = buffer;
		extended_impl->m_buffer_size = size;
		extended_impl->m_interf = this;
		registerODFOOXMLCommandHandler("attrName", &CommandHandlersSet::onOOXMLAttribute);
		registerODFOOXMLCommandHandler("c", &CommandHandlersSet::onOOXMLCell);
		registerODFOOXMLCommandHandler("headerFooter", &CommandHandlersSet::onOOXMLHeaderFooter);
		registerODFOOXMLCommandHandler("commentReference", &CommandHandlersSet::onOOXMLCommentReference);
		registerODFOOXMLCommandHandler("br", &CommandHandlersSet::onOOXMLBreak);
		registerODFOOXMLCommandHandler("instrText", &CommandHandlersSet::onOOXMLInstrtext);
	}
	catch (std::bad_alloc& ba)
	{
		if (extended_impl)
			delete extended_impl;
		cleanUp();
		throw;
	}
}

ODFOOXMLParser::~ODFOOXMLParser()
{
	if (extended_impl)
		delete extended_impl;
}

bool ODFOOXMLParser::isODFOOXML()
{
	setError(false);
	if (!extended_impl->m_buffer)	//check file
	{
		FILE* f = fopen(extended_impl->m_file_name.c_str(), "r");
		if (f == NULL)
		{
			getLogStream() << "Error opening file " << extended_impl->m_file_name << ".\n";
			setError(true);
			return false;
		}
		fclose(f);
	}
	else if (extended_impl->m_buffer_size == 0)
	{
		getLogStream() << "Memory buffer is empty.\n";
		setError(true);
		return false;
	}
	DocToTextUnzip zipfile;
	if (extended_impl->m_buffer)
		zipfile.setBuffer(extended_impl->m_buffer, extended_impl->m_buffer_size);
	else
		zipfile.setArchiveFile(extended_impl->m_file_name);
	if (&getLogStream() != &std::cerr)
		zipfile.setLogStream(getLogStream());
	if (!zipfile.open())
		return false;
	string main_file_name = locate_main_file(zipfile, getLogStream());
	if (main_file_name == "")
	{
		zipfile.close();
		return false;
	}
	string contents;
	if (!zipfile.read(main_file_name, &contents, 1))
	{
		zipfile.close();
		return false;
	}
	zipfile.close();
	return true;
}

string ODFOOXMLParser::plainText(XmlParseMode mode, FormattingStyle& options)
{
	setError(false);
	DocToTextUnzip zipfile;
	if (extended_impl->m_buffer)
		zipfile.setBuffer(extended_impl->m_buffer, extended_impl->m_buffer_size);
	else
		zipfile.setArchiveFile(extended_impl->m_file_name);
	if (&getLogStream() != &std::cerr)
		zipfile.setLogStream(getLogStream());
	if (!zipfile.open())
	{
		getLogStream() << "Error opening file " << extended_impl->m_file_name.c_str() << endl;
		setError(true);
		return "";
	}
	string main_file_name = locate_main_file(zipfile, getLogStream());
	if (main_file_name == "")
	{
		getLogStream() << "Could not locate main file.\n";
		setError(true);
		zipfile.close();
		return "";
	}
	//according to the ODF specification, we must skip blank nodes. Otherwise output may be messed up.
	if (main_file_name == "content.xml")
		setXmlOptions(XML_PARSE_NOBLANKS);
	if (zipfile.exists("word/comments.xml"))
		if (!extended_impl->readOOXMLComments(zipfile, mode, options))
		{
			getLogStream() << "Error parsing comments.\n";
			setError(true);
			zipfile.close();
			return "";
		}
	if (zipfile.exists("styles.xml"))
		extended_impl->readStyles(zipfile, mode, options);
	string content;
	string text;
	if (main_file_name == "ppt/presentation.xml")
	{
		if (!zipfile.loadDirectory())
		{
			getLogStream() << "Error loading zip directory of file " << extended_impl->m_file_name.c_str() << endl;
			setError(true);
			return "";
		}
		for (int i = 1; zipfile.read("ppt/slides/slide" + int_to_str(i) + ".xml", &content) && i < 2500; i++)
		{
			string slide_text;
			if (!extractText(content, mode, options, &zipfile, slide_text, getInnerLinks()))
			{
				getLogStream() << "Parser error." << endl;
				setError(true);
				zipfile.close();
				return "";
			}
			text += slide_text;
		}
	}
	else if (main_file_name == "xl/workbook.xml")
	{
		if (!zipfile.read("xl/sharedStrings.xml", &content))
		{
			//file may not exist, but this is not reason to report an error.
			if (verbose())
				getLogStream() << "xl/sharedStrings.xml does not exist" << endl;
		}
		else
		{
			std::string xml;
			if (mode == FIX_XML)
			{
				DocToTextXmlFixer xml_fixer;
				xml = xml_fixer.fix(content);
			}
			else if (mode == PARSE_XML)
				xml = content;
			else
			{
				getLogStream() << "XML stripping not possible for xlsx format" << endl;
				setError(true);
				zipfile.close();
				return "";
			}
			XmlStream xml_stream(xml, manageXmlParser(), getXmlOptions());
			if (!xml_stream)
			{
				getLogStream() << "Error parsing xl/sharedStrings.xml" << endl;
				setError(true);
				zipfile.close();
				return "";
			}
			xml_stream.levelDown();
			while (xml_stream)
			{
				if (xml_stream.name() == "si")
				{
					xml_stream.levelDown();
					SharedString shared_string;
					shared_string.m_text = parseXmlData(xml_stream, mode, options, &zipfile, shared_string.m_links);
					getSharedStrings().push_back(shared_string);
					xml_stream.levelUp();
				}
				xml_stream.next();
			}
		}
		for (int i = 1; zipfile.read("xl/worksheets/sheet" + int_to_str(i) + ".xml", &content); i++)
		{
			string sheet_text;
			if (!extractText(content, mode, options, &zipfile, sheet_text, getInnerLinks()))
			{
				getLogStream() << "parser error" << endl;
				setError(true);
				zipfile.close();
				return "";
			}
			text += sheet_text;
		}
	}
	else
	{
		if (!zipfile.read(main_file_name, &content))
		{
			getLogStream() << "Error reading " << main_file_name << endl;
			setError(true);
			zipfile.close();
			return "";
		}
		if (!extractText(content, mode, options, &zipfile, text, getInnerLinks()))
		{
			getLogStream() << "Parser error." << endl;
			setError(true);
			zipfile.close();
			return "";
		}
	}
	decodeSpecialLinkBlocks(text, getInnerLinks(), getLogStream());
	zipfile.close();
	return text;
}

Metadata ODFOOXMLParser::metaData()
{
	setError(false);
	Metadata meta;
	DocToTextUnzip zipfile;
	if (extended_impl->m_buffer)
		zipfile.setBuffer(extended_impl->m_buffer, extended_impl->m_buffer_size);
	else
		zipfile.setArchiveFile(extended_impl->m_file_name);
	if (&getLogStream() != &std::cerr)
		zipfile.setLogStream(getLogStream());
	if (!zipfile.open())
	{
		getLogStream() << "Error opening file " << extended_impl->m_file_name.c_str() << endl;
		setError(true);
		return meta;
	}
	if (zipfile.exists("meta.xml"))
	{
		std::string meta_xml;
		if (!zipfile.read("meta.xml", &meta_xml))
		{
			getLogStream() << "Error reading meta.xml" << endl;
			zipfile.close();
			setError(true);
			return meta;
		}
		if (!parseODFMetadata(meta_xml, meta))
		{
			getLogStream() << "Error reading meta.xml" << endl;
			zipfile.close();
			setError(true);
			return meta;
		}
	}
	else if (zipfile.exists("docProps/core.xml"))
	{
		std::string core_xml;
		if (!zipfile.read("docProps/core.xml", &core_xml))
		{
			getLogStream() << "Error reading docProps/core.xml" << endl;
			zipfile.close();
			setError(true);
			return meta;
		}
		XmlStream xml_stream(core_xml, manageXmlParser(), getXmlOptions());
		if (!xml_stream)
		{
			setError(true);
			return meta;
		}
		xml_stream.levelDown();
		while (xml_stream)
		{
			if (xml_stream.name() == "creator")
				meta.setAuthor(xml_stream.stringValue());
			if (xml_stream.name() == "created")
			{
				tm creation_date;
				string_to_date(xml_stream.stringValue(), creation_date);
				meta.setCreationDate(creation_date);
			}
			if (xml_stream.name() == "lastModifiedBy")
				meta.setLastModifiedBy(xml_stream.stringValue());
			if (xml_stream.name() == "modified")
			{
				tm last_modification_date;
				string_to_date(xml_stream.stringValue(), last_modification_date);
				meta.setLastModificationDate(last_modification_date);
			}
			xml_stream.next();
		}

		std::string app_xml;
		if (!zipfile.read("docProps/app.xml", &app_xml))
		{
			getLogStream() << "Error reading docProps/app.xml" << endl;
			zipfile.close();
			setError(true);
			return meta;
		}
		XmlStream app_stream(app_xml, manageXmlParser(), getXmlOptions());
		if (!app_stream)
		{
			setError(true);
			return meta;
		}
		app_stream.levelDown();
		while (app_stream)
		{
			if (app_stream.name() == "Pages")
				meta.setPageCount(str_to_int(app_stream.stringValue()));
			if (app_stream.name() == "Words")
				meta.setWordCount(str_to_int(app_stream.stringValue()));
			app_stream.next();
		}
	}
	if (meta.pageCount() == -1)
	{
		// If we are processing PPT use slide count as page count
		if (zipfile.exists("ppt/presentation.xml"))
		{
			int page_count = 0;
			for (int i = 1; zipfile.exists("ppt/slides/slide" + int_to_str(i) + ".xml"); i++)
				page_count++;
			meta.setPageCount(page_count);
		}
		else if (zipfile.exists("content.xml"))
		{
			// If we are processing ODP use slide count as page count
			// If we are processing ODG extract page count the same way
			std::string content;
			if (zipfile.read("content.xml", &content) &&
				(content.find("<office:presentation") != std::string::npos ||
				content.find("<office:drawing") != std::string::npos))
			{
				int page_count = 0;
				std::string page_str = "<draw:page ";
				for (size_t pos = content.find(page_str); pos != std::string::npos;
						pos = content.find(page_str, pos + page_str.length()))
					page_count++;
				meta.setPageCount(page_count);
			}
		}
	}
	zipfile.close();
	return meta;
}
