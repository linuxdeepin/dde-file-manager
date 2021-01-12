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

#ifndef WVLOG_H
#define WVLOG_H

#include <iostream>
#include <string>     // Make gcc 2.95.x happy

/**
 * @file A very primitve logging mechanism used to disable any
 * debug output for release builds. Use it like std::cerr, as it
 * is std::cerr (if it's enabled).
 */
namespace wvWare
{

    class wvdebugstream
    {
    public:
#if defined(__GNUC__) && __GNUC__ < 3
        const wvdebugstream& operator<<( ostream& (*__pf)( ostream& ) ) const { std::cerr << __pf; return *this; }
        const wvdebugstream& operator<<( ios (*__pf)( ios& ) ) const { std::cerr << __pf; return *this; }
#else
        const wvdebugstream& operator<<( std::basic_ostream<char>& (*__pf)( std::basic_ostream<char>& ) ) const { std::cerr << __pf; return *this; }
        const wvdebugstream& operator<<( std::ios (*__pf)( std::ios& ) ) const { std::cerr << __pf; return *this; }
        const wvdebugstream& operator<<( std::ios_base& (*__pf) ( std::ios_base& ) ) const { std::cerr << __pf; return *this; }
#endif
        const wvdebugstream& operator<<( long l ) const { std::cerr << l; return *this; }
        const wvdebugstream& operator<<( unsigned long l ) const { std::cerr << l; return *this; }
        const wvdebugstream& operator<<( bool b ) const { std::cerr << b; return *this; }
        const wvdebugstream& operator<<( short s ) const { std::cerr << s; return *this; }
        const wvdebugstream& operator<<( unsigned short s ) const { std::cerr << s; return *this; }
        const wvdebugstream& operator<<( int i ) const { std::cerr << i; return *this; }
        const wvdebugstream& operator<<( unsigned int i ) const { std::cerr << i; return *this; }
        const wvdebugstream& operator<<( double d ) const { std::cerr << d; return *this; }
        const wvdebugstream& operator<<( float f ) const { std::cerr << f; return *this; }
        const wvdebugstream& operator<<( long double d ) const { std::cerr << d; return *this; }
        const wvdebugstream& operator<<( const void* cv ) const { std::cerr << cv; return *this; }
#if defined(__GNUC__) && __GNUC__ < 3
        const wvdebugstream& operator<<( streambuf* s ) const { std::cerr << s; return *this; }
#else
        const wvdebugstream& operator<<( std::basic_streambuf<char>* s ) const { std::cerr << s; return *this; }
#endif
        const wvdebugstream& operator<<( signed char c ) const { std::cerr << c; return *this; }
        const wvdebugstream& operator<<( unsigned char c ) const { std::cerr << c; return *this; }
        const wvdebugstream& operator<<( const char* s ) const { std::cerr << s; return *this; }
        const wvdebugstream& operator<<( const std::string& s ) const { std::cerr << s; return *this; }
    };


    class wvnodebugstream
    {
    public:
#if defined(__GNUC__) && __GNUC__ < 3
        const wvnodebugstream& operator<<( ostream& (*__pf)( ostream& ) ) const { std::cerr << __pf; return *this; }
        const wvnodebugstream& operator<<( ios (*__pf)( ios& ) ) const { std::cerr << __pf; return *this; }
#else
        const wvnodebugstream& operator<<( std::basic_ostream<char>& (*__pf)( std::basic_ostream<char>& ) ) const { return *this; }
        const wvnodebugstream& operator<<( std::ios (*__pf)( std::ios& ) ) const { return *this; }
        const wvnodebugstream& operator<<( std::ios_base& (*__pf) ( std::ios_base& ) ) const { return *this; }
#endif
        const wvnodebugstream& operator<<( long ) const { return *this; }
        const wvnodebugstream& operator<<( unsigned long ) const { return *this; }
        const wvnodebugstream& operator<<( bool ) const { return *this; }
        const wvnodebugstream& operator<<( short ) const { return *this; }
        const wvnodebugstream& operator<<( unsigned short ) const { return *this; }
        const wvnodebugstream& operator<<( int ) const { return *this; }
        const wvnodebugstream& operator<<( unsigned int ) const { return *this; }
        const wvnodebugstream& operator<<( double ) const { return *this; }
        const wvnodebugstream& operator<<( float ) const { return *this; }
        const wvnodebugstream& operator<<( long double ) const { return *this; }
        const wvnodebugstream& operator<<( const void* ) const { return *this; }
#if defined(__GNUC__) && __GNUC__ < 3
        const wvnodebugstream& operator<<( streambuf* ) const { return *this; }
#else
        const wvnodebugstream& operator<<( std::basic_streambuf<char>* ) const { return *this; }
#endif
        const wvnodebugstream& operator<<( signed char ) const { return *this; }
        const wvnodebugstream& operator<<( unsigned char ) const { return *this; }
        const wvnodebugstream& operator<<( const char* ) const { return *this; }
        const wvnodebugstream& operator<<( const std::string& ) const { return *this; }
    };

#ifdef NDEBUG
    typedef wvnodebugstream wvlogstream;
#else
    typedef wvdebugstream wvlogstream;
#endif

    extern const wvlogstream wvlog;

} // wvWare

#endif // WVLOG_H
