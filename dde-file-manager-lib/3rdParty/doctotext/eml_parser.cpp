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

#include "eml_parser.h"

#include "data_stream.h"
#include "attachment.h"
#include "html_parser.h"
#include <fstream>
#include "htmlcxx/html/CharsetConverter.h"
#include <iostream>
#include "metadata.h"
#include <stdint.h>
#include "mimetic/mimetic.h"
#include "mimetic/rfc822/rfc822.h"
#include "txt_parser.h"

using namespace mimetic;

const std::string attachment = "attachment";
const std::string multipart = "multipart/";
const std::string text = "text/";
const std::string text_html = "text/html";
const std::string text_xhtml = "text/xhtml";

static tm rfc_date_time_to_tm(const DateTime& date_time)
{
	tm tm_date_time;
	tm_date_time.tm_year = date_time.year() - 1900;
	tm_date_time.tm_mon = date_time.month().ordinal() - 1;
	tm_date_time.tm_mday = date_time.day();
	tm_date_time.tm_hour = date_time.hour();
	tm_date_time.tm_min = date_time.minute();
	tm_date_time.tm_sec = date_time.second();
	return tm_date_time;
}

struct EMLParser::Implementation
{
	bool m_error;
	std::string m_file_name;
	bool m_verbose_logging;
	std::ostream* m_log_stream;
	std::istream* m_data_stream;
	std::vector<Link> m_links;
	std::vector<doctotext::Attachment> m_attachments;

	std::string getFilename(const MimeEntity& mime_entity)
	{
		if (mime_entity.hasField(ContentDisposition::label))
		{
			const ContentDisposition& cd = mime_entity.header().contentDisposition();
			if (cd.param("filename").length())
				return std::string(cd.param("filename").c_str());
		}
		else if (mime_entity.hasField(ContentType::label))
		{
			const ContentType& ct = mime_entity.header().contentType();
			return std::string("unnamed_" + ct.type() + "." + ct.subtype()).c_str();
		}
		return std::string("Unknown attachment");
	}

	void convertToUtf8(const std::string& charset, std::string& text)
	{
		try
		{
			htmlcxx::CharsetConverter converter(charset, "UTF-8");
			text = converter.convert(text);
		}
		catch (htmlcxx::CharsetConverter::Exception& ex)
		{
			*m_log_stream << "Warning: Cant convert text to UTF-8 from " + charset;
		}
	}

	void decodeBin(const MimeEntity& me, std::string& out)
	{
		std::stringstream os(std::ios::out);
		std::ostreambuf_iterator<char> oi(os);
		const ContentTransferEncoding& cte = me.header().contentTransferEncoding();
		mimetic::istring enc_algo = cte.mechanism();
		if (enc_algo == ContentTransferEncoding::base64)
		{
			if (m_verbose_logging)
				*m_log_stream << "Using base64 decoding\n";
			Base64::Decoder b64;
			decode(me.body().begin(), me.body().end(), b64, oi);
		}
		else if (enc_algo == ContentTransferEncoding::quoted_printable)
		{
			if (m_verbose_logging)
				*m_log_stream << "Using quoted_printable decoding\n";
			QP::Decoder qp;
			decode(me.body().begin(), me.body().end(), qp, oi);
		}
		else if (enc_algo == ContentTransferEncoding::eightbit ||
			enc_algo == ContentTransferEncoding::sevenbit ||
			enc_algo == ContentTransferEncoding::binary)
		{
			if (m_verbose_logging)
				*m_log_stream << "Using eightbit/sevenbit/binary decoding\n";
			copy(me.body().begin(), me.body().end(), oi);
		}
		else
		{
			*m_log_stream << "Unknown encoding: " + enc_algo + "\n";
			copy(me.body().begin(), me.body().end(), oi);
		}
		os.flush();
		out = os.rdbuf()->str();
	}

	void extractPlainText(const MimeEntity& mime_entity, std::string& output, const FormattingStyle& formatting)
	{
		const Header& header = mime_entity.header();
		if (header.contentDisposition().str().find(attachment.c_str()) == std::string::npos &&
				header.contentType().str().substr(0, text.length()) == text)
		{
			std::string plain;
			decodeBin(mime_entity, plain);

			bool skip_charset_decoding = false;
			if (header.contentType().param("charset").length())
			{
				std::string charset(header.contentType().param("charset").c_str());
				convertToUtf8(charset, plain);
				skip_charset_decoding = true;
			}
			if (header.contentType().str().substr(0, text_html.length()) == text_html ||
					header.contentType().str().substr(0, text_xhtml.length()) == text_xhtml)
			{
				HTMLParser html(plain.c_str(), plain.length());
				html.setLogStream(*m_log_stream);
				html.setVerboseLogging(m_verbose_logging);
				if (skip_charset_decoding)
					html.skipCharsetDecoding();
				plain = html.plainText(formatting);
				html.getLinks(m_links);
				if (html.error())
					*m_log_stream << "Warning: Error while parsing html\n";
				//Update positions of the links.
				if (m_links.size() > 0)
				{
					size_t link_offset = output.length();
					for (size_t i = 0; i <m_links.size(); ++i)
						m_links[i].setLinkTextPosition(m_links[i].getLinkTextPosition() + link_offset);
				}
			}
			else
			{
				if (!skip_charset_decoding)
				{
					TXTParser txt(plain.c_str(), plain.length());
					txt.setLogStream(*m_log_stream);
					txt.setVerboseLogging(m_verbose_logging);
					plain = txt.plainText();
					if (txt.error())
						*m_log_stream << "Warning: Error while parsing txt";
				}
			}
			output += plain;
			output += "\n\n";
			return;
		}
		else if (header.contentType().str().substr(0, multipart.length()) != multipart)
		{
			std::string plain;
			decodeBin(mime_entity, plain);

			m_attachments.push_back(doctotext::Attachment(getFilename(mime_entity)));
			m_attachments[m_attachments.size() - 1].setBinaryContent(plain);
			std::string content_type = header.contentType().str();
			std::string content_disposition = header.contentDisposition().str();
			std::string content_description = header.contentDescription().str();
			std::string content_id = header.contentId().str();
			if (!content_type.empty())
				m_attachments[m_attachments.size() - 1].addField("Content-Type", content_type);
			if (!content_disposition.empty())
				m_attachments[m_attachments.size() - 1].addField("Content-Disposition", content_disposition);
			if (!content_description.empty())
				m_attachments[m_attachments.size() - 1].addField("Content-Description", content_description);
			if (!content_id.empty())
				m_attachments[m_attachments.size() - 1].addField("Content-ID", content_id);
		}
		const MimeEntityList& parts = mime_entity.body().parts();
		MimeEntityList::const_iterator mbit = parts.begin(), meit = parts.end();
		for(; mbit != meit; ++mbit)
			extractPlainText(**mbit, output, formatting);
	}
};

EMLParser::EMLParser(const std::string& file_name)
{
	impl = NULL;
	try
	{
		impl = new Implementation;
		impl->m_data_stream = NULL;
		impl->m_data_stream = new std::ifstream(file_name.c_str());
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

EMLParser::EMLParser(const char* buffer, size_t size)
{
	impl = NULL;
	try
	{
		impl = new Implementation;
		impl->m_data_stream = NULL;
		impl->m_data_stream = new std::stringstream(std::string(buffer, size));
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

EMLParser::~EMLParser()
{
	if (impl)
	{
		if (impl->m_data_stream)
			delete impl->m_data_stream;
		delete impl;
	}
}

void EMLParser::setVerboseLogging(bool verbose)
{
	impl->m_verbose_logging = verbose;
}

void EMLParser::setLogStream(std::ostream& log_stream)
{
	impl->m_log_stream = &log_stream;
}

bool EMLParser::isEML()
{
	if (!impl->m_data_stream->good())
	{
		*impl->m_log_stream << "Error opening file " + impl->m_file_name + "\n";
		impl->m_error = true;
		return false;
	}
	MimeEntity mime_entity(*impl->m_data_stream);
	//"From" and "Date" are obligatory according to the RFC standard.
	Header header = mime_entity.header();
	if (!header.hasField("From") || !header.hasField("Date"))
		return false;
	return true;
}

void EMLParser::getLinks(std::vector<Link>& links)
{
	links = impl->m_links;
}

void EMLParser::getAttachments(std::vector<doctotext::Attachment>& attachments)
{
	attachments = impl->m_attachments;
}

std::string EMLParser::plainText(const FormattingStyle& formatting)
{
	std::string text;
	if (!isEML())
	{
		impl->m_error = true;
		return text;
	}
	impl->m_data_stream->seekg(0, std::ios_base::beg);
	MimeEntity mime_entity(*impl->m_data_stream);
	impl->extractPlainText(mime_entity, text, formatting);
	return text;
}

Metadata EMLParser::metaData()
{
	Metadata metadata;
	if (!isEML())
	{
		impl->m_error = true;
		return metadata;
	}
	impl->m_data_stream->seekg(0, std::ios_base::beg);
	MimeEntity mime_entity(*impl->m_data_stream);
	Header header = mime_entity.header();
	metadata.setAuthor(header.from().str());
	DateTime creation_date(header.field("Date").value());
	metadata.setCreationDate(rfc_date_time_to_tm(creation_date));

	//in EML file format author is visible under key "From". And creation date is visible under key "Data".
	//So, should I repeat the same values or skip them?
	metadata.addField("From", header.from().str());
	metadata.addField("Date", rfc_date_time_to_tm(creation_date));

	std::string to = header.to().str();
	if (!to.empty())
		metadata.addField("To", to);

	std::string subject = header.subject();
	if (!subject.empty())
		metadata.addField("Subject", subject);
	std::string mime_version = header.mimeVersion().str();
	if (!mime_version.empty())
		metadata.addField("MIME-Version", mime_version);
	std::string content_type = header.contentType().str();
	if (!content_type.empty())
		metadata.addField("Content-Type", content_type);
	std::string content_disposition = header.contentDisposition().str();
	if (!content_disposition.empty())
		metadata.addField("Content-Disposition", content_disposition);
	std::string content_description = header.contentDescription().str();
	if (!content_description.empty())
		metadata.addField("Content-Description", content_description);
	std::string content_id = header.contentId().str();
	if (!content_id.empty())
		metadata.addField("Content-ID", content_id);
	std::string message_id = header.messageid().str();
	if (!message_id.empty())
		metadata.addField("Message-ID", message_id);
	std::string reply_to = header.replyto().str();
	if (!reply_to.empty())
		metadata.addField("Reply-To", reply_to);
	std::string sender = header.sender().str();
	if (!sender.empty())
		metadata.addField("Sender", sender);
}

bool EMLParser::error()
{
	return impl->m_error;
}
