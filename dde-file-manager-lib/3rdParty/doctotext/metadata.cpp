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

#include "metadata.h"

using namespace doctotext;

struct Metadata::Implementation
{
	DataType author_type;
	std::string author;
	DataType creation_date_type;
	tm creation_date;
	DataType last_modified_by_type;
	std::string last_modified_by;
	DataType last_modification_date_type;
	tm last_modification_date;
	DataType page_count_type;
	int page_count;
	DataType word_count_type;
	int word_count;
	std::map<std::string, Variant> m_fields;
};

Metadata::Metadata()
{
	impl = new Implementation();
	impl->author_type = NONE;
	impl->last_modified_by_type = NONE;
	impl->creation_date_type = NONE;
	impl->creation_date = (tm){0};
	impl->last_modification_date_type = NONE;
	impl->last_modification_date = (tm){0};
	impl->page_count_type = NONE;
	impl->page_count = -1;
	impl->word_count_type = NONE;
	impl->word_count = -1;
};

Metadata::Metadata(const Metadata& r)
{
	impl = new Implementation();
	*impl = *r.impl;
}

Metadata::~Metadata()
{
	delete impl;
}

Metadata& Metadata::operator=(const Metadata& r)
{
	*impl = *r.impl;
	return *this;
}

Metadata::DataType Metadata::authorType()
{
	return impl->author_type;
}

void Metadata::setAuthorType(DataType type)
{
	impl->author_type = type;
}

const char* Metadata::author()
{
	return impl->author.c_str();
}

void Metadata::setAuthor(const std::string& author)
{
	impl->author = author;
	impl->m_fields["author"] = author;
}

Metadata::DataType Metadata::creationDateType()
{
	return impl->creation_date_type;
}

void Metadata::setCreationDateType(DataType type)
{
	impl->creation_date_type = type;
}

const tm& Metadata::creationDate()
{
	return impl->creation_date;
}

void Metadata::setCreationDate(const tm& creation_date)
{
	impl->creation_date = creation_date;
	impl->m_fields["creation date"] = creation_date;
}

Metadata::DataType Metadata::lastModifiedByType()
{
	return impl->last_modified_by_type;
}

void Metadata::setLastModifiedByType(DataType type)
{
	impl->last_modified_by_type = type;
}

const char* Metadata::lastModifiedBy()
{
	return impl->last_modified_by.c_str();
}

void Metadata::setLastModifiedBy(const std::string& last_modified_by)
{
	impl->last_modified_by = last_modified_by;
	impl->m_fields["last modified by"] = last_modified_by;
}

Metadata::DataType Metadata::lastModificationDateType()
{
	return impl->last_modification_date_type;
}

void Metadata::setLastModificationDateType(DataType type)
{
	impl->last_modification_date_type = type;
}

const tm& Metadata::lastModificationDate()
{
	return impl->last_modification_date;
}

void Metadata::setLastModificationDate(const tm& last_modification_date)
{
	impl->last_modification_date = last_modification_date;
	impl->m_fields["last modification date"] = last_modification_date;
}

Metadata::DataType Metadata::pageCountType()
{
	return impl->page_count_type;
}

void Metadata::setPageCountType(DataType type)
{
	impl->page_count_type = type;
}

int Metadata::pageCount()
{
	return impl->page_count;
}

void Metadata::setPageCount(int page_count)
{
	impl->page_count = page_count;
	impl->m_fields["page count"] = (size_t)page_count;
}

Metadata::DataType Metadata::wordCountType()
{
	return impl->word_count_type;
}

void Metadata::setWordCountType(DataType type)
{
	impl->word_count_type = type;
}

int Metadata::wordCount()
{
	return impl->word_count;
}

void Metadata::setWordCount(int word_count)
{
	impl->word_count = word_count;
	impl->m_fields["word count"] = (size_t)word_count;
}

void Metadata::addField(const std::string& field_name, const Variant& field_value)
{
	impl->m_fields[field_name] = field_value;
}

bool Metadata::hasField(const std::string& field_name) const
{
	return impl->m_fields.find(field_name) != impl->m_fields.end();
}

const Variant& Metadata::getField(const std::string& field_name) const
{
	return impl->m_fields[field_name];
}

const std::map<std::string, Variant>& Metadata::getFields() const
{
	return impl->m_fields;
}
