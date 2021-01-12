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

#ifndef WORD_HELPER_H
#define WORD_HELPER_H

#include "olestream.h"
#include "wvlog.h"

#include <vector>
#include <string.h>

namespace wvWare
{
    namespace Word97 { class PHE; class BTE; }
    namespace Word95
    {
        class PHE;
        class BTE;
        Word97::PHE toWord97( const Word95::PHE& phe ); // fake, to make gcc 3.4 happy :-(
        Word97::BTE toWord97( const Word95::BTE& s );   // fake, to make gcc 3.4 happy :-(
    }

	class AbstractOLEStreamReader;
    class ParagraphProperties;
    class UString;

    class STTBF
    {
    public:
		STTBF( U16 lid, AbstractOLEStreamReader* reader, bool preservePos = false );
        STTBF( U16 lid, const U8* ptr );
        STTBF( const STTBF& rhs );
        ~STTBF();

        unsigned int count() const;
        unsigned int extraDataLength() const { return m_extraDataLength; }

        UString firstString() const;
        UString nextString() const;
        UString prevString() const;
        UString lastString() const;
        UString stringAt( unsigned int index ) const;

        const U8* firstExtra() const;
        const U8* nextExtra() const;
        const U8* prevExtra() const;
        const U8* lastExtra() const;
        const U8* extraAt( unsigned int index ) const;

        void dumpStrings() const;

    private:
        STTBF& operator=( const STTBF& rhs );

        // Internal helper methods to avoid code duplication in the CTORs
		void init( U16 lid, AbstractOLEStreamReader* reader, const U8* ptr );
		U16 readU16( AbstractOLEStreamReader* reader, const U8** ptr ) const;
		U8 readU8( AbstractOLEStreamReader* reader, const U8** ptr ) const;
		bool read( AbstractOLEStreamReader* reader, const U8** ptr, U8* buffer, size_t length ) const;

        std::vector<UString> m_strings;
        mutable std::vector<UString>::const_iterator m_stringIt;
        U16 m_extraDataLength;
        std::vector<U8*> m_extraData;
        mutable std::vector<U8*>::const_iterator m_extraIt;
    };

    // Attention: structs used as template parameters for this class need to
    // enable the sizeOf field in the code generator (see comment (7)) in
    // the header of generate.pl
    // Use the short count for "broken" plfs like the LSTF one (surprisingly
    // labeled plcflst(!) (see spec_defects for more information)
    template<class T, bool shortCount = false> class PLF
    {
    public:
		PLF( AbstractOLEStreamReader* reader, bool preservePos = false );
        PLF( const U8* ptr );
        ~PLF();

        size_t count() const { return m_items.size(); }

        const T* first() const { it = m_items.begin(); if ( it != m_items.end() ) return *it;  return 0; }
        const T* next() const;
        const T* prev() const;
        const T* last() const { it = m_items.end(); if ( it == m_items.begin() ) return 0; --it; return *it; }
        const T* current() const { if ( it != m_items.end() ) return *it;  return 0; }
        const T* at( unsigned int index ) const { if ( index < m_items.size() ) return m_items[ index ]; return 0; }

    private:
        // don't copy or assign it
        PLF( const PLF<T, shortCount>& rhs );
        PLF<T, shortCount>& operator=( const PLF<T, shortCount>& rhs );

        std::vector<T*> m_items;
        mutable typename std::vector<T*>::const_iterator it;
    };

    template<class T, bool shortCount>
	PLF<T, shortCount>::PLF( AbstractOLEStreamReader* reader, bool preservePos )
    {
        if ( preservePos )
            reader->push();

        U32 count = 0;
        if ( shortCount ) // work around a broken spec, e.g. for LSTF
            count = reader->readU16();
        else
            count = reader->readU32();

        for ( U32 i = 0; i < count; ++i )
            m_items.push_back( new T( reader, false ) );
        if ( preservePos )
            reader->pop();
        it = m_items.begin();
    }

    template<class T, bool shortCount>
    PLF<T, shortCount>::PLF( const U8* ptr )
    {
        U32 count = 0;
        if ( shortCount ) { // work around a broken spec, e.g. for LSTF
            count = readU16( ptr );
            ptr += 2;
        }
        else {
            count = readU32( ptr );
            ptr += 4;
        }

        for ( U32 i = 0; i < count; ++i, ptr += T::sizeOf )
            m_items.push_back( new T( ptr ) );
        it = m_items.begin();
    }

    template<class T, bool shortCount>
    PLF<T, shortCount>::~PLF()
    {
        for ( it = m_items.begin(); it != m_items.end(); ++it )
            delete *it;
    }

    template<class T, bool shortCount>
    const T* PLF<T, shortCount>::next() const
    {
        if ( it == m_items.end() )
            return 0;
        ++it;
        if ( it != m_items.end() )
            return *it;
        return 0;
    }

    template<class T, bool shortCount>
    const T* PLF<T, shortCount>::prev() const
    {
        if ( m_items.size() == 0 )
            return 0;
        if ( it != m_items.begin() )
            --it;
        return *it;
    }


    template<class T> class PLCF;
    template<class T> class PLCFIterator;
    template<typename OldT, typename NewT> PLCF<NewT>* convertPLCF( const PLCF<OldT>& old );  // evil, eh? :-)

    template<class T> class PLCF
    {
        friend class PLCFIterator<T>;
        template<typename OldT, typename NewT> friend PLCF<NewT>* convertPLCF( const PLCF<OldT>& old );
    public:
		PLCF( U32 length, AbstractOLEStreamReader *reader, bool preservePos = false );
        PLCF( U32 length, const U8* ptr );
        PLCF( const PLCF<T>& rhs );
        ~PLCF();

        PLCFIterator<T> at( unsigned int index ) const;
        size_t count() const { return m_items.size(); }
        bool isEmpty() const { return m_items.empty(); }

        // Inserts the given index/item pair at the end of the PLCF (but before(!) the final n+1 index)
        // Normally you won't need that method. The ownership of the item is transferred.
        void insert( U32 index, T* item );

        void dumpCPs() const;
    private:
        // don't assign it
        PLCF<T>& operator=( const PLCF<T>& rhs );

        // An empty default constructor for the convertPLCF friend. Don't use it
        // unless you know what you are doing :-)
        PLCF() {}

        U32 calculateCount( U32 length );

        std::vector<U32> m_indices;
        std::vector<T*> m_items;
    };

    template<class T>
	PLCF<T>::PLCF( U32 length, AbstractOLEStreamReader* reader, bool preservePos )
    {
        if ( preservePos )
            reader->push();
        U32 count = calculateCount( length );
        for ( U32 i = 0; i < count + 1; ++i )  // n+1 CPs/FCs
            m_indices.push_back( reader->readU32() );
        for ( U32 i = 0; i < count; ++i )  // n "T"s
            m_items.push_back( new T( reader, false ) );
        if ( preservePos )
            reader->pop();
    }

    template<class T>
    PLCF<T>::PLCF( U32 length, const U8* ptr )
    {
        U32 count = calculateCount( length );
        for ( U32 i = 0; i < count + 1; ++i, ptr += 4 )  // n+1 CPs/FCs
            m_indices.push_back( readU32( ptr ) );
        for ( U32 i = 0; i < count; ++i, ptr += T::sizeOf )  // n "T"s
            m_items.push_back( new T( ptr ) );
    }

    template<class T>
    PLCF<T>::PLCF( const PLCF<T>& rhs ) : m_indices( rhs.m_indices )
    {
        typename std::vector<T*>::const_iterator it = rhs.m_items.begin();
        typename std::vector<T*>::const_iterator end = rhs.m_items.end();
        for ( ; it != end; ++it )
            m_items.push_back( new T( **it ) );
    }

    template<class T>
    PLCF<T>::~PLCF()
    {
        typename std::vector<T*>::const_iterator it = m_items.begin();
        for ( ; it != m_items.end(); ++it )
            delete *it;
    }

    template<class T>
    PLCFIterator<T> PLCF<T>::at( unsigned int index ) const
    {
        PLCFIterator<T> it( *this );
        it.m_itemIt += index;
        it.m_indexIt += index;
        return it;
    }

    template<class T>
    void PLCF<T>::insert( U32 index, T* item )
    {
        if ( m_indices.empty() ) {
            delete item;
            return;
        }
        std::vector<U32>::iterator it( m_indices.end() );
        --it;
        m_indices.insert( it, index );
        m_items.push_back( item );
    }

    template<class T>
    void PLCF<T>::dumpCPs() const
    {
        wvlog << "PLCF: count=" << count() << std::endl;
        std::vector<U32>::const_iterator it = m_indices.begin();
        std::vector<U32>::const_iterator end = m_indices.end();
        for ( ; it != end; ++it )
            wvlog << "    " << ( *it ) << std::endl;
        wvlog << "PLCF done." << std::endl;
    }

    template<class T>
    U32 PLCF<T>::calculateCount( U32 length )
    {
        if ( ( length - 4 ) % ( T::sizeOf + 4 ) ) {
            wvlog << "Warning: PLCF size seems to be screwed" << std::endl;
            wvlog << "Warning: length: " << length << ", size: " << T::sizeOf << ", mod: " << ( length - 4 ) % ( T::sizeOf + 4 ) << std::endl;
            return 0;
        }
        return ( length - 4 ) / ( T::sizeOf + 4 );
    }


    // A method to "upgrade" the type of a PLCF from Word 6/7 to Word 8
    // data structures, using the generated conversion code. Tricky :-)
    // The ownership of the new PLCF is transferred to you!
    template<typename OldT, typename NewT> PLCF<NewT>* convertPLCF( const PLCF<OldT>& old )
    {
        PLCF<NewT>* ret( new PLCF<NewT> );
        ret->m_indices = old.m_indices;  // the indices remain the same

        typename std::vector<OldT*>::const_iterator oldIt( old.m_items.begin() );
        typename std::vector<OldT*>::const_iterator oldEnd( old.m_items.end() );
        for ( ; oldIt != oldEnd; ++oldIt )
            ret->m_items.push_back( new NewT( Word95::toWord97( **oldIt ) ) );
        return ret;
    }


    template<class T> class PLCFIterator
    {
        friend PLCFIterator<T> PLCF<T>::at( unsigned int ) const;
    public:
        PLCFIterator( const PLCF<T>& plcf ) : m_plcf( plcf )
        {
            m_itemIt = m_plcf.m_items.begin();
            m_indexIt = m_plcf.m_indices.begin();
        }

        unsigned int count() const { return m_plcf.m_items.count(); }
        bool isEmpty() const { return m_plcf.m_items.count() == 0; }

        T* toFirst();
        T* toLast();

        U32 currentStart() const { if ( m_itemIt != m_plcf.m_items.end() ) return *m_indexIt; return 0; }
        U32 currentLim() const;
        T* current() const { if ( m_itemIt != m_plcf.m_items.end() ) return *m_itemIt; return 0; }

        U32 currentRun() const { return currentLim() - currentStart(); }

        PLCFIterator& operator++();
        PLCFIterator& operator--();

    private:
        // don't assign it
        PLCFIterator<T>& operator=( const PLCFIterator<T>& rhs );

        const PLCF<T>& m_plcf;
        typename std::vector<T*>::const_iterator m_itemIt;
        std::vector<U32>::const_iterator m_indexIt;
    };

    template<class T>
    T* PLCFIterator<T>::toFirst()
    {
        m_itemIt = m_plcf.m_items.begin();
        m_indexIt = m_plcf.m_indices.begin();
        if ( m_itemIt != m_plcf.m_items.end() )
            return *m_itemIt;
        return 0;
    }

    // Note: m_indexIt-=2 as we have n+1 indices!
    template<class T>
    T* PLCFIterator<T>::toLast()
    {
        m_itemIt = m_plcf.m_items.end();
        m_indexIt = m_plcf.m_indices.end();
        if ( m_itemIt == m_plcf.m_items.begin() )
            return 0;
        --m_itemIt;
        m_indexIt -= 2;
        return *m_itemIt;
    }

    template<class T>
    U32 PLCFIterator<T>::currentLim() const
    {
        std::vector<U32>::const_iterator it = m_indexIt;
        if ( m_itemIt == m_plcf.m_items.end() )
            return 0;
        ++it;
        return *it;
    }

    template<class T>
    PLCFIterator<T>& PLCFIterator<T>::operator++()
    {
        if ( m_itemIt == m_plcf.m_items.end() )
            return *this;
        ++m_itemIt;
        ++m_indexIt;
        return *this;
    }

    template<class T>
    PLCFIterator<T>& PLCFIterator<T>::operator--()
    {
        if ( m_plcf.m_items.size() != 0 && m_itemIt != m_plcf.m_items.begin() ) {
            --m_itemIt;
            --m_indexIt;
        }
        return *this;
    }


    template<class PHE> struct BX;
    template<class Offset> class FKP;
    template<class Offset> class FKPIterator;
    FKP< BX<Word97::PHE> >* convertFKP( const FKP< BX<Word95::PHE> >& old );

    template<class Offset> class FKP
    {
        friend class FKPIterator<Offset>;
        friend FKP< BX<Word97::PHE> >* convertFKP( const FKP< BX<Word95::PHE> >& old );
    public:
		FKP( AbstractOLEStreamReader* reader, bool preservePos = false );
        FKP( const U8* ptr );
        FKP( const FKP<Offset>& rhs );
        ~FKP() { delete [] m_rgfc; delete [] m_rgb; delete [] m_fkp; }

        unsigned int crun() const { return m_crun; }
        bool isEmpty() const { return m_crun == 0; }

    private:
        // don't assign it
        FKP<Offset>& operator=( const FKP<Offset>& rhs );

        // An empty default constructor for the convertFKP friend. Don't use it
        // unless you know what you are doing :-)
        FKP() {}

        U8 m_crun;
        U32* m_rgfc;  // array of FCs (crun+1)
        Offset* m_rgb;  // array of offsets/BXs
        U16 m_internalOffset;  // offset to the start position of the "rest"
        U8* m_fkp;  // the "rest" of the FKP
    };

    template<class Offset>
	FKP<Offset>::FKP( AbstractOLEStreamReader* reader, bool preservePos )
    {
        if ( preservePos )
            reader->push();
        reader->push();
        reader->seek( 511, SEEK_CUR );
        m_crun = reader->readU8();
        reader->pop();

        m_rgfc = new U32[ m_crun + 1 ];
        for ( U8 i = 0; i <= m_crun; ++i )  // <= crun, because of crun+1 FCs!
            m_rgfc[ i ] = reader->readU32();

        m_rgb = new Offset[ m_crun ];
        for ( U8 i = 0; i < m_crun; ++i )
            m_rgb[ i ].read( reader, false );

        m_internalOffset = ( static_cast<U16>( m_crun ) + 1 ) * 4 + static_cast<U16>( m_crun ) * Offset::sizeOf;

        // store the rest of the FKP in an internal array for later use
        const U16 length = 511 - m_internalOffset;
        m_fkp = new U8[ length ];  // 511, because we don't need crun
        for ( U16 i = 0; i < length; ++i )
            m_fkp[ i ] = reader->readU8();

        if ( preservePos )
            reader->pop();
    }

    template<class Offset>
    FKP<Offset>::FKP( const U8* ptr )
    {
        m_crun = ptr[ 511 ];

        m_rgfc = new U32[ m_crun + 1 ];
        for ( U8 i = 0; i <= m_crun; ++i, ptr += 4 )  // <= crun, because of crun+1 FCs!
            m_rgfc[ i ] = readU32( ptr );

        m_rgb = new Offset[ m_crun ];
        for ( U8 i = 0; i < m_crun; ++i, ptr += Offset::sizeOf )
            m_rgb[ i ].readPtr( ptr );

        m_internalOffset = ( static_cast<U16>( m_crun ) + 1 ) * 4 + static_cast<U16>( m_crun ) * Offset::sizeOf;

        // store the rest of the FKP in an internal array for later use
        U16 length = 511 - m_internalOffset;
        m_fkp = new U8[ length ];  // 511, because we don't need crun
        for ( U16 i = 0; i < length; ++i, ++ptr )
            m_fkp[ i ] = *ptr;
    }

    template<class Offset>
    FKP<Offset>::FKP( const FKP<Offset>& rhs ) :
        m_crun( rhs.m_crun ), m_internalOffset( rhs.m_internalOffset )
    {
        m_rgfc = new U32[ m_crun + 1 ];
        ::memcpy( m_rgfc, rhs.m_rgfc, sizeof( U32 ) * ( m_crun + 1 ) );

        m_rgb = new Offset[ m_crun ];
        for ( U8 i = 0; i < m_crun; ++i )
            m_rgb[ i ] = rhs.m_rgb[ i ];

        const U16 length = 511 - m_internalOffset;
        m_fkp = new U8[ length ];  // 511, because we don't need crun
        ::memcpy( m_fkp, rhs.m_fkp, sizeof( U8 ) * length );
    }


    // Attention: This iterator has a non standard behavior of the current()
    // method. Use it that way: for( ; !it.atEnd(); ++it)
    template<class Offset> class FKPIterator
    {
    public:
        FKPIterator( const FKP<Offset>& fkp ) : m_fkp( fkp ), m_index( 0 ) {}

        void toFirst() { m_index = 0; }
        void toLast() { m_index = m_fkp.m_crun - 1; }

        U32 currentStart() const { if ( m_index < m_fkp.m_crun ) return m_fkp.m_rgfc[ m_index ]; return 0; }
        U32 currentLim() const { if ( m_index < m_fkp.m_crun ) return m_fkp.m_rgfc[ m_index + 1 ]; return 0; }
        Offset currentOffset() const { if ( m_index < m_fkp.m_crun ) return m_fkp.m_rgb[ m_index ]; return Offset(); }

        // Pointer to the start of the current CHPX/PAPX/..., 0 if we are at the end of the array
        // Attention: This iterator has a non standard behavior of the current()
        // method. Use it that way: for( ; !it.atEnd(); ++it)
        const U8* current() const;

        FKPIterator& operator++() { if ( m_index < m_fkp.m_crun ) ++m_index; return *this; }
        FKPIterator& operator--() { if ( m_index > 0 ) --m_index; return *this; }

        U8 index() const { return m_index; }
        void setIndex( U8 index ) { if ( index < m_fkp.m_crun ) m_index = index; }

        bool atEnd() { return m_index >= m_fkp.m_crun; }

    private:
        // don't copy or assign it
        FKPIterator( const FKPIterator<Offset>& rhs );
        FKPIterator<Offset>& operator=( const FKPIterator<Offset>& rhs );

        const FKP<Offset>& m_fkp;
        U8 m_index;
    };

    template<class Offset>
    const U8* FKPIterator<Offset>::current() const
    {
        if ( m_index < m_fkp.m_crun ) {
            // Note: The first byte of the "offset" types (BX or U8) is always
            // the word offset into the array (or 0!).
            const U8 tmp = m_fkp.m_rgb[ m_index ].offset;
            // Now we have to calculate the real offset and then locate it
            // within our cached array...
            if ( tmp != 0 ) {
                const int pos = tmp * 2 - m_fkp.m_internalOffset;
                if ( pos < 0 ) {
                    wvlog << "ERROR: FKP internalOffset (" << m_fkp.m_internalOffset << ") is bigger than " <<
                        "2*" << (int)tmp << ", FKP array index would be negative!" << std::endl;
                    return 0;
                } else if ( pos >= 511 - m_fkp.m_internalOffset ) {
                    wvlog << "ERROR: FKP array index (" << pos << " is bigger than allocated size ("
                          << 511 - m_fkp.m_internalOffset << ")" << std::endl;
                    return 0;
                } else {
                    return &m_fkp.m_fkp[ pos ];
                }
            }
        }
        return 0;
    }


    /**
     * BX entry in a PAP FKP (1 byte word offset + 1 PHE). This is a template, as Word 95
     * PHEs are shorter than Word 97 ones.
     */
    template<typename PHE> struct BX
    {
        /**
         * Creates an empty BX structure and sets the defaults
         */
        BX() { clear(); }
        /**
         * Simply calls read(...)
         */
		BX( AbstractOLEStreamReader* stream, bool preservePos = false )
        {
            clear();
            read( stream, preservePos );
        }
        /**
         * Simply calls readPtr(...)
         */
        BX( const U8* ptr )
        {
            clear();
            readPtr( ptr );
        }

        /**
         * This method reads the BX structure from the stream.
         * If  preservePos is true we push/pop the position of
         * the stream to save the state. If it's false the state
         * of stream will be changed!
         */
		bool read( AbstractOLEStreamReader* stream, bool preservePos = false )
        {
            if ( preservePos )
                stream->push();
            offset = stream->readU8();
            phe.read( stream, false );
            if ( preservePos )
                stream->pop();
            return true;
        }
        /**
         * This method reads the struct from a pointer
         */
        void readPtr( const U8* ptr )
        {
            offset = *ptr;
            ++ptr;
            phe.readPtr( ptr );
		}

        /**
         * Set all the fields to the inital value (default is 0)
         */
        void clear()
        {
            offset = 0;
            phe.clear();
        }

        // Size of the structure
        static const unsigned int sizeOf;

        // Data
        /**
         * One byte offset. This byte is a word offset, so that we can
         * address 512 bytes with one unsigned char and all the CHPX/PAPX
         * grpprls are aligned at 2 byte boundaries.
         */
        U8 offset;

        /**
         * The PHE for the paragraph described by this BX structure.
         */
        PHE phe;

    };

    template<typename PHE> const unsigned int BX<PHE>::sizeOf = 1 + PHE::sizeOf;

    template<typename PHE> bool operator==( const BX<PHE>& lhs, const BX<PHE>& rhs )
    {
        return lhs.offset == rhs.offset && lhs.phe == rhs.phe;
    }

    template<typename PHE> bool operator!=( const BX<PHE>& lhs, const BX<PHE>& rhs )
    {
        return !( lhs == rhs );
    }


    /**
     * A fake "BX" entry in a CHP FKP (1 byte word offset only).
     */
    struct CHPFKP_BX
    {
        /**
         * Creates an empty CHPFKP_BX structure and sets the defaults
         */
        CHPFKP_BX();
        /**
         * Simply calls read(...)
         */
		CHPFKP_BX( AbstractOLEStreamReader* stream, bool preservePos = false );
        /**
         * Simply calls readPtr(...)
         */
        CHPFKP_BX( const U8* ptr );

        /**
         * This method reads the CHPFKP_BX structure from the stream.
         * If  preservePos is true we push/pop the position of
         * the stream to save the state. If it's false the state
         * of stream will be changed!
         */
		bool read( AbstractOLEStreamReader* stream, bool preservePos = false );
        /**
         * This method reads the struct from a pointer
         */
        void readPtr( const U8* ptr );

        /**
         * Set all the fields to the inital value (default is 0)
         */
        void clear();

        // Size of the structure
        static const unsigned int sizeOf;

        // Data
        /**
         * One byte offset. This byte is a word offset, so that we can
         * address 512 bytes with one unsigned char and all the CHPX/PAPX
         * grpprls are aligned at 2 byte boundaries.
         */
        U8 offset;
    };

    bool operator==( const CHPFKP_BX& lhs, const CHPFKP_BX& rhs );
    bool operator!=( const CHPFKP_BX& lhs, const CHPFKP_BX& rhs );


    // This enum is a "convenience enum" for reading the piece table
    typedef enum {
        clxtGrpprl = 1,
        clxtPlcfpcd = 2
    } clxtENUM;

} // namespace wvWare

#endif // WORD_HELPER_H
