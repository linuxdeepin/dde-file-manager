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

#ifndef TEXTCONVERTER_H
#define TEXTCONVERTER_H

#include "global.h"
#include <string>

namespace wvWare
{

class UString;

/**
 * A small tool class to provide a convenient interface to iconv.
 */
class TextConverter
{
public:
    /**
     * Construct a TextConverter
     */
    TextConverter( const std::string& toCode, const std::string& fromCode );
    /**
     * Constructs a TextConverter which converts to UCS-2
     */
    TextConverter( const std::string& fromCode );
    /**
     * Constructs a TextConverter which converts from the proper
     * codepage for the given lid to UCS-2. This is probably what
     * you want to use.
     */
    TextConverter( U16 lid );

    /**
     * Properly cleans up
     */
    ~TextConverter();

    /**
     * Is the converter okay?
     */
    bool isOk() const;

    /**
     * Change the converter to convert to that code
     */
    void setToCode( const std::string& toCode );
    std::string toCode() const;

    /**
     * Change the converter to convert from that code
     */
    void setFromCode( const std::string& fromCode );
    std::string fromCode() const;

    /**
     * Convert the string to the specified encoding (most likely UCS-2).
     * Note: No other conversions are really supported, as we don't
     * need them anyway.
     */
    UString convert( const std::string& input ) const;

    /**
     * Convert the string to the specified encoding (most likely UCS-2).
     * Note: No other conversions are really supported, as we don't
     * need them anyway.
     */
    UString convert( const char* input, unsigned int length ) const;

    /**
     * "stolen" from wvWare, guesses a lid for some locales.
     */
    static U16 locale2LID( U8 nLocale );

    /**
     * "stolen" from the wvWare, guesses a lang for some lids.
     */
    static const char* LID2lang( U16 lid );

    /**
     * returns the appropriate codepage for the given lid.
     */
    static const char* LID2Codepage( U16 lid );

private:
    /**
     * Don't copy us
     */
    TextConverter( const TextConverter& rhs );
    /**
     * Don't assign us
     */
    TextConverter& operator=( const TextConverter& rhs );

    /**
     * Closes the old converter
     */
    void close();

    /**
     * Opens a converter
     */
    void open();

    static U16 fixLID( U16 nLocale );

    // We need this private impl. to hide the iconv_t in the source file. The
    // reason is that we can't #include <config.h> in the header file!
    class Private;
    Private* d;
};

} // namespace wvWare

#endif // TEXTCONVERTER_H
