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

#include "plain_text_extractor.h"

#include <algorithm>
#include <cstring>
#include "doc_parser.h"
#include "eml_parser.h"
#include "html_parser.h"
#include "iwork_parser.h"
#include "pdf_parser.h"
#include "ppt_parser.h"
#include "rtf_parser.h"
#include "odf_ooxml_parser.h"
#include "odfxml_parser.h"
#include <iostream>
#include <stdio.h>
#include <sstream>
#include <sys/stat.h>
#include <time.h>
#ifdef WIN32
	#include <windows.h>
	#include "accctrl.h"
	#include "aclapi.h"
#else
	#include <pwd.h>
#endif
#include "xls_parser.h"
#include "xlsb_parser.h"
#include "txt_parser.h"

using namespace doctotext;

struct PlainTextExtractor::Implementation
{
	ParserType m_parser_type;
	FormattingStyle m_formatting_style;
	XmlParseMode m_xml_parse_mode;
	bool m_verbose_logging;
	std::ostream* m_log_stream;
	bool m_manage_xml_parser;
	std::vector<Link> m_links;
	std::vector<Attachment> m_attachments;
	PlainTextExtractor* m_interf;

	bool isRTF(RTFParser& rtf, bool& error, ParserType& parser_type)
	{
		if (m_verbose_logging)
			rtf.setVerboseLogging(true);
		if (m_log_stream != &std::cerr)
			rtf.setLogStream(*m_log_stream);
		bool is_rtf = rtf.isRTF();
		if (rtf.error())
		{
			error = true;
			return false;
		}
		if (is_rtf)
		{
			parser_type = PARSER_RTF;
			return true;
		}
		return false;
	}

	bool isODFOOXML(ODFOOXMLParser& odfooxml, bool& error, ParserType& parser_type)
	{
		if (m_verbose_logging)
			odfooxml.setVerboseLogging(true);
		if (m_log_stream != &std::cerr)
			odfooxml.setLogStream(*m_log_stream);
		if (!m_manage_xml_parser)
			odfooxml.setManageXmlParser(false);
		bool is_odfooxml = odfooxml.isODFOOXML();
		if (odfooxml.error())
		{
			error = true;
			return false;
		}
		if (is_odfooxml)
		{
			parser_type = PARSER_ODF_OOXML;
			return true;
		}
		return false;
	}

	bool isXLS(XLSParser& xls, bool& error, ParserType& parser_type)
	{
		if (m_verbose_logging)
			xls.setVerboseLogging(true);
		if (m_log_stream != &std::cerr)
			xls.setLogStream(*m_log_stream);
		bool is_xls = xls.isXLS();
		if (xls.error())
		{
			error = true;
			return false;
		}
		if (is_xls)
		{
			parser_type = PARSER_XLS;
			return true;
		}
		return false;
	}

	bool isDOC(DOCParser& doc, bool& error, ParserType& parser_type)
	{
		if (m_verbose_logging)
			doc.setVerboseLogging(true);
		if (m_log_stream != &std::cerr)
			doc.setLogStream(*m_log_stream);
		bool is_doc = doc.isDOC();
		if (doc.error())
		{
			error = true;
			return false;
		}
		if (is_doc)
		{
			parser_type = PARSER_DOC;
			return true;
		}
		return false;
	}

	bool isPPT(PPTParser& ppt, bool& error, ParserType& parser_type)
	{
		if (m_verbose_logging)
			ppt.setVerboseLogging(true);
		if (m_log_stream != &std::cerr)
			ppt.setLogStream(*m_log_stream);
		bool is_ppt = ppt.isPPT();
		if (ppt.error())
		{
			error = true;
			return false;
		}
		if (is_ppt)
		{
			parser_type = PARSER_PPT;
			return true;
		}
		return false;
	}

	bool isHTML(HTMLParser& html, bool& error, ParserType& parser_type)
	{
		if (m_verbose_logging)
			html.setVerboseLogging(true);
		if (m_log_stream != &std::cerr)
			html.setLogStream(*m_log_stream);
		bool is_html = html.isHTML();
		if (html.error())
		{
			error = true;
			return false;
		}
		if (is_html)
		{
			parser_type = PARSER_HTML;
			return true;
		}
		return false;
	}

	bool isIWork(IWorkParser& iwork, bool& error, ParserType& parser_type)
	{
		if (m_verbose_logging)
			iwork.setVerboseLogging(true);
		if (m_log_stream != &std::cerr)
			iwork.setLogStream(*m_log_stream);
		bool is_iwork = iwork.isIWork();
		if (iwork.error())
		{
			error = true;
			return false;
		}
		if (is_iwork)
		{
			parser_type = PARSER_IWORK;
			return true;
		}
		return false;
	}

	bool isXLSB(XLSBParser& xlsb, bool& error, ParserType& parser_type)
	{
		if (m_verbose_logging)
			xlsb.setVerboseLogging(true);
		if (m_log_stream != &std::cerr)
			xlsb.setLogStream(*m_log_stream);
		bool is_xlsb = xlsb.isXLSB();
		if (xlsb.error())
		{
			error = true;
			return false;
		}
		if (is_xlsb)
		{
			parser_type = PARSER_XLSB;
			return true;
		}
		return false;
	}

	bool isPDF(PDFParser& pdf, bool& error, ParserType& parser_type)
	{
		if (m_verbose_logging)
			pdf.setVerboseLogging(true);
		if (m_log_stream != &std::cerr)
			pdf.setLogStream(*m_log_stream);
		bool is_pdf = pdf.isPDF();
		if (pdf.error())
		{
			error = true;
			return false;
		}
		if (is_pdf)
		{
			parser_type = PARSER_PDF;
			return true;
		}
		return false;
	}

	bool isEML(EMLParser& eml, bool& error, ParserType& parser_type)
	{
		if (m_verbose_logging)
			eml.setVerboseLogging(true);
		if (m_log_stream != &std::cerr)
			eml.setLogStream(*m_log_stream);
		bool is_eml = eml.isEML();
		if (eml.error())
		{
			error = true;
			return false;
		}
		if (is_eml)
		{
			parser_type = PARSER_EML;
			return true;
		}
		return false;
	}

	bool isODFXML(ODFXMLParser& odfxml, bool& error, ParserType& parser_type)
	{
		if (m_verbose_logging)
			odfxml.setVerboseLogging(true);
		if (m_log_stream != &std::cerr)
			odfxml.setLogStream(*m_log_stream);
		bool is_odfxml = odfxml.isODFXML();
		if (odfxml.error())
		{
			error = true;
			return false;
		}
		if (is_odfxml)
		{
			parser_type = PARSER_ODFXML;
			return true;
		}
		return false;
	}

	bool parseRTF(RTFParser& rtf, std::string& text)
	{
		if (m_verbose_logging)
			rtf.setVerboseLogging(true);
		if (m_log_stream != &std::cerr)
			rtf.setLogStream(*m_log_stream);
//		*m_log_stream << "Using RTF parser.\n";
		text = rtf.plainText();
		m_links.clear();
		rtf.getLinks(m_links);
		return rtf.error();
	}

	bool parseODFOOXML(ODFOOXMLParser& odfooxml, std::string& text)
	{
		if (m_verbose_logging)
			odfooxml.setVerboseLogging(true);
		if (m_log_stream != &std::cerr)
			odfooxml.setLogStream(*m_log_stream);
		if (!m_manage_xml_parser)
			odfooxml.setManageXmlParser(false);
//		*m_log_stream << "Using ODF/OOXML parser.\n";
		text = odfooxml.plainText(m_xml_parse_mode, m_formatting_style);
		m_links.clear();
		odfooxml.getLinks(m_links);
		return odfooxml.error();
	}

	bool parseXLS(XLSParser& xls, std::string& text)
	{
		if (m_verbose_logging)
			xls.setVerboseLogging(true);
		if (m_log_stream != &std::cerr)
			xls.setLogStream(*m_log_stream);
//		*m_log_stream << "Using XLS parser.\n";
		text = xls.plainText(m_formatting_style);
		m_links.clear();
		xls.getLinks(m_links);
		return xls.error();
	}

	bool parseDOC(DOCParser& doc, std::string& text)
	{
		if (m_verbose_logging)
			doc.setVerboseLogging(true);
		if (m_log_stream != &std::cerr)
			doc.setLogStream(*m_log_stream);
//		*m_log_stream << "Using DOC parser.\n";
		text = doc.plainText(m_formatting_style);
		m_links.clear();
		doc.getLinks(m_links);
		return doc.error();
	}

	bool parsePPT(PPTParser& ppt, std::string& text)
	{
		if (m_verbose_logging)
			ppt.setVerboseLogging(true);
		if (m_log_stream != &std::cerr)
			ppt.setLogStream(*m_log_stream);
//		*m_log_stream << "Using PPT parser.\n";
		text = ppt.plainText(m_formatting_style);
		m_links.clear();
		ppt.getLinks(m_links);
		return ppt.error();
	}

	bool parseHTML(HTMLParser& html, std::string& text)
	{
		if (m_verbose_logging)
			html.setVerboseLogging(true);
		if (m_log_stream != &std::cerr)
			html.setLogStream(*m_log_stream);
//		*m_log_stream << "Using HTML parser.\n";
		text = html.plainText(m_formatting_style);
		m_links.clear();
		html.getLinks(m_links);
		return html.error();
	}

	bool parseIWork(IWorkParser& iwork, std::string& text)
	{
		if (m_verbose_logging)
			iwork.setVerboseLogging(true);
		if (m_log_stream != &std::cerr)
			iwork.setLogStream(*m_log_stream);
//		*m_log_stream << "Using iWork parser.\n";
		text = iwork.plainText(m_formatting_style);
		m_links.clear();
		iwork.getLinks(m_links);
		return iwork.error();
	}

	bool parseXLSB(XLSBParser& xlsb, std::string& text)
	{
		if (m_verbose_logging)
			xlsb.setVerboseLogging(true);
		if (m_log_stream != &std::cerr)
			xlsb.setLogStream(*m_log_stream);
//		*m_log_stream << "Using XLSB parser.\n";
		text = xlsb.plainText(m_formatting_style);
		m_links.clear();
		xlsb.getLinks(m_links);
		return xlsb.error();
	}

	//解析 pdf 文件
	bool parsePDF(PDFParser& pdf, std::string& text)
	{
		if (m_verbose_logging)
		{
			pdf.setVerboseLogging(true);
		}

		if (m_log_stream != &std::cerr)
		{
			pdf.setLogStream(*m_log_stream);
		}

//		*m_log_stream << "Using PDF parser.\n";

		//进行解析
		text = pdf.plainText(m_formatting_style);

		m_links.clear();
		pdf.getLinks(m_links);

		return pdf.error();
	}

	bool parseTXT(TXTParser& txt, std::string& text)
	{
		if (m_verbose_logging)
			txt.setVerboseLogging(true);
		if (m_log_stream != &std::cerr)
			txt.setLogStream(*m_log_stream);
//		*m_log_stream << "Using TXT parser.\n";
		text = txt.plainText();
		m_links.clear();
		return txt.error();
	}

	bool parseEML(EMLParser& eml, std::string& text)
	{
		if (m_verbose_logging)
			eml.setVerboseLogging(true);
		if (m_log_stream != &std::cerr)
			eml.setLogStream(*m_log_stream);
//		*m_log_stream << "Using EML parser.\n";
		text = eml.plainText(m_formatting_style);
		m_links.clear();
		eml.getLinks(m_links);
		m_attachments.clear();
		eml.getAttachments(m_attachments);
		//try to get additional metadata
		for (size_t i = 0; i < m_attachments.size(); ++i)
		{
			Metadata metadata;
			if (m_interf->extractMetadata(m_attachments[i].binaryContent(), m_attachments[i].binaryContentSize(), metadata))
			{
				if (metadata.authorType() != Metadata::NONE)
					m_attachments[i].addField("author", Variant(metadata.author()));
				if (metadata.creationDateType() != Metadata::NONE)
					m_attachments[i].addField("creation date", metadata.creationDate());
				if (metadata.lastModificationDateType() != Metadata::NONE)
					m_attachments[i].addField("last modification date", metadata.lastModificationDate());
				if (metadata.lastModifiedByType() != Metadata::NONE)
					m_attachments[i].addField("last modified by", Variant(metadata.lastModifiedBy()));
				if (metadata.pageCountType() != Metadata::NONE)
					m_attachments[i].addField("page count", (size_t)metadata.pageCount());
				if (metadata.wordCountType() != Metadata::NONE)
					m_attachments[i].addField("word count", (size_t)metadata.wordCount());
			}
		}
		return eml.error();
	}

	bool parseODFXML(ODFXMLParser& odfxml, std::string& text)
	{
		if (m_verbose_logging)
			odfxml.setVerboseLogging(true);
		if (m_log_stream != &std::cerr)
			odfxml.setLogStream(*m_log_stream);
		if (!m_manage_xml_parser)
			odfxml.setManageXmlParser(false);
//		*m_log_stream << "Using ODFXML parser.\n";
		text = odfxml.plainText(m_xml_parse_mode, m_formatting_style);
		m_links.clear();
		odfxml.getLinks(m_links);
		return odfxml.error();
	}

	bool parseRTFMetadata(RTFParser& rtf, Metadata& metadata)
	{
		if (m_verbose_logging)
			rtf.setVerboseLogging(true);
		if (m_log_stream != &std::cerr)
			rtf.setLogStream(*m_log_stream);
//		*m_log_stream << "Using RTF parser.\n";
		metadata = rtf.metaData();
		return rtf.error();
	}

	bool parseODFOOXMLMetadata(ODFOOXMLParser& odfooxml, Metadata& metadata)
	{
		if (m_verbose_logging)
			odfooxml.setVerboseLogging(true);
		if (m_log_stream != &std::cerr)
			odfooxml.setLogStream(*m_log_stream);
		if (!m_manage_xml_parser)
			odfooxml.setManageXmlParser(false);
//		*m_log_stream << "Using ODF/OOXML parser.\n";
		metadata = odfooxml.metaData();
		return odfooxml.error();
	}

	bool parseXLSMetadata(XLSParser& xls, Metadata& metadata)
	{
		if (m_verbose_logging)
			xls.setVerboseLogging(true);
		if (m_log_stream != &std::cerr)
			xls.setLogStream(*m_log_stream);
//		*m_log_stream << "Using XLS parser.\n";
		metadata = xls.metaData();
		return xls.error();
	}

	bool parseDOCMetadata(DOCParser& doc, Metadata& metadata)
	{
		if (m_verbose_logging)
			doc.setVerboseLogging(true);
		if (m_log_stream != &std::cerr)
			doc.setLogStream(*m_log_stream);
//		*m_log_stream << "Using DOC parser.\n";
		metadata = doc.metaData();
		return doc.error();
	}

	bool parsePPTMetadata(PPTParser& ppt, Metadata& metadata)
	{
		if (m_verbose_logging)
			ppt.setVerboseLogging(true);
		if (m_log_stream != &std::cerr)
			ppt.setLogStream(*m_log_stream);
//		*m_log_stream << "Using PPT parser.\n";
		metadata = ppt.metaData();
		return ppt.error();
	}

	bool parseHTMLMetadata(HTMLParser& html, Metadata& metadata)
	{
		if (m_verbose_logging)
			html.setVerboseLogging(true);
		if (m_log_stream != &std::cerr)
			html.setLogStream(*m_log_stream);
//		*m_log_stream << "Using HTML parser.\n";
		metadata = html.metaData();
		return html.error();
	}

	bool parseIWorkMetadata(IWorkParser& iwork, Metadata& metadata)
	{
		if (m_verbose_logging)
			iwork.setVerboseLogging(true);
		if (m_log_stream != &std::cerr)
			iwork.setLogStream(*m_log_stream);
//		*m_log_stream << "Using IWork parser.\n";
		metadata = iwork.metaData();
		return iwork.error();
	}

	bool parseXLSBMetadata(XLSBParser& xlsb, Metadata& metadata)
	{
		if (m_verbose_logging)
			xlsb.setVerboseLogging(true);
		if (m_log_stream != &std::cerr)
			xlsb.setLogStream(*m_log_stream);
//		*m_log_stream << "Using XLSB parser.\n";
		metadata = xlsb.metaData();
		return xlsb.error();
	}

	bool parsePDFMetadata(PDFParser& pdf, Metadata& metadata)
	{
		if (m_verbose_logging)
			pdf.setVerboseLogging(true);
		if (m_log_stream != &std::cerr)
			pdf.setLogStream(*m_log_stream);
//		*m_log_stream << "Using PDF parser.\n";
		metadata = pdf.metaData();
		return pdf.error();
	}

	bool parseEMLMetadata(EMLParser& eml, Metadata& metadata)
	{
		if (m_verbose_logging)
			eml.setVerboseLogging(true);
		if (m_log_stream != &std::cerr)
			eml.setLogStream(*m_log_stream);
//		*m_log_stream << "Using EML parser.\n";
		metadata = eml.metaData();
		return eml.error();
	}

	bool parseODFXMLMetadata(ODFXMLParser& odfxml, Metadata& metadata)
	{
		if (m_verbose_logging)
			odfxml.setVerboseLogging(true);
		if (m_log_stream != &std::cerr)
			odfxml.setLogStream(*m_log_stream);
		if (!m_manage_xml_parser)
			odfxml.setManageXmlParser(false);
//		*m_log_stream << "Using ODFXML parser.\n";
		metadata = odfxml.metaData();
		return odfxml.error();
	}
};

PlainTextExtractor::PlainTextExtractor(ParserType parser_type)
{
	impl = new Implementation();
	impl->m_interf = this;
	impl->m_parser_type = parser_type;
	impl->m_formatting_style.table_style = TABLE_STYLE_TABLE_LOOK;
	impl->m_formatting_style.list_style.setPrefix(" * ");
	impl->m_formatting_style.url_style = URL_STYLE_UNDERSCORED;
	impl->m_xml_parse_mode = PARSE_XML;
	impl->m_verbose_logging = false;
	impl->m_log_stream = &std::cerr;
	impl->m_manage_xml_parser = true;
}

PlainTextExtractor::~PlainTextExtractor()
{
	delete impl;
}

void PlainTextExtractor::setVerboseLogging(bool verbose)
{
	impl->m_verbose_logging = verbose;
}

void PlainTextExtractor::setLogStream(std::ostream& log_stream)
{
	impl->m_log_stream = &log_stream;
}

void PlainTextExtractor::setFormattingStyle(const FormattingStyle& style)
{
	impl->m_formatting_style = style;
}

void PlainTextExtractor::setXmlParseMode(XmlParseMode mode)
{
	impl->m_xml_parse_mode = mode;
}

void PlainTextExtractor::setManageXmlParser(bool manage)
{
	impl->m_manage_xml_parser = manage;
}

PlainTextExtractor::ParserType PlainTextExtractor::parserTypeByFileExtension(const std::string& file_name)
{
	ParserType parser_type = PARSER_AUTO;
	std::string ext = file_name.substr(file_name.find_last_of(".") + 1);
	std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
	if (ext == "rtf")
		parser_type = PARSER_RTF;
	else if (ext == "odt" || ext == "ods" || ext == "odp" || ext == "odg" || ext == "docx" || ext == "xlsx" || ext == "pptx" || ext == "ppsx")
		parser_type = PARSER_ODF_OOXML;
	else if (ext == "xls")
		parser_type = PARSER_XLS;
	else if (ext == "xlsb")
		parser_type = PARSER_XLSB;
	else if (ext == "doc")
		parser_type = PARSER_DOC;
	else if (ext == "ppt" || ext == "pps")
		parser_type = PARSER_PPT;
	else if (ext == "htm" || ext =="html")
		parser_type = PARSER_HTML;
	else if (ext == "pages" || ext == "key" || ext == "numbers")
		parser_type = PARSER_IWORK;
	else if (ext == "pdf")
		parser_type = PARSER_PDF;
	else if (ext == "txt" || ext == "text")
		parser_type = PARSER_TXT;
	else if (ext == "eml")
		parser_type = PARSER_EML;
	else if (ext == "fodp" || ext == "fodt" || ext == "fods" || ext == "fodg")
		parser_type = PARSER_ODFXML;
	return parser_type;
}

PlainTextExtractor::ParserType PlainTextExtractor::parserTypeByFileExtension(const char* file_name)
{
	return parserTypeByFileExtension(std::string(file_name));
}

bool PlainTextExtractor::parserTypeByFileContent(const std::string& file_name, ParserType& parser_type)
{
	bool error = false;
	*impl->m_log_stream << "Trying to detect document format by its content.\n";

	RTFParser rtf(file_name);
	if (impl->isRTF(rtf, error, parser_type))
		return true;
	else if (error)
		return false;

	ODFOOXMLParser odfooxml(file_name);
	if (impl->isODFOOXML(odfooxml, error, parser_type))
		return true;
	else if (error)
		return false;

	XLSParser xls(file_name);
	if (impl->isXLS(xls, error, parser_type))
		return true;
	else if (error)
		return false;

	DOCParser doc(file_name);
	if (impl->isDOC(doc, error, parser_type))
		return true;
	else if (error)
		return false;

	PPTParser ppt(file_name);
	if (impl->isPPT(ppt, error, parser_type))
		return true;
	else if (error)
		return false;

	//We have to check if file is EML before HTML, because HTML can be included inside EML.
	EMLParser eml(file_name);
	if (impl->isEML(eml, error, parser_type))
		return true;
	else if (error)
		return false;

	HTMLParser html(file_name);
	if (impl->isHTML(html, error, parser_type))
		return true;
	else if (error)
		return false;

	IWorkParser iwork(file_name);
	if (impl->isIWork(iwork, error, parser_type))
		return true;
	else if (error)
		return false;

	XLSBParser xlsb(file_name);
	if (impl->isXLSB(xlsb, error, parser_type))
		return true;
	else if (error)
		return false;

	PDFParser pdf(file_name);
	if (impl->isPDF(pdf, error, parser_type))
		return true;
	else if (error)
		return false;

	ODFXMLParser odfxml(file_name);
	if (impl->isODFXML(odfxml, error, parser_type))
		return true;
	else if (error)
		return false;

	*impl->m_log_stream << "No maching parser found.\n";
	return false;
}

bool PlainTextExtractor::parserTypeByFileContent(const char* buffer, size_t size, ParserType& parser_type)
{
	bool error = false;
	*impl->m_log_stream << "Trying to detect document format by its content.\n";

	RTFParser rtf(buffer, size);
	if (impl->isRTF(rtf, error, parser_type))
		return true;
	else if (error)
		return false;

	ODFOOXMLParser odfooxml(buffer, size);
	if (impl->isODFOOXML(odfooxml, error, parser_type))
		return true;
	else if (error)
		return false;

	XLSParser xls(buffer, size);
	if (impl->isXLS(xls, error, parser_type))
		return true;
	else if (error)
		return false;

	DOCParser doc(buffer, size);
	if (impl->isDOC(doc, error, parser_type))
		return true;
	else if (error)
		return false;

	PPTParser ppt(buffer, size);
	if (impl->isPPT(ppt, error, parser_type))
		return true;
	else if (error)
		return false;

	//We have to check if file is EML before HTML, because HTML can be included inside EML.
	EMLParser eml(buffer, size);
	if (impl->isEML(eml, error, parser_type))
		return true;
	else if (error)
		return false;

	HTMLParser html(buffer, size);
	if (impl->isHTML(html, error, parser_type))
		return true;
	else if (error)
		return false;

	IWorkParser iwork(buffer, size);
	if (impl->isIWork(iwork, error, parser_type))
		return true;
	else if (error)
		return false;

	XLSBParser xlsb(buffer, size);
	if (impl->isXLSB(xlsb, error, parser_type))
		return true;
	else if (error)
		return false;

	PDFParser pdf(buffer, size);
	if (impl->isPDF(pdf, error, parser_type))
		return true;
	else if (error)
		return false;

	ODFXMLParser odfxml(buffer, size);
	if (impl->isODFXML(odfxml, error, parser_type))
		return true;
	else if (error)
		return false;

	*impl->m_log_stream << "No maching parser found.\n";
	return false;
}

bool PlainTextExtractor::parserTypeByFileContent(const char* file_name, ParserType& parser_type)
{
	return parserTypeByFileContent(std::string(file_name), parser_type);
}

/**
 * @brief 解析文件获取内容
 *
 * @param file_name 文件名称
 * @param text      解析后的文件内容
 *
 * @return 
 */
bool PlainTextExtractor::processFile(const std::string& file_name, std::string& text)
{
	ParserType parser_type = impl->m_parser_type;
	bool fallback = false;

	if (parser_type == PARSER_AUTO)
	{
		//根据后缀判断文件类型
		parser_type = parserTypeByFileExtension(file_name);
	}

	if (parser_type == PARSER_AUTO)
	{
		//根据内容判断文件类型
		if (!parserTypeByFileContent(file_name, parser_type))
		{
			return false;
		}
	}
	else
	{
		//根据后缀判断的类型可能不是正真的类型，需要留有后路，解析过程出错了
		//就重新做识别，再解析一遍
		fallback = true;
	}

	//解析文件
	return processFile(parser_type, fallback, file_name, text);
}

bool PlainTextExtractor::processFile(const char* file_name, char*& text)
{
	std::string plain_text;
	if (!processFile(std::string(file_name), plain_text))
		return false;
	text = new char[plain_text.length() + 1];
	std::strcpy(text, plain_text.c_str());
	return true;
}

bool PlainTextExtractor::processFile(const char* buffer, size_t size, char*& text)
{
	std::string plain_text;
	if (!processFile(buffer, size, plain_text))
		return false;
	text = new char[plain_text.length() + 1];
	std::strcpy(text, plain_text.c_str());
	return true;
}

bool PlainTextExtractor::processFile(const char* buffer, size_t size, std::string& text)
{
	ParserType parser_type = impl->m_parser_type;
	bool fallback = false;
	if (parser_type == PARSER_AUTO)
	{
		if (!parserTypeByFileContent(buffer, size, parser_type))
			return false;
	}
	else
		fallback = true;
	return processFile(parser_type, fallback, buffer, size, text);
}


/**
 * @brief 根据文件类型进行
 *
 * @param parser_type 文件类型
 * @param fallback    二次解析标识
 * @param file_name   文件名称
 * @param text        文件内容
 *
 * @return 
 */
bool PlainTextExtractor::processFile(ParserType parser_type, bool fallback, const std::string& file_name, std::string& text)
{
	if (parser_type == PARSER_AUTO)
	{
		return processFile(file_name, text);
	}

	bool error = true;
	switch (parser_type)
	{
		case PARSER_RTF:
		{
			RTFParser rtf(file_name);
			error = impl->parseRTF(rtf, text);
			break;
		}
		case PARSER_ODF_OOXML:
		{
			ODFOOXMLParser odfooxml(file_name);
			error = impl->parseODFOOXML(odfooxml, text);
			break;
		}
		case PARSER_XLS:
		{
			XLSParser xls(file_name);
			error = impl->parseXLS(xls, text);
			break;
		}
		case PARSER_DOC:
		{
			DOCParser doc(file_name);
			error = impl->parseDOC(doc, text);
			break;
		}
		case PARSER_PPT:
		{
			PPTParser ppt(file_name);
			error = impl->parsePPT(ppt, text);
			break;
		}
		case PARSER_HTML:
		{
			HTMLParser html(file_name);
			error = impl->parseHTML(html, text);
			break;
		}
		case PARSER_IWORK:
		{
			IWorkParser iwork(file_name);
			error = impl->parseIWork(iwork, text);
			break;
		}
		case PARSER_XLSB:
		{
			XLSBParser xlsb(file_name);
			error = impl->parseXLSB(xlsb, text);
			break;
		}
		case PARSER_PDF:
		{
			PDFParser pdf(file_name);
			error = impl->parsePDF(pdf, text);
			break;
		}
		case PARSER_TXT:
		{
			TXTParser txt(file_name);
			error = impl->parseTXT(txt, text);
			break;
		}
		case PARSER_EML:
		{
			EMLParser eml(file_name);
			error = impl->parseEML(eml, text);
			break;
		}
		case PARSER_ODFXML:
		{
			ODFXMLParser odfxml(file_name);
			error = impl->parseODFXML(odfxml, text);
			break;
		}
	}

	if (error && fallback)
	{
		*impl->m_log_stream << "It is possible that wrong parser was selected. Trying different parsers.\n";
		ParserType parser_type;
		if (!parserTypeByFileContent(file_name, parser_type))
			return false;
		return processFile(parser_type, false, file_name, text);
	}
		
	return !error;
}

bool PlainTextExtractor::processFile(ParserType parser_type, bool fallback, const char* buffer, size_t size, char*& text)
{
	std::string plain_text;
	if (!processFile(parser_type, fallback, buffer, size, plain_text))
		return false;
	text = new char[plain_text.length() + 1];
	std::strcpy(text, plain_text.c_str());
	return true;
}

bool PlainTextExtractor::processFile(ParserType parser_type, bool fallback, const char* buffer, size_t size, std::string& text)
{
	if (parser_type == PARSER_AUTO)
		return processFile(buffer, size, text);

	bool error = true;
	switch (parser_type)
	{
		case PARSER_RTF:
		{
			RTFParser rtf(buffer, size);
			error = impl->parseRTF(rtf, text);
			break;
		}
		case PARSER_ODF_OOXML:
		{
			ODFOOXMLParser odfooxml(buffer, size);
			error = impl->parseODFOOXML(odfooxml, text);
			break;
		}
		case PARSER_XLS:
		{
			XLSParser xls(buffer, size);
			error = impl->parseXLS(xls, text);
			break;
		}
		case PARSER_DOC:
		{
			DOCParser doc(buffer, size);
			error = impl->parseDOC(doc, text);
			break;
		}
		case PARSER_PPT:
		{
			PPTParser ppt(buffer, size);
			error = impl->parsePPT(ppt, text);
			break;
		}
		case PARSER_HTML:
		{
			HTMLParser html(buffer, size);
			error = impl->parseHTML(html, text);
			break;
		}
		case PARSER_IWORK:
		{
			IWorkParser iwork(buffer, size);
			error = impl->parseIWork(iwork, text);
			break;
		}
		case PARSER_XLSB:
		{
			XLSBParser xlsb(buffer, size);
			error = impl->parseXLSB(xlsb, text);
			break;
		}
		case PARSER_PDF:
		{
			PDFParser pdf(buffer, size);
			error = impl->parsePDF(pdf, text);
			break;
		}
		case PARSER_TXT:
		{
			TXTParser txt(buffer, size);
			error = impl->parseTXT(txt, text);
			break;
		}
		case PARSER_EML:
		{
			EMLParser eml(buffer, size);
			error = impl->parseEML(eml, text);
			break;
		}
		case PARSER_ODFXML:
		{
			ODFXMLParser odfxml(buffer, size);
			error = impl->parseODFXML(odfxml, text);
			break;
		}
	}

	if (error && fallback)
	{
		*impl->m_log_stream << "It is possible that wrong parser was selected. Trying different parsers.\n";
		ParserType parser_type;
		if (!parserTypeByFileContent(buffer, size, parser_type))
			return false;
		return processFile(parser_type, false, buffer, size, text);
	}

	return !error;
}

bool PlainTextExtractor::processFile(ParserType parser_type, bool fallback, const char* file_name, char*& text)
{
	std::string plain_text;
	if (!processFile(parser_type, fallback, std::string(file_name), plain_text))
		return false;
	text = new char[plain_text.length() + 1];
	std::strcpy(text, plain_text.c_str());
	return true;
}

bool get_file_owner(const std::string& file_name, std::ostream& log_stream, std::string& owner)
{
	#ifdef WIN32
		HANDLE file = CreateFile(file_name.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (file == INVALID_HANDLE_VALUE)
		{
			log_stream << "Error opening file \"" << file_name << "\" for getting owner information.\n";
			CloseHandle(file);
			return false;
		}
		PSID sid_owner;
		PSECURITY_DESCRIPTOR sec_desc;
		DWORD ret_code = GetSecurityInfo(file, SE_FILE_OBJECT, OWNER_SECURITY_INFORMATION, &sid_owner, NULL, NULL, NULL, &sec_desc);
		CloseHandle(file);
		if (ret_code != ERROR_SUCCESS)
		{
			log_stream << "Error getting owner SID.\n";
			return false;
		}
		LPTSTR account_name = NULL;
		LPTSTR domain_name = NULL;
		DWORD account_name_size = 0;
		DWORD domain_name_size = 0;
		SID_NAME_USE sid_type;
		BOOL res = LookupAccountSid(NULL, sid_owner, account_name, (LPDWORD)&account_name_size, domain_name, (LPDWORD)&domain_name_size, &sid_type);
		if (!res && GetLastError() != ERROR_INSUFFICIENT_BUFFER)
		{
			log_stream << "Error getting buffer sizes.\n";
			return false;
		}
		account_name = (LPTSTR)GlobalAlloc(GMEM_FIXED, account_name_size);
		if (account_name == NULL)
		{
			log_stream << "Error allocating memory.\n";
			return false;
		}
		domain_name = (LPTSTR)GlobalAlloc(GMEM_FIXED, domain_name_size);
		if (domain_name == NULL)
		{
			log_stream << "Error allocating memory.\n";
			return false;
		}
		res = LookupAccountSid(NULL, sid_owner, account_name, (LPDWORD)&account_name_size, domain_name, (LPDWORD)&domain_name_size, &sid_type);
		GlobalFree(domain_name);
		if (!res)
		{
			log_stream << "Error getting account owner.\n";
			return false;
		}
		owner = account_name;
		GlobalFree(account_name);
	#else
		struct stat buf;
		if (stat(file_name.c_str(), &buf) != 0)
		{
			log_stream << "Error retrieving file information.\n";
			return false;
		}
		struct passwd* pwd = getpwuid(buf.st_uid);
		if (pwd == NULL)
		{
			log_stream << "Incorrect time value.\n";
			return false;
		}
		owner = pwd->pw_name;
	#endif
	return true;
}

bool get_file_modification_time(const std::string& file_name, std::ostream& log_stream, tm& time)
{
	struct stat buf;
	if (stat(file_name.c_str(), &buf) != 0)
	{
		log_stream << "Error retrieving file information.\n";
		return false;
	}
	tm* res = gmtime(&buf.st_mtime);
	if (res == NULL)
	{
		log_stream << "Incorrect time value.\n";
		return false;
	}
	time = *res;
	return true;
}

bool PlainTextExtractor::extractMetadata(const std::string& file_name, Metadata& metadata)
{
	ParserType parser_type = impl->m_parser_type;
	bool fallback = false;
	if (parser_type == PARSER_AUTO)
		parser_type = parserTypeByFileExtension(file_name);
	if (parser_type == PARSER_AUTO)
	{
		if (!parserTypeByFileContent(file_name, parser_type))
			return false;
	}
	else
		fallback = true;
	return extractMetadata(parser_type, fallback, file_name, metadata);
}

bool PlainTextExtractor::extractMetadata(const char* file_name, Metadata& metadata)
{
	return extractMetadata(std::string(file_name), metadata);
}

bool PlainTextExtractor::extractMetadata(const char* buffer, size_t size, Metadata& metadata)
{
	ParserType parser_type = impl->m_parser_type;
	bool fallback = false;
	if (parser_type == PARSER_AUTO)
	{
		if (!parserTypeByFileContent(buffer, size, parser_type))
			return false;
	}
	else
		fallback = true;
	return extractMetadata(parser_type, fallback, buffer, size, metadata);
}

bool PlainTextExtractor::extractMetadata(ParserType parser_type, bool fallback, const std::string& file_name, Metadata& metadata)
{
	if (parser_type == PARSER_AUTO)
		return extractMetadata(file_name, metadata);

	bool error = true;
	switch (parser_type)
	{
		case PARSER_RTF:
		{
			RTFParser rtf(file_name);
			error = impl->parseRTFMetadata(rtf, metadata);
			break;
		}
		case PARSER_ODF_OOXML:
		{
			ODFOOXMLParser odfooxml(file_name);
			error = impl->parseODFOOXMLMetadata(odfooxml, metadata);
			break;
		}
		case PARSER_XLS:
		{
			XLSParser xls(file_name);
			error = impl->parseXLSMetadata(xls, metadata);
			break;
		}
		case PARSER_DOC:
		{
			DOCParser doc(file_name);
			error = impl->parseDOCMetadata(doc, metadata);
			break;
		}
		case PARSER_PPT:
		{
			PPTParser ppt(file_name);
			error = impl->parsePPTMetadata(ppt, metadata);
			break;
		}
		case PARSER_HTML:
		{
			HTMLParser html(file_name);
			error = impl->parseHTMLMetadata(html, metadata);
			break;
		}
		case PARSER_IWORK:
		{
			IWorkParser iwork(file_name);
			error = impl->parseIWorkMetadata(iwork, metadata);
			break;
		}
		case PARSER_XLSB:
		{
			XLSBParser xlsb(file_name);
			error = impl->parseXLSBMetadata(xlsb, metadata);
			break;
		}
		case PARSER_PDF:
		{
			PDFParser pdf(file_name);
			error = impl->parsePDFMetadata(pdf, metadata);
			break;
		}
		case PARSER_TXT:
		{
			//I dont think we can find metadata in txt...
			error = false;
			break;
		}
		case PARSER_EML:
		{
			EMLParser eml(file_name);
			error = impl->parseEMLMetadata(eml, metadata);
			break;
		}
		case PARSER_ODFXML:
		{
			ODFXMLParser odfxml(file_name);
			error = impl->parseODFXMLMetadata(odfxml, metadata);
			break;
		}
		default:
			return false;
	}

	if (error && fallback)
	{
		*impl->m_log_stream << "It is possible that wrong parser was selected. Trying different parsers.\n";
		ParserType parser_type;
		if (!parserTypeByFileContent(file_name, parser_type))
			return false;
		return extractMetadata(parser_type, false, file_name, metadata);
	}

	if (error)
		return false;

	if (!std::string(metadata.author()).empty())
		metadata.setAuthorType(Metadata::EXTRACTED);
	else
	{
		std::string author;
		if (!get_file_owner(file_name, *impl->m_log_stream, author))
			return false;
		metadata.setAuthor(author);
		metadata.setAuthorType(Metadata::ESTIMATED);
	}
	if (!std::string(metadata.lastModifiedBy()).empty())
		metadata.setLastModifiedByType(Metadata::EXTRACTED);
	else
	{
		if (metadata.authorType() == Metadata::EXTRACTED)
			metadata.setLastModifiedBy(metadata.author());
		else
		{
			std::string last_modified_by;
			if (!get_file_owner(file_name, *impl->m_log_stream, last_modified_by))
				return false;
			metadata.setLastModifiedBy(last_modified_by);
		}
		metadata.setLastModifiedByType(Metadata::ESTIMATED);
	}
	if (metadata.creationDate().tm_year != 0)
		metadata.setCreationDateType(Metadata::EXTRACTED);
	else
	{
		#warning TODO: It is possible that st_ctime can be used on win32.
		tm creation_date;
		if (!get_file_modification_time(file_name, *impl->m_log_stream, creation_date))
			return false;
		metadata.setCreationDate(creation_date);
		metadata.setCreationDateType(Metadata::ESTIMATED);
	}
	if (metadata.lastModificationDate().tm_year != 0)
		metadata.setLastModificationDateType(Metadata::EXTRACTED);
	else
	{
		if (metadata.creationDateType() == Metadata::EXTRACTED)
			metadata.setLastModificationDate(metadata.creationDate());
		else
		{
			tm last_modification_date;
			if (!get_file_modification_time(file_name, *impl->m_log_stream, last_modification_date))
				return false;
			metadata.setLastModificationDate(last_modification_date);
		}
		metadata.setLastModificationDateType(Metadata::ESTIMATED);
	}
	if (metadata.wordCount() != -1)
		metadata.setWordCountType(Metadata::EXTRACTED);
	else
	{
		*impl->m_log_stream << "Estimating number of words.\n";
		std::string text;
		if (!processFile(file_name, text))
			return false;
		std::istringstream in(text);
		std::string tmp;
		int word_count = 0;
		while (in >> tmp)
			word_count++;
		metadata.setWordCount(word_count);
		metadata.setWordCountType(Metadata::ESTIMATED);
	}
	if (metadata.pageCount() != -1)
		metadata.setPageCountType(Metadata::EXTRACTED);
	else if (metadata.wordCount() != Metadata::NONE)
	{
		// 250 words per page is a well known industry standard
		metadata.setPageCount(metadata.wordCount() / 250);
		if (metadata.pageCount() == 0)
			metadata.setPageCount(1);
		metadata.setPageCountType(Metadata::ESTIMATED);
	}
	return true;
}

bool PlainTextExtractor::extractMetadata(ParserType parser_type, bool fallback, const char* file_name, Metadata& metadata)
{
	return extractMetadata(parser_type, fallback, std::string(file_name), metadata);
}

bool PlainTextExtractor::extractMetadata(ParserType parser_type, bool fallback, const char* buffer, size_t size, Metadata& metadata)
{
	if (parser_type == PARSER_AUTO)
		return extractMetadata(buffer, size, metadata);

	bool error = true;
	switch (parser_type)
	{
		case PARSER_RTF:
		{
			RTFParser rtf(buffer, size);
			error = impl->parseRTFMetadata(rtf, metadata);
			break;
		}
		case PARSER_ODF_OOXML:
		{
			ODFOOXMLParser odfooxml(buffer, size);
			error = impl->parseODFOOXMLMetadata(odfooxml, metadata);
			break;
		}
		case PARSER_XLS:
		{
			XLSParser xls(buffer, size);
			error = impl->parseXLSMetadata(xls, metadata);
			break;
		}
		case PARSER_DOC:
		{
			DOCParser doc(buffer, size);
			error = impl->parseDOCMetadata(doc, metadata);
			break;
		}
		case PARSER_PPT:
		{
			PPTParser ppt(buffer, size);
			error = impl->parsePPTMetadata(ppt, metadata);
			break;
		}
		case PARSER_HTML:
		{
			HTMLParser html(buffer, size);
			error = impl->parseHTMLMetadata(html, metadata);
			break;
		}
		case PARSER_IWORK:
		{
			IWorkParser iwork(buffer, size);
			error = impl->parseIWorkMetadata(iwork, metadata);
			break;
		}
		case PARSER_XLSB:
		{
			XLSBParser xlsb(buffer, size);
			error = impl->parseXLSBMetadata(xlsb, metadata);
			break;
		}
		case PARSER_PDF:
		{
			PDFParser pdf(buffer, size);
			error = impl->parsePDFMetadata(pdf, metadata);
			break;
		}
		case PARSER_TXT:
		{
			//I dont think we can find metadata in txt...
			error = false;
			break;
		}
		case PARSER_EML:
		{
			EMLParser eml(buffer, size);
			error = impl->parseEMLMetadata(eml, metadata);
			break;
		}
		case PARSER_ODFXML:
		{
			ODFXMLParser odfxml(buffer, size);
			error = impl->parseODFXMLMetadata(odfxml, metadata);
			break;
		}
		default:
			return false;
	}

	if (error && fallback)
	{
		*impl->m_log_stream << "It is possible that wrong parser was selected. Trying different parsers.\n";
		ParserType parser_type;
		if (!parserTypeByFileContent(buffer, size, parser_type))
			return false;
		return extractMetadata(parser_type, false, buffer, size, metadata);
	}

	if (error)
		return false;

	if (!std::string(metadata.author()).empty())
		metadata.setAuthorType(Metadata::EXTRACTED);
	else
		metadata.setAuthorType(Metadata::NONE);
	if (!std::string(metadata.lastModifiedBy()).empty())
		metadata.setLastModifiedByType(Metadata::EXTRACTED);
	else
	{
		if (metadata.authorType() == Metadata::EXTRACTED)
		{
			metadata.setLastModifiedByType(Metadata::ESTIMATED);
			metadata.setLastModifiedBy(metadata.author());
		}
		else
			metadata.setLastModifiedByType(Metadata::NONE);
	}
	if (metadata.creationDate().tm_year != 0)
		metadata.setCreationDateType(Metadata::EXTRACTED);
	else
		metadata.setCreationDateType(Metadata::NONE);
	if (metadata.lastModificationDate().tm_year != 0)
		metadata.setLastModificationDateType(Metadata::EXTRACTED);
	else
	{
		if (metadata.creationDateType() == Metadata::EXTRACTED)
		{
			metadata.setLastModificationDateType(Metadata::ESTIMATED);
			metadata.setLastModificationDate(metadata.creationDate());
		}
		else
			metadata.setLastModificationDateType(Metadata::NONE);
	}
	if (metadata.wordCount() != -1)
		metadata.setWordCountType(Metadata::EXTRACTED);
	else
	{
		*impl->m_log_stream << "Estimating number of words.\n";
		std::string text;
		if (!processFile(buffer, size, text))
			return false;
		std::istringstream in(text);
		std::string tmp;
		int word_count = 0;
		while (in >> tmp)
			word_count++;
		metadata.setWordCount(word_count);
		metadata.setWordCountType(Metadata::ESTIMATED);
	}
	if (metadata.pageCount() != -1)
		metadata.setPageCountType(Metadata::EXTRACTED);
	else if (metadata.wordCount() != Metadata::NONE)
	{
		// 250 words per page is a well known industry standard
		metadata.setPageCount(metadata.wordCount() / 250);
		if (metadata.pageCount() == 0)
			metadata.setPageCount(1);
		metadata.setPageCountType(Metadata::ESTIMATED);
	}
	return true;
}

size_t PlainTextExtractor::getNumberOfLinks() const
{
	return impl->m_links.size();
}

void PlainTextExtractor::getParsedLinks(std::vector<Link>& links) const
{
	links = impl->m_links;
}

void PlainTextExtractor::getParsedLinks(const Link*& links, size_t& number_of_links) const
{
	if (impl->m_links.size() == 0)
	{
		links = NULL;
		number_of_links = 0;
	}
	else
	{
		links = &impl->m_links[0];
		number_of_links = impl->m_links.size();
	}
}

const Link* PlainTextExtractor::getParsedLinks() const
{
	if (impl->m_links.size() == 0)
		return NULL;
	return &impl->m_links[0];
}

size_t PlainTextExtractor::getNumberOfAttachments() const
{
	return impl->m_attachments.size();
}

void PlainTextExtractor::getAttachments(std::vector<Attachment> &eml_attachments) const
{
	eml_attachments = impl->m_attachments;
}

void PlainTextExtractor::getAttachments(const Attachment *&attachments, size_t &number_of_attachments) const
{
	if (impl->m_attachments.size() == 0)
	{
		attachments = NULL;
		number_of_attachments = 0;
	}
	else
	{
		attachments = &impl->m_attachments[0];
		number_of_attachments = impl->m_attachments.size();
	}
}

const Attachment* PlainTextExtractor::getAttachments() const
{
	if (impl->m_attachments.size() == 0)
		return NULL;
	return &impl->m_attachments[0];
}
