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

#include "doctotext_c_api.h"

#include "exception.h"
#include <iostream>
#include "metadata.h"
#include <stdlib.h>
#include <string.h>
#include "plain_text_extractor.h"

using namespace doctotext;

extern "C"
{

#warning TODO: FILE structure (outside doctotext library) cannot be used, because of the cross-compilers problems \
(for example, MinGW with MSVC). Either file descriptors dont work. For now we only support logging to the external file.\
But in the future, maybe we will manage to force FILE struct to work properly. So StreamBuf is still in use.

class StreamBuf : public std::streambuf
{
	private:
		int flush();
		char m_buffer[4092];
		FILE* m_stream;

	public:
		StreamBuf(FILE* stream);
		~StreamBuf();
		int overflow(int ch);
		int sync();
		void close();
};

StreamBuf::StreamBuf(FILE *stream)
{
	m_stream = stream;
	setp(m_buffer, m_buffer + 4091);
}

StreamBuf::~StreamBuf()
{
	close();
}

void StreamBuf::close()
{
	flush();
}

int StreamBuf::overflow(int ch)
{
	if (ch != EOF)
	{
		*pptr() = ch;
		pbump(1);
	}
	if (flush() == -1)
		return EOF;
	return traits_type::to_int_type(ch);
}

int StreamBuf::sync()
{
	return flush();
}

int StreamBuf::flush()
{
	int to_write = pptr() - pbase();
	if (to_write > 0)
	{
		if (fwrite(pbase(), 1, to_write, m_stream) != to_write)
			return -1;
		pbump(-to_write);
	}
	return 0;
}

struct DocToTextFormattingStyle
{
	FormattingStyle m_formatting_style;
};

DocToTextFormattingStyle* DOCTOTEXT_CALL doctotext_create_formatting_style()
{
	try
	{
		return new DocToTextFormattingStyle;
	}
	catch (std::bad_alloc& ba)
	{
		return NULL;
	}
}

void DOCTOTEXT_CALL doctotext_free_formatting_style(DocToTextFormattingStyle* formatting_style)
{
	delete formatting_style;
}

void DOCTOTEXT_CALL doctotext_formatting_style_set_table_style(DocToTextFormattingStyle* formatting_style, doctotext_table_style table_style)
{
	formatting_style->m_formatting_style.table_style = (TableStyle)table_style;
}

void DOCTOTEXT_CALL doctotext_formatting_style_set_url_style(DocToTextFormattingStyle* formatting_style, doctotext_url_style url_style)
{
	formatting_style->m_formatting_style.url_style = (UrlStyle)url_style;
}

void DOCTOTEXT_CALL doctotext_formatting_style_set_list_prefix(DocToTextFormattingStyle* formatting_style, const char* prefix)
{
	try
	{
		formatting_style->m_formatting_style.list_style.setPrefix(prefix);
	}
	catch (std::bad_alloc& ba)
	{
		//does setPrefix allocate memory?
	}
}

struct DocToTextExtractorParams
{
	bool m_verbose;
	std::string m_log_file;
	XmlParseMode m_xml_parse_mode;
	PlainTextExtractor::ParserType m_parser_type;
	int m_manage_xml_parser;
	FormattingStyle m_formatting_style;

	DocToTextExtractorParams()
	{
		m_verbose = 0;
		m_xml_parse_mode = PARSE_XML;
		m_parser_type = PlainTextExtractor::PARSER_AUTO;
		m_manage_xml_parser = 1;
	}
};

DocToTextExtractorParams* DOCTOTEXT_CALL doctotext_create_extractor_params()
{
	try
	{
		return new DocToTextExtractorParams;
	}
	catch (std::bad_alloc& ba)
	{
		return NULL;
	}
}

void DOCTOTEXT_CALL doctotext_free_extractor_params(DocToTextExtractorParams* extractor_params)
{
	delete extractor_params;
}

void DOCTOTEXT_CALL doctotext_extractor_params_set_verbose_logging(DocToTextExtractorParams* extractor_params, int verbose)
{
	if (verbose != 0)
		extractor_params->m_verbose = true;
	else
		extractor_params->m_verbose = false;
}

void DOCTOTEXT_CALL doctotext_extractor_params_set_log_file(DocToTextExtractorParams* extractor_params, const char* log_file_name)
{
	try
	{
		extractor_params->m_log_file = log_file_name;
	}
	catch (std::bad_alloc& ba)
	{
		//Can action above allocate some memory?
	}
}

void DOCTOTEXT_CALL doctotext_extractor_params_set_parser_type(DocToTextExtractorParams* extractor_params, doctotext_parser_type parser_type)
{
	extractor_params->m_parser_type = (PlainTextExtractor::ParserType)parser_type;
}

void DOCTOTEXT_CALL doctotext_extractor_params_set_xml_parse_mode(DocToTextExtractorParams* extractor_params, doctotext_xml_parse_mode xml_parse_mode)
{
	extractor_params->m_xml_parse_mode = (XmlParseMode)xml_parse_mode;
}

void DOCTOTEXT_CALL doctotext_extractor_params_set_manage_xml_parser(DocToTextExtractorParams* extractor_params, int manage_xml_parser)
{
	extractor_params->m_manage_xml_parser = manage_xml_parser;
}

void DOCTOTEXT_CALL doctotext_extractor_params_set_formatting_style(DocToTextExtractorParams* extractor_params, DocToTextFormattingStyle* formatting_style)
{
	try
	{
		extractor_params->m_formatting_style = formatting_style->m_formatting_style;
	}
	catch (std::bad_alloc& ba)
	{
	}
}

struct DocToTextException
{
	Exception m_exception;
};

void DOCTOTEXT_CALL doctotext_free_exception(DocToTextException* exception)
{
	delete exception;
}

size_t DOCTOTEXT_CALL doctotext_exception_error_messages_count(DocToTextException* exception)
{
	exception->m_exception.getErrorCount();
}

const char* DOCTOTEXT_CALL doctotext_exception_get_error_message(DocToTextException* exception, size_t index_message)
{
	std::list<std::string>::iterator it = exception->m_exception.getErrorIterator();
	while (index_message > 0)
	{
		++it;
		--index_message;
	}
	return (*it).c_str();
}

struct DocToTextLink
{
	Link* m_link;
};

const char* DOCTOTEXT_CALL doctotext_link_get_url(DocToTextLink* link)
{
	return link->m_link->getLinkUrl();
}

const char* DOCTOTEXT_CALL doctotext_link_get_link_text(DocToTextLink* link)
{
	return link->m_link->getLinkText();
}

size_t DOCTOTEXT_CALL doctotext_link_get_link_position(DocToTextLink* link)
{
	return link->m_link->getLinkTextPosition();
}

struct DocToTextVariant
{
	const Variant* m_variant;
};

int DOCTOTEXT_CALL doctotext_variant_is_null(DocToTextVariant* variant)
{
	return variant->m_variant->isEmpty() ? 1 : 0;
}

int DOCTOTEXT_CALL doctotext_variant_is_string(DocToTextVariant* variant)
{
	return variant->m_variant->isString() ? 1 : 0;
}

int DOCTOTEXT_CALL doctotext_variant_is_number(DocToTextVariant* variant)
{
	return variant->m_variant->isNumber() ? 1 : 0;
}

int DOCTOTEXT_CALL doctotext_variant_is_date_time(DocToTextVariant* variant)
{
	return variant->m_variant->isDateTime() ? 1 : 0;
}

const char* DOCTOTEXT_CALL doctotext_variant_get_string(DocToTextVariant* variant)
{
	return variant->m_variant->getString();
}

size_t DOCTOTEXT_CALL doctotext_variant_get_number(DocToTextVariant* variant)
{
	return variant->m_variant->getNumber();
}

const struct tm* DOCTOTEXT_CALL doctotext_variant_get_date_time(DocToTextVariant* variant)
{
	return &variant->m_variant->getDateTime();
}

struct DocToTextAttachment
{
	Attachment* m_attachment;
	char** m_metadata_field_keys;
	std::map<std::string, DocToTextVariant> m_metadata_cached_fields;
	size_t m_metadata_fields_count;

	DocToTextAttachment()
	{
		m_metadata_field_keys = NULL;
		m_metadata_fields_count = 0;
	}

	~DocToTextAttachment()
	{
		if (m_metadata_field_keys)
		{
			for (size_t i = 0; i < m_metadata_fields_count; ++i)
			{
				if (m_metadata_field_keys[i])
					delete[] m_metadata_field_keys[i];
			}
			delete[] m_metadata_field_keys;
		}
	}
};

const char* DOCTOTEXT_CALL doctotext_attachment_get_file_name(DocToTextAttachment* attachment)
{
	return attachment->m_attachment->filename();
}

const char* DOCTOTEXT_CALL doctotext_attachment_get_binary_content(DocToTextAttachment* attachment)
{
	return attachment->m_attachment->binaryContent();
}

size_t DOCTOTEXT_CALL doctotext_attachment_get_binary_content_size(DocToTextAttachment* attachment)
{
	return attachment->m_attachment->binaryContentSize();
}

int DOCTOTEXT_CALL doctotext_attachment_has_field(DocToTextAttachment* attachment, const char* key)
{
	return attachment->m_attachment->hasField(key) ? 1 : 0;
}

DocToTextVariant* DOCTOTEXT_CALL doctotext_attachment_get_field(DocToTextAttachment* attachment, const char* key)
{
	if (attachment->m_metadata_cached_fields.find(key) == attachment->m_metadata_cached_fields.end())
		attachment->m_metadata_cached_fields[key].m_variant = &attachment->m_attachment->getField(key);
	return &attachment->m_metadata_cached_fields[key];
}

size_t DOCTOTEXT_CALL doctotext_attachment_fields_count(DocToTextAttachment* attachment)
{
	return attachment->m_attachment->getFields().size();
}

char** DOCTOTEXT_CALL doctotext_attachment_get_keys(DocToTextAttachment* attachment)
{
	if (attachment->m_metadata_field_keys)
		return attachment->m_metadata_field_keys;
	attachment->m_metadata_fields_count = doctotext_attachment_fields_count(attachment);
	if (attachment->m_metadata_fields_count == 0)
		return NULL;
	try
	{
		const std::map<std::string, Variant>& fields = attachment->m_attachment->getFields();
		std::map<std::string, Variant>::const_iterator it = fields.begin();
		attachment->m_metadata_field_keys = new char*[attachment->m_metadata_fields_count];
		memset(attachment->m_metadata_field_keys, 0, sizeof(char*) * attachment->m_metadata_fields_count);
		size_t i = 0;
		while (it != fields.end())
		{
			attachment->m_metadata_field_keys[i] = strdup(it->first.c_str());
			if (!attachment->m_metadata_field_keys[i])
				return NULL;
			++it;
			++i;
		}
		return attachment->m_metadata_field_keys;
	}
	catch (std::bad_alloc& ba)
	{
		return NULL;
	}
}

struct DocToTextExtractedData
{
	std::string m_text;
	std::vector<Link> m_links;
	std::vector<Attachment> m_attachments;
	DocToTextAttachment** m_attachment_handlers;
	DocToTextLink** m_link_handlers;

	DocToTextExtractedData()
	{
		m_attachment_handlers = NULL;
		m_link_handlers = NULL;
	}

	~DocToTextExtractedData()
	{
		if (m_attachment_handlers)
		{
			for (size_t i = 0; i < m_attachments.size(); ++i)
			{
				if (m_attachment_handlers[i])
					delete m_attachment_handlers[i];
			}
			delete[] m_attachment_handlers;
		}
		if (m_link_handlers)
		{
			for (size_t i = 0; i < m_links.size(); ++i)
			{
				if (m_link_handlers[i])
					delete m_link_handlers[i];
			}
			delete[] m_link_handlers;
		}
	}
};

void DOCTOTEXT_CALL doctotext_free_extracted_data(DocToTextExtractedData* extracted_data)
{
	delete extracted_data;
}

const char* DOCTOTEXT_CALL doctotext_extracted_data_get_text(DocToTextExtractedData* extracted_data)
{
	return extracted_data->m_text.c_str();
}

DocToTextLink** DOCTOTEXT_CALL doctotext_extracted_data_get_links(DocToTextExtractedData* extracted_data)
{
	try
	{
		if (extracted_data->m_links.size() == 0)
			return NULL;
		extracted_data->m_link_handlers = new DocToTextLink*[extracted_data->m_links.size()];
		memset(extracted_data->m_link_handlers, 0, sizeof(DocToTextLink*) * extracted_data->m_links.size());
		for (size_t i = 0; i < extracted_data->m_links.size(); ++i)
		{
			extracted_data->m_link_handlers[i] = new DocToTextLink;
			extracted_data->m_link_handlers[i]->m_link = &extracted_data->m_links[i];
		}
		return extracted_data->m_link_handlers;
	}
	catch (std::bad_alloc& ba)
	{
		return NULL;
	}
}

size_t DOCTOTEXT_CALL doctotext_extracted_data_get_links_count(DocToTextExtractedData* extracted_data)
{
	return extracted_data->m_links.size();
}

DocToTextAttachment** DOCTOTEXT_CALL doctotext_extracted_data_get_attachments(DocToTextExtractedData* extracted_data)
{
	try
	{
		if (extracted_data->m_attachments.size() == 0)
			return NULL;
		extracted_data->m_attachment_handlers = new DocToTextAttachment*[extracted_data->m_attachments.size()];
		memset(extracted_data->m_attachment_handlers, 0, sizeof(DocToTextAttachment*) * extracted_data->m_attachments.size());
		for (size_t i = 0; i < extracted_data->m_attachments.size(); ++i)
		{
			extracted_data->m_attachment_handlers[i] = new DocToTextAttachment;
			extracted_data->m_attachment_handlers[i]->m_attachment = &extracted_data->m_attachments[i];
		}
		return extracted_data->m_attachment_handlers;
	}
	catch (std::bad_alloc& ba)
	{
		return NULL;
	}
}

size_t DOCTOTEXT_CALL doctotext_extracted_data_get_attachments_count(DocToTextExtractedData* extracted_data)
{
	return extracted_data->m_attachments.size();
}

struct DocToTextMetadata
{
	Metadata m_metadata;
	char** m_metadata_field_keys;
	std::map<std::string, DocToTextVariant> m_metadata_cached_fields;
	size_t m_metadata_fields_count;

	DocToTextMetadata()
	{
		m_metadata_field_keys = NULL;
		m_metadata_fields_count = 0;
	}

	~DocToTextMetadata()
	{
		if (m_metadata_field_keys)
		{
			for (size_t i = 0; i < m_metadata_fields_count; ++i)
			{
				if (m_metadata_field_keys[i])
					delete m_metadata_field_keys[i];
			}
			delete[] m_metadata_field_keys;
		}
	}
};

void DOCTOTEXT_CALL doctotext_free_metadata(DocToTextMetadata* metadata)
{
	delete metadata;
}

const char* DOCTOTEXT_CALL doctotext_metadata_author(DocToTextMetadata* metadata)
{
	return metadata->m_metadata.author();
}

doctotext_metadata_type DOCTOTEXT_CALL doctotext_metadata_author_type(DocToTextMetadata* metadata)
{
	return (Metadata::DataType)metadata->m_metadata.authorType();
}

const char* DOCTOTEXT_CALL doctotext_metadata_last_modify_by(DocToTextMetadata* metadata)
{
	return metadata->m_metadata.lastModifiedBy();
}

doctotext_metadata_type DOCTOTEXT_CALL doctotext_metadata_last_modify_by_type(DocToTextMetadata* metadata)
{
	return (Metadata::DataType)metadata->m_metadata.lastModifiedByType();
}

const struct tm* DOCTOTEXT_CALL doctotext_metadata_creation_date(DocToTextMetadata* metadata)
{
	return &metadata->m_metadata.creationDate();
}

doctotext_metadata_type DOCTOTEXT_CALL doctotext_metadata_creation_date_type(DocToTextMetadata* metadata)
{
	return (Metadata::DataType)metadata->m_metadata.creationDateType();
}

const struct tm* DOCTOTEXT_CALL doctotext_metadata_last_modification_date(DocToTextMetadata* metadata)
{
	return &metadata->m_metadata.lastModificationDate();
}

doctotext_metadata_type DOCTOTEXT_CALL doctotext_metadata_last_modification_date_type(DocToTextMetadata* metadata)
{
	return (Metadata::DataType)metadata->m_metadata.lastModificationDateType();
}

size_t DOCTOTEXT_CALL doctotext_metadata_pages_count(DocToTextMetadata* metadata)
{
	return metadata->m_metadata.pageCount();
}

doctotext_metadata_type DOCTOTEXT_CALL doctotext_metadata_pages_count_type(DocToTextMetadata* metadata)
{
	return (Metadata::DataType)metadata->m_metadata.pageCountType();
}

size_t DOCTOTEXT_CALL doctotext_metadata_words_count(DocToTextMetadata* metadata)
{
	return metadata->m_metadata.wordCount();
}

doctotext_metadata_type DOCTOTEXT_CALL doctotext_metadata_words_count_type(DocToTextMetadata* metadata)
{
	return (Metadata::DataType)metadata->m_metadata.wordCountType();
}

int DOCTOTEXT_CALL doctotext_metadata_has_field(DocToTextMetadata* metadata, const char* key)
{
	return metadata->m_metadata.hasField(key) ? 1 : 0;
}

DocToTextVariant* DOCTOTEXT_CALL doctotext_metadata_get_field(DocToTextMetadata* metadata, const char* key)
{
	if (metadata->m_metadata_cached_fields.find(key) == metadata->m_metadata_cached_fields.end())
		metadata->m_metadata_cached_fields[key].m_variant = &metadata->m_metadata.getField(key);
	return &metadata->m_metadata_cached_fields[key];
}

size_t DOCTOTEXT_CALL doctotext_metadata_fields_count(DocToTextMetadata* metadata)
{
	return metadata->m_metadata.getFields().size();
}

char** DOCTOTEXT_CALL doctotext_metadata_get_keys(DocToTextMetadata* metadata)
{
	if (metadata->m_metadata_field_keys)
		return metadata->m_metadata_field_keys;
	metadata->m_metadata_fields_count = doctotext_metadata_fields_count(metadata);
	if (metadata->m_metadata_fields_count == 0)
		return NULL;
	try
	{
		const std::map<std::string, Variant>& fields = metadata->m_metadata.getFields();
		std::map<std::string, Variant>::const_iterator it = fields.begin();
		metadata->m_metadata_field_keys = new char*[metadata->m_metadata_fields_count];
		memset(metadata->m_metadata_field_keys, 0, sizeof(char*) * metadata->m_metadata_fields_count);
		size_t i = 0;
		while (it != fields.end())
		{
			metadata->m_metadata_field_keys[i] = strdup(it->first.c_str());
			if (!metadata->m_metadata_field_keys[i])
				return NULL;
			++it;
			++i;
		}
		return metadata->m_metadata_field_keys;
	}
	catch (std::bad_alloc& ba)
	{
		return NULL;
	}
}

static void init_log_stream(DocToTextExtractorParams* extractor_params, FILE*& c_log_stream, bool& use_file_stream)
{
	static const char on_open_error[] = "Warning! Could not create log file. Using stderr instead\n";
	if (extractor_params->m_log_file.length() > 0)
	{
		c_log_stream = fopen(extractor_params->m_log_file.c_str(), "w+");
		if (!c_log_stream)
		{
			c_log_stream = stderr;
			fwrite(on_open_error, 1, strlen(on_open_error), stderr);
		}
		else
		{
			use_file_stream = true;
		}
	}
}

static void init_extractor(PlainTextExtractor& extractor, DocToTextExtractorParams* extractor_params)
{
	extractor.setVerboseLogging(extractor_params->m_verbose);
	extractor.setFormattingStyle(extractor_params->m_formatting_style);
	extractor.setManageXmlParser(extractor_params->m_manage_xml_parser);
	extractor.setXmlParseMode(extractor_params->m_xml_parse_mode);
}

static void handle_bad_alloc(DocToTextException** exception)
{
	try
	{
		*exception = new DocToTextException;
		(*exception)->m_exception.appendError("Cannot parse file: Bad alloc");
	}
	catch (std::bad_alloc& ba)
	{
		if (*exception)
			delete *exception;
		*exception = NULL;
	}
}

static void handle_doctotext_exception(Exception& ex, DocToTextException** exception)
{
	try
	{
		*exception = new DocToTextException;
		(*exception)->m_exception = ex;
	}
	catch (std::bad_alloc& ba)
	{
		if (*exception)
			delete *exception;
		*exception = NULL;
	}
}

DocToTextExtractedData* DOCTOTEXT_CALL doctotext_process_file(const char* file_name, DocToTextExtractorParams* extractor_params, DocToTextException** exception)
{
	DocToTextExtractedData* extracted_data = NULL;
	if (exception)
		*exception = NULL;
	FILE* c_log_stream = stderr;
	bool use_file_stream = false;
	try
	{
		extracted_data = new DocToTextExtractedData;
		PlainTextExtractor extractor(extractor_params->m_parser_type);
		init_extractor(extractor, extractor_params);
		init_log_stream(extractor_params, c_log_stream, use_file_stream);
		StreamBuf stream_buf(c_log_stream);
		std::ostream log_stream(&stream_buf);
		extractor.setLogStream(log_stream);

		bool success = extractor.processFile(file_name, extracted_data->m_text);
		if (use_file_stream)
		{
			stream_buf.close();
			fclose(c_log_stream);
			c_log_stream = NULL;
		}
		if (!success)
		{
			delete extracted_data;
			return NULL;
		}
		extractor.getParsedLinks(extracted_data->m_links);
		extractor.getAttachments(extracted_data->m_attachments);
		return extracted_data;
	}
	catch (std::bad_alloc& ba)
	{
		if (extracted_data)
			delete extracted_data;
		if (exception)
			handle_bad_alloc(exception);
	}
	catch (Exception& ex)
	{
		if (extracted_data)
			delete extracted_data;
		if (exception)
			handle_doctotext_exception(ex, exception);
	}
	return NULL;
}

DocToTextExtractedData* DOCTOTEXT_CALL doctotext_process_file_from_buffer(const char* buffer, size_t size, DocToTextExtractorParams* extractor_params, DocToTextException** exception)
{
	DocToTextExtractedData* extracted_data = NULL;
	if (exception)
		*exception = NULL;
	FILE* c_log_stream = stderr;
	bool use_file_stream = false;
	try
	{
		extracted_data = new DocToTextExtractedData;
		PlainTextExtractor extractor(extractor_params->m_parser_type);
		init_extractor(extractor, extractor_params);
		init_log_stream(extractor_params, c_log_stream, use_file_stream);
		StreamBuf stream_buf(c_log_stream);
		std::ostream log_stream(&stream_buf);
		extractor.setLogStream(log_stream);

		bool success = extractor.processFile(buffer, size, extracted_data->m_text);
		if (use_file_stream)
		{
			stream_buf.close();
			fclose(c_log_stream);
			c_log_stream = NULL;
		}
		if (!success)
		{
			delete extracted_data;
			return NULL;
		}
		extractor.getParsedLinks(extracted_data->m_links);
		extractor.getAttachments(extracted_data->m_attachments);
		return extracted_data;
	}
	catch (std::bad_alloc& ba)
	{
		if (extracted_data)
			delete extracted_data;
		if (exception)
			handle_bad_alloc(exception);
	}
	catch (Exception& ex)
	{
		if (extracted_data)
			delete extracted_data;
		if (exception)
			handle_doctotext_exception(ex, exception);
	}
	return NULL;
}

DocToTextMetadata* DOCTOTEXT_CALL doctotext_extract_metadata(const char* file_name, DocToTextExtractorParams* extractor_params, DocToTextException** exception)
{
	DocToTextMetadata* extracted_data = NULL;
	if (exception)
		*exception = NULL;
	FILE* c_log_stream = stderr;
	bool use_file_stream = false;
	try
	{
		extracted_data = new DocToTextMetadata;
		PlainTextExtractor extractor(extractor_params->m_parser_type);
		init_extractor(extractor, extractor_params);
		init_log_stream(extractor_params, c_log_stream, use_file_stream);
		StreamBuf stream_buf(c_log_stream);
		std::ostream log_stream(&stream_buf);
		extractor.setLogStream(log_stream);

		bool success = extractor.extractMetadata(file_name, extracted_data->m_metadata);
		if (use_file_stream)
		{
			stream_buf.close();
			fclose(c_log_stream);
			c_log_stream = NULL;
		}
		if (!success)
		{
			delete extracted_data;
			return NULL;
		}
		return extracted_data;
	}
	catch (std::bad_alloc& ba)
	{
		if (extracted_data)
			delete extracted_data;
		if (exception)
			handle_bad_alloc(exception);
	}
	catch (Exception& ex)
	{
		if (extracted_data)
			delete extracted_data;
		if (exception)
			handle_doctotext_exception(ex, exception);
	}
	return NULL;
}

DocToTextMetadata* DOCTOTEXT_CALL doctotext_extract_metadata_from_buffer(const char* buffer, size_t size, DocToTextExtractorParams* extractor_params, DocToTextException** exception)
{
	DocToTextMetadata* extracted_data = NULL;
	if (exception)
		*exception = NULL;
	FILE* c_log_stream = stderr;
	bool use_file_stream = false;
	try
	{
		extracted_data = new DocToTextMetadata;
		PlainTextExtractor extractor(extractor_params->m_parser_type);
		init_extractor(extractor, extractor_params);
		init_log_stream(extractor_params, c_log_stream, use_file_stream);
		StreamBuf stream_buf(c_log_stream);
		std::ostream log_stream(&stream_buf);
		extractor.setLogStream(log_stream);

		bool success = extractor.extractMetadata(buffer, size, extracted_data->m_metadata);
		if (use_file_stream)
		{
			stream_buf.close();
			fclose(c_log_stream);
			c_log_stream = NULL;
		}
		if (!success)
		{
			delete extracted_data;
			return NULL;
		}
		return extracted_data;
	}
	catch (std::bad_alloc& ba)
	{
		if (extracted_data)
			delete extracted_data;
		if (exception)
			handle_bad_alloc(exception);
	}
	catch (Exception& ex)
	{
		if (extracted_data)
			delete extracted_data;
		if (exception)
			handle_doctotext_exception(ex, exception);
	}
	return NULL;
}

int DOCTOTEXT_CALL doctotext_parser_type_by_file_extension(const char* file_name, doctotext_parser_type* parser_type)
{
	try
	{
		PlainTextExtractor extractor;
		*parser_type = (doctotext_parser_type)extractor.parserTypeByFileExtension(file_name);
		return 1;
	}
	catch (std::bad_alloc& ba)
	{
		return 0;
	}
	catch (Exception& ex)
	{
		return 0;
	}
}

int DOCTOTEXT_CALL doctotext_parser_type_by_file_content(const char *file_name, doctotext_parser_type *parser_type)
{
	try
	{
		PlainTextExtractor extractor;
		*parser_type = (doctotext_parser_type)PlainTextExtractor::PARSER_AUTO;
		PlainTextExtractor::ParserType type;
		bool success = extractor.parserTypeByFileContent(file_name, type);
		*parser_type = (doctotext_parser_type)type;
		return success ? 1 : 0;
	}
	catch (std::bad_alloc& ba)
	{
		return 0;
	}
	catch (Exception& ex)
	{
		return 0;
	}
}

int DOCTOTEXT_CALL doctotext_parser_type_by_file_content_from_buffer(const char* buffer, size_t size, doctotext_parser_type* parser_type)
{
	try
	{
		PlainTextExtractor extractor;
		*parser_type = (doctotext_parser_type)PlainTextExtractor::PARSER_AUTO;
		PlainTextExtractor::ParserType type;
		bool success = extractor.parserTypeByFileContent(buffer, size, type);
		*parser_type = (doctotext_parser_type)type;
		return success ? 1 : 0;
	}
	catch (std::bad_alloc& ba)
	{
		return 0;
	}
	catch (Exception& ex)
	{
		return 0;
	}
}

}
