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
// generating the sources. If you want to add some additional code, some
// includes or any other stuff, please add it to the template file!
// For information about the script and the "hidden features" please read
// the comments at the begin of the script.

// If you find bugs or strange behavior please contact Werner Trobin
// <trobin@kde.org>
@@license-end@@

### Everything between @@includes-start@@ and -end will be added to
### the generated includes (e.g. "#include <string>")
@@includes-start@@
####include <string>  // test
@@includes-end@@

### These are the "borders" of the namespace (e.g. namespace Word97 { .. })
### Everything you add in here will be added to the generated code
@@namespace-start@@
// FFN implementation, located in template-Word97.cpp
FFN::FFN() {
    clearInternal();
}

FFN::FFN(OLEStreamReader *stream, Version version, bool preservePos) {
    clearInternal();
    read(stream, version, preservePos);
}

bool FFN::read(OLEStreamReader *stream, Version version, bool preservePos) {

    U8 shifterU8;

    if(preservePos)
        stream->push();

    cbFfnM1=stream->readU8();
    shifterU8=stream->readU8();
    prq=shifterU8;
    shifterU8>>=2;
    fTrueType=shifterU8;
    shifterU8>>=1;
    unused1_3=shifterU8;
    shifterU8>>=1;
    ff=shifterU8;
    shifterU8>>=3;
    unused1_7=shifterU8;
    wWeight=stream->readS16();
    chs=stream->readU8();
    ixchSzAlt=stream->readU8();

    U8 remainingSize = cbFfnM1 - 5;

    if ( version == Word97 ) {
        for(int _i=0; _i<(10); ++_i)
            panose[_i]=stream->readU8();
        for(int _i=0; _i<(24); ++_i)
            fs[_i]=stream->readU8();
        remainingSize -= 34;

        // Remaining size in bytes -> shorts
        remainingSize /= 2;
        XCHAR* string = new XCHAR[ remainingSize ];
        for ( int i = 0; i < remainingSize; ++i )
            string[ i ] = stream->readU16();
        if ( ixchSzAlt == 0 )
            xszFfn = UString( reinterpret_cast<const wvWare::UChar *>( string ), remainingSize - 1 );
        else {
            xszFfn = UString( reinterpret_cast<const wvWare::UChar *>( string ), ixchSzAlt - 1 );
            xszFfnAlt = UString( reinterpret_cast<const wvWare::UChar *>( &string[ ixchSzAlt ] ), remainingSize - 1 - ixchSzAlt );
        }
        delete [] string;
    }
    else {
        U8* string = new U8[ remainingSize ];
        stream->read( string, remainingSize );
        // ###### Assume plain latin1 strings, maybe we'll have to use a textconverter here...
        if ( ixchSzAlt == 0 )
            xszFfn = UString( reinterpret_cast<char*>( string ) );
        else {
            xszFfn = UString( reinterpret_cast<char*>( string ) ); // The strings are 0-terminated, according to the SPEC
            xszFfnAlt = UString( reinterpret_cast<char*>( &string[ ixchSzAlt ] ) );
        }
        delete [] string;
    }

    if(preservePos)
        stream->pop();
    return true;
}

void FFN::clear() {
    clearInternal();
}

void FFN::clearInternal() {
    cbFfnM1=0;
    prq=0;
    fTrueType=0;
    unused1_3=0;
    ff=0;
    unused1_7=0;
    wWeight=0;
    chs=0;
    ixchSzAlt=0;
    for(int _i=0; _i<(10); ++_i)
        panose[_i]=0;
    for(int _i=0; _i<(24); ++_i)
        fs[_i]=0;
    xszFfn = UString::null;
    xszFfnAlt = UString::null;
}

// There can be only one tab at a given position, no matter what the other options are
bool operator==( const TabDescriptor& lhs, const TabDescriptor& rhs ) { return lhs.dxaTab == rhs.dxaTab; }
bool operator!=( const TabDescriptor& lhs, const TabDescriptor& rhs ) { return lhs.dxaTab != rhs.dxaTab; }
bool operator<( const TabDescriptor& lhs, const TabDescriptor& rhs ) { return lhs.dxaTab < rhs.dxaTab; }
bool operator>( const TabDescriptor& lhs, const TabDescriptor& rhs ) { return lhs.dxaTab > rhs.dxaTab; }

### This tag "expands" to all the structs :)
@@generated-code@@
### nothing
@@namespace-end@@
