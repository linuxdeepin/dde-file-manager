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

#include "word_helper.h"
#include "olestream.h"
#include "textconverter.h"
#include "ustring.h"

namespace wvWare
{

	STTBF::STTBF( U16 lid, AbstractOLEStreamReader* reader, bool preservePos )
    {
        if ( preservePos )
            reader->push();
        init( lid, reader, 0 );
        if ( preservePos )
            reader->pop();
    }

    STTBF::STTBF( U16 lid, const U8* ptr )
    {
        init( lid, 0, ptr );
    }

    STTBF::STTBF( const STTBF& rhs ) : m_strings( rhs.m_strings ), m_extraDataLength( rhs.m_extraDataLength )
    {
        std::vector<U8*>::const_iterator it = rhs.m_extraData.begin();
        std::vector<U8*>::const_iterator end = rhs.m_extraData.end();
        for ( ; it != end; ++it ) {
            U8* tmp = new U8[ m_extraDataLength ];
            memcpy( tmp, *it, m_extraDataLength );
            m_extraData.push_back( tmp );
        }
    }

    STTBF::~STTBF()
    {
        std::vector<U8*>::const_iterator it = m_extraData.begin();
        for ( ; it != m_extraData.end(); ++it )
            delete [] *it;
    }

    unsigned int STTBF::count() const
    {
        return m_strings.size();
    }

    UString STTBF::firstString() const
    {
        m_stringIt = m_strings.begin();
        if ( m_stringIt != m_strings.end() )
            return *m_stringIt;
        return UString::null;
    }

    UString STTBF::nextString() const
    {
        if ( m_stringIt == m_strings.end() )
            return UString::null;
        ++m_stringIt;
        if ( m_stringIt != m_strings.end() )
            return *m_stringIt;
        return UString::null;
    }

    UString STTBF::prevString() const
    {
        if ( m_strings.size() == 0 )
            return UString::null;
        if ( m_stringIt != m_strings.begin() )
            --m_stringIt;
        return *m_stringIt;
    }

    UString STTBF::lastString() const
    {
        m_stringIt = m_strings.end();
        if ( m_stringIt == m_strings.begin() )
            return UString::null;
        --m_stringIt;
        return *m_stringIt;
    }

    UString STTBF::stringAt( unsigned int index ) const
    {
        if ( index < m_strings.size() )
            return m_strings[ index ];
        return UString::null;
    }

    const U8* STTBF::firstExtra() const
    {
        m_extraIt = m_extraData.begin();
        if ( m_extraIt != m_extraData.end() )
            return *m_extraIt;
        return 0;
    }

    const U8* STTBF::nextExtra() const
    {
        if ( m_extraIt == m_extraData.end() )
            return 0;
        ++m_extraIt;
        if ( m_extraIt != m_extraData.end() )
            return *m_extraIt;
        return 0;
    }

    const U8* STTBF::prevExtra() const
    {
        if ( m_extraData.size() == 0 )
            return 0;
        if ( m_extraIt != m_extraData.begin() )
            --m_extraIt;
        return *m_extraIt;
    }

    const U8* STTBF::lastExtra() const
    {
        m_extraIt = m_extraData.end();
        if ( m_extraIt == m_extraData.begin() )
            return 0;
        --m_extraIt;
        return *m_extraIt;
    }

    const U8* STTBF::extraAt( unsigned int index ) const
    {
        if ( index < m_extraData.size() )
            return m_extraData[ index ];
        return 0;
    }

    void STTBF::dumpStrings() const
    {
        wvlog << "STTBF::dumpStrings(): count=" << count() << " extraDataLength="
              << extraDataLength() << std::endl;
        std::vector<UString>::const_iterator it = m_strings.begin();
        std::vector<UString>::const_iterator end = m_strings.end();
        for ( ; it != end; ++it )
            wvlog << "   '" << ( *it ).ascii() << "'" << std::endl;
    }

	void STTBF::init( U16 lid, AbstractOLEStreamReader* reader, const U8* ptr )
    {
        bool extended = false;
        U16 count = readU16( reader, &ptr );
        // "extended" characters?
        if ( count == 0xffff ) {
            extended = true;
            // read the real size
            count = readU16( reader, &ptr );
        }
        m_extraDataLength = readU16( reader, &ptr );

        // If we don't read unicode strings we have to set up a text converter
        TextConverter* textconverter = 0;
        if ( !extended )
            textconverter = new TextConverter( lid );

        // read one string after the other
        for ( U16 i = 0; i < count; ++i ) {
            U16 len = 0;
            if ( extended ) // double byte count!
                len = readU16( reader, &ptr );
            else
                len = readU8( reader, &ptr );

            if ( len != 0 ) {
                if ( extended ) {
                    XCHAR* string = new XCHAR[ len ];
                    for ( U16 j = 0; j < len; ++j )
                        string[ j ] = readU16( reader, &ptr );
                    UString ustring( reinterpret_cast<const wvWare::UChar *>( string ), len );
                    delete [] string;
                    m_strings.push_back( ustring );
                }
                else {
                    U8* string = new U8[ len ];
                    read( reader, &ptr, string, len );
                    UString ustring( textconverter->convert( reinterpret_cast<char*>( string ),
                                                             static_cast<unsigned int>( len ) ) );
                    delete [] string;
                    m_strings.push_back( ustring );
                }
            }
            else
                m_strings.push_back( UString("") );
            if ( m_extraDataLength != 0 ) {
                U8* extra = new U8[ m_extraDataLength ];
                read( reader, &ptr, extra, m_extraDataLength );
                m_extraData.push_back( extra );
            }
        }
        delete textconverter;
    }

	U16 STTBF::readU16( AbstractOLEStreamReader* reader, const U8** ptr ) const
    {
        if ( reader )
            return reader->readU16();
        else if ( *ptr ) {
            U16 ret = wvWare::readU16( *ptr );
            *ptr += 2;
            return ret;
        }
        else {
            wvlog << "Warning: Internal error while reading STTBF" << std::endl;
            return 0;
        }
    }

	U8 STTBF::readU8( AbstractOLEStreamReader* reader, const U8** ptr ) const
    {
        if ( reader )
            return reader->readU8();
        else if ( *ptr ) {
            U8 ret = **ptr;
            *ptr += 1;
            return ret;
        }
        else {
            wvlog << "Warning: Internal error while reading STTBF" << std::endl;
            return 0;
        }
    }

	bool STTBF::read( AbstractOLEStreamReader* reader, const U8** ptr, U8* buffer, size_t length ) const
    {
        if ( reader )
            return reader->read( buffer, length );
        else if ( *ptr ) {
            memcpy( buffer, *ptr, length );
            *ptr += length;
            return true;
        }
        else {
            wvlog << "Warning: Internal error while reading STTBF" << std::endl;
            return false;
        }
    }


    CHPFKP_BX::CHPFKP_BX()
    {
        clear();
    }

	CHPFKP_BX::CHPFKP_BX( AbstractOLEStreamReader* stream, bool preservePos )
    {
        clear();
        read( stream, preservePos );
    }

    CHPFKP_BX::CHPFKP_BX( const U8* ptr )
    {
        clear();
        readPtr( ptr );
    }

	bool CHPFKP_BX::read( AbstractOLEStreamReader* stream, bool preservePos )
    {
        if ( preservePos )
            stream->push();
        offset = stream->readU8();
        if ( preservePos )
            stream->pop();
        return true;
    }

    void CHPFKP_BX::readPtr( const U8* ptr )
    {
        offset = *ptr;
	}

    void CHPFKP_BX::clear()
    {
        offset = 0;
    }

    bool operator==( const CHPFKP_BX& lhs, const CHPFKP_BX& rhs )
    {
        return lhs.offset == rhs.offset;
    }

    bool operator!=( const CHPFKP_BX& lhs, const CHPFKP_BX& rhs )
    {
        return !( lhs == rhs );
    }

    const unsigned int CHPFKP_BX::sizeOf = 1;

} // namespace wvWare
