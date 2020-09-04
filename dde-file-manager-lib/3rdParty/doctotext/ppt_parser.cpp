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

#include "ppt_parser.h"

#include <iostream>
#include <map>
#include <math.h>
#include "metadata.h"
#include "misc.h"
#include "oshared.h"
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wv2/ustring.h>
#include "textconverter.h"
#include "utilitiesrename.h"
#include <vector>
#include <list>
#include <stack>
#include "thread_safe_ole_stream_reader.h"
#include "thread_safe_ole_storage.h"

using namespace wvWare;

enum RecordType {
    RT_CSTRING = 0xFBA,
    RT_DOCUMENT = 0x03E8,
    RT_DRAWING = 0x040C,
    RT_END_DOCUMENT_ATOM = 0x03EA,
    RT_LIST = 0x07D0,
    RT_MAIN_MASTER = 0x03F8,
    RT_SLIDE_BASE = 0x03EC, // Not in MS specification
    RT_SLIDE_LIST_WITH_TEXT = 0x0FF0,
    RT_SLIDE = 0x03EE,
    RT_TEXT_BYTES_ATOM = 0x0FA8,
    RT_TEXT_CHARS_ATOM = 0x0FA0,
    OFFICE_ART_CLIENT_TEXTBOX = 0xF00D,
    OFFICE_ART_DG_CONTAINER = 0xF002,
    OFFICE_ART_SPGR_CONTAINER = 0xF003,
    OFFICE_ART_SP_CONTAINER = 0xF004
};

struct PPTParser::Implementation {
    bool m_error;
    std::string m_file_name;
    bool m_verbose_logging;
    std::ostream *m_log_stream;
    const char *m_buffer;
    size_t m_buffer_size;

    U16 getU16LittleEndian(std::vector<unsigned char>::const_iterator buffer)
    {
        return (unsigned short int)(*buffer) | ((unsigned short int)(*(buffer + 1)) << 8);
    }

    U32 getU32LittleEndian(std::vector<unsigned char>::const_iterator buffer)
    {
        return (unsigned long)(*buffer) | ((unsigned long)(*(buffer + 1 )) << 8L) | ((unsigned long)(*(buffer + 2)) << 16L) | ((unsigned long)(*(buffer + 3)) << 24L);
    }

    void parseRecord(int rec_type, unsigned long rec_len, ThreadSafeOLEStreamReader &reader, std::string &text)
    {
        switch (rec_type) {
        case RT_CSTRING:
        case RT_TEXT_CHARS_ATOM: {
            if (m_verbose_logging)
                *m_log_stream << "RT_TextCharsAtom or RT_CString\n";
            std::vector<unsigned char> buf(2);
            unsigned long text_len = rec_len / 2;
            if (text_len * 2 > reader.size() - reader.tell()) {
                text_len = (reader.size() - reader.tell()) / 2;
                *m_log_stream << "Warning: Read past EOF\n";
            }
            for (int i = 0; i < text_len; i++) {
                reader.read((uint8_t *)&*buf.begin(), 2);
                U32 u = getU16LittleEndian(buf.begin());
                if (u == 0x0D || u == 0x0B)
                    text += '\n';
                else {
                    if (utf16_unichar_has_4_bytes(u) && ++i < text_len) {
                        reader.read((uint8_t *)&*buf.begin(), 2);
                        u = (u << 16) | getU16LittleEndian(buf.begin());
                    }
                    text += unichar_to_utf8(u);
                }
            }
            text += '\n';
            break;
        }
        case RT_DOCUMENT:
            if (m_verbose_logging)
                *m_log_stream << "RT_Document\n";
            break;
        case RT_DRAWING:
            if (m_verbose_logging)
                *m_log_stream << "RT_Drawing\n";
            break;
        case RT_END_DOCUMENT_ATOM: {
            if (m_verbose_logging)
                *m_log_stream << "RT_DocumentEnd\n";
            unsigned long len = rec_len;
            if (reader.tell() + len > reader.size()) {
                *m_log_stream << "Warning: Read past EOF\n";
                len = reader.size() - reader.tell();
            }
            reader.seek(len, SEEK_CUR);
            break;
        }
        case RT_LIST:
            if (m_verbose_logging)
                *m_log_stream << "RT_List\n";
            break;
        case RT_MAIN_MASTER: {
            if (m_verbose_logging)
                *m_log_stream << "RT_MainMaster\n";
#warning TODO: Make extracting text from main master slide configurable
            unsigned long len = rec_len;
            if (reader.tell() + len > reader.size()) {
                *m_log_stream << "Warning: Read past EOF\n";
                len = reader.size() - reader.tell();
            }
            reader.seek(len, SEEK_CUR);
            break;
        }
        case RT_SLIDE:
            if (m_verbose_logging)
                *m_log_stream << "RT_Slide\n";
            break;
        case RT_SLIDE_BASE:
            break;
        case RT_SLIDE_LIST_WITH_TEXT:
            if (m_verbose_logging)
                *m_log_stream << "RT_SlideListWithText\n";
            break;
        case RT_TEXT_BYTES_ATOM: {
            if (m_verbose_logging)
                *m_log_stream << "RT_TextBytesAtom\n";
            std::vector<unsigned char> buf(2);
            unsigned long text_len = rec_len;
            buf[0] = buf[1] = 0;
            if (text_len > reader.size() - reader.tell()) {
                text_len = reader.size() - reader.tell();
                *m_log_stream << "Warning: Read past EOF\n";
            }
            for (int i = 0; i < text_len; i++) {
                reader.read((uint8_t *)&*buf.begin(), 1);
                U32 u = getU16LittleEndian(buf.begin());
                if (u == 0x0B || u == 0x0D)
                    text += '\n';
                else
                    text += unichar_to_utf8(u);
            }
            text += '\n';
            break;
        }
        case OFFICE_ART_CLIENT_TEXTBOX:
            if (m_verbose_logging)
                *m_log_stream << "OfficeArtClientTextbox\n";
            break;
        case OFFICE_ART_DG_CONTAINER:
            if (m_verbose_logging)
                *m_log_stream << "OfficeArtDgContainer\n";
            break;
        case OFFICE_ART_SPGR_CONTAINER:
            if (m_verbose_logging)
                *m_log_stream << "OfficeArtSpgrContainer\n";
            break;
        case OFFICE_ART_SP_CONTAINER:
            if (m_verbose_logging)
                *m_log_stream << "OfficeArtSpContainer\n";
            break;
        default:
            unsigned long len = rec_len;
            if (reader.tell() + len > reader.size()) {
                *m_log_stream << "Warning: Read past EOF\n";
                len = reader.size() - reader.tell();
            }
            reader.seek(len, SEEK_CUR);
        }
        if (!reader.isValid()) {
            *m_log_stream << reader.getLastError() + "\n";
            m_error = true;
        }
    }

    bool oleEof(ThreadSafeOLEStreamReader &reader)
    {
        return reader.tell() == reader.size();
    }

    void parseOldPPT(ThreadSafeOLEStorage &storage, ThreadSafeOLEStreamReader &reader, std::string &text)
    {
        std::vector<unsigned char> content(reader.size());
        if (!reader.read(&*content.begin(), reader.size())) { //this stream should only contain text
            *m_log_stream << "Error reading Text_Content stream.\n";
            m_error = true;
            return;
        }
        text = std::string(content.begin(), content.end());
        std::string codepage;
        if (get_codepage_from_document_summary_info(storage, *m_log_stream, codepage)) {
            TextConverter tc(codepage);
            text = ustring_to_string(tc.convert(text));
        }
        //new lines problem?
        for (size_t i = 0; i < text.length(); ++i)
            if (text[i] == 11)
                text[i] = 13;
    }

    void parsePPT(ThreadSafeOLEStreamReader &reader, std::string &text)
    {
        std::vector<unsigned char> rec(8);
        bool read_status = true;
        std::stack<long> container_ends;
        while (read_status) {
            int pos = reader.tell();
            read_status = reader.read(&*rec.begin(), 8);
            if (oleEof(reader)) {
                parseRecord(RT_END_DOCUMENT_ATOM, 0, reader, text);
                return;
            }
            if (rec.size() < 8)
                break;
            int rec_type = getU16LittleEndian(rec.begin() + 2);
            U32 rec_len = getU32LittleEndian(rec.begin() + 4);
            if (m_verbose_logging) {
                while (!container_ends.empty() && pos + rec_len - 1 > container_ends.top())
                    container_ends.pop();
                std::string indend;
                for (int i = 0; i < container_ends.size(); i++)
                    indend += "\t";
                *m_log_stream << indend << "record=0x" << std::hex << rec_type << ", begin=0x" << pos << ", end=0x" << pos + rec_len - 1 << "\n";
                container_ends.push(pos + rec_len - 1);
            }
            parseRecord(rec_type, rec_len, reader, text);
        }
    }
};

PPTParser::PPTParser(const std::string &file_name)
{
    impl = new Implementation();
    impl->m_error = false;
    impl->m_file_name = file_name;
    impl->m_verbose_logging = false;
    impl->m_log_stream = &std::cerr;
    impl->m_buffer = NULL;
    impl->m_buffer_size = 0;
}

PPTParser::PPTParser(const char *buffer, size_t size)
{
    impl = new Implementation();
    impl->m_error = false;
    impl->m_file_name = "Memory buffer";
    impl->m_verbose_logging = false;
    impl->m_log_stream = &std::cerr;
    impl->m_buffer = buffer;
    impl->m_buffer_size = size;
}

PPTParser::~PPTParser()
{
    delete impl;
}

void PPTParser::setVerboseLogging(bool verbose)
{
    impl->m_verbose_logging = verbose;
}

void PPTParser::setLogStream(std::ostream &log_stream)
{
    impl->m_log_stream = &log_stream;
}

bool PPTParser::isPPT()
{
    impl->m_error = false;
    ThreadSafeOLEStorage *storage = NULL;
    if (impl->m_buffer)
        storage = new ThreadSafeOLEStorage(impl->m_buffer, impl->m_buffer_size);
    else
        storage = new ThreadSafeOLEStorage(impl->m_file_name);
    if (!storage->isValid()) {
        delete storage;
        return false;
    }
    AbstractOLEStreamReader *reader = storage->createStreamReader("PowerPoint Document");
    if (reader == NULL) {
        delete storage;
        return false;
    }
    delete reader;
    delete storage;
    return true;
}

void PPTParser::getLinks(std::vector<Link> &links)
{
#warning TODO: Implement this functionality.
}

std::string PPTParser::plainText(const FormattingStyle &formatting)
{
    impl->m_error = false;
    ThreadSafeOLEStorage *storage = NULL;
    if (impl->m_buffer)
        storage = new ThreadSafeOLEStorage(impl->m_buffer, impl->m_buffer_size);
    else
        storage = new ThreadSafeOLEStorage(impl->m_file_name);
    if (!storage->isValid()) {
        *impl->m_log_stream << "Error opening " << impl->m_file_name << " as OLE file.\n";
        impl->m_error = true;
        delete storage;
        return "";
    }
    std::string text;
    std::vector<std::string> dirs;
    if (storage->getStreamsAndStoragesList(dirs)) {
        for (size_t i = 0; i < dirs.size(); ++i) {
#warning TODO: Older PPT files (like Microsoft PowerPoint 4.0) have stream named "Text_Content". \
I have only one file of this type and I cant find any documentation about it. I am not sure \
if parsing those files is correct now... but for this one file that I have it works. It should be tested \
more, one file is not enough. Stream "Text_Content" does not exist in newer versions of PowerPoint. \
In older versions of PowerPoint (the file I have is an example) reading stream "PowerPoint Document" does not work.
#warning TODO: Check if there is a better way to get PowerPoint version.
            if (dirs[i] == "Text_Content") {
                ThreadSafeOLEStreamReader *reader = (ThreadSafeOLEStreamReader *)storage->createStreamReader("Text_Content");
                if (reader != NULL) {
                    impl->parseOldPPT(*storage, *reader, text);
                    delete reader;
                } else {
                    *impl->m_log_stream << storage->getLastError() + "\n";
                    impl->m_error = true;
                }
                delete storage;
                return text;
            }
        }
    }
    ThreadSafeOLEStreamReader *reader = (ThreadSafeOLEStreamReader *)storage->createStreamReader("PowerPoint Document");
    if (reader == NULL) {
        *impl->m_log_stream << "Error opening " << impl->m_file_name << " as OLE file.\n";
        impl->m_error = true;
        delete storage;
        return "";
    }
    impl->parsePPT(*reader, text);
    delete reader;
    delete storage;
    return text;
}

Metadata PPTParser::metaData()
{
    impl->m_error = false;
    Metadata meta;
    ThreadSafeOLEStorage *storage = NULL;
    if (impl->m_buffer)
        storage = new ThreadSafeOLEStorage(impl->m_buffer, impl->m_buffer_size);
    else
        storage = new ThreadSafeOLEStorage(impl->m_file_name);
    if (!parse_oshared_summary_info(*storage, *impl->m_log_stream, meta)) {
        impl->m_error = true;
        delete storage;
        return meta;
    }
    // If page count not found use slide count as page count
    if (meta.pageCount() == -1) {
        int slide_count = 150;
        impl->m_error = !parse_oshared_document_summary_info(*storage, *impl->m_log_stream, slide_count);
        if (!impl->m_error)
            meta.setPageCount(slide_count);
    }
    delete storage;
    return meta;
}

bool PPTParser::error()
{
    return impl->m_error;
}
