/* This file is part of the wvWare 2 project
   Copyright (C) 2002-2003 Werner Trobin <trobin@kde.org>

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

#include "utilitiesrename.h"
#include <stdio.h>

namespace wvWare {

std::string int2string( int i )
{
    char buf[ 40 ];
    snprintf( buf, 40, "%d", i );
    return std::string( buf );
}

std::string uint2string( unsigned int i )
{
    char buf[ 40 ];
    snprintf( buf, 40, "%u", i );
    return std::string( buf );
}
} // namespace wvWare
