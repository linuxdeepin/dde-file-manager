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

#include "xls_parser.h"

#include <iostream>
#include <map>
#include <math.h>
#include "misc.h"
#include "oshared.h"
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ustring.h"
#include "textconverter.h"
#include "utilitiesrename.h"
#include <vector>
#include <time.h>
#include "thread_safe_ole_stream_reader.h"
#include "thread_safe_ole_storage.h"

using namespace wvWare;

enum RecordType {
    XLS_BOF = 0x809,
    XLS_CODEPAGE = 0x42,
    XLS_BLANK = 0x201,
    XLS_CONTINUE = 0x3C,
    XLS_DATE_1904 = 0x22,
    XLS_FILEPASS = 0x2F,
    XLS_FORMAT = 0x41E,
    XLS_FORMULA = 0x06,
    XLS_INTEGER_CELL = 0x202, // Not in MS specification
    XLS_LABEL = 0x204,
    XLS_RSTRING = 0xD6,
    XLS_LABEL_SST = 0xFD,
    XLS_MULBLANK = 0xBE,
    XLS_MULRK = 0xBD,
    XLS_NUMBER = 0x203,
    XLS_RK = 0x27E,
    XLS_SST = 0xFC,
    XLS_STRING = 0x207,
    XLS_XF = 0xE0,
    XLS_EOF = 0x0A
};

struct XLSParser::Implementation {
    bool m_error;
    const char *m_buffer;
    size_t m_buffer_size;
    std::string m_codepage;
    std::string m_file_name;
    bool m_verbose_logging;
    std::ostream *m_log_stream;
    enum BiffVersion { BIFF2, BIFF3, BIFF4, BIFF5, BIFF8 };
    BiffVersion m_biff_version;
    struct XFRecord {
        short int num_format_id;
    };
    std::vector<XFRecord> m_xf_records;
    double m_date_shift;
    std::vector<std::string> m_shared_string_table;
    std::vector<unsigned char> m_shared_string_table_buf;
    std::vector<size_t> m_shared_string_table_record_sizes;
    int m_prev_rec_type;
    int m_last_string_formula_row;
    int m_last_string_formula_col;
    std::set<int> m_defined_num_format_ids;
    int m_last_row, m_last_col;

    U16 getU16LittleEndian(std::vector<unsigned char>::const_iterator buffer)
    {
        return (unsigned short int)(*buffer) | ((unsigned short int)(*(buffer + 1)) << 8);
    }

    S32 getS32LittleEndian(std::vector<unsigned char>::const_iterator buffer)
    {
        return (long)(*buffer) | ((long)(*(buffer + 1)) << 8L) | ((long)(*(buffer + 2)) << 16L) | ((long)(*(buffer + 3)) << 24L);
    }

    class StandardDateFormats : public std::map<int, std::string>
    {
    public:
        StandardDateFormats()
        {
            insert(value_type(0x0E, "%m-%d-%y"));
            insert(value_type(0x0F, "%d-%b-%y"));
            insert(value_type(0x10, "%d-%b"));
            insert(value_type(0x11, "%b-%d"));
            insert(value_type(0x12, "%l:%M %p"));
            insert(value_type(0x13, "%l:%M:%S %p"));
            insert(value_type(0x14, "%H:%M"));
            insert(value_type(0x15, "%H:%M:%S"));
            insert(value_type(0x16, "%m-%d-%y %H:%M"));
            insert(value_type(0x2d, "%M:%S"));
            insert(value_type(0x2e, "%H:%M:%S"));
            insert(value_type(0x2f, "%M:%S"));
            insert(value_type(0xa4, "%m.%d.%Y %l:%M:%S %p"));
        }
    };

    bool oleEof(ThreadSafeOLEStreamReader &reader)
    {
        return reader.tell() == reader.size();
    }

    std::string getStandardDateFormat(int xf_index)
    {
        static StandardDateFormats formats;
        if (xf_index >= m_xf_records.size()) {
            *m_log_stream << "Incorrect format code " << xf_index << ".\n";
            return "";
        }
        int num_format_id = m_xf_records[xf_index].num_format_id;
        if (m_defined_num_format_ids.count(num_format_id))
            return "";
        StandardDateFormats::iterator i = formats.find(num_format_id);
        if (i == formats.end())
            return "";
        else
            return i->second;
    }

    std::string xlsDateToString(double xls_date, std::string date_fmt)
    {
        time_t time = rint((long double)((xls_date - m_date_shift) * 86400));
        char buffer[128];
        strftime(buffer, 127, date_fmt.c_str(), gmtime(&time));
        return buffer;
    }

    std::string formatXLSNumber(double number, short int xf_index)
    {
        std::string date_fmt = getStandardDateFormat(xf_index);
        if (date_fmt != "")
            return xlsDateToString(number, date_fmt);
        else {
            char buffer[128];
            sprintf(buffer, "%.12g", number);
            return buffer;
        }
    }

    std::string parseXNum(std::vector<unsigned char>::const_iterator src, int xf_index)
    {
        union {
            unsigned char xls_num[8];
            double num;
        } xnum_conv;
#ifdef WORDS_BIGENDIAN
        std::reverse_copy(src, src + 8, xnum_conv.xls_num)
#else
        std::copy(src, src + 8, xnum_conv.xls_num);
#endif
        return formatXLSNumber(xnum_conv.num, xf_index);
    }

    std::string parseRkRec(std::vector<unsigned char>::const_iterator src, short int xf_index)
    {
        double number;
        if ((*src) & 0x02)
            number = (double)(getS32LittleEndian(src) >> 2);
        else {
            union {
                unsigned char xls_num[8];
                double num;
            } rk_num_conv;
            std::fill(rk_num_conv.xls_num, rk_num_conv.xls_num + 8, '\0');
#ifdef WORDS_BIGENDIAN
std:
            reverse_copy(src, src + 4, dconv.xls_num);
            rk_num_conv.xls_num[0] &= 0xfc;
#else
            std::copy(src, src + 4, rk_num_conv.xls_num + 4);
            rk_num_conv.xls_num[3] &= 0xfc;
#endif
            number = rk_num_conv.num;
        }
        if ((*src) & 0x01)
            number *= 0.01;
        return formatXLSNumber(number, xf_index);
    }

    std::string parseXLUnicodeString(std::vector<unsigned char>::const_iterator *src, std::vector<unsigned char>::const_iterator src_end, const std::vector<size_t> &record_sizes, size_t &record_index, size_t &record_pos)
    {
        if (record_pos >= record_sizes[record_index]) {
            size_t diff = record_pos - record_sizes[record_index];
            if (diff > 0)
                *m_log_stream << "Warning: XLUnicodeString starts " << diff << " bytes past record boundary.\n";
            record_pos = diff;
            record_index++;
        }
        /*
            This part is mostly based on OpenOffice/LibreOffice XLS format documentation and filter source code.
            record id   BIFF    ->  XF type     String type
            0x0004      2-7     ->  3 byte      8-bit length, byte string
            0x0004      8       ->  3 byte      16-bit length, unicode string
            0x0204      2-7     ->  2 byte      16-bit length, byte string
            0x0204      8       ->  2 byte      16-bit length, unicode string
        */
#warning TODO: Add support for record 0x0004 (in BIFF2).
        if (src_end - *src < 2) {
            *m_log_stream << "Unexpected end of buffer.\n";
            *src = src_end;
            return "";
        }
        int count = getU16LittleEndian(*src);
        *src += 2;
        record_pos += 2;
        if (src_end - *src < 1) {
            *m_log_stream << "Unexpected end of buffer.\n";
            *src = src_end;
            return "";
        }
        int flags = 0;
        if (m_biff_version >= BIFF8) {
            flags = **src;
            *src += 1;
            record_pos += 1;
        }
        int char_size = (flags & 0x01) ? 2 : 1;
        int after_text_block_len = 0;
        if (flags & 0x08) { // rich text
            if (m_verbose_logging)
                *m_log_stream << "Rich text flag enabled.\n";
            if (src_end - *src < 2) {
                *m_log_stream << "Unexpected end of buffer.\n";
                *src = src_end;
                return "";
            }
            after_text_block_len += 4 * getU16LittleEndian(*src);
            *src += 2;
            record_pos += 2;
        }
        if (flags & 0x04) { // asian
            if (m_verbose_logging)
                *m_log_stream << "Asian flag enabled.\n";
            if (src_end - *src < 4) {
                *m_log_stream << "Unexpected end of buffer.\n";
                *src = src_end;
                return "";
            }
            after_text_block_len += getS32LittleEndian(*src);
            *src += 4;
            record_pos += 4;
        }
        if (m_verbose_logging && after_text_block_len > 0)
            *m_log_stream << "Additional formatting blocks found, size " << after_text_block_len << " bytes.\n";
        std::string dest;
        std::vector<unsigned char>::const_iterator s = *src;
        int char_count = 0;
        for (int i = 0; i < count; i++, s += char_size, record_pos += char_size) {
            if (s >= src_end) {
                *m_log_stream << "Unexpected end of buffer.\n";
                *src = src_end;
                return dest;
            }
            if (record_pos > record_sizes[record_index])
                *m_log_stream << "Warning: record boundary crossed.\n";
            if (record_pos == record_sizes[record_index]) {
                if (m_verbose_logging)
                    *m_log_stream << "Record boundary reached.\n";
                record_index++;
                record_pos = 0;
                // At the beginning of each CONTINUE record the option flags byte is repeated.
                // Only the character size flag will be set in this flags byte, the Rich-Text flag and the Far-East flag are set to zero.
                // In each CONTINUE record it is possible that the character size changes
                // from 8‑bit characters to 16‑bit characters and vice versa.
                if (s >= src_end) {
                    *m_log_stream << "Unexpected end of buffer.\n";
                    *src = src_end;
                    return dest;
                }
                if ((*s) != 0 && (*s) != 1)
                    *m_log_stream << "Incorrect XLUnicodeString flag.\n";
                char_size = ((*s) & 0x01) ? 2 : 1;
                if (char_size == 2) {
                    s--;
                    record_pos--;
                }
                i--;
                continue;
            }
            if (char_size == 2) {
                if (src_end - *src < 2) {
                    *m_log_stream << "Unexpected end of buffer.\n";
                    *src = src_end;
                    return dest;
                }
                unsigned int uc = getU16LittleEndian(s);
#warning TODO: Find explanation (documentation) of NULL characters (OO skips them).
                if (uc == 0)
                    continue;
                if (utf16_unichar_has_4_bytes(uc)) {
                    record_pos += 2;
                    s += 2;
                    if (src_end - *src < 2) {
                        *m_log_stream << "Unexpected end of buffer.\n";
                        *src = src_end;
                        return dest;
                    }
                    uc = (uc << 16) | getU16LittleEndian(s);
                }
                dest += unichar_to_utf8(uc);
                char_count++;
            } else {
                if (s >= src_end) {
                    *m_log_stream << "Unexpected end of buffer.\n";
                    *src = src_end;
                    return dest;
                }
                std::string c2(1, *s);
                if (m_codepage != "ASCII") {
                    TextConverter tc(m_codepage);
                    dest += ustring_to_string(tc.convert(c2));
                } else
                    dest += c2;
                char_count++;
            }
        }
        *src = s + after_text_block_len;
        record_pos += after_text_block_len;
        return dest;
    }

    void parseSharedStringTable(const std::vector<unsigned char> &sst_buf)
    {
        if (m_verbose_logging)
            *m_log_stream << "Parsing shared string table.\n";
        int sst_size = getS32LittleEndian(sst_buf.begin() + 4);
        std::vector<unsigned char>::const_iterator src = sst_buf.begin() + 8;
        size_t record_index = 0;
        size_t record_pos = 8;
        while (src < sst_buf.end() && m_shared_string_table.size() <= sst_size)
            m_shared_string_table.push_back(parseXLUnicodeString(&src, sst_buf.end(), m_shared_string_table_record_sizes, record_index, record_pos));
    }

    std::string cellText(int row, int col, const std::string &s)
    {
        std::string r;
        while (row > m_last_row) {
            r += "\n";
            ++m_last_row;
            m_last_col = 0;
        }
        if (col > 0 && col <= m_last_col)
            r += "\t";
        while (col > m_last_col) {
            r += "\t";
            ++m_last_col;
        }
        r += s;
        return r;
    }

    bool processRecord(int rec_type, const std::vector<unsigned char> &rec, std::string &text)
    {
        if (m_verbose_logging)
            *m_log_stream << std::hex << "record=0x" << rec_type << std::endl;
        if (rec_type != XLS_CONTINUE && m_prev_rec_type == XLS_SST)
            parseSharedStringTable(m_shared_string_table_buf);
        switch (rec_type) {
        case XLS_BLANK: {
            int row = getU16LittleEndian(rec.begin());
            int col = getU16LittleEndian(rec.begin() + 2);
            text += cellText(row, col, "");
            break;
        }
        case XLS_BOF: {
            m_last_row = 0;
            m_last_col = 0;
#warning TODO: Check for stream type, ignore charts, or make it configurable
#warning TODO: Mark beginning of sheet (configurable)
            break;
        }
        case XLS_CODEPAGE: {
            if (rec.size() == 2) {
                int codepage = getU16LittleEndian(rec.begin());
                if (codepage == 1200)
                    break;
                else if (codepage == 367)
                    m_codepage = "ASCII";
                else
                    m_codepage = "cp" + int2string(codepage);
            }
            break;
        }
        case XLS_CONTINUE: {
            if (m_prev_rec_type != XLS_SST)
                return true; // do not change m_prev_rec_type
            m_shared_string_table_buf.reserve(m_shared_string_table_buf.size() + rec.size());
            m_shared_string_table_buf.insert(m_shared_string_table_buf.end(), rec.begin(), rec.begin() + rec.size());
            m_shared_string_table_record_sizes.push_back(rec.size());
            if (m_verbose_logging)
                *m_log_stream << "XLS_CONTINUE record for XLS_SST found. Index: " << m_shared_string_table_record_sizes.size() - 1 << ", size:" << rec.size() << ".\n";
            return true;
        }
        case XLS_DATE_1904:
            m_date_shift = 24107.0;
            break;
        case XLS_EOF: {
            text += "\n";
#warning TODO: Mark end of sheet (configurable)
            break;
        }
        case XLS_FILEPASS: {
            *m_log_stream << "XLS file is encrypted.\n";
            U16 encryption_type = getU16LittleEndian(rec.begin());
            if (encryption_type == 0x0000)
                *m_log_stream << "XOR obfuscation encryption type detected.\n";
            else if (encryption_type == 0x0001) {
                *m_log_stream << "RC4 encryption type detected.\n";
                U16 header_type = getU16LittleEndian(rec.begin() + 2);
                if (header_type == 0x0001)
                    *m_log_stream << "RC4 encryption header found.\n";
                else if (header_type == 0x0002 || header_type == 0x0003)
                    *m_log_stream << "RC4 CryptoAPI encryption header found.\n";
                else
                    *m_log_stream << "Unknown RC4 encryption header.\n";
            } else
                *m_log_stream << "Unknown encryption type.\n";
            return false;
        }
        case XLS_FORMAT: {
            int num_format_id = getU16LittleEndian(rec.begin());
            m_defined_num_format_ids.insert(num_format_id);
            break;
        }
        case XLS_FORMULA: {
            m_last_string_formula_row = -1;
            int row = getU16LittleEndian(rec.begin());
            int col = getU16LittleEndian(rec.begin() + 2);
            if (((unsigned char)rec[12] == 0xFF) && (unsigned char)rec[13] == 0xFF) {
                if (rec[6] == 0) {
                    m_last_string_formula_row = row;
                    m_last_string_formula_col = col;
                } else if (rec[6] == 1) {
#warning TODO: check and test boolean formulas
                    text += (rec[8] ? "TRUE" : "FALSE");
                } else if (rec[6] == 2)
                    text += "ERROR";
            } else {
                int xf_index = getU16LittleEndian(rec.begin() + 4);
                text += cellText(row, col, parseXNum(rec.begin() + 6, xf_index));
            }
            break;
        }
        case XLS_INTEGER_CELL: {
            int row = getU16LittleEndian(rec.begin());
            int col = getU16LittleEndian(rec.begin() + 2);
            text += cellText(row, col, int2string(getU16LittleEndian(rec.begin() + 7)));
            break;
        }
        case XLS_RSTRING:
        case XLS_LABEL: {
            m_last_string_formula_row = -1;
            int row = getU16LittleEndian(rec.begin());
            int col = getU16LittleEndian(rec.begin() + 2);
            std::vector<unsigned char>::const_iterator src = rec.begin() + 6;
            std::vector<size_t> sizes;
            sizes.push_back(rec.size() - 6);
            size_t record_index = 0;
            size_t record_pos = 0;
            text += cellText(row, col, parseXLUnicodeString(&src, rec.end(), sizes, record_index, record_pos));
            break;
        }
        case XLS_LABEL_SST: {
            m_last_string_formula_row = -1;
            int row = getU16LittleEndian(rec.begin());
            int col = getU16LittleEndian(rec.begin() + 2);
            int sst_index = getU16LittleEndian(rec.begin() + 6);
            if (sst_index >= m_shared_string_table.size() || sst_index < 0) {
                *m_log_stream << "Incorrect SST index.\n";
                return false;
            } else
                text += cellText(row, col, m_shared_string_table[sst_index]);
            break;
        }
        case XLS_MULBLANK: {
            int row = getU16LittleEndian(rec.begin());
            int start_col = getU16LittleEndian(rec.begin() + 2);
            int end_col = getU16LittleEndian(rec.begin() + rec.size() - 2);
            for (int c = start_col; c <= end_col; c++)
                text += cellText(row, c, "");
            break;
        }
        case XLS_MULRK: {
            m_last_string_formula_row = -1;
            int row = getU16LittleEndian(rec.begin());
            int start_col = getU16LittleEndian(rec.begin() + 2);
            int end_col = getU16LittleEndian(rec.begin() + rec.size() - 2);
            for (int offset = 4, col = start_col; col <= end_col; offset += 6, col++) {
                int xf_index = getU16LittleEndian(rec.begin() + offset);
                text += cellText(row, col, parseRkRec(rec.begin() + offset + 2, xf_index));
            }
            break;
        }
        case XLS_NUMBER:
        case 0x03:
        case 0x103:
        case 0x303: {
            m_last_string_formula_row = -1;
            int row = getU16LittleEndian(rec.begin());
            int col = getU16LittleEndian(rec.begin() + 2);
            text += cellText(row, col, parseXNum(rec.begin() + 6, getU16LittleEndian(rec.begin() + 4)));
            break;
        }
        case XLS_RK: {
            m_last_string_formula_row = -1;
            int row = getU16LittleEndian(rec.begin());
            int col = getU16LittleEndian(rec.begin() + 2);
            int xf_index = getU16LittleEndian(rec.begin() + 4);
            text += cellText(row, col, parseRkRec(rec.begin() + 6, xf_index));
            break;
        }
        case XLS_SST: {
            m_shared_string_table_buf.clear();
            m_shared_string_table_record_sizes.clear();
            m_shared_string_table.clear();
            m_shared_string_table_buf.reserve(rec.size());
            m_shared_string_table_buf.insert(m_shared_string_table_buf.end(), rec.begin(), rec.begin() + rec.size());
            m_shared_string_table_record_sizes.push_back(rec.size());
            break;
        }
        case XLS_STRING: {
            std::vector<unsigned char>::const_iterator src = rec.begin();
            if (m_last_string_formula_row < 0) {
                *m_log_stream << "String record without preceeding string formula.\n";
                break;
            }
            std::vector<size_t> sizes;
            sizes.push_back(rec.size());
            size_t record_index = 0;
            size_t record_pos = 0;
            text += cellText(m_last_string_formula_row, m_last_string_formula_col, parseXLUnicodeString(&src, rec.end(), sizes, record_index, record_pos));
            break;
        }
        case XLS_XF:
        case 0x43: {
            XFRecord xf_record;
            xf_record.num_format_id = getU16LittleEndian(rec.begin() + 2);
            m_xf_records.push_back(xf_record);
            break;
        }
        }
        m_prev_rec_type = rec_type;
        return true;
    }

    void parseXLS(ThreadSafeOLEStreamReader &reader, std::string &text)
    {
        m_xf_records.clear();
        m_date_shift = 25569.0;
        m_shared_string_table.clear();
        m_shared_string_table_buf.clear();
        m_shared_string_table_record_sizes.clear();
        m_prev_rec_type = 0;
        int m_last_string_formula_row = -1;
        int m_last_string_formula_col = -1;
        m_defined_num_format_ids.clear();
        m_last_row = 0;
        m_last_col = 0;

        std::vector<unsigned char> rec;
        bool read_status = true;
        while (read_status) {
            if (oleEof(reader)) {
                *m_log_stream << "No BOF record found.\n";
                return;
            }
            U16 rec_type, rec_len;
            if (!reader.readU16(rec_type) || !reader.readU16(rec_len)) {
                *m_log_stream << reader.getLastError() << "\n";
                return;
            }
            enum BofRecordTypes {
                BOF_BIFF_2 = 0x009,
                BOF_BIFF_3 = 0x209,
                BOF_BIFF_4 = 0x0409,
                BOF_BIFF_5_AND_8 = XLS_BOF
            };
            if (rec_type == BOF_BIFF_2 || rec_type == BOF_BIFF_3 || rec_type == BOF_BIFF_4 || rec_type == BOF_BIFF_5_AND_8) {
                int bof_struct_size;
#warning Has all BOF records size 8 or 16?
                if (rec_len == 8 || rec_len == 16) {
                    switch (rec_type) {
                    case BOF_BIFF_5_AND_8: {
                        U16 biff_ver, data_type;
                        if (!reader.readU16(biff_ver) || !reader.readU16(data_type)) {
                            *m_log_stream << reader.getLastError() << "\n";
                            return;
                        }
                        //On microsoft site there is documentation only for "BIFF8". Documentation from OpenOffice is better:
                        /*
                            BIFF5:
                            Offset  Size    Contents
                            0       2       BIFF version (always 0500H for BIFF5). ...
                            2       2       Type of the following data: 0005H = Workbook globals
                                                                        0006H = Visual Basic module
                                                                        0010H = Sheet or dialogue
                                                                        0020H = Chart
                                                                        0040H = Macro sheet
                                                                        0100H = Workspace (BIFF5W only)
                            4       2       Build identifier, must not be 0
                            6       2       Build year
                        */
                        /*
                            BIFF8:
                            Offset  Size    Contents
                            0       2       BIFF version (always 0600H for BIFF8)
                            2       2       Type of the following data: 0005H = Workbook globals
                                                                        0006H = Visual Basic module
                                                                        0010H = Sheet or dialogue
                                                                        0020H = Chart
                                                                        0040H = Macro sheet
                                                                        0100H = Workspace (BIFF8W only)
                            4       2       Build identifier, must not be 0
                            6       2       Build year, must not be 0
                            8       4       File history flags
                            12      4       Lowest Excel version that can read all records in this file
                        */
                        if (biff_ver == 0x600) {
                            if (m_verbose_logging)
                                *m_log_stream << "Detected BIFF8 version\n";
                            rec.resize(8);
                            read_status = reader.read(&*rec.begin(), 8);
                            m_biff_version = BIFF8;
                            bof_struct_size = 16;
                        } else {
                            if (m_verbose_logging)
                                *m_log_stream << "Detected BIFF5 version\n";
                            m_biff_version = BIFF5;
                            bof_struct_size = 8;
                        }
                        break;
                    }
                    case BOF_BIFF_3:
                        if (m_verbose_logging)
                            *m_log_stream << "Detected BIFF3 version\n";
                        m_biff_version = BIFF3;
                        bof_struct_size = 6;
                        break;
                    case BOF_BIFF_4:
                        if (m_verbose_logging)
                            *m_log_stream << "Detected BIFF4 version\n";
                        m_biff_version = BIFF4;
                        bof_struct_size = 6;
                        break;
                    default:
                        if (m_verbose_logging)
                            *m_log_stream << "Detected BIFF2 version\n";
                        m_biff_version = BIFF2;
                        bof_struct_size = 4;
                    }
                    rec.resize(rec_len - (bof_struct_size - 4));
                    read_status = reader.read(&*rec.begin(), rec_len - (bof_struct_size - 4));
                    break;
                } else {
                    *m_log_stream << "Invalid BOF record.\n";
                    return;
                }
            } else {
                rec.resize(126);
                read_status = reader.read(&*rec.begin(), 126);
            }
        }
        if (oleEof(reader)) {
            *m_log_stream << "No BOF record found.\n";
            return;
        }
        bool eof_rec_found = false;
        while (read_status) {
            U16 rec_type;
            if (!reader.readU16(rec_type)) {
                *m_log_stream << reader.getLastError() << "\n";
                return;
            }
            if (oleEof(reader)) {
                processRecord(XLS_EOF, std::vector<unsigned char>(), text);
                return;
            }
            U16 rec_len;
            if (!reader.readU16(rec_len)) {
                *m_log_stream << reader.getLastError() << "\n";
                return;
            }
            if (rec_len > 0) {
                rec.resize(rec_len);
                read_status = reader.read(&*rec.begin(), rec_len);
            } else
                rec.clear();
            if (eof_rec_found) {
                if (rec_type != XLS_BOF)
                    break;
            }
            if (!processRecord(rec_type, rec, text))
                return;
            if (rec_type == XLS_EOF)
                eof_rec_found = true;
            else
                eof_rec_found = false;
        }
    }
};

XLSParser::XLSParser(const std::string &file_name)
{
    impl = new Implementation();
    impl->m_codepage = "cp1251";
    impl->m_error = false;
    impl->m_file_name = file_name;
    impl->m_verbose_logging = false;
    impl->m_log_stream = &std::cerr;
    impl->m_buffer = NULL;
    impl->m_buffer_size = 0;
}

XLSParser::XLSParser(const char *buffer, size_t size)
{
    impl = new Implementation();
    impl->m_codepage = "cp1251";
    impl->m_error = false;
    impl->m_file_name = "Memory buffer";
    impl->m_verbose_logging = false;
    impl->m_log_stream = &std::cerr;
    impl->m_buffer = buffer;
    impl->m_buffer_size = size;
}

XLSParser::~XLSParser()
{
    delete impl;
}

void XLSParser::setVerboseLogging(bool verbose)
{
    impl->m_verbose_logging = verbose;
}

void XLSParser::setLogStream(std::ostream &log_stream)
{
    impl->m_log_stream = &log_stream;
}

bool XLSParser::isXLS()
{
    impl->m_error = false;
    ThreadSafeOLEStorage *storage;
    if (impl->m_buffer)
        storage = new ThreadSafeOLEStorage(impl->m_buffer, impl->m_buffer_size);
    else
        storage = new ThreadSafeOLEStorage(impl->m_file_name);
    if (!storage->isValid()) {
        delete storage;
        return false;
    }
    AbstractOLEStreamReader *reader = storage->createStreamReader("Workbook");
    if (reader == NULL)
        reader = storage->createStreamReader("Book");
    if (reader == NULL) {
        delete storage;
        return false;
    }
    delete reader;
    delete storage;
    return true;
}

void XLSParser::getLinks(std::vector<Link> &links)
{
#warning TODO: Implement this functionality.
}

std::string XLSParser::plainText(const FormattingStyle &formatting)
{
    impl->m_error = false;
    ThreadSafeOLEStorage *storage;
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
    std::string text = plainText(*storage, formatting);
    delete storage;
    return text;
}

std::string XLSParser::plainText(ThreadSafeOLEStorage &storage, const FormattingStyle &formatting)
{
    impl->m_error = false;
    ThreadSafeOLEStreamReader *reader = (ThreadSafeOLEStreamReader *)storage.createStreamReader("Workbook");
    if (reader == NULL)
        reader = (ThreadSafeOLEStreamReader *)storage.createStreamReader("Book");
    if (reader == NULL) {
        *impl->m_log_stream << "Error opening " << impl->m_file_name << " as OLE file.\n";
        impl->m_error = true;
        return "";
    }
    std::string text;
    impl->parseXLS(*reader, text);
    delete reader;
    return text;
}

Metadata XLSParser::metaData()
{
    ThreadSafeOLEStorage *storage = NULL;
    impl->m_error = false;
    Metadata meta;
    if (impl->m_buffer)
        storage = new ThreadSafeOLEStorage(impl->m_buffer, impl->m_buffer_size);
    else
        storage = new ThreadSafeOLEStorage(impl->m_file_name);
    impl->m_error = !parse_oshared_summary_info(*storage, *impl->m_log_stream, meta);
    delete storage;
    return meta;
}

bool XLSParser::error()
{
    return impl->m_error;
}
