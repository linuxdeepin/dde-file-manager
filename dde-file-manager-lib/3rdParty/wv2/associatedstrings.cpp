/* This file is part of the wvWare 2 project
   Copyright (C) 2002, 2003 Werner Trobin <trobin@kde.org>

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

#include "associatedstrings.h"
#include "ustring.h"
#include "olestream.h"
#include "word_helper.h"
#include "wvlog.h"

using namespace wvWare;

AssociatedStrings::AssociatedStrings( U32 fcSttbfAssoc, U32 lcbSttbfAssoc, U16 lid, AbstractOLEStreamReader* tableStream ) :
    m_sttbf( 0 )
{
    tableStream->push();
    tableStream->seek( fcSttbfAssoc );
    m_sttbf = new STTBF( lid, tableStream );
    if ( tableStream->tell() - fcSttbfAssoc != lcbSttbfAssoc )
        wvlog << "Warning: Associated strings have a different size than expected!" << std::endl;
    tableStream->pop();
}

AssociatedStrings::AssociatedStrings( const AssociatedStrings& rhs ) :
    m_sttbf( new STTBF( *rhs.m_sttbf ) )
{
}

AssociatedStrings::~AssociatedStrings()
{
    delete m_sttbf;
}

UString AssociatedStrings::associatedTemplate() const
{
    return m_sttbf->stringAt( 1 );
}

UString AssociatedStrings::title() const
{
    return m_sttbf->stringAt( 2 );
}

UString AssociatedStrings::subject() const
{
    return m_sttbf->stringAt( 3 );
}

UString AssociatedStrings::keywords() const
{
    return m_sttbf->stringAt( 4 );
}

UString AssociatedStrings::comments() const
{
    return m_sttbf->stringAt( 5 );
}

UString AssociatedStrings::author() const
{
    return m_sttbf->stringAt( 6 );
}

UString AssociatedStrings::lastRevBy() const
{
    return m_sttbf->stringAt( 7 );
}

UString AssociatedStrings::dataDocument() const
{
    return m_sttbf->stringAt( 8 );
}

UString AssociatedStrings::headerDocument() const
{
    return m_sttbf->stringAt( 9 );
}
