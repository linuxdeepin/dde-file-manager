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

#include "oshared.h"

#include <iostream>
#include "metadata.h"
#include "utilitiesrename.h"
#include <time.h>
#include "thread_safe_ole_stream_reader.h"
#include "thread_safe_ole_storage.h"

using namespace wvWare;

static bool read_vt_string(ThreadSafeOLEStreamReader *reader, std::ostream &log_stream, std::string &s)
{
    U16 string_type;
    if (!reader->readU16(string_type) || string_type != 0x1E) {
        log_stream << "Incorrect string type.\n";
        return false;
    }
    reader->seek(2, SEEK_CUR); //padding
    U32 len;
    reader->readU32(len);
    s = "";
    for (int j = 0; j < len - 1; j++) {
        S8 ch;
        reader->readS8(ch);
        if (ch == '\0')
            break;
        s += ch;
    }
    if (!reader->isValid()) {
        log_stream << reader->getLastError() << "\n";
        return false;
    }
    return true;
}

static bool read_vt_i4(ThreadSafeOLEStreamReader *reader, std::ostream &log_stream, S32 &i)
{
    U16 string_type;
    if (!reader->readU16(string_type) || string_type != 0x0003) {
        log_stream << "Incorrect value type.\n";
        return false;
    }
    reader->seek(2, SEEK_CUR); //padding
    reader->readS32(i);
    if (!reader->isValid()) {
        log_stream << reader->getLastError() << "\n";
        return false;
    }
    return true;
}

static bool read_vt_i2(ThreadSafeOLEStreamReader *reader, std::ostream &log_stream, S16 &i)
{
    U16 string_type;
    if (!reader->readU16(string_type) || string_type != 0x0002) {
        log_stream << "Incorrect value type.\n";
        return false;
    }
    reader->seek(2, SEEK_CUR); //padding
    reader->readS16(i);
    if (!reader->isValid()) {
        log_stream << reader->getLastError() << "\n";
        return false;
    }
    return true;
}

static bool read_vt_filetime(ThreadSafeOLEStreamReader *reader, std::ostream &log_stream, tm &time)
{
    U16 type;
    if (!reader->readU16(type) || type != 0x0040) {
        log_stream << "Incorrect variable type.\n";
        return false;
    }
    reader->seek(2, SEEK_CUR); //padding
    U32 file_time_low, file_time_high;
    reader->readU32(file_time_low);
    reader->readU32(file_time_high);
    if (!reader->isValid()) {
        log_stream << reader->getLastError() << "\n";
        return false;
    }
    if (file_time_low == 0 && file_time_high == 0) {
        // Sometimes field exists but date is zero.
        // Last modification time saved by LibreOffice 3.5 when document is created is an example.
        time = (tm) {
            0
        };
        return true;
    }
    unsigned long long int file_time = ((unsigned long long int)file_time_high << 32) | (unsigned long long int)file_time_low;
    time_t t = (time_t)(file_time / 10000000 - 11644473600LL);
    tm *res = gmtime(&t);
    if (res == NULL) {
        log_stream << "Incorrect time value.\n";
        return false;
    }
    time = *res;
    return true;
}

bool parse_oshared_summary_info(ThreadSafeOLEStorage &storage, std::ostream &log_stream, Metadata &meta)
{
    log_stream << "Extracting metadata.\n";
    if (!storage.isValid()) {
        log_stream << "Error opening " << storage.name() << " as OLE file.\n";
        return false;
    }
    ThreadSafeOLEStreamReader *reader = (ThreadSafeOLEStreamReader *)storage.createStreamReader("\005SummaryInformation");
    if (reader == NULL) {
        log_stream << "Error opening SummaryInformation stream.\n";
        return false;
    }
    size_t field_set_stream_start = reader->tell();
    U16 byte_order;
    if (!reader->readU16(byte_order) || byte_order != 0xFFFE) {
        delete reader;
        log_stream << "Incorrect ByteOrder value.\n";
        return false;
    }
    U16 version;
    if (!reader->readU16(version) || version != 0x00) {
        delete reader;
        log_stream << "Incorrect Version value.\n";
        return false;
    }
    reader->seek(4, SEEK_CUR); //system indentifier
    for (int i = 0; i < 4; i++) {
        U32 clsid_part;
        if (!reader->readU32(clsid_part) || clsid_part != 0x00) {
            delete reader;
            log_stream << "Incorrect CLSID value.\n";
            return false;
        }
    }
    U32 num_property_sets;
    if (!reader->readU32(num_property_sets) || (num_property_sets != 0x01 && num_property_sets != 0x02)) {
        delete reader;
        log_stream << "Incorrect number of property sets.\n";
        return false;
    }
    reader->seek(16, SEEK_CUR); // fmtid0_part
    U32 offset;
    reader->readU32(offset);
    int property_set_pos = field_set_stream_start + offset;
    reader->seek(property_set_pos, SEEK_SET);
    U32 size, num_props = 0;
    reader->readU32(size);
    reader->readU32(num_props);
    for (int i = 0; i < num_props; i++) {
        U32 prop_id, offset;
        reader->readU32(prop_id);
        reader->readU32(offset);
        int p = reader->tell();
        switch (prop_id) {
        case 0x00000004: {
            reader->seek(property_set_pos + offset, SEEK_SET);
            std::string author;
            if (read_vt_string(reader, log_stream, author))
                meta.setAuthor(author);
            break;
        }
        case 0x00000008: {
            reader->seek(property_set_pos + offset, SEEK_SET);
            std::string last_modified_by;
            if (read_vt_string(reader, log_stream, last_modified_by))
                meta.setLastModifiedBy(last_modified_by);
            break;
        }
        case 0x0000000C: {
            reader->seek(property_set_pos + offset, SEEK_SET);
            tm creation_date;
            if (read_vt_filetime(reader, log_stream, creation_date))
                meta.setCreationDate(creation_date);
            break;
        }
        case 0x0000000D: {
            reader->seek(property_set_pos + offset, SEEK_SET);
            tm last_modification_date;
            if (read_vt_filetime(reader, log_stream, last_modification_date))
                meta.setLastModificationDate(last_modification_date);
            break;
        }
        case 0x0000000E: {
            reader->seek(property_set_pos + offset, SEEK_SET);
            S32 page_count;
            if (read_vt_i4(reader, log_stream, page_count))
                meta.setPageCount(page_count);
            break;
        }
        case 0x0000000F: {
            reader->seek(property_set_pos + offset, SEEK_SET);
            S32 word_count;
            if (read_vt_i4(reader, log_stream, word_count))
                meta.setWordCount(word_count);
            break;
        }
        }
        reader->seek(p, SEEK_SET);
        if (!reader->isValid()) {
            log_stream << reader->getLastError() << "\n";
            delete reader;
            return false;
        }
    }
    if (!reader->isValid()) {
        log_stream << reader->getLastError() << "\n";
        delete reader;
        return false;
    }
    delete reader;
    return true;
}

static ThreadSafeOLEStreamReader *open_oshared_document_summary_info(ThreadSafeOLEStorage &storage, std::ostream &log_stream, size_t &field_set_stream_start)
{
    ThreadSafeOLEStreamReader *reader = (ThreadSafeOLEStreamReader *)storage.createStreamReader("\005DocumentSummaryInformation");
    if (reader == NULL) {
        log_stream << "Error opening DocumentSummaryInformation stream.\n";
        return NULL;
    }
    field_set_stream_start = reader->tell();
    U16 byte_order;
    if (!reader->readU16(byte_order) || byte_order != 0xFFFE) {
        log_stream << "Incorrect ByteOrder value.\n";
        delete reader;
        return NULL;
    }
    U16 version;
    if (!reader->readU16(version) || version != 0x00) {
        log_stream << "Incorrect Version value.\n";
        delete reader;
        return NULL;
    }
    reader->seek(4, SEEK_CUR);  // system indentifier
    for (int i = 0; i < 4; i++) {
        U32 clsid_part;;
        if (!reader->readU32(clsid_part) || clsid_part != 0x00) {
            log_stream << "Incorrect CLSID value.\n";
            delete reader;
            return NULL;
        }
    }
    U32 num_property_sets;
    if (!reader->readU32(num_property_sets) || (num_property_sets != 0x01 && num_property_sets != 0x02)) {
        log_stream << "Incorrect number of property sets.\n";
        delete reader;
        return NULL;
    }
    reader->seek(16, SEEK_CUR); //fmtid0_part
    if (!reader->isValid()) {
        log_stream << reader->getLastError() << "\n";
        delete reader;
        return NULL;
    }
    return reader;
}

bool get_codepage_from_document_summary_info(ThreadSafeOLEStorage &storage, std::ostream &log_stream, std::string &codepage)
{
    size_t field_set_stream_start;
    if (!storage.isValid()) {
        log_stream << "Error opening " << storage.name() << " as OLE file.\n";
        return false;
    }
    ThreadSafeOLEStreamReader *reader = open_oshared_document_summary_info(storage, log_stream, field_set_stream_start);
    if (reader == NULL)
        return false;
    U32 offset;
    reader->readU32(offset);
    int property_set_pos = field_set_stream_start + offset;
    reader->seek(property_set_pos, SEEK_SET);
    U32 size, num_props = 0;
    reader->readU32(size);
    reader->readU32(num_props);
    for (int i = 0; i < num_props; i++) {
        U32 prop_id, offset;
        reader->readU32(prop_id);
        reader->readU32(offset);
        int p = reader->tell();
        switch (prop_id) {
        case 0x00000001:
            reader->seek(property_set_pos + offset, SEEK_SET);
            S16 icodepage;
            if (!read_vt_i2(reader, log_stream, icodepage)) {
                delete reader;
                return false;
            }
            codepage = "cp" + int2string(icodepage);
            delete reader;
            return true;
        }
        reader->seek(p, SEEK_SET);
        if (!reader->isValid()) {
            log_stream << reader->getLastError() << "\n";
            delete reader;
            return false;
        }
    }
    if (!reader->isValid()) {
        log_stream << reader->getLastError() << "\n";
        delete reader;
        return false;
    }
    delete reader;
    return false;
}

bool parse_oshared_document_summary_info(ThreadSafeOLEStorage &storage, std::ostream &log_stream, int &slide_count)
{
    log_stream << "Extracting additional metadata.\n";
    size_t field_set_stream_start;
    if (!storage.isValid()) {
        log_stream << "Error opening " << storage.name() << " as OLE file.\n";
        return false;
    }
    ThreadSafeOLEStreamReader *reader = open_oshared_document_summary_info(storage, log_stream, field_set_stream_start);
    if (reader == NULL)
        return false;
    U32 offset;
    reader->readU32(offset);
    int property_set_pos = field_set_stream_start + offset;
    reader->seek(property_set_pos, SEEK_SET);
    U32 size, num_props = 0;
    reader->readU32(size);
    reader->readU32(num_props);
    bool slide_count_found = false;
    for (int i = 0; i < num_props; i++) {
        U32 prop_id, offset;
        reader->readU32(prop_id);
        reader->readU32(offset);
        int p = reader->tell();
        switch (prop_id) {
        case 0x00000007:
            reader->seek(property_set_pos + offset, SEEK_SET);
            if (!read_vt_i4(reader, log_stream, slide_count)) {
                delete reader;
                return false;
            }
            slide_count_found = true;
            break;
        }
        reader->seek(p, SEEK_SET);
        if (!reader->isValid()) {
            log_stream << reader->getLastError() << "\n";
            delete reader;
            return false;
        }
    }
    if (!slide_count_found)
        slide_count = -1;
    if (!reader->isValid()) {
        log_stream << reader->getLastError() << "\n";
        delete reader;
        return false;
    }
    delete reader;
    return true;
}
