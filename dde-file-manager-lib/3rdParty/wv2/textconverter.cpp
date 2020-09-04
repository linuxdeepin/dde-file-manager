/* This file is part of the wvWare 2 project
   Copyright (C) 2001-2003 Werner Trobin <trobin@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "textconverter.h"
#include "ustring.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_ICONV_H
#include <iconv.h>
#endif
#ifdef HAVE_SYS_ICONV_H
#include <sys/iconv.h>
#endif

#include "wvlog.h"
#include <errno.h>

using namespace wvWare;

class TextConverter::Private
{
public:
    Private( const std::string &toCode, const std::string &fromCode ) :
        m_toCode( toCode ), m_fromCode( fromCode ), m_iconv( reinterpret_cast<iconv_t>( -1 ) ) {}

    Private( const std::string &fromCode ) :
#ifdef WORDS_BIGENDIAN
        m_toCode( "UNICODEBIG" ),
#else
        m_toCode( "UNICODELITTLE" ),
#endif
        m_fromCode( fromCode ), m_iconv( reinterpret_cast<iconv_t>( -1 ) ) {}

    Private( U16 lid ) :
#ifdef WORDS_BIGENDIAN
        m_toCode( "UNICODEBIG" ),
#else
        m_toCode( "UNICODELITTLE" ),
#endif
        m_fromCode( TextConverter::LID2Codepage( lid ) ),
        m_iconv( reinterpret_cast<iconv_t>( -1 ) ) {}


    std::string m_toCode, m_fromCode;
    iconv_t m_iconv;
    bool m_swap;
};

TextConverter::TextConverter( const std::string &toCode, const std::string &fromCode ) :
    d( new Private( toCode, fromCode ) )
{
    open();
}

TextConverter::TextConverter( const std::string &fromCode ) : d( new Private( fromCode ) )
{
    open();
}

TextConverter::TextConverter( U16 lid ) : d( new Private( lid ) )
{
    open();
}

TextConverter::~TextConverter()
{
    close();
    delete d;
}

bool TextConverter::isOk() const
{
    return d->m_iconv != reinterpret_cast<iconv_t>( -1 );
}

void TextConverter::setToCode( const std::string &toCode )
{
    d->m_toCode = toCode;
    close();
    open();
}

std::string TextConverter::toCode() const
{
    return d->m_toCode;
}

void TextConverter::setFromCode( const std::string &fromCode )
{
    d->m_fromCode = fromCode;
    close();
    open();
}

std::string TextConverter::fromCode() const
{
    return d->m_fromCode;
}

UString TextConverter::convert( const std::string &input ) const
{
    return convert( input.c_str(), input.size() );
}

UString TextConverter::convert( const char *input, unsigned int length ) const
{
    if ( !isOk() ) {
        wvlog << "Error: I don't have any open converter." << std::endl;
        return UString();
    }

    // WinWord doesn't have multi-byte characters encoded in compressed-unicode
    // sections, right?
    UChar *output = new UChar[ length ];
    char *p_output = reinterpret_cast<char *>( output );
    size_t outputLen = length << 1;

    const char *p_input = input;
    size_t inputLen = length;

    if ( static_cast<size_t>( -1 ) == iconv( d->m_iconv, const_cast<ICONV_CONST char **>( &p_input ), &inputLen, &p_output, &outputLen ) ) {
        delete [] output;
        // If we got more than one character, try to return as much text as possible...
        // To convert the text with as few iconv calls as possible we are using a divide
        // and conquer approach.
        if ( length > 1 )  {
            UString ustring( convert( input, length / 2 ) );
            ustring += convert( input + length / 2, ( length + 1 ) / 2 );
            return ustring;
        } else {
            wvlog << "Error: The conversion was not successful: " << errno << std::endl;
            return UString();
        }
    }

    if ( outputLen != 0 || ( outputLen & 0x00000001 ) == 1 )
        wvlog << "Strange, got an outputLen of " << outputLen << std::endl;

    UString ustring( output, length - ( outputLen >> 1 ) );
    delete [] output;
    return ustring;
}

U16 TextConverter::locale2LID( U8 nLocale )
{
    switch ( nLocale ) {
    case 134:          // Chinese Simplified
        return 0x804;
    case 136:          // Chinese Traditional
        return 0x404;
    // Add Japanese, Korean and whatever nLocale you see fit.
    default:
        return 0x0;
    }
}

const char *TextConverter::LID2lang( U16 lid )
{
    switch ( lid ) {
    case 0x0405:
        return "cs-CZ";
    case 0x0406:
        return "da-DK";
    case 0x0807: /* swiss german */
    case 0x0407: /* german */
        return "de-DE";
    case 0x0809: /* british english */
        return "en-GB";
    case 0x0c09:
        return "en-AU";
    case 0x0413: /* dutch */
        return "da-NL"; /* netherlands */
    case 0x040a: /* castillian */
    case 0x080a: /* mexican */
        return "es-ES";
    case 0x040b:
        return "fi-FI";
    case 0x040c:
        return "fr-FR";
    case 0x0410:
        return "it-IT";
    case 0x040d: /* hebrew */
        return "iw-IL";
    case 0x0416: /* brazilian */
    case 0x0816: /* portugese */
        return "pt-PT";
    case 0x0419:
        return "ru-RU";
    case 0x041d:
        return "sv-SE";
    case 0x0400:
        return "-none-";
    case 0x0409:
    default:
        return "en-US";
    }
}

const char *TextConverter::LID2Codepage( U16 lid )
{
    static const char *cp874 = "CP874";
    static const char *cp932 = "CP932";
    static const char *cp936 = "CP936";
    static const char *cp949 = "CP949";
    static const char *cp950 = "CP950";
    static const char *cp1250 = "CP1250";
    static const char *cp1251 = "CP1251";
    static const char *cp1252 = "CP1252";
    static const char *cp1253 = "CP1253";
    static const char *cp1254 = "CP1254";
    static const char *cp1255 = "CP1255";
    static const char *cp1256 = "CP1256";
    static const char *cp1257 = "CP1257";
    static const char *unknown = "not known";

    // Work around spec bugs. Thomas Zander's documents had very
    // weird lid codes. Mac Word?
    if ( lid < 999 )
        lid = fixLID( lid );

    switch ( lid ) {
    case 0x0401:    /*Arabic*/
        return cp1256;
    case 0x0402:    /*Bulgarian*/
        return cp1251;
    case 0x0403:    /*Catalan*/
        return cp1252;
    case 0x0404:    /*Traditional Chinese*/
        return cp950;
    case 0x0804:    /*Simplified Chinese*/
        return cp936;
    case 0x0405:    /*Czech*/
        return cp1250;
    case 0x0406:    /*Danish*/
        return cp1252;
    case 0x0407:    /*German*/
        return cp1252;
    case 0x0807:    /*Swiss German*/
        return cp1252;
    case 0x0408:    /*Greek*/
        return cp1253;
    case 0x0409:    /*U.S. English*/
        return cp1252;
    case 0x0809:    /*U.K. English*/
        return cp1252;
    case 0x0c09:    /*Australian English*/
        return cp1252;
    case 0x040a:    /*Castilian Spanish*/
        return cp1252;
    case 0x080a:    /*Mexican Spanish*/
        return cp1252;
    case 0x0c0a:    /*Traditional Spanish*/
        return cp1252; // TBD: Undocumented!
    case 0x040b:    /*Finnish*/
        return cp1252;
    case 0x040c:    /*French*/
        return cp1252;
    case 0x080c:    /*Belgian French*/
        return cp1252;
    case 0x0c0c:    /*Canadian French*/
        return cp1252;
    case 0x100c:    /*Swiss French*/
        return cp1252;
    case 0x040d:    /*Hebrew*/
        return cp1255;
    case 0x040e:    /*Hungarian*/
        return cp1250;
    case 0x040f:    /*Icelandic*/
        return cp1252;
    case 0x0410:    /*Italian*/
        return cp1252;
    case 0x0810:    /*Swiss Italian*/
        return cp1252;
    case 0x0411:    /*Japanese*/
        return cp932;
    case 0x0412:    /*Korean*/
        return cp949;
    case 0x0413:    /*Dutch*/
        return cp1252;
    case 0x0813:    /*Belgian Dutch*/
        return cp1252;
    case 0x0414:    /*Norwegian - Bokmal*/
        return cp1252;
    case 0x0814:    /*Norwegian - Nynorsk*/
        return cp1252;
    case 0x0415:    /*Polish*/
        return cp1250;
    case 0x0416:    /*Brazilian Portuguese*/
        return cp1252;
    case 0x0816:    /*Portuguese*/
        return cp1252;
    case 0x0417:    /*Rhaeto-Romanic*/
        return cp1252;
    case 0x0418:    /*Romanian*/
        return cp1252;
    case 0x0419:    /*Russian*/
        return cp1251;
    case 0x041a:    /*Croato-Serbian (Latin)*/
        return cp1250;
    case 0x081a:    /*Serbo-Croatian (Cyrillic) */
        return cp1252;
    case 0x041b:    /*Slovak*/
        return cp1250;
    case 0x041c:    /*Albanian*/
        return cp1251;
    case 0x041d:    /*Swedish*/
        return cp1252;
    case 0x041e:    /*Thai*/
        return cp874;
    case 0x041f:    /*Turkish*/
        return cp1254;
    case 0x0420:    /*Urdu*/
        return cp1256;
    case 0x0421:    /*Bahasa*/
        return cp1256;
    case 0x0422:    /*Ukrainian*/
        return cp1251;
    case 0x0423:    /*Byelorussian*/
        return cp1251;
    case 0x0424:    /*Slovenian*/
        return cp1250;
    case 0x0425:    /*Estonian*/
        return cp1257;
    case 0x0426:    /*Latvian*/
        return cp1257;
    case 0x0427:    /*Lithuanian*/
        return cp1257;
    case 0x0429:    /*Farsi*/
        return cp1256;
    case 0x042D:    /*Basque*/
        return cp1252;
    case 0x042F:    /*Macedonian*/
        return cp1251;
    case 0x0436:    /*Afrikaans*/
        return cp1252;
    case 0x043E:    /*Malaysian*/
        return cp1251;
    default:
        return unknown;
    }
}

void TextConverter::close()
{
    if ( d->m_iconv != reinterpret_cast<iconv_t>( -1 ) )
        iconv_close( d->m_iconv );
    d->m_iconv = reinterpret_cast<iconv_t>( -1 );
}

void TextConverter::open()
{
    if ( d->m_iconv != reinterpret_cast<iconv_t>( -1 ) ) {
        wvlog << "Warning: Do you really want to get rid of the current converter?" << std::endl;
        close();
    }
#ifdef WORDS_BIGENDIAN
    if ( d->m_toCode != "UNICODEBIG" )
        wvlog << "Warning: Do you really want to do convert to something else than UNICODEBIG?" << std::endl;
#else
    if ( d->m_toCode != "UNICODELITTLE" )
        wvlog << "Warning: Do you really want to do convert to something else than UNICODELITTLE?" << std::endl;
#endif
    if ( d->m_fromCode == "not known" )
        wvlog << "Warning: We don't know the current charset you want to convert from!" << std::endl;

    if ( !d->m_toCode.empty() && !d->m_fromCode.empty() )
        d->m_iconv = iconv_open( d->m_toCode.c_str(), d->m_fromCode.c_str() );
}

U16 TextConverter::fixLID( U16 nLocale )
{
    // I have no idea which code these nLocale numbers are, but the
    // files in ftp://dkuug.dk/i18n/charmaps look promising. If another
    // one of those files turns out to be right I'll add the remaining
    // mappings.
    switch ( nLocale )  {
    case 0x13:      /* Dutch */
        return 0x0413;
    default:
        return nLocale;
    }
}
