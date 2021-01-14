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

#include "global.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

const char *const libwv2_version = "0.2.3";

namespace wvWare {

// Endianness fun
U16 toLittleEndian( U16 data )
{
#if defined(WORDS_BIGENDIAN)
    return ( ( data & 0x00ffU ) << 8 ) | ( ( data & 0xff00U ) >> 8 );
#else
    return data;
#endif
}

U16 toBigEndian( U16 data )
{
#if defined(WORDS_BIGENDIAN)
    return data;
#else
    return ( ( data & 0x00ffU ) << 8 ) | ( ( data & 0xff00U ) >> 8 );
#endif
}

U32 toLittleEndian( U32 data )
{
#if defined(WORDS_BIGENDIAN)
    return ( ( data & 0x000000ffU ) << 24 ) | ( ( data & 0x0000ff00U ) <<  8 ) |
           ( ( data & 0x00ff0000U ) >>  8 ) | ( ( data & 0xff000000U ) >> 24 );
#else
    return data;
#endif
}

U32 toBigEndian( U32 data )
{
#if defined(WORDS_BIGENDIAN)
    return data;
#else
    return ( ( data & 0x000000ffU ) << 24 ) | ( ( data & 0x0000ff00U ) <<  8 ) |
           ( ( data & 0x00ff0000U ) >>  8 ) | ( ( data & 0xff000000U ) >> 24 );
#endif
}

} // namespace wvWare
