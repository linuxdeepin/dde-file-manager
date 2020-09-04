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

#include "xlsb_parser.h"

#include <algorithm>
#include "doctotext_unzip.h"
#include <iostream>
#include <map>
#include "metadata.h"
#include "misc.h"
#include <sstream>
#include <stdint.h>
#include <vector>

struct XLSBParser::Implementation
{
	struct XLSBContent
	{
		class ErrorsCodes : public std::map<uint32_t, std::string>
		{
			public:
				ErrorsCodes()
				{
					insert(std::pair<uint32_t, std::string>(0x00, "#NULL!"));
					insert(std::pair<uint32_t, std::string>(0x07, "#DIV/0!"));
					insert(std::pair<uint32_t, std::string>(0x0F, "#VALUE!"));
					insert(std::pair<uint32_t, std::string>(0x17, "#REF!"));
					insert(std::pair<uint32_t, std::string>(0x1D, "#NAME?"));
					insert(std::pair<uint32_t, std::string>(0x24, "#NUM!"));
					insert(std::pair<uint32_t, std::string>(0x2A, "#N/A"));
					insert(std::pair<uint32_t, std::string>(0x2B, "#GETTING_DATA"));
				}
		};

		ErrorsCodes m_error_codes;
		std::vector<std::string> m_shared_strings;
		uint32_t m_row_start, m_row_end, m_col_start, m_col_end;
		uint32_t m_current_column, m_current_row;

		XLSBContent()
		{
			m_row_start = 0;
			m_row_end = 0;
			m_col_start = 0;
			m_col_end = 0;
			m_current_column = 0;
			m_current_row = 0;
		}
	};

	bool m_error;
	const char* m_buffer;
	size_t m_buffer_size;
	std::string m_file_name;
	bool m_verbose_logging;
	std::ostream* m_log_stream;
	XLSBContent m_xlsb_content;

	class XLSBReader
	{
		public:
			enum RecordType
			{
				BRT_ROW_HDR = 0x0,
				BRT_CELL_BLANK = 0x1,
				BRT_CELL_RK = 0x2,
				BRT_CELL_ERROR = 0x3,
				BRT_CELL_BOOL = 0x4,
				BRT_CELL_REAL = 0x5,
				BRT_CELL_ST = 0x6,
				BRT_CELL_ISST = 0x7,
				BRT_FMLA_STRING = 0x8,
				BRT_FMLA_NUM = 0x9,
				BRT_FMLA_BOOL = 0xA,
				BRT_FMLA_ERROR = 0xB,
				BRT_SS_ITEM = 0x13,
				BRT_WS_DIM = 0x94,
				BRT_BEGIN_SST = 0x9F
			};

			struct Record
			{
				uint32_t m_type;
				uint32_t m_size;

				Record()
				{
					m_type = 0;
					m_size = 0;
				}
			};

		private:
			DocToTextUnzip* m_zipfile;
			std::vector<unsigned char> m_chunk;
			int m_chunk_len;
			int m_pointer;
			unsigned long m_file_size;
			unsigned long m_readed;
			std::ostream* m_log_stream;
			std::string m_file_name;

		public:
			XLSBReader(DocToTextUnzip& zipfile, const std::string& file_name, std::ostream* log_stream)
			{
				m_zipfile = &zipfile;
				m_file_name = file_name;
				m_log_stream = log_stream;
				m_chunk_len = 0;
				m_pointer = 0;
				m_readed = 0;
				m_chunk.reserve(1024);
				m_file_size = 0;
				zipfile.getFileSize(file_name, m_file_size);
			}

			bool done()
			{
				return m_readed == m_file_size;
			}

			bool readNum(uint32_t& value, int bytes)
			{
				value = 0;
				if (m_chunk_len - m_pointer < bytes)
				{
					*m_log_stream << "Unexpected end of buffer\n";
					return false;
				}
				for (int i = 0; i < bytes; ++i)
					value += (m_chunk[m_pointer++] << (i * 8));
				return true;
			}

			bool readUint8(uint32_t& value)
			{
				return readNum(value, 1);
			}

			bool readUint16(uint32_t& value)
			{
				return readNum(value, 2);
			}

			bool readUint32(uint32_t& value)
			{
				return readNum(value, 4);
			}

			bool readXnum(double& value)
			{
				uint8_t* val_ptr = (uint8_t*)&value;
				if (m_chunk_len - m_pointer < 8)
				{
					*m_log_stream << "Unexpected end of buffer\n";
					return false;
				}
				for (int i = 0; i < 8; ++i)
					val_ptr[8 - i] = m_chunk[m_pointer++];
				return true;
			}

			bool readRkNumber(double& value, bool& is_int)
			{
				value = 0;
				uint32_t uvalue;
				if (!readNum(uvalue, 4))
					return false;
				bool fx100 = (uvalue & 0x00000001) > 0;
				bool fint = (uvalue & 0x00000002) > 0;
				if (fint)
				{
					is_int = true;
					int svalue = (int) uvalue;
					svalue /= 4;	//remove 2 last bits
					if (fx100)
						svalue /= 100;
					value = svalue;
				}
				else
				{
					is_int = false;
					uvalue = uvalue & 0xFFFFFFFC;
					uint64_t* val_ptr = (uint64_t*)&value;
					uint32_t* uval_ptr = (uint32_t*)&uvalue;
					*val_ptr = (uint64_t)*uval_ptr << 32;
					if (fx100)
						value /= 100.0;
				}
				return true;
			}

			bool readXlWideString(std::string& str)
			{
				uint32_t str_size;
				if (!readNum(str_size, 4))
					return false;
				if (str_size * 2 > m_chunk_len - m_pointer)
				{
					*m_log_stream << "XLWideString is bigger than size of the record\n";
					return false;
				}
				str.reserve(2 * str_size);
				for (int i = 0; i < str_size; ++i)
				{
					unsigned int uc = *((unsigned short*)&m_chunk[m_pointer]);
					m_pointer += 2;
					if (uc != 0)
					{
						if (utf16_unichar_has_4_bytes(uc))
						{
							if (++i >= str_size)
							{
								*m_log_stream << "Unexpected end of buffer.\n";
								return false;
							}
							uc = (uc << 16) | *((unsigned short*)&m_chunk[m_pointer]);
							m_pointer += 2;
						}
						str += unichar_to_utf8(uc);
					}
				}
				return true;
			}

			bool readRichStr(std::string& str)
			{
				//skip first byte
				if (m_chunk_len == m_pointer)
				{
					*m_log_stream << "Unexpected end of buffer\n";
					return false;
				}
				++m_pointer;
				if (!readXlWideString(str))
					return false;
				return true;
			}

			bool readRecord(Record& record)
			{
				record.m_type = 0;
				record.m_size = 0;
				for (int i = 0; i < 2; ++i)	//read record type
				{
					if (!readChunk(1))
						return false;
					uint32_t byte = m_chunk[m_pointer++];
					record.m_type += ((byte & 0x7F) << (i * 7));
					if (byte < 128)
						break;
				}
				for (int i = 0; i < 4; ++i)	//read record size
				{
					if (m_pointer == m_chunk_len && !readChunk(1))
						return false;
					uint32_t byte = m_chunk[m_pointer++];
					record.m_size += ((byte & 0x7F) << (i * 7));
					if (byte < 128)
						break;
				}
				if (!readChunk(record.m_size))
					return false;
				return true;
			}

			bool skipBytes(uint32_t bytes_to_skip)
			{
				if (bytes_to_skip <= m_chunk_len - m_pointer)
					m_pointer += bytes_to_skip;
				else
					*m_log_stream << "Unexpected end of buffer\n";
				return true;
			}

			bool readChunk(uint32_t len)
			{
				if (len == 0)
					return true;
				m_chunk.reserve(len + 1);
				if (!m_zipfile->readChunk(m_file_name, (char*)&m_chunk[0], len, m_chunk_len))
				{
					*m_log_stream << "Error while reading file: " + m_file_name + "\n";
					return false;
				}
				m_readed += m_chunk_len;
				if (m_chunk_len != len)
				{
					*m_log_stream << "Read past EOF during reading " + m_file_name + "\n";
					return false;
				}
				m_pointer = 0;
				return true;
			}
	};

	bool parseRecordForSharedStrings(XLSBReader& xlsb_reader, XLSBReader::Record& record)
	{
		switch (record.m_type)
		{
			case XLSBReader::BRT_BEGIN_SST:
			{
				if (!xlsb_reader.skipBytes(4))
					return false;
				uint32_t strings_count = 0;
				if (!xlsb_reader.readUint32(strings_count))
					return false;
				m_xlsb_content.m_shared_strings.reserve(strings_count);
				return true;
			}
			case XLSBReader::BRT_SS_ITEM:
			{
				m_xlsb_content.m_shared_strings.push_back(std::string());
				std::string* new_string = &m_xlsb_content.m_shared_strings[m_xlsb_content.m_shared_strings.size() - 1];
				if (!xlsb_reader.readRichStr(*new_string))
					return false;
				return true;
			}
		}
		return true;
	}

	bool parseColumn(XLSBReader& xlsb_reader, std::string& text)
	{
		uint32_t column;
		if (!xlsb_reader.readUint32(column))
			return false;
		if (m_xlsb_content.m_current_column > 0)
			text += "	";
		while (column > m_xlsb_content.m_current_column)
		{
			text += "	";
			++m_xlsb_content.m_current_column;
		}
		if (!xlsb_reader.skipBytes(4))
			return false;
		m_xlsb_content.m_current_column = column + 1;
		return true;
	}

	bool parseRecordForWorksheets(XLSBReader& xlsb_reader, XLSBReader::Record& record, std::string& text)
	{
		switch (record.m_type)
		{
			case XLSBReader::BRT_CELL_BLANK:
			{
				if (!parseColumn(xlsb_reader, text))
					return false;
				return true;
			}
			case XLSBReader::BRT_FMLA_ERROR:
			case XLSBReader::BRT_CELL_ERROR:
			{
				if (!parseColumn(xlsb_reader, text))
					return false;
				uint32_t value;
				if (!xlsb_reader.readUint8(value))
					return false;
				text += m_xlsb_content.m_error_codes[value];
				return true;
			}
			case XLSBReader::BRT_FMLA_BOOL:
			case XLSBReader::BRT_CELL_BOOL:
			{
				if (!parseColumn(xlsb_reader, text))
					return false;
				uint32_t value;
				if (!xlsb_reader.readUint8(value))
					return false;
				if (value)
					text += "1";
				else
					text += "0";
				return true;
			}
			case XLSBReader::BRT_FMLA_NUM:
			case XLSBReader::BRT_CELL_REAL:
			{
				if (!parseColumn(xlsb_reader, text))
					return false;
				double value;
				if (!xlsb_reader.readXnum(value))
					return false;
				std::ostringstream os;
				os << value;
				text += os.str();
				return true;
			}
			case XLSBReader::BRT_FMLA_STRING:
			case XLSBReader::BRT_CELL_ST:
			{
				if (!parseColumn(xlsb_reader, text))
					return false;
				if (!xlsb_reader.readXlWideString(text))
					return false;
				return true;
			}
			case XLSBReader::BRT_WS_DIM:
			{
				bool s = true;
				s = s && xlsb_reader.readUint32(m_xlsb_content.m_row_start);
				s = s && xlsb_reader.readUint32(m_xlsb_content.m_row_end);
				s = s && xlsb_reader.readUint32(m_xlsb_content.m_col_start);
				s = s && xlsb_reader.readUint32(m_xlsb_content.m_col_end);
				return s;
			}
			case XLSBReader::BRT_ROW_HDR:
			{
				uint32_t row;
				if (!xlsb_reader.readUint32(row))
					return false;
				for (int i = m_xlsb_content.m_current_row; i < row; ++i)
					text += "\n";
				m_xlsb_content.m_current_row = row;
				m_xlsb_content.m_current_column = 0;
				return true;
			}
			case XLSBReader::BRT_CELL_RK:
			{
				if (!parseColumn(xlsb_reader, text))
					return false;
				double value;
				bool is_int;
				if (!xlsb_reader.readRkNumber(value, is_int))
					return false;
				std::ostringstream os;
				if (!is_int)
					os << value;
				else
					os << (int)value;
				text += os.str();
				return true;
			}
			case XLSBReader::BRT_CELL_ISST:
			{
				if (!parseColumn(xlsb_reader, text))
					return false;
				uint32_t str_index;
				if (!xlsb_reader.readUint32(str_index))
					return false;
				if (str_index >= m_xlsb_content.m_shared_strings.size())
					*m_log_stream << "Warning: Detected reference to string that does not exist\n";
				else
					text += m_xlsb_content.m_shared_strings[str_index];
				return true;
			}
		};
		return true;
	}

	bool parseSharedStrings(DocToTextUnzip& unzip)
	{
		XLSBReader::Record record;
		std::string file_name = "xl/sharedStrings.bin";
		if (!unzip.exists(file_name))
		{
			//file may not exist, nothing wrong is with that.
			if (m_verbose_logging)
				*m_log_stream << "File: " + file_name + " does not exist\n";
			return true;
		}
		XLSBReader xlsb_reader(unzip, file_name, m_log_stream);
		while (!xlsb_reader.done())
		{
			if (!xlsb_reader.readRecord(record))
			{
				m_error = true;
				return false;
			}
			if (!parseRecordForSharedStrings(xlsb_reader, record))
			{
				m_error = true;
				return false;
			}
		}
		unzip.closeReadingFileForChunks();
		return true;
	}

	bool parseWorksheets(DocToTextUnzip& unzip, std::string& text)
	{
		XLSBReader::Record record;
		int sheet_index = 1;
		std::string sheet_file_name = "xl/worksheets/sheet1.bin";
		while (unzip.exists(sheet_file_name))
		{
			XLSBReader xlsb_reader(unzip, sheet_file_name, m_log_stream);
			while (!xlsb_reader.done())
			{
				if (!xlsb_reader.readRecord(record))
				{
					m_error = true;
					return false;
				}
				if (!parseRecordForWorksheets(xlsb_reader, record, text))
				{
					m_error = true;
					return false;
				}
			}
			unzip.closeReadingFileForChunks();
			++sheet_index;
			std::ostringstream os;
			os << sheet_index;
			sheet_file_name = "xl/worksheets/sheet" + os.str() + ".bin";
			text += "\n\n";
		}
		return true;
	}

	void parseXLSB(DocToTextUnzip& unzip, std::string& text)
	{
		text.reserve(1024 * 1024);
		if (!unzip.loadDirectory())
		{
			*m_log_stream << "Cant load zip directory";
			m_error = true;
			return;
		}
		if (!parseSharedStrings(unzip))
			return;
		if (!parseWorksheets(unzip, text))
			return;
	}

	void readMetadata(DocToTextUnzip& unzip, Metadata& metadata)
	{
		std::string data;
		if (!unzip.read("docProps/app.xml", &data))
		{
			*m_log_stream << "Error reading docProps/app.xml\n";
			m_error = true;
			return;
		}
		if (data.find("<TitlesOfParts>") != std::string::npos && data.find("</TitlesOfParts>") != std::string::npos)
		{
			data.erase(data.find("</TitlesOfParts>"));
			data.erase(0, data.find("<TitlesOfParts>") + 15);
			size_t pages_count_start = data.find("<vt:vector size=\"");
			if (pages_count_start != std::string::npos)
				metadata.setPageCount(strtol(data.c_str() + pages_count_start + 17, NULL, 10));
		}
		data.clear();
		if (!unzip.read("docProps/core.xml", &data))
		{
			*m_log_stream << "Error reading docProps/core.xml\n";
			m_error = true;
			return;
		}
		bool author_exist = data.find("<dc:creator/>") == std::string::npos && data.find("<dc:creator") != std::string::npos;
		if (author_exist)
		{
			size_t author_start = data.find("<dc:creator");
			size_t author_end = data.find("</dc:creator>");
			std::string author = data.substr(author_start + 11, author_end - author_start - 11);
			if (author.find(">") != std::string::npos)
			{
				author.erase(0, author.find(">") + 1);
				if (author.length() > 0)
					metadata.setAuthor(author);
			}
		}
		bool last_modify_by_exist = data.find("<cp:lastModifiedBy/>") == std::string::npos && data.find("<cp:lastModifiedBy") != std::string::npos;
		if (last_modify_by_exist)
		{
			size_t last_modify_by_start = data.find("<cp:lastModifiedBy");
			size_t last_modify_by_end = data.find("</cp:lastModifiedBy>");
			std::string last_modify_by = data.substr(last_modify_by_start + 18, last_modify_by_end - last_modify_by_start - 18);
			if (last_modify_by.find(">") != std::string::npos)
			{
				last_modify_by.erase(0, last_modify_by.find(">") + 1);
				if (last_modify_by.length() > 0)
					metadata.setLastModifiedBy(last_modify_by);
			}
		}
		bool creation_date_exist = data.find("<dcterms:created/>") == std::string::npos && data.find("<dcterms:created") != std::string::npos;
		if (creation_date_exist)
		{
			size_t creation_date_start = data.find("<dcterms:created");
			size_t creation_date_end = data.find("</dcterms:created>");
			std::string creation_date = data.substr(creation_date_start + 16, creation_date_end - creation_date_start - 16);
			if (creation_date.find(">") != std::string::npos)
			{
				creation_date.erase(0, creation_date.find(">") + 1);
				tm creation_date_tm;
				if (creation_date.length() > 0 && string_to_date(creation_date, creation_date_tm))
					metadata.setCreationDate(creation_date_tm);
			}
		}
		bool last_modification_date_exist = data.find("<dcterms:modified/>") == std::string::npos && data.find("<dcterms:modified") != std::string::npos;
		if (last_modification_date_exist)
		{
			size_t last_modification_date_start = data.find("<dcterms:modified");
			size_t last_modification_date_end = data.find("</dcterms:modified>");
			std::string last_modification_date = data.substr(last_modification_date_start + 17, last_modification_date_end - last_modification_date_start - 17);
			if (last_modification_date.find(">") != std::string::npos)
			{
				last_modification_date.erase(0, last_modification_date.find(">") + 1);
				tm last_modification_date_tm;
				if (last_modification_date.length() > 0 && string_to_date(last_modification_date, last_modification_date_tm))
					metadata.setLastModificationDate(last_modification_date_tm);
			}
		}
	}
};

XLSBParser::XLSBParser(const std::string& file_name)
{
	impl = new Implementation();
	impl->m_error = false;
	impl->m_file_name = file_name;
	impl->m_verbose_logging = false;
	impl->m_log_stream = &std::cerr;
	impl->m_buffer = NULL;
	impl->m_buffer_size = 0;
}

XLSBParser::XLSBParser(const char *buffer, size_t size)
{
	impl = new Implementation();
	impl->m_error = false;
	impl->m_file_name = "Memory buffer";
	impl->m_verbose_logging = false;
	impl->m_log_stream = &std::cerr;
	impl->m_buffer = buffer;
	impl->m_buffer_size = size;
}

XLSBParser::~XLSBParser()
{
	delete impl;
}

void XLSBParser::setVerboseLogging(bool verbose)
{
	impl->m_verbose_logging = verbose;
}

void XLSBParser::setLogStream(std::ostream& log_stream)
{
	impl->m_log_stream = &log_stream;
}

bool XLSBParser::isXLSB()
{
	DocToTextUnzip unzip;
	if (impl->m_buffer)
		unzip.setBuffer(impl->m_buffer, impl->m_buffer_size);
	else
		unzip.setArchiveFile(impl->m_file_name);
	impl->m_error = false;
	bool res_open = unzip.open();
	if (res_open == false)
	{
		*impl->m_log_stream << "Cannot unzip file.\n";
		return false;
	}
	if (!unzip.exists("xl/workbook.bin"))
	{
		unzip.close();
		*impl->m_log_stream << "Cannot find xl/woorkbook.bin.\n";
		return false;
	}
	unzip.close();
	return true;
}

void XLSBParser::getLinks(std::vector<Link>& links)
{
	#warning TODO: Implement this functionality.
}

Metadata XLSBParser::metaData()
{
	Metadata metadata;
	impl->m_error = false;
	if (!isXLSB())
	{
		impl->m_error = true;
		*impl->m_log_stream << "This file is not proper xlsb file.\n";
		return metadata;
	}
	DocToTextUnzip unzip;
	if (impl->m_buffer)
		unzip.setBuffer(impl->m_buffer, impl->m_buffer_size);
	else
		unzip.setArchiveFile(impl->m_file_name);
	bool res_open = unzip.open();
	if (res_open == false)
	{
		*impl->m_log_stream << "Cannot unzip file.\n";
		impl->m_error = true;
		return metadata;
	}
	if (!unzip.exists("docProps/app.xml"))
	{
		unzip.close();
		*impl->m_log_stream << "Cannot find docProps/app.xml.\n";
		impl->m_error = true;
		return metadata;
	}
	if (!unzip.exists("docProps/core.xml"))
	{
		unzip.close();
		*impl->m_log_stream << "Cannot find docProps/core.xml.\n";
		impl->m_error = true;
		return metadata;
	}
	impl->readMetadata(unzip, metadata);
	unzip.close();
	return metadata;
}

std::string XLSBParser::plainText(const FormattingStyle& formatting)
{
	std::string text;
	impl->m_error = false;
	if (!isXLSB())
	{
		impl->m_error = true;
		*impl->m_log_stream << "This file is not proper xlsb file.\n";
		return text;
	}
	DocToTextUnzip unzip;
	if (impl->m_buffer)
		unzip.setBuffer(impl->m_buffer, impl->m_buffer_size);
	else
		unzip.setArchiveFile(impl->m_file_name);
	bool res_open = unzip.open();
	if (res_open == false)
	{
		*impl->m_log_stream << "Cannot unzip file.\n";
		impl->m_error = true;
		return text;
	}
	impl->parseXLSB(unzip, text);
	unzip.close();
	return text;
}

bool XLSBParser::error()
{
	return impl->m_error;
}
