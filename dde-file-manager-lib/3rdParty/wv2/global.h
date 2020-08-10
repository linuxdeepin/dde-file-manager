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

#ifndef GLOBAL_H
#define GLOBAL_H

#include "dllmagic.h"

/** @file
 * We use this typedefs to be compatible with the types from
 * the MS HTML specifications.
 */

// A few defines used for "inline" debugging
#define WV2_DUMP_PIECE_TABLE 0 // has to be defined as we just #if it
//#define WV2_DUMP_FIB 1

//#define WV2_DEBUG_STYLESHEET 1
//#define WV2_DEBUG_SPRMS 1

//#define WV2_DEBUG_LIST_READING 1
//#define WV2_DEBUG_LIST_PROCESSING 1

//#define WV2_DEBUG_FIELDS 1

//#define WV2_DEBUG_FOOTNOTES 1

//#define WV2_DEBUG_HEADERS 1

//#define WV2_DEBUG_TABLES 1

//#define WV2_DEBUG_PICTURES 1

// This define should only be commented out for releases (if at all)
#define WV2_CHECKING 1

extern const char* const libwv2_version;

namespace wvWare
{

typedef signed char S8;
typedef unsigned char U8;
typedef signed short S16;
typedef unsigned short U16;
typedef signed int S32;
typedef unsigned int U32;
typedef U16 XCHAR;
typedef U32 FC;

/**
 * This enum tells the apply* methods in the PAP/CHP/... structs what grpprls to
 * expect. Unfortunately the size of the SPRMs changed for Word 8.
 */
enum WordVersion { Word67, Word8 };
const int Word8nFib = 193;

inline U8 readU8( const U8* in )
{
    return *in;
}

inline S8 readS8( const U8* in )
{
    return static_cast<S8>( *in );
}

// reads a U16/S16 or U32/S32 from a little-endian byte
// "array" in an endian-correct way
inline U16 readU16( const U8* in )
{
    return static_cast<U16>( in[0] ) | ( static_cast<U16>( in[1] ) << 8 );
}

inline S16 readS16( const U8* in )
{
    return static_cast<S16>( readU16( in ) );
}

// writes a U16 to a little-endian byte "array" in an endian-correct way
inline void write( U8* out, U16 data )
{
    out[ 0 ] = data & 0x00ff;
    out[ 1 ] = data >> 8;
}

inline U32 readU32( const U8* in )
{
    return static_cast<U32>( in[0] ) | ( static_cast<U32>( in[1] ) << 8 ) |
        ( static_cast<U32>( in[2] ) << 16 ) | ( static_cast<U32>( in[3] ) << 24 );
}

inline S32 readS32(const U8* in)
{
    return static_cast<S32>( readU32( in ) );
}

// Endianness fun
U16 toLittleEndian( U16 data );

inline S16 toLittleEndian( S16 data )
{
    return static_cast<S16>( toLittleEndian( static_cast<U16>( data ) ) );
}

U16 toBigEndian( U16 data );

inline S16 toBigEndian( S16 data )
{
    return static_cast<S16>( toBigEndian( static_cast<U16>( data ) ) );
}

inline U16 swapEndianness( U16 data )
{
    return ( ( data & 0x00ffU ) << 8 ) | ( ( data & 0xff00U ) >> 8 );
}

inline S16 swapEndianness( S16 data )
{
    return ( ( data & 0x00ffU ) << 8 ) | ( ( data & 0xff00U ) >> 8 );
}

U32 toLittleEndian( U32 data );

inline S32 toLittleEndian( S32 data )
{
    return static_cast<S32>( toLittleEndian( static_cast<U32>( data ) ) );
}

U32 toBigEndian( U32 data );

inline S32 toBigEndian( S32 data )
{
    return static_cast<S32>( toBigEndian( static_cast<U32>( data ) ) );
}

inline U32 swapEndianness( U32 data )
{
    return ( ( data & 0x000000ffU ) << 24 ) | ( ( data & 0x0000ff00U ) <<  8 ) |
        ( ( data & 0x00ff0000U ) >>  8 ) | ( ( data & 0xff000000U ) >> 24 );
}

inline S32 swapEndianness( S32 data )
{
    return ( ( data & 0x000000ffU ) << 24 ) | ( ( data & 0x0000ff00U ) <<  8 ) |
        ( ( data & 0x00ff0000U ) >>  8 ) | ( ( data & 0xff000000U ) >> 24 );
}

} // namespace wvWare

#endif // GLOBAL_H
