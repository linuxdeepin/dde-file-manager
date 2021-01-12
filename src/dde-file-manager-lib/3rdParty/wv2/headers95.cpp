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

#include "headers95.h"
#include "wvlog.h"

using namespace wvWare;

Headers95::Headers95( U32 fcPlcfhdd, U32 lcbPlcfhdd, AbstractOLEStreamReader* tableStream, U8 dop_grpfIhdt ) :
    Headers( fcPlcfhdd, lcbPlcfhdd, tableStream, Word67 ), ihddOffset( countOnes( dop_grpfIhdt, 0x40 ) )
{
    m_headerCount.push_back( 0 );
}

std::pair<U32, U32> Headers95::findHeader( int sectionNumber, unsigned char mask ) const
{
    if ( m_grpfIhdt.size() <= static_cast<std::vector<U32>::size_type>( sectionNumber ) ) {
        wvlog << "Warning: You are trying to access a section that has not been registered yet!" << std::endl;
        return std::make_pair( 0u, 0u );
    }

    // The offset to the begin of the section + offset due to mask/grpfIhdt
    const int ihdd = ihddOffset + m_headerCount[ sectionNumber ] +
                     countOnes( m_grpfIhdt[ sectionNumber ], mask );
    return std::make_pair( m_headers[ ihdd ], m_headers[ ihdd + 1 ] );
}

void Headers95::headerMask( U8 sep_grpfIhdt )
{
    m_grpfIhdt.push_back( sep_grpfIhdt );
    m_headerCount.push_back( countOnes( sep_grpfIhdt, 0x40 ) + m_headerCount.back() );
}

int Headers95::countOnes( U8 mask, U8 limit ) const
{
    int count = 0;
    for ( U8 m = 1; m < limit; m <<= 1 )
        if ( m & mask )
            ++count;
    return count;
}
