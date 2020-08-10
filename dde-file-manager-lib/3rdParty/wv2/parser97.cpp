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

#include "parser97.h"
#include "headers97.h"

using namespace wvWare;


Parser97::Parser97( AbstractOLEStorage* storage, AbstractOLEStreamReader* wordDocument ) :
    Parser9x( storage, wordDocument, Word97::FIB( wordDocument, true ) )
{
    if ( !isOk() )
        return;

    // Initialize the remaining data structures
    init();
}

Parser97::~Parser97()
{
}

void Parser97::init()
{
    if ( m_fib.ccpHdd != 0 )
        m_headers = new Headers97( m_fib.fcPlcfhdd, m_fib.lcbPlcfhdd, m_table );
}
