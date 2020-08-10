### As you recognized already, "###" at the begin of a line
### marks a comment.

### Everything between start and end will be put into the file
@@license-start@@
/* This file is part of the wvWare 2 project
   Copyright (C) 2001 Werner Trobin <trobin@kde.org>

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

// This code is generated from the Microsoft HTML specification of the
// WinWord format. Do NOT edit this code, but fix the spec or the script
// generating the sources.
// For information about the script and the "hidden features" please read
// the comments at the begin of the script.

// If you find bugs or strange behavior please contact Werner Trobin
// <trobin@kde.org>
@@license-end@@

### Everything between @@includes-start@@ and -end will be added to
### the generated includes (e.g. "#include <string>")
@@includes-start@@
#include "ustring.h"
#include <vector> // for Word97::PAP
@@includes-end@@

### These are the "borders" of the namespace (e.g. namespace Word97 { .. })
### Everything you add in here will be added to the generated code
@@namespace-start@@
/**
 * Font Family Name (FFN), this code is located in the template-Word97.h
 */
struct FFN {
    enum Version { Word95, Word97 };
    /**
     * Creates an empty FFN structure and sets the defaults
     */
    FFN();
    /**
     * Simply calls read(...)
     */
    FFN(OLEStreamReader *stream, Version version, bool preservePos=false);

    /**
     * This method reads the FFN structure from the stream.
     * If  preservePos is true we push/pop the position of
     * the stream to save the state. If it's false the state
     * of stream will be changed!
     */
    bool read(OLEStreamReader *stream, Version version, bool preservePos=false);

    /**
     * Same as reading, not implemented yet
     */
    bool write(OLEStreamWriter *stream, bool preservePos=false) const;

    /**
     * Set all the fields to the inital value (default is 0)
     */
    void clear();

    // Data
    /**
     * total length of FFN - 1.
     */
    U8 cbFfnM1;

    /**
     * pitch request
     */
    U8 prq:2;

    /**
     * when 1, font is a TrueType font
     */
    U8 fTrueType:1;

    /**
     * reserved
     */
    U8 unused1_3:1;

    /**
     * font family id
     */
    U8 ff:3;

    /**
     * reserved
     */
    U8 unused1_7:1;

    /**
     * base weight of font
     */
    S16 wWeight;

    /**
     * character set identifier
     */
    U8 chs;

    /**
     * index into ffn.szFfn to the name of the alternate font
     */
    U8 ixchSzAlt;

    /**
     * ? This is supposed to be of type PANOSE.
     */
    U8 panose[10];

    /**
     * ? This is supposed to be of type FONTSIGNATURE.
     */
    U8 fs[24];

    /**
     * zero terminated string that records name of font. Possibly followed
     * by a second xsz which records the name of an alternate font to use if the
     * first named font does not exist on this system. Maximal size of xszFfn
     * is 65 characters.
     */
    //U8 *xszFfn;   //    U8 xszFfn[];
    /**
     * We are using two UStrings here, the alternative string (xszFfnAlt) will
     * contain the alternative font name in case ixchSzAlt is != 0
     */
    UString xszFfn;
    UString xszFfnAlt;

private:
    FFN(const FFN &rhs);
    FFN &operator=(const FFN &rhs);

    void clearInternal();
}; // FFN

/**
 * Tab Descriptor (TBD)
 */
struct TBD
{
    TBD() : jc( 0 ), tlc( 0 ), unused0_6( 0 ) {}
    TBD( U8 tbd )
    {
        jc = tbd;
        tbd >>= 3;
        tlc = tbd;
        tbd >>= 3;
        unused0_6 = tbd;
    }

    /**
     * justification code
     * 0 left tab
     * 1 centered tab
     * 2 right tab
     * 3 decimal tab
     * 4 bar
     */
    U8 jc:3;

    /**
     * tab leader code
     * 0 no leader
     * 1 dotted leader
     * 2 hyphenated leader
     * 3 single line leader
     * 4 heavy line leader
     */
    U8 tlc:3;

    /**
     * reserved
     */
    U8 unused0_6:2;
};

/**
 * Convenient structure for describing tabs
 * It's difficult to sort two arrays in parallel, so instead of rgdxaTab[] and rgtbd[]
 * we combine all the data for one tab into this struct, and the PAP has
 * a vector<TabDescriptor>
 */
struct TabDescriptor
{
    /**
     * Position of the tab
     */
    S16 dxaTab;
    /**
     * Options (justification and tab-leading code)
     */
    Word97::TBD tbd;

}; // TabDescriptor

// There can be only one tab at a given position, no matter what the other options are
bool operator==( const TabDescriptor& lhs, const TabDescriptor& rhs );
bool operator!=( const TabDescriptor& lhs, const TabDescriptor& rhs );
bool operator<( const TabDescriptor& lhs, const TabDescriptor& rhs );
bool operator>( const TabDescriptor& lhs, const TabDescriptor& rhs );

### This tag "expands" to all the structs :)
@@generated-code@@

### nothing
@@namespace-end@@
