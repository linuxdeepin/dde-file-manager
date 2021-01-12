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

#include "doc_parser.h"
#include <iostream>

#include "misc.h"

//Dirty hack, be we now what we are doing and this is internal library.
#define protected public
#define private public
#include "parser.h"
#include "parser9x.h"
#undef protected
#undef private

#include "fields.h"
#include "handlers.h"
#include "metadata.h"
#include "oshared.h"
#include "paragraphproperties.h"
#include "parserfactory.h"
#include <stdio.h>
#include <sstream>
#include <unistd.h>
#include "wv2/ustring.h"
#include <vector>
#ifdef WIN32
#include <windows.h>
#endif
#include "word_helper.h"
#include "word97_generated.h"
#include "wvlog.h"
#include "xls_parser.h"
#include "thread_safe_ole_stream_reader.h"
#include "thread_safe_ole_storage.h"

using namespace wvWare;

struct CurrentTable {
    bool in_table;
    std::string curr_cell_text;
    svector curr_row_cells;
    std::vector<svector> rows;
    CurrentTable() : in_table(false) {};
};

struct CurrentHeaderFooter {
    bool in_header;
    bool in_footer;
    UString header;
    UString footer;
    CurrentHeaderFooter() : in_header(false), in_footer(false) {};
};

// constants are from MS Word Binary File Format Specification
enum FieldType {
    FLT_NONE = 0,
    FLT_FILLIN = 0x27,
    FLT_EMBED = 0x3A,
    FLT_HYPERLINK = 0x58
};

enum FieldPart {
    FIELD_PART_NONE,
    FIELD_PART_PARAMS,
    FIELD_PART_VALUE
};

struct CurrentState {
    CurrentTable table;
    CurrentHeaderFooter header_footer;
    std::list<std::string> obj_texts;
    std::list<std::string>::iterator obj_texts_iter;
    FieldType field_type;
    FieldPart field_part;
    UString field_params;
    UString field_value;
    CurrentState() : field_type(FLT_NONE), field_part(FIELD_PART_NONE) {};
};

struct Comment {
    int fc;
    std::string text;
    std::string author;
};

static bool cp_to_stream_offset(const Parser *parser, std::ostream &log_stream, U32 cp, U32 &stream_offset, bool *unicode_detected = NULL)
{
    const Parser9x *parser9 = dynamic_cast<const Parser9x *>(parser);
    if (parser9 == NULL) {
        log_stream << "This is not a 9x parser.\n";
        return false;
    }
    U32 piece = 0;
    U32 offset = cp;
    if (parser9->m_plcfpcd == NULL) {
        log_stream << "No pieces table found.\n";
        return false;
    }
    // Copied from wv2 code.
    PLCFIterator<Word97::PCD> it( *parser9->m_plcfpcd );
    for ( ; it.current(); ++it, ++piece ) {
        if ( it.currentLim() > cp && it.currentStart() <= cp )
            break;
        offset -= it.currentRun();
    }
    log_stream << "Piece: " << piece << ", offset: " << offset << "\n";
    PLCFIterator<Word97::PCD> it2(parser9->m_plcfpcd->at( piece ) );
    if (!it2.current()) {
        log_stream << "Specified piece not found.\n";
        return false;
    }
    U32 fc = it2.current()->fc;   // Start FC of this piece
    log_stream << "Piece start at FC " << fc << ".\n";
    bool unicode;
    parser9->realFC(fc, unicode);
    log_stream << "After unicode transition piece start at FC " << fc << ".\n";
    if (offset != 0 )
        fc += unicode ? offset * 2 : offset;
    log_stream << "Stream offset is " << fc << ".\n";
    stream_offset = fc;
    if (unicode_detected != NULL)
        *unicode_detected = unicode;
    return true;
}

static bool parse_comments(const Parser *parser, std::ostream &log_stream, std::vector<Comment> &comments)
{
    if (parser->fib().lcbPlcfandTxt == 0) {
        log_stream << "No annotations.\n";
        return true;
    }

    U32 atn_part_cp = parser->fib().ccpText + parser->fib().ccpFtn + parser->fib().ccpHdd + parser->fib().ccpMcr;
    log_stream << "Annotations part at CP " << atn_part_cp << ".\n";
    AbstractOLEStreamReader *reader = parser->m_storage->createStreamReader("WordDocument");
    if (!reader) {
        log_stream << "Error opening WordDocument stream.\n";
        return false;
    }
    const Parser9x *parser9 = dynamic_cast<const Parser9x *>(parser);
    AbstractOLEStreamReader *table_reader = parser->m_storage->createStreamReader(parser9->tableStream());
    if (!table_reader) {
        log_stream << "Error opening table stream..\n";
        delete reader;
        return false;
    }

    U32 annotation_txts_offset = parser->fib().fcPlcfandTxt;
    log_stream << "Annotation texts table at offset " << annotation_txts_offset << ".\n";
    table_reader->seek(annotation_txts_offset);
    U32 annotation_begin_cp = table_reader->readU32();
    std::vector<std::string> annotations;
    for (;;) {
        U32 annotation_end_cp = table_reader->readU32();
        log_stream << "Annotation text position (CP) from " << annotation_begin_cp << " to " << annotation_end_cp << ".\n";
        U32 stream_begin_offset;
        bool unicode;
        if (!cp_to_stream_offset(parser, log_stream, atn_part_cp + annotation_begin_cp, stream_begin_offset, &unicode)) {
            log_stream << "Converting annotation start position to stream offset failed.\n";
            delete reader;
            delete table_reader;
            return false;
        }
        U32 stream_end_offset;
        if (!cp_to_stream_offset(parser, log_stream, atn_part_cp + annotation_end_cp, stream_end_offset)) {
            log_stream << "Converting annotation end position to stream offset failed.\n";
            delete reader;
            delete table_reader;
            return false;
        }
        log_stream << "Annotation text stream position from " << stream_begin_offset << " to " << stream_end_offset << ".\n";
        reader->seek(stream_begin_offset);
        U8 annotation_mark = reader->readU8();
        if (!annotation_mark == 0x05) {
            log_stream << "Incorrect annotation mark.\n";
            delete reader;
            delete table_reader;
            return false;
        }
        std::string annotation;
        while (reader->tell() < stream_end_offset - 1) {
            log_stream << "Stream pos " << reader->tell() << "\n";
#warning TODO: Unicode support in comments
            if (unicode)
                reader->seek(1, SEEK_CUR); // skip unicode byte
            S8 ch = reader->readS8();
            if (ch >= 32 || (ch >= 8 && ch <= 13)) {
                if (ch == '\r')
                    annotation += "\n";
                else
                    annotation += ch;
            }
        }
        log_stream << "Annotation text: \"" << annotation << "\"\n";
        annotations.push_back(annotation);
        if (annotation_end_cp >= parser->fib().ccpAtn - 1)
            break;
        annotation_begin_cp = annotation_end_cp;
    }

    U32 annotation_owners_offset = parser->fib().fcGrpXstAtnOwners;
    U32 annotation_owners_len = parser->fib().lcbGrpXstAtnOwners;
    log_stream << "Annotation owners table at offset " << annotation_owners_offset << " has length " << annotation_owners_len << ".\n";
    table_reader->seek(annotation_owners_offset);
    std::vector<std::string> owners;
    for (int total_len = 0; total_len < annotation_owners_len;) {
        U16 len = table_reader->readU16();
        total_len += 2;
        std::string owner;
        if (len * 2 + total_len > annotation_owners_len) {
            log_stream << "Something is wrong with XST table. Authors of comments could not be parsed.\n";
            owners.clear();
            break;
        }
        for (int i = 0; i < len; i++) {
            S16 ch = table_reader->readS16();
            owner += (char)ch;
            total_len += 2;
        }
        log_stream << "Owner \"" << owner << "\" found.\n";
        owners.push_back(owner);
    }

    U32 annotation_refs_offset = parser->fib().fcPlcfandRef;
    log_stream << "Annotation refs table at offset " << annotation_refs_offset << ".\n";
    table_reader->seek(annotation_refs_offset);
    for (int i = 0;; i++) {
        U32 annotation_ref_cp = table_reader->readU32();
        if (annotation_ref_cp >= parser->fib().ccpText)
            break;
        log_stream << "Annotation " << i << " references text at CP " << annotation_ref_cp << ".\n";
        U32 stream_offset;
        if (!cp_to_stream_offset(parser, log_stream, annotation_ref_cp, stream_offset)) {
            log_stream << "Converting annotation reference position to stream offset failed.\n";
            delete reader;
            delete table_reader;
            return false;
        }
        if (i < annotations.size()) {
            log_stream << "Annotation " << i << " references text at stream offset " << stream_offset << ".\n";
            Comment c;
            c.fc = stream_offset;
            c.text = annotations[i];
            comments.push_back(c);
        }
    }
    table_reader->seek(annotation_refs_offset + 4 * (comments.size() + 1));
    for (int i = 0; i < comments.size(); i++) {
        Word97::ATRD atrd(table_reader);
        if (atrd.ibst < owners.size()) {
            log_stream << "Annotation owner is \"" << owners[atrd.ibst] << "\", index " << atrd.ibst << ".\n";
            comments[i].author = owners[atrd.ibst];
        }
    }
    delete table_reader;
    delete reader;
    return true;
}

static std::string format_comment(const std::string &author, const std::string &text)
{
    std::string comment = "\n[[[COMMENT BY " + author + "]]]\n" + text;
    if (text.empty() || *text.rbegin() != '\n')
        comment += "\n";
    comment += "[[[---]]]\n";
    return comment;
}

class DocToTextTextHandler : public TextHandler
{
private:
    const Parser *m_parser;
    UString *Text;
    CurrentState *m_curr_state;
    FormattingStyle m_formatting;
    bool m_verbose_logging;
    std::ostream *m_log_stream;
    bool m_comments_parsed;
    std::vector<Comment> m_comments;
    U32 m_prev_par_fc;

public:
    DocToTextTextHandler(const Parser *parser, UString *text, CurrentState *curr_state,
                         const FormattingStyle &formatting, bool verbose_logging, std::ostream &log_stream)
        : m_parser(parser), m_curr_state(curr_state), m_comments_parsed(false), m_prev_par_fc(0)
    {
        Text = text;
        m_curr_state = curr_state;
        m_formatting = formatting;
        m_verbose_logging = verbose_logging;
        m_log_stream = &log_stream;
    }

    void sectionStart(SharedPtr<const Word97::SEP> sep)
    {
    }

    void sectionEnd()
    {
    }

    void pageBreak()
    {
    }

    void paragraphStart(SharedPtr<const ParagraphProperties>
                        paragraphProperties)
    {
        if (((Parser9x *)m_parser)->m_currentParagraph->size() > 0) {
            if (m_comments_parsed) {
                for (int i = 0; i < m_comments.size(); i++)
                    if (m_comments[i].fc >= m_prev_par_fc && m_comments[i].fc < ((Parser9x *)m_parser)->m_currentParagraph->back().m_startFC)
                        (*Text) += utf8_to_ustring(format_comment(m_comments[i].author, m_comments[i].text));
            } else {
                if (!parse_comments(m_parser, *m_log_stream, m_comments)) {
                    *m_log_stream << "Parsing comments failed.\n";
                    m_comments.clear();
                }
                m_comments_parsed = true;
            }
        }
        if (m_curr_state->table.in_table && (!paragraphProperties->pap().fInTable)) {
            m_curr_state->table.in_table = false;
            (*Text) += utf8_to_ustring(formatTable(m_curr_state->table.rows, m_formatting));
            m_curr_state->table.rows.clear();
        }
    }

    void paragraphEnd()
    {
        if (m_curr_state->table.in_table)
            m_curr_state->table.curr_cell_text += "\n";
        else if (m_curr_state->header_footer.in_header)
            m_curr_state->header_footer.header += UString("\n");
        else if (m_curr_state->header_footer.in_footer)
            m_curr_state->header_footer.footer += UString("\n");
        else
            (*Text) += UString("\n");
        m_prev_par_fc = ((Parser9x *)m_parser)->m_currentParagraph->back().m_startFC;
    }

    void runOfText (const UString &text, SharedPtr< const Word97::CHP > chp)
    {
        if (m_curr_state->field_part == FIELD_PART_PARAMS)
            m_curr_state->field_params += text;
        else if (m_curr_state->field_part == FIELD_PART_VALUE)
            m_curr_state->field_value += text;
        else if (m_curr_state->table.in_table)
            m_curr_state->table.curr_cell_text += ustring_to_string(text);
        else if (m_curr_state->header_footer.in_header)
            m_curr_state->header_footer.header += text;
        else if (m_curr_state->header_footer.in_footer)
            m_curr_state->header_footer.footer += text;
        else
            (*Text) += text;
    }

    void specialCharacter(SpecialCharacter character,
                          SharedPtr<const Word97::CHP> chp)
    {
    }

    void footnoteFound (FootnoteData::Type type, UChar character,
                        SharedPtr<const Word97::CHP> chp,
                        const FootnoteFunctor &parseFootnote)
    {
    }

    void footnoteAutoNumber(SharedPtr<const Word97::CHP> chp)
    {
    }

    void fieldStart(const FLD *fld,
                    SharedPtr<const Word97::CHP> chp)
    {
        m_curr_state->field_type = (FieldType)fld->flt;
        m_curr_state->field_part = FIELD_PART_PARAMS;
        switch (fld->flt) {
        case FLT_EMBED:
            if (m_verbose_logging)
                *m_log_stream << "Embedded OLE object reference found.\n";
            if (m_curr_state->obj_texts_iter == m_curr_state->obj_texts.end())
                *m_log_stream << "Unexpected OLE object reference.\n";
            else {
                (*Text) += utf8_to_ustring(*m_curr_state->obj_texts_iter);
                m_curr_state->obj_texts_iter++;
            }
            break;
        default:
            m_curr_state->field_params = "";
            m_curr_state->field_value = "";
        }
    }

    void fieldSeparator(const FLD *fld,
                        SharedPtr<const Word97::CHP> chp)
    {
        m_curr_state->field_part = FIELD_PART_VALUE;
    }

    void fieldEnd(const FLD *fld,
                  SharedPtr<const Word97::CHP> chp)
    {
        UString params = m_curr_state->field_params;
        int i = 0;
        while (i < params.length() && params[i] == ' ') i++;
        UString field_type;
        while (i < params.length() && params[i] != ' ') {
            field_type += UString(params[i]);
            i++;
        }
        while (i < params.length() && params[i] == ' ') i++;
        params = params.substr(i);
        UString res_text;
        switch (m_curr_state->field_type) {
        case FLT_FILLIN:
            res_text = params + UString(" ") + m_curr_state->field_value;
            break;
        case FLT_EMBED:
            break;
        case FLT_HYPERLINK:
            if (params[0] == '"') {
                UString hyperlink_url;
                for (i = 1; i < params.length() && params[i] != '"'; i++)
                    hyperlink_url += UString(params[i]);
                res_text = UString(formatUrl(ustring_to_string(hyperlink_url), ustring_to_string(m_curr_state->field_value), m_formatting).c_str());
            } else
                res_text = params + UString(" ") + m_curr_state->field_value;
            break;
        default:
            res_text = m_curr_state->field_value;
        }
        m_curr_state->field_type = FLT_NONE;
        m_curr_state->field_part = FIELD_PART_NONE;
        if (m_curr_state->table.in_table)
            m_curr_state->table.curr_cell_text += ustring_to_string(res_text);
        else if (m_curr_state->header_footer.in_header)
            m_curr_state->header_footer.header += res_text;
        else if (m_curr_state->header_footer.in_footer)
            m_curr_state->header_footer.footer += res_text;
        else
            (*Text) += res_text;
    }

    void endOfDocument()
    {
        if (m_comments_parsed)
            for (int i = 0; i < m_comments.size(); i++)
                if (m_comments[i].fc >= m_prev_par_fc)
                    (*Text) += utf8_to_ustring(format_comment(m_comments[i].author, m_comments[i].text));
    }
};

class DocToTextTableHandler : public TableHandler
{
private:
    UString *Text;
    CurrentTable *m_curr_table;

public:
    DocToTextTableHandler(UString *text, CurrentTable *curr_table)
    {
        Text = text;
        m_curr_table = curr_table;
    }

    void tableRowStart(SharedPtr<const Word97::TAP> tap)
    {
        m_curr_table->in_table = true;
    }

    void tableRowEnd()
    {
        m_curr_table->rows.push_back(m_curr_table->curr_row_cells);
        m_curr_table->curr_row_cells.clear();
    }

    void tableCellStart()
    {
    }

    void tableCellEnd()
    {
        m_curr_table->curr_row_cells.push_back(m_curr_table->curr_cell_text);
        m_curr_table->curr_cell_text = "";
    }
};

class DocToTextSubDocumentHandler : public SubDocumentHandler
{
private:
    CurrentHeaderFooter *m_curr_header_footer;

public:
    DocToTextSubDocumentHandler(CurrentHeaderFooter *curr_header_footer)
        : m_curr_header_footer(curr_header_footer)
    {
    }

    virtual void headerStart(HeaderData::Type type)
    {
        switch (type) {
        case HeaderData::HeaderOdd:
        case HeaderData::HeaderEven:
        case HeaderData::HeaderFirst:
            m_curr_header_footer->in_header = true;
            break;
        case HeaderData::FooterOdd:
        case HeaderData::FooterEven:
        case HeaderData::FooterFirst:
            m_curr_header_footer->in_footer = true;
            break;
        }
    }

    virtual void headerEnd()
    {
        m_curr_header_footer->in_header = false;
        m_curr_header_footer->in_footer = false;
    }
};

struct DOCParser::Implementation {
    bool m_error;
    const char *m_buffer;
    size_t m_buffer_size;
    std::string m_file_name;
    bool m_verbose_logging;
    std::ostream *m_log_stream;
    std::streambuf *m_cerr_buf_backup;

    void modifyCerr()
    {
        if (m_verbose_logging) {
            if (m_log_stream != &std::cerr)
                m_cerr_buf_backup = std::cerr.rdbuf(m_log_stream->rdbuf());
        } else
            std::cerr.setstate(std::ios::failbit);
    }

    void restoreCerr()
    {
        if (m_verbose_logging) {
            if (m_log_stream != &std::cerr)
                std::cerr.rdbuf(m_cerr_buf_backup);
        } else
            std::cerr.clear();
    }
};

DOCParser::DOCParser(const std::string &file_name)
{
    impl = new Implementation();
    impl->m_error = false;
    impl->m_file_name = file_name;
    impl->m_verbose_logging = false;
    impl->m_log_stream = &std::cerr;
    impl->m_buffer = NULL;
    impl->m_buffer_size = 0;
}

DOCParser::DOCParser(const char *buffer, size_t size)
{
    impl = new Implementation();
    impl->m_error = false;
    impl->m_file_name = "Memory buffer";
    impl->m_verbose_logging = false;
    impl->m_log_stream = &std::cerr;
    impl->m_buffer = buffer;
    impl->m_buffer_size = size;
}

DOCParser::~DOCParser()
{
    delete impl;
}

void DOCParser::setVerboseLogging(bool verbose)
{
    impl->m_verbose_logging = verbose;
}

void DOCParser::setLogStream(std::ostream &log_stream)
{
    impl->m_log_stream = &log_stream;
}

bool DOCParser::isDOC()
{
    impl->m_error = false;
    FILE *f = NULL;
    if (!impl->m_buffer)
        f = fopen(impl->m_file_name.c_str(), "r");
    if (f == NULL && !impl->m_buffer) {
        *impl->m_log_stream << "Error opening file " << impl->m_file_name << ".\n";
        impl->m_error = true;
        return false;
    }
    if (f)
        fclose(f);
    impl->modifyCerr();
    ThreadSafeOLEStorage *storage = NULL;
    //storage will be deleted inside parser from wv2 library
    if (impl->m_buffer)
        storage = new ThreadSafeOLEStorage(impl->m_buffer, impl->m_buffer_size);
    else
        storage = new ThreadSafeOLEStorage(impl->m_file_name);
    SharedPtr<Parser> parser = ParserFactory::createParser(storage);
    impl->restoreCerr();
    if (!parser || !parser->isOk()) {
        *impl->m_log_stream << "Creating parser failed.\n";
        return false;
    }
    return true;
}

void DOCParser::getLinks(std::vector<Link> &links)
{
#warning TODO: Implement this functionality.
}

std::string DOCParser::plainText(const FormattingStyle &formatting)
{
    impl->m_error = false;
    CurrentState curr_state;
    if (impl->m_verbose_logging)
        *impl->m_log_stream << "Opening " << impl->m_file_name << " as OLE file to parse all embedded objects in supported formats.\n";
    //Pointer to storage will be passed to parser from wv2 library. This pointer will be deleted, so allocate storage on heap
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
    if (storage->enterDirectory("ObjectPool")) {
        if (impl->m_verbose_logging)
            *impl->m_log_stream << "ObjectPool found, embedded OLE objects probably exist.\n";
        std::vector<std::string> obj_list;
        if (!storage->getStreamsAndStoragesList(obj_list)) {
            *impl->m_log_stream << storage->getLastError() << "\n";
            impl->m_error = true;
            delete storage;
            return "";
        }
        for (size_t i = 0; i < obj_list.size(); ++i) {
            if (impl->m_verbose_logging)
                *impl->m_log_stream << "OLE object entry found: " << obj_list[i] << "\n";
            std::string obj_text;
            if (storage->enterDirectory(obj_list[i])) {
                std::vector<std::string> obj_list;
                if (!storage->getStreamsAndStoragesList(obj_list)) {
                    *impl->m_log_stream << storage->getLastError() << "\n";
                    impl->m_error = true;
                    delete storage;
                    return "";
                }
                if (find(obj_list.begin(), obj_list.end(), "Workbook") != obj_list.end()) {
                    *impl->m_log_stream << "Embedded MS Excel workbook detected.\n";
                    *impl->m_log_stream << "Using XLS parser.\n";
                    XLSParser xls("");
                    std::string xls_text = xls.plainText(*storage, formatting);
                    if (!xls.error())
                        obj_text = xls_text;
                }
                storage->leaveDirectory();
            }
            curr_state.obj_texts.push_back(obj_text);
        }
    } else {
        if (impl->m_verbose_logging)
            *impl->m_log_stream << "No ObjectPool found, embedded OLE objects probably do not exist.\n";
    }
    storage->leaveDirectory();
    curr_state.obj_texts_iter = curr_state.obj_texts.begin();
    impl->modifyCerr();
    SharedPtr<Parser> parser = ParserFactory::createParser(storage);
    impl->restoreCerr();
    if (!parser || !parser->isOk()) {
        *impl->m_log_stream << "Creating parser failed.\n";
        impl->m_error = true;
        return "";
    }
    UString text;
    DocToTextTextHandler text_handler(parser, &text, &curr_state, formatting,
                                      impl->m_verbose_logging, *impl->m_log_stream);
    parser->setTextHandler(&text_handler);
    DocToTextTableHandler table_handler(&text, &curr_state.table);
    parser->setTableHandler(&table_handler);
    DocToTextSubDocumentHandler sub_document_handler(&curr_state.header_footer);
    parser->setSubDocumentHandler(&sub_document_handler);
    impl->modifyCerr();
    bool res = parser->parse();
    impl->restoreCerr();
    if (!res) {
        *impl->m_log_stream << "Parsing document failed.\n";
        impl->m_error = true;
        return "";
    }
    text_handler.endOfDocument();
    if (curr_state.header_footer.header != "")
        text = curr_state.header_footer.header + UString("\n") + text;
    if (curr_state.header_footer.footer != "")
        text += UString("\n") + curr_state.header_footer.footer;
    std::string s = ustring_to_string(text);
    // 0x0b character (vertical tab) is used as no-breaking carraige return.
    std::replace(s.begin(), s.end(), '\x0b', '\n');
    return s;
}

Metadata DOCParser::metaData()
{
    impl->m_error = false;
    Metadata meta;
    ThreadSafeOLEStorage *storage = NULL;
    if (impl->m_buffer)
        storage = new ThreadSafeOLEStorage(impl->m_buffer, impl->m_buffer_size);
    else
        storage = new ThreadSafeOLEStorage(impl->m_file_name);
    impl->m_error = !parse_oshared_summary_info(*storage, *impl->m_log_stream, meta);
    delete storage;
    return meta;
}

bool DOCParser::error()
{
    return impl->m_error;
}
