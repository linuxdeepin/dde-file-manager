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

#include "olestream.h"
#include <stdio.h> // FILE,...

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

using namespace wvWare;

OLEStream::OLEStream( AbstractOLEStorage *storage ) : m_storage( storage )
{
}

OLEStream::~OLEStream()
{
    if ( m_storage )
        m_storage->streamDestroyed( this );
}

void OLEStream::push()
{
    m_positions.push( tell() );
}

bool OLEStream::pop()
{
    if ( m_positions.empty() )
        return false;
    seek( m_positions.top(), SEEK_SET );
    m_positions.pop();
    return true;
}
