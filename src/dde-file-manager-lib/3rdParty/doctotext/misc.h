#ifndef DOCTOTEXT_MISC_H
#define DOCTOTEXT_MISC_H

#include "formatting_style.h"
#include <string>
#include <vector>
#include "wv2/ustring.h"


namespace doctotext {
class Link;
}
struct tm;

using namespace doctotext;
using namespace wvWare;

typedef std::vector<std::string> svector;

std::string formatTable(std::vector<svector> &mcols, const FormattingStyle &options);
std::string formatUrl(const std::string &mlink_url, const std::string &mlink_text, const FormattingStyle &options);
std::string formatList(std::vector<std::string> &mlist, const FormattingStyle &options);
std::string formatNumberedList(std::vector<std::string> &mlist);

std::string ustring_to_string(const UString &s);

/**
    Parses date and time in %Y-%m-%dT%H:%M:%S or %Y%m%d;%H%M%S format
**/
bool string_to_date(const std::string &s, tm &date);

std::string date_to_string(const tm &date);

std::string uint_to_string(unsigned int value);

UString utf8_to_ustring(const std::string &src);

std::string unichar_to_utf8(unsigned int unichar);

/**
    UTF16 characters take from 2 to 4 bytes length.
    Code points from 0x0000 to 0xFFFF require two bytes (so called BMP, most popular characters).
    But there are rare used characters which use codes between 0x10000 to 0x10FFFF.
    In that case utf16 character requires 4 bytes. Algorithm:

    0x10000 is subtracted from the code point, leaving a 20 bit number in the range 0..0xFFFFF.
    The top ten bits (a number in the range 0..0x3FF) are added to 0xD800 to give the first code
    unit or lead surrogate, which will be in the range 0xD800..0xDBFF (previous versions of the
    Unicode Standard referred to these as high surrogates). The low ten bits (also in the range 0..0x3FF)
    are added to 0xDC00 to give the second code unit or trail surrogate, which will be in the range
    0xDC00..0xDFFF (previous versions of the Unicode Standard referred to these as low surrogates).

    In BMP set, there are no 16-bit characters which first 6 bits have 110110 or 110111. They are reserved.

    In the following function below, we check if first two bytes require another two bytes.
**/
inline bool utf16_unichar_has_4_bytes(unsigned int ch)
{
    return (ch & 0xFC00) == 0xD800;
}

/**
    All parsers should support Link class. We need to determine where all links lie in final text.
    But it may be sometimes difficult, simply because we are not parsing text in "correct" order all the time.
    Sometimes we are parsing lists, attachments, tables etc. It is easier to mark special place in the text
    where single link lies. We have two functions to help us. If we want to "save" a space for the link in text,
    we can use insertSpecialLinkBlockIntoText function. Make sure that text is already encoded in UTF8.
    This function will insert a block of "data" into given text to mark a place for link.
**/
void insertSpecialLinkBlockIntoText(std::string &text, const Link &link);

/**
    All parsers should support Link class. We need to determine where all links lie in final text.
    But it may be sometimes difficult, simply because we are not parsing text in "correct" order all the time.
    Sometimes we are parsing lists, attachments, tables etc. It is easier to mark special place in the text
    where single link lies. We have two functions to help us. First one is insertSpecialLinkBlockIntoText.
    When we are done parsing a file, we need to obtain positions of all parsed links. In that case, we need to call
    decodeSpecialLinkBlocks. It takes extracted text and vector of links. Then for each link, function
    calculates position and removes "special" characters which we have used in order to mark links.

    There is one important requirement: links in "links" vector (2 argument) must contain records
    in correct order.
**/
void decodeSpecialLinkBlocks(std::string &text, std::vector<Link> &links, std::ostream &log_stream);

std::string int_to_str(int i);

int str_to_int(const std::string &s);

#endif
