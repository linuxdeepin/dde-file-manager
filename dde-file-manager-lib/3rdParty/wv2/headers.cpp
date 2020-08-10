/* This file is part of the wvWare 2 project
   Copyright (C) 2003 Werner Trobin <trobin@kde.org>

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

#include "headers.h"
#include "olestream.h"
#include "wvlog.h"

using namespace wvWare;

const int Headers::headerTypes = 6;

Headers::Headers( U32 fcPlcfhdd, U32 lcbPlcfhdd, AbstractOLEStreamReader* tableStream, WordVersion version )
{
    if ( lcbPlcfhdd == 0 )
        return;

    tableStream->push();
#ifdef WV2_DEBUG_HEADERS
    wvlog << "Headers::Headers(): fc=" << fcPlcfhdd << " lcb=" << lcbPlcfhdd << std::endl;
    if ( version == Word8 )
        wvlog << "   there is/are " << lcbPlcfhdd / 4 - 2 << " header(s)" << std::endl;
#endif

    // remove later (do we want asserts in here???)
    if ( lcbPlcfhdd % 4 )
        wvlog << "Bug: m_fib.lcbPlcfhdd % 4 != 0!" << std::endl;
    else if ( version == Word8 && ( lcbPlcfhdd / 4 - 2 ) % headerTypes )
        wvlog << "Bug: #headers % " << headerTypes << " != 0!" << std::endl;

    tableStream->seek( fcPlcfhdd, SEEK_SET );

    U32 i = 0;
    if ( version == Word8 )
        for ( ; i < headerTypes * sizeof( U32 ); i += sizeof( U32 ) )
            if ( tableStream->readU32() )
                wvlog << "Bug: Read a value != 0 where I expected a 0!" << std::endl;
    for ( ; i < lcbPlcfhdd; i += sizeof( U32 ) )
        m_headers.push_back( tableStream->readU32() );

    tableStream->pop();
}

Headers::~Headers()
{
}

void Headers::headerMask( U8 /*sep_grpfIhdt*/ )
{
}
