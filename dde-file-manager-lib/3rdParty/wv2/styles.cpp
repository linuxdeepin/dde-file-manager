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

#include "styles.h"
#include "olestream.h"
#include "word97_helper.h"
#include "convert.h"
#include "paragraphproperties.h"

#include "wvlog.h"
#include <algorithm>
#include <string.h>

namespace wvWare
{

namespace Word97
{

// STD implementation
STD::STD()
{
    clearInternal();
}

STD::STD( U16 baseSize, U16 totalSize, AbstractOLEStreamReader* stream, bool preservePos )
{
    clearInternal();
    read( baseSize, totalSize, stream, preservePos );
}

STD::STD( const STD& rhs ) : xstzName( rhs.xstzName )
{
    sti = rhs.sti;
    fScratch = rhs.fScratch;
    fInvalHeight = rhs.fInvalHeight;
    fHasUpe = rhs.fHasUpe;
    fMassCopy = rhs.fMassCopy;
    sgc = rhs.sgc;
    istdBase = rhs.istdBase;
    cupx = rhs.cupx;
    istdNext = rhs.istdNext;
    bchUpe = rhs.bchUpe;
    fAutoRedef = rhs.fAutoRedef;
    fHidden = rhs.fHidden;
    unused8_3 = rhs.unused8_3;
    grupxLen = rhs.grupxLen;

    // ...and the UPXes
    grupx = new U8[ grupxLen ];
    memcpy( grupx, rhs.grupx, grupxLen );
}

STD::~STD()
{
    delete [] grupx;
}

STD& STD::operator=( const STD& rhs )
{
    // Check for assignment to self
    if ( this == &rhs )
        return *this;

    sti = rhs.sti;
    fScratch = rhs.fScratch;
    fInvalHeight = rhs.fInvalHeight;
    fHasUpe = rhs.fHasUpe;
    fMassCopy = rhs.fMassCopy;
    sgc = rhs.sgc;
    istdBase = rhs.istdBase;
    cupx = rhs.cupx;
    istdNext = rhs.istdNext;
    bchUpe = rhs.bchUpe;
    fAutoRedef = rhs.fAutoRedef;
    fHidden = rhs.fHidden;
    unused8_3 = rhs.unused8_3;
    grupxLen = rhs.grupxLen;

    // assign the name
    xstzName = rhs.xstzName;

    // ...and the UPXes
    delete [] grupx;
    grupx = new U8[ grupxLen ];
    memcpy( grupx, rhs.grupx, grupxLen );

    return *this;
}

bool STD::read( U16 baseSize, U16 totalSize, AbstractOLEStreamReader* stream, bool preservePos )
{
    U16 shifterU16;
    S32 startOffset=stream->tell();  // address where the STD starts

    if (preservePos)
        stream->push();

    shifterU16 = stream->readU16();
    sti = shifterU16;
    shifterU16 >>= 12;
    fScratch = shifterU16;
    shifterU16 >>= 1;
    fInvalHeight = shifterU16;
    shifterU16 >>= 1;
    fHasUpe = shifterU16;
    shifterU16 >>= 1;
    fMassCopy = shifterU16;
    shifterU16 = stream->readU16();
    sgc = shifterU16;
#ifdef WV2_DEBUG_STYLESHEET
    wvlog << "##### sgc: " << static_cast<int>( sgc ) << std::endl;
#endif
    shifterU16 >>= 4;
    istdBase = shifterU16;
#ifdef WV2_DEBUG_STYLESHEET
    wvlog << "     istdBase: " << istdBase << std::endl;
#endif
    shifterU16 = stream->readU16();
    cupx = shifterU16;
    shifterU16 >>= 4;
    istdNext = shifterU16;
    bchUpe = stream->readU16();

    // Skip the end of the Word97::STD in older documents with baseSize <= 8
    if ( baseSize > 8 ) {
        shifterU16 = stream->readU16();
        fAutoRedef = shifterU16;
        shifterU16 >>= 1;
        fHidden = shifterU16;
        shifterU16 >>= 1;
        unused8_3 = shifterU16;
    }

    // read the name of the style.
    // Note: Starts at an even address within the STD after the
    // stshi.cbSTDBaseInFile part.
#ifdef WV2_DEBUG_STYLESHEET
    wvlog << "baseSize: " << baseSize << std::endl;
    wvlog << "start offset: " << startOffset << std::endl;
    wvlog << "curr. position: " << stream->tell() << std::endl;
#endif
    baseSize += ( baseSize & 0x0001 ) ? 1 : 0;  // next even address
    stream->seek( startOffset + baseSize, SEEK_SET );
#ifdef WV2_DEBUG_STYLESHEET
    wvlog << "new position: " << stream->tell() << std::endl;
#endif

    readStyleName( baseSize, stream );

    // even byte address within the STD?
    if ( ( stream->tell() - startOffset ) & 1 ) {
#ifdef WV2_DEBUG_STYLESHEET
        wvlog << "Adjusting the position... from " << stream->tell() - startOffset;
#endif
        stream->seek( 1, SEEK_CUR );
#ifdef WV2_DEBUG_STYLESHEET
        wvlog << " to " << stream->tell() - startOffset << std::endl;
#endif
    }

#ifdef WV2_DEBUG_STYLESHEET
    wvlog << "cupx: " << static_cast<int>( cupx ) << std::endl;
    wvlog << "size: " << totalSize - ( stream->tell() - startOffset ) << std::endl;
#endif
    grupxLen = totalSize - ( stream->tell() - startOffset );
    grupx = new U8[ grupxLen ];
    int offset = 0;
    for ( U8 i = 0; i < cupx; ++i) {
        U16 cbUPX = stream->readU16();  // size of the next UPX
        stream->seek( -2, SEEK_CUR ); // rewind the "lookahead"
        cbUPX += 2;                     // ...and correct the size
#ifdef WV2_DEBUG_STYLESHEET
        wvlog << "cbUPX: " << cbUPX << std::endl;
#endif
        for ( U16 j = 0; j < cbUPX; ++j ) {
            grupx[ offset + j ] = stream->readU8();  // read the whole UPX
#ifdef WV2_DEBUG_STYLESHEET
            wvlog << "  read: " << static_cast<int>( grupx[ offset + j ] ) << std::endl;
#endif
        }
        offset += cbUPX;  // adjust the offset in the grupx array
        // Don't forget to adjust the position
        if ( ( stream->tell() - startOffset ) & 1 ) {
#ifdef WV2_DEBUG_STYLESHEET
            wvlog << "Adjusting the UPX position... from " << stream->tell() - startOffset;
#endif
            stream->seek( 1, SEEK_CUR );
#ifdef WV2_DEBUG_STYLESHEET
            wvlog << " to " << stream->tell() - startOffset << std::endl;
#endif
        }
    }

    if ( preservePos )
        stream->pop();
    return true;
}

void STD::clear()
{
    delete [] grupx;
    clearInternal();
}

void STD::clearInternal()
{
    sti = 0; fScratch = 0; fInvalHeight = 0;
    fHasUpe = 0; fMassCopy = 0; sgc = 0;
    istdBase = 0; cupx = 0; istdNext = 0;
    bchUpe = 0; fAutoRedef = 0; fHidden = 0;
    unused8_3 = 0; grupx = 0; grupxLen = 0;
}

void STD::readStyleName( U16 baseSize, AbstractOLEStreamReader* stream )
{
    // Read the length of the string. It seems that the spec is
    // buggy and the "length byte" is actually a short in Word97+
    if ( baseSize > 8 ) {
        U16 length = stream->readU16();
#ifdef WV2_DEBUG_STYLESHEET
        wvlog << "len: " << length << std::endl;
#endif
        // question: Is the \0 included in the length spec?
        XCHAR *name = new XCHAR[ length + 1 ];
        for ( U16 i = 0; i < length + 1; ++i ) {
            name[ i ] = stream->readU16();
#ifdef WV2_DEBUG_STYLESHEET
            wvlog << "xstzName[" << static_cast<int>( i ) << "]: " << name[i] << std::endl;
#endif
        }
        if ( name[ length ] != 0 )
            wvlog << "Warning: Illegal trailing character: " << static_cast<int>( name[ length ] ) << std::endl;

        xstzName = UString( reinterpret_cast<const wvWare::UChar *>( name ), length );
        delete [] name;
    }
    else {
        // Word versions older than Word97 have a plain lenght byte and
        // a char* string as name
        U8 length = stream->readU8();
#ifdef WV2_DEBUG_STYLESHEET
        wvlog << "len: " << static_cast<int>( length ) << std::endl;
#endif
        // question: Is the \0 included in the length spec?
        U8 *name = new U8[ length + 1 ];
        stream->read( name, length + 1 );
#ifdef WV2_DEBUG_STYLESHEET
        for ( U16 i = 0; i < length + 1; ++i )
            wvlog << "xstzName[" << static_cast<int>( i ) << "]: " << static_cast<int>( name[i] ) << std::endl;
#endif
        if ( name[ length ] != 0 ) {
            wvlog << "Warning: Illegal trailing character: " << static_cast<int>( name[ length ] ) << std::endl;
            name[ length ] = 0;
        }

        xstzName = UString( reinterpret_cast<const char *>( name ) );
        delete [] name;
    }
#ifdef WV2_DEBUG_STYLESHEET
    wvlog << "ASCII Name: '" << xstzName.ascii() << "'" << std::endl;
#endif
}

bool operator==( const STD& lhs, const STD& rhs )
{
    if ( lhs.grupxLen != rhs.grupxLen )
        return false;
    for ( U8 i = 0; i < lhs.grupxLen; ++i ) {
        if ( lhs.grupx[ i ] != rhs.grupx[ i ] )
            return false;
    }

    return lhs.xstzName == rhs.xstzName &&
           lhs.sti == rhs.sti &&
           lhs.fScratch == rhs.fScratch &&
           lhs.fInvalHeight == rhs.fInvalHeight &&
           lhs.fHasUpe == rhs.fHasUpe &&
           lhs.fMassCopy == rhs.fMassCopy &&
           lhs.sgc == rhs.sgc &&
           lhs.istdBase == rhs.istdBase &&
           lhs.cupx == rhs.cupx &&
           lhs.istdNext == rhs.istdNext &&
           lhs.bchUpe == rhs.bchUpe &&
           lhs.fAutoRedef == rhs.fAutoRedef &&
           lhs.fHidden == rhs.fHidden &&
           lhs.unused8_3 == rhs.unused8_3;
}

bool operator!=( const STD& lhs, const STD& rhs )
{
    return !( lhs == rhs );
}

} // namespace Word97
} // namespace wvWare


using namespace wvWare;

Style::Style( U16 baseSize, AbstractOLEStreamReader* tableStream, U16* ftc ) : m_isEmpty( false ),
    m_isWrapped( true ), m_std( 0 ), m_properties( 0 ), m_chp( 0 ), m_upechpx( 0 )
{
    // size of the STD
    U16 cb = tableStream->readU16();
    if ( cb == 0 ) {  // empty slot
#ifdef WV2_DEBUG_STYLESHEET
        wvlog << "Empty style found: " << tableStream->tell() << std::endl;
#endif
        m_isEmpty = true;
        m_isWrapped = false;
        return;
    }
    S32 offset = tableStream->tell();
    m_std = new Word97::STD( baseSize, cb, tableStream, false );
    if ( tableStream->tell() != offset + cb ) {
        wvlog << "Warning: Found a \"hole\"" << std::endl;
        tableStream->seek( cb, SEEK_CUR );  // correct the offset
    }

    if ( m_std->sgc == sgcPara ) {
        m_chp = new Word97::CHP();
        m_properties = new ParagraphProperties();
        m_chp->ftc = *ftc;         // Same value for ftc and ftcAscii
        m_chp->ftcAscii = *ftc++;
        m_chp->ftcFE = *ftc++;
        m_chp->ftcOther = *ftc;
    }
    else if ( m_std->sgc == sgcChp )
        m_upechpx = new UPECHPX();
    else
        wvlog << "Attention: New kind of style in the stylesheet" << std::endl;
}

Style::~Style()
{
    delete m_std;
    delete m_properties;
    delete m_chp;
    delete m_upechpx;
}

void Style::unwrapStyle( const StyleSheet& stylesheet, WordVersion version )
{
    if ( !m_isWrapped || !m_std )
        return;

    if ( m_std->sgc == sgcPara ) {
        const Style* parentStyle = 0;
        // only try to unwrap the "parent" if the style isn't the Nil style
        if ( m_std->istdBase != 0x0fff ) {
            parentStyle = stylesheet.styleByIndex( m_std->istdBase );
            if ( parentStyle ) {
                const_cast<Style*>( parentStyle )->unwrapStyle( stylesheet, version );
                m_properties->pap() = parentStyle->paragraphProperties().pap();
                *m_chp = parentStyle->chp();
            }
        }

        U8 *data = m_std->grupx;

        // paragraph
        U16 cbUPX = readU16( data );
        data += 2;
        m_properties->pap().istd = readU16( data );
        data += 2;
        cbUPX -= 2;
#ifdef WV2_DEBUG_SPRMS
        wvlog << "############# Applying paragraph exceptions: " << cbUPX << std::endl;
#endif
        m_properties->pap().apply( data, cbUPX, parentStyle, 0, version );  // try without data stream for now
        data += cbUPX;
#ifdef WV2_DEBUG_SPRMS
        wvlog << "############# done" << std::endl;
#endif

        // character
        cbUPX = readU16( data );
        data += 2;
#ifdef WV2_DEBUG_SPRMS
        wvlog << "############# Applying character exceptions: " << cbUPX << std::endl;
#endif
        m_chp->apply( data, cbUPX, parentStyle, 0, version );  // try without data stream for now
#ifdef WV2_DEBUG_SPRMS
        wvlog << "############# done" << std::endl;
#endif
    }
    else if ( m_std->sgc == sgcChp ) {
        const Style* parentStyle = 0;
        // only try to unwrap the "parent" if the style isn't the Nil style
        if ( m_std->istdBase != 0x0fff ) {
            parentStyle = stylesheet.styleByIndex( m_std->istdBase );
            if ( parentStyle ) {
                const_cast<Style*>( parentStyle )->unwrapStyle( stylesheet, version );
                bool ok;
                m_upechpx->istd = stylesheet.indexByID( m_std->sti, ok );
                mergeUpechpx( parentStyle, version );
            }
            else
                wvlog << "################# NO parent style for this character style found" << std::endl;
        }
    }
    else
        wvlog << "Warning: Unknown style type code detected" << std::endl;
    m_isWrapped = false;
}

U16 Style::sti() const
{
    if ( m_std )
        return static_cast<U16>( m_std->sti );
    return 0x0fff;   // stiNil
}

UString Style::name() const
{
    if ( m_std )
        return m_std->xstzName;
    return UString::null;
}

U16 Style::followingStyle() const
{
    return m_std ? m_std->istdNext : 0x0fff;
}

Style::StyleType Style::type() const
{
    if ( m_std ) {
        if ( m_std->sgc == sgcPara )
            return sgcPara;
        else if ( m_std->sgc == sgcChp )
            return sgcChp;
    }
    return sgcUnknown;
}

const ParagraphProperties& Style::paragraphProperties() const
{
    if ( !m_properties ) {
        wvlog << "You requested the PAP of a character style? Hmm..." << std::endl;
        m_properties = new ParagraphProperties(); // let's return a default PAP, better than crashing
    }
    return *m_properties;
}

const Word97::CHP& Style::chp() const
{
    if ( !m_chp ) {
        wvlog << "You requested the CHP of a character style? Hmm..." << std::endl;
        m_chp = new Word97::CHP(); // let's return a default CHP, better than crashing
    }
    return *m_chp;
}

const UPECHPX& Style::upechpx() const
{
    if ( !m_upechpx ) {
        wvlog << "You requested the CHPX of a paragraph style? Hmm..." << std::endl;
        m_upechpx = new UPECHPX(); // let's return a default UPECHPX, better than crashing
    }
    return *m_upechpx;
}

// Some code to assist in merging CHPXes
namespace {
    struct SprmEntry
    {
        SprmEntry( U16 sp, U16 offs ) : sprm( sp ), offset( offs ) {}
        U16 sprm;
        U16 offset;
    };

    bool operator<( const SprmEntry& rhs, const SprmEntry& lhs )
    {
        return rhs.sprm < lhs.sprm;
    }

    bool operator==( const SprmEntry& rhs, const SprmEntry& lhs )
    {
        return rhs.sprm == lhs.sprm;
    }

    void analyzeGrpprl( U8* grpprl, U16 count, std::vector<SprmEntry>& entries, WordVersion version )
    {
        U16 offset = 0;
        while ( offset < count ) {
            U16 sprm;
            if ( version == Word8 ) {
                sprm = readU16( grpprl );
#ifdef WV2_DEBUG_SPRMS
                wvlog << "####### offset: " << offset << " sprm: 0x" << std::hex << sprm << std::dec << std::endl;
#endif
                grpprl += sizeof( U16 );
            }
            else {
                sprm = *grpprl++;
#ifdef WV2_DEBUG_SPRMS
                wvlog << "####### offset: " << offset << " sprm (Word6/7): " << sprm << std::endl;
#endif
            }
            entries.push_back( SprmEntry( sprm, offset ) );
            const U16 len = wvWare::Word97::SPRM::determineParameterLength( sprm, grpprl, version );
            grpprl += len;
            offset += len + ( version == Word8 ? 2 : 1 );
        }
    }

    U16 copySprm( U8* destGrpprl, U8* srcGrpprl, const SprmEntry& entry, WordVersion version )
    {
        srcGrpprl += entry.offset;

        U16 sprm;
        if ( version == Word8 ) {
            sprm = readU16( srcGrpprl );
#ifdef WV2_DEBUG_SPRMS
            wvlog << "####### Copying sprm 0x" << std::hex << sprm << std::dec << std::endl;
#endif
            srcGrpprl += sizeof( U16 );
        }
        else {
            sprm = *srcGrpprl++;
#ifdef WV2_DEBUG_SPRMS
            wvlog << "####### Copying sprm (Word6/7) " << sprm << std::endl;
#endif
        }

        if ( sprm != entry.sprm )
            wvlog << "Ooops, we messed up the CHPX merging!" << std::endl;
        const U16 len = wvWare::Word97::SPRM::determineParameterLength( sprm, srcGrpprl, version ) + ( version == Word8 ? 2 : 1 );
        srcGrpprl -= version == Word8 ? 2 : 1; // back to the start
        memcpy( destGrpprl, srcGrpprl, len );
        return len;
    }
}

void Style::mergeUpechpx( const Style* parentStyle, WordVersion version )
{
    // Analyze the source and the base grpprls
    U8* srcGrpprl = m_std->grupx;
    U16 cbUPX = readU16( srcGrpprl );
    srcGrpprl += 2;
    std::vector<SprmEntry> source;
    analyzeGrpprl( srcGrpprl, cbUPX, source, version );

    U8* baseGrpprl = parentStyle->upechpx().grpprl;
    std::vector<SprmEntry> base;
    analyzeGrpprl( baseGrpprl, parentStyle->upechpx().cb, base, version );

    // Sort the created vectors
    std::sort( source.begin(), source.end() );
    std::sort( base.begin(), base.end() );

    // Get enough room for the sprms of both chpxes
    m_upechpx->grpprl = new U8[ parentStyle->upechpx().cb + cbUPX ];
    U16 destCount = 0;

    std::vector<SprmEntry>::const_iterator sourceIt = source.begin();
    std::vector<SprmEntry>::const_iterator sourceEnd = source.end();
    std::vector<SprmEntry>::const_iterator baseIt = base.begin();
    std::vector<SprmEntry>::const_iterator baseEnd = base.end();
    // First merge till one array is empty.
    while ( sourceIt != sourceEnd && baseIt != baseEnd ) {
        if ( *sourceIt < *baseIt ) {
            destCount += copySprm( &m_upechpx->grpprl[ destCount ], srcGrpprl, *sourceIt, version );
            ++sourceIt;
        }
        else if ( *sourceIt == *baseIt ) {  // prefer the "new" entry
            destCount += copySprm( &m_upechpx->grpprl[ destCount ], srcGrpprl, *sourceIt, version );
            ++sourceIt;
            ++baseIt;
        }
        else {
            destCount += copySprm( &m_upechpx->grpprl[ destCount ], baseGrpprl, *baseIt, version );
            ++baseIt;
        }
    }

    // Then copy the rest of the longer grpprl
    while ( sourceIt != sourceEnd ) {
        destCount += copySprm( &m_upechpx->grpprl[ destCount ], srcGrpprl, *sourceIt, version );
        ++sourceIt;
    }
    while ( baseIt != baseEnd ) {
        destCount += copySprm( &m_upechpx->grpprl[ destCount ], baseGrpprl, *baseIt, version );
        ++baseIt;
    }
    m_upechpx->cb = destCount;
}


StyleSheet::StyleSheet( AbstractOLEStreamReader* tableStream, U32 fcStshf, U32 lcbStshf )
{
    WordVersion version = Word8;

    tableStream->push();
    tableStream->seek( fcStshf, SEEK_SET );

    const U16 cbStshi = tableStream->readU16();

#ifdef WV2_DEBUG_STYLESHEET
    wvlog << "StyleSheet::StyleSheet(): fcStshf=" << fcStshf << " lcbStshf=" << lcbStshf
          << " cbStshi=" << cbStshi << std::endl;
#endif

    // First read the STSHI
    if ( cbStshi == Word95::STSHI::sizeOf ) {
        Word95::STSHI stsh( tableStream, false );
        m_stsh = Word95::toWord97( stsh );
        version = Word67;   // okay, it's Word 6/7 after all
    }
    else if ( cbStshi == Word97::STSHI::sizeOf )
        m_stsh.read( tableStream, false );
    else {
        wvlog << "Detected a different STSHI, check this (trying to read Word97 one)" << std::endl;
        m_stsh.read( tableStream, false );
    }

    if ( tableStream->tell() != static_cast<int>( fcStshf + cbStshi + 2 ) ) {
        wvlog << "Warning: STSHI too big? New version?"
              << " Expected: " << cbStshi + 2 << " Read: " << tableStream->tell() - fcStshf << std::endl;
        tableStream->seek( fcStshf + 2 + cbStshi, SEEK_SET );
    }

    // read all the styles
#ifdef WV2_DEBUG_STYLESHEET
    wvlog << "Reading in " << m_stsh.cstd << " styles." << std::endl;
#endif
    for( U16 i = 0; i < m_stsh.cstd; ++i )
        m_styles.push_back( new Style( m_stsh.cbSTDBaseInFile,
                                       tableStream,
                                       m_stsh.rgftcStandardChpStsh ) );

#ifdef WV2_DEBUG_STYLESHEET
    wvlog << "Done reading the styles: " << tableStream->tell()
          << " expected: " << fcStshf + lcbStshf << std::endl;
#endif
    if ( tableStream->tell() < static_cast<int>( fcStshf + lcbStshf ) )
        wvlog << "Warning: Didn't read all bytes of the stylesheet..." << std::endl;
    else if ( tableStream->tell() > static_cast<int>( fcStshf + lcbStshf ) )
        wvlog << "BUG: Read past the stylesheet area!" << std::endl;

#ifdef WV2_DEBUG_STYLESHEET
    wvlog << "##### Starting to unwrap the styles" << std::endl;
    int i = 0;
#endif
    // "unzip" them and build up the PAPs and CHPs
    for ( std::vector<Style*>::iterator it = m_styles.begin(); it != m_styles.end(); ++it ) {
#ifdef WV2_DEBUG_STYLESHEET
        wvlog << "Unwrapping style: " << i << std::endl;
        ++i;
#endif
        (*it)->unwrapStyle( *this, version );
    }
#ifdef WV2_DEBUG_STYLESHEET
    wvlog << "##### Styles unwrapped" << std::endl;
#endif
    tableStream->pop();
}

StyleSheet::~StyleSheet()
{
    for ( std::vector<Style*>::iterator it = m_styles.begin(); it != m_styles.end(); ++it )
        delete *it;
}

unsigned int StyleSheet::size() const
{
    return m_styles.size();
}

const Style* StyleSheet::styleByIndex( U16 istd ) const
{
    if ( istd < m_styles.size() )
        return m_styles[ istd ];
    return 0;
}

const Style* StyleSheet::styleByID( U16 sti ) const
{
    for ( std::vector<Style*>::const_iterator it = m_styles.begin(); it != m_styles.end(); ++it ) {
        if ( (*it)->sti() == sti )
            return *it;
    }
    return 0;
}

U16 StyleSheet::indexByID( U16 sti, bool& ok ) const
{
    ok = true;
    U16 istd = 0;
    for ( std::vector<Style*>::const_iterator it = m_styles.begin(); it != m_styles.end(); ++it, ++istd ) {
        if ( (*it)->sti() == sti )
            return istd;
    }
    ok = false;
    return 0;
}
