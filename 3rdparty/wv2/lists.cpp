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

#include "lists.h"
#include "olestream.h"
#include "word97_generated.h"
#include "word_helper.h"
#include "utilitiesrename.h"
#include "styles.h"
#include "crc32.h"

#include "wvlog.h"
#include <algorithm>

// Private API
// This might look a bit over-the-top at a first glance, but the reason
// that I put all that stuff in separate classes is that Word 6 has a
// completely different list info implementation, and I somehow have to
// hide that fact.
namespace wvWare
{
    namespace Word97
    {
        struct LSTF;
        struct LVLF;
    }
    namespace
    {
        const int maxListLevels = 9;
        const int oldStyleIlfo = 2047;
        const U16 usLid = 0x409; // The list names in the STTBF are always Unicode, the lid doesn't matter
        const U16 istdNil = 4095; // default style

        // some sprms we need here
        const U16 sprmCFBold = 0x0835;
        const U16 sprmCFItalic = 0x0836;
        const U16 sprmCFSmallCaps = 0x083A;
        const U16 sprmCFCaps = 0x083B;
        const U16 sprmCFStrike = 0x0837;
        const U16 sprmCKul = 0x2A3E;
        const U16 sprmCIco = 0x2A42;
        const U16 sprmCRgFtc0 = 0x4A4F;
        const U16 sprmCHps = 0x4A43;

        unsigned int createFakeLSID( const Word97::ANLD& anld )
        {
            // As our structure contains padding bytes we have to use
            // an array which is guaranteed not to have "holes" for the
            // CRC32 sum...
            unsigned char buffer[102];
            buffer[0] = anld.nfc;
            buffer[1] = anld.cxchTextBefore;
            buffer[2] = anld.cxchTextAfter;
            buffer[3] = anld.jc;
            buffer[4] = anld.fPrev;
            buffer[5] = anld.fHang;
            buffer[6] = anld.fSetBold;
            buffer[7] = anld.fSetItalic;
            buffer[8] = anld.fSetSmallCaps;
            buffer[9] = anld.fSetCaps;
            buffer[10] = anld.fSetStrike;
            buffer[11] = anld.fSetKul;
            buffer[12] = anld.fPrevSpace;
            buffer[13] = anld.fBold;
            buffer[14] = anld.fItalic;
            buffer[15] = anld.fSmallCaps;
            buffer[16] = anld.fCaps;
            buffer[17] = anld.fStrike;
            buffer[18] = anld.kul;
            buffer[19] = anld.ico;
            unsigned short* tmp = reinterpret_cast<unsigned short*>( buffer ) + 10;
            *tmp++ = anld.ftc;
            *tmp++ = anld.hps;
            *tmp++ = anld.iStartAt;
            *tmp++ = anld.dxaIndent;
            *tmp++ = anld.dxaSpace;
            buffer[30] = anld.fNumber1;
            buffer[31] = anld.fNumberAcross;
            buffer[32] = anld.fRestartHdn;
            buffer[33] = anld.fSpareX;
            tmp += 2;
            for ( int i = 0; i < 32; ++i )
                *tmp++ = anld.rgxch[i];
            // 98 bytes up to this point + 4 for the CRC32 sum
            buffer[98] = 0;
            buffer[99] = 0;
            buffer[100] = 0;
            buffer[101] = 0;
            return CalcCRC32( buffer, 102, 98, 4 );
        }
    }

    class ListLevel
    {
    public:
		explicit ListLevel( AbstractOLEStreamReader* tableStream );
        explicit ListLevel( const Word97::ANLD& anld );
        ~ListLevel();

        S32 startAt() const;
        U8 numberFormat() const;
        U8 alignment() const;
        bool isLegal() const;
        bool notRestarted() const;
        bool prev() const;
        bool prevSpace() const;
        bool isWord6() const;  // ###### Do we want to have that?
        UString text() const;
        U8 followingChar() const;

        void applyGrpprlPapx( Word97::PAP* pap ) const;
        void applyGrpprlChpx( Word97::CHP* chp, const Style* style ) const;

    private:
        ListLevel( const ListLevel& rhs );
        ListLevel& operator=( const ListLevel& rhs );

        int writeCharProperty( U16 sprm, U8 value, U8** grpprl );
        int writeCharProperty( U16 sprm, U16 value, U8** grpprl );

        Word97::LVLF m_lvlf;
        U8* m_grpprlPapx;
        U8* m_grpprlChpx;
        UString m_numberText;
    };


    class ListData
    {
    public:
		explicit ListData( AbstractOLEStreamReader* tableStream );
        ListData( S32 lsid, bool fRestartHdn );
        ~ListData();

        S32 lsid() const;
        U16 istdForLevel( U8 level ) const;
        bool isSimpleList() const;
        bool restartingCounter() const;

        // The ownership is transferred to us
        void appendListLevel( ListLevel* listLevel );
        const ListLevel* listLevel( U8 level ) const;

        void applyGrpprlPapx( Word97::PAP* pap ) const;

    private:
        ListData( const ListData& rhs );
        ListData& operator=( const ListData& rhs );

        Word97::LSTF m_lstf;
        std::vector<ListLevel*> m_listLevels;
    };


    class ListFormatOverrideLVL
    {
    public:
		ListFormatOverrideLVL( AbstractOLEStreamReader* tableStream );
        ~ListFormatOverrideLVL();

        S32 startAt() const;
        U8 level() const;

        bool overridesStartAt() const;
        void resetStartAtFlag();
        bool overridesFormat() const;

        const ListLevel* listLevel() const;

        void dump() const;
    private:
        ListFormatOverrideLVL( const ListFormatOverrideLVL& rhs );
        ListFormatOverrideLVL& operator=( const ListFormatOverrideLVL& rhs );

        Word97::LFOLVL m_lfolvl;
        ListLevel* m_level;
    };


    class ListFormatOverride
    {
    public:
		explicit ListFormatOverride( AbstractOLEStreamReader* tableStream );
        explicit ListFormatOverride( S32 lsid );
        ~ListFormatOverride();

        S32 lsid() const;
        U8 countOfLevels() const;
        const ListFormatOverrideLVL* overrideLVL( U8 level ) const;

        // The ownership is transferred to us
        void appendListFormatOverrideLVL( ListFormatOverrideLVL* listFormatOverrideLVL );

    private:
        ListFormatOverride( const ListFormatOverride& rhs );
        ListFormatOverride& operator=( const ListFormatOverride& rhs );

        Word97::LFO m_lfo;
        std::vector<ListFormatOverrideLVL*> m_lfoLevels;
    };

    // Compares the lisd variables, needed to generate a new unique ID
    // for converted ANLDs
    bool operator<( const ListFormatOverride& lhs, const ListFormatOverride& rhs )
    {
        return lhs.lsid() < rhs.lsid();
    }

} // namespace wvWare

using namespace wvWare;

ListLevel::ListLevel( AbstractOLEStreamReader* tableStream ) :
    m_lvlf( tableStream, false ), m_grpprlPapx( 0 ), m_grpprlChpx( 0 )
{
#ifdef WV2_DEBUG_LIST_READING
    wvlog << "   ListLevel::ListLevel() ######" << std::endl
          << "      iStartAt=" << static_cast<int>( m_lvlf.iStartAt ) << " nfc=" << static_cast<int>( m_lvlf.nfc )
          << " jc=" << static_cast<int>( m_lvlf.jc ) << std::endl << "      fLegal=" << static_cast<int>( m_lvlf.fLegal )
          << " fNoRestart=" << static_cast<int>( m_lvlf.fNoRestart ) << " fPrev=" << static_cast<int>( m_lvlf.fPrev )
          << std::endl << "      fPrevSpace=" << static_cast<int>( m_lvlf.fPrevSpace ) << " fWord6="
          << static_cast<int>( m_lvlf.fWord6 ) << std::endl << "      cbGrpprlPapx=" << static_cast<int>( m_lvlf.cbGrpprlPapx )
          << " cbGrpprlChpx=" << static_cast<int>( m_lvlf.cbGrpprlChpx ) << std::endl;
#endif

    if ( m_lvlf.cbGrpprlPapx != 0 ) {
        m_grpprlPapx = new U8[ m_lvlf.cbGrpprlPapx ];
        tableStream->read( m_grpprlPapx, m_lvlf.cbGrpprlPapx );
    }
    if ( m_lvlf.cbGrpprlChpx != 0 ) {
        m_grpprlChpx = new U8[ m_lvlf.cbGrpprlChpx ];
        tableStream->read( m_grpprlChpx, m_lvlf.cbGrpprlChpx );
    }

    U16 len = tableStream->readU16();
    if ( len != 0 ) {
        XCHAR* string = new XCHAR[ len ];
        for ( int i = 0; i < len; ++i )
            string[ i ] = tableStream->readU16();
        m_numberText = UString( reinterpret_cast<UChar*>( string ), len );
		delete[] string;
    }
}

ListLevel::ListLevel( const Word97::ANLD& anld ) : m_grpprlPapx( 0 ), m_grpprlChpx( 0 )
{
    m_lvlf.iStartAt = anld.iStartAt;
    m_lvlf.nfc = anld.nfc;
    m_lvlf.jc = anld.jc;
    m_lvlf.fPrev = anld.fPrev;
    m_lvlf.fPrevSpace = anld.fPrevSpace;
    m_lvlf.fWord6 = true;
    m_lvlf.dxaSpace = anld.dxaSpace;
    m_lvlf.dxaIndent = anld.dxaIndent;

    // number text
#ifdef WV2_DEBUG_LIST_PROCESSING
    wvlog << "cxchTextBefore=" << static_cast<int>( anld.cxchTextBefore ) << " cxchTextAfter="
          << static_cast<int>( anld.cxchTextAfter ) << std::endl;
#endif
    if ( anld.cxchTextBefore > 0 && anld.cxchTextBefore <= 32 ) {
        m_numberText = UString( reinterpret_cast<const UChar*>( &anld.rgxch[ 0 ] ), anld.cxchTextBefore );
#ifdef WV2_DEBUG_LIST_PROCESSING
        wvlog << "String (before): '" << m_numberText.ascii() << "'" << std::endl;
#endif
    }
    m_numberText += UString( static_cast<char>( 0 ) ); // we are faking list level 0, so we have
                                                       // to insert the "variable"
    if ( anld.cxchTextAfter > 0 && anld.cxchTextAfter <= 32 ) {
        int start = anld.cxchTextAfter > anld.cxchTextBefore ? anld.cxchTextBefore : 0;
        m_numberText += UString( reinterpret_cast<const UChar*>( &anld.rgxch[ start ] ), anld.cxchTextAfter - start );
#ifdef WV2_DEBUG_LIST_PROCESSING
        wvlog << "String (after): '" << UString( reinterpret_cast<const UChar*>( &anld.rgxch[ start ] ), anld.cxchTextAfter - start ).ascii() << "'" << std::endl;
#endif
    }

    // grpprls
    // ###### TODO: PAPX
    m_lvlf.cbGrpprlPapx = 0;

    // Allocate enough for all sprms and set the cb as limit
    m_grpprlChpx = new U8[ 29 ];
    U8* grpprl = m_grpprlChpx;
    m_lvlf.cbGrpprlChpx = 0;

    if ( anld.fSetBold )
        m_lvlf.cbGrpprlChpx += writeCharProperty( sprmCFBold, anld.fBold, &grpprl );
    if ( anld.fSetItalic )
        m_lvlf.cbGrpprlChpx += writeCharProperty( sprmCFItalic, anld.fItalic, &grpprl );
    if ( anld.fSetSmallCaps )
        m_lvlf.cbGrpprlChpx += writeCharProperty( sprmCFSmallCaps, anld.fSmallCaps, &grpprl );
    if ( anld.fSetCaps )
        m_lvlf.cbGrpprlChpx += writeCharProperty( sprmCFCaps, anld.fCaps, &grpprl );
    if ( anld.fSetStrike )
        m_lvlf.cbGrpprlChpx += writeCharProperty( sprmCFStrike, anld.fStrike, &grpprl );
    if ( anld.fSetKul )
        m_lvlf.cbGrpprlChpx += writeCharProperty( sprmCKul, anld.kul, &grpprl );

    m_lvlf.cbGrpprlChpx += writeCharProperty( sprmCIco, anld.ico, &grpprl );
    m_lvlf.cbGrpprlChpx += writeCharProperty( sprmCRgFtc0, static_cast<U16>( anld.ftc ), &grpprl );
    m_lvlf.cbGrpprlChpx += writeCharProperty( sprmCHps, anld.hps, &grpprl );

#ifdef WV2_DEBUG_LIST_PROCESSING
    wvlog << "The CHPX is " << static_cast<int>( m_lvlf.cbGrpprlChpx ) << " bytes long" << std::endl;
#endif
}

ListLevel::~ListLevel()
{
    delete [] m_grpprlChpx;
    delete [] m_grpprlPapx;
}

S32 ListLevel::startAt() const
{
    return m_lvlf.iStartAt;
}

U8 ListLevel::numberFormat() const
{
    return m_lvlf.nfc;
}

U8 ListLevel::alignment() const
{
    return m_lvlf.jc;
}

bool ListLevel::isLegal() const
{
    return m_lvlf.fLegal;
}

bool ListLevel::notRestarted() const
{
    return m_lvlf.fNoRestart;
}

bool ListLevel::prev() const
{
    return m_lvlf.fPrev;
}

bool ListLevel::prevSpace() const
{
    return m_lvlf.fPrevSpace;
}

bool ListLevel::isWord6() const
{
    return m_lvlf.fWord6;
}

UString ListLevel::text() const
{
    return m_numberText;
}

U8 ListLevel::followingChar() const
{
    return m_lvlf.ixchFollow;
}

void ListLevel::applyGrpprlPapx( Word97::PAP* pap ) const
{
#ifdef WV2_DEBUG_LIST_PROCESSING
    wvlog << "      ListLevel::applyGrpprlPapx: cbGrpprlPapx=" << static_cast<int>( m_lvlf.cbGrpprlPapx ) << std::endl;
#endif
    if ( m_grpprlPapx )
        pap->apply( m_grpprlPapx, m_lvlf.cbGrpprlPapx, 0, 0, Word8 );
}

void ListLevel::applyGrpprlChpx( Word97::CHP* chp, const Style* style ) const
{
#ifdef WV2_DEBUG_LIST_PROCESSING
    wvlog << "      ListLevel::applyGrpprlChpx: cbGrpprlChpx=" << static_cast<int>( m_lvlf.cbGrpprlChpx ) << std::endl;
#endif
    if ( m_grpprlChpx )
        chp->apply( m_grpprlChpx, m_lvlf.cbGrpprlChpx, style, 0, Word8 );
}

int ListLevel::writeCharProperty( U16 sprm, U8 value, U8** grpprl )
{
    write( *grpprl, sprm );
    *grpprl += sizeof( U16 );
    **grpprl = value;
    ++( *grpprl );
    return 3;
}

int ListLevel::writeCharProperty( U16 sprm, U16 value, U8** grpprl )
{
    write( *grpprl, sprm );
    *grpprl += sizeof( U16 );
    write( *grpprl, value );
    *grpprl += sizeof( U16 );
    return 4;
}

ListData::ListData( AbstractOLEStreamReader* tableStream ) : m_lstf( tableStream, false )
{
#ifdef WV2_DEBUG_LIST_READING
    wvlog << "   ListData::ListData() ######" << std::endl
          << "      lsid=" << m_lstf.lsid << " fSimpleList=" << static_cast<int>( m_lstf.fSimpleList )
          << " tlpc=" << m_lstf.tplc << std::endl;
#endif
}

ListData::ListData( S32 lsid, bool fRestartHdn )
{
    m_lstf.lsid = lsid;
    m_lstf.fSimpleList = true;
    for ( int i = 0; i < 9; ++i )
        m_lstf.rgistd[ i ] = istdNil;
    m_lstf.fRestartHdn = fRestartHdn;
}

ListData::~ListData()
{
    std::for_each( m_listLevels.begin(), m_listLevels.end(), Delete<ListLevel>() );
}

S32 ListData::lsid() const
{
    return m_lstf.lsid;
}

U16 ListData::istdForLevel( U8 level ) const
{
    return m_lstf.rgistd[ level ];
}

bool ListData::isSimpleList() const
{
    return m_lstf.fSimpleList;
}

bool ListData::restartingCounter() const
{
    return m_lstf.fRestartHdn;
}

void ListData::appendListLevel( ListLevel* listLevel )
{
#ifdef WV2_DEBUG_LIST_READING
    wvlog << "      ListData::appendListLevel() this=" << std::hex << reinterpret_cast<int>( this ) << std::dec << std::endl;
#endif
    m_listLevels.push_back( listLevel );
}

const ListLevel* ListData::listLevel( U8 level ) const
{
    if ( level < m_listLevels.size() )
        return m_listLevels[ level ];
    return 0;
}

void ListData::applyGrpprlPapx( Word97::PAP* pap ) const
{
#ifdef WV2_DEBUG_LIST_PROCESSING
    wvlog << "   ListData::applyGrpprlPapx(): level=" << static_cast<int>( pap->ilvl ) << std::endl;
#endif

    if ( !pap || pap->ilvl >= maxListLevels || ( m_lstf.fSimpleList && pap->ilvl != 0 ) )
        return;
    ListLevel* lvl = m_listLevels[ pap->ilvl ];
    if ( lvl )
        lvl->applyGrpprlPapx( pap );
    else
        wvlog << "Bug: Didn't find the level " << pap->ilvl << " in the LSTF!" << std::endl;
}


ListFormatOverrideLVL::ListFormatOverrideLVL( AbstractOLEStreamReader* tableStream ) :
    m_lfolvl( tableStream, false ), m_level( 0 )
{
    if ( m_lfolvl.fFormatting )
        m_level = new ListLevel( tableStream );
}

ListFormatOverrideLVL::~ListFormatOverrideLVL()
{
    delete m_level;
}

S32 ListFormatOverrideLVL::startAt() const
{
    return m_lfolvl.iStartAt;
}

U8 ListFormatOverrideLVL::level() const
{
    return m_lfolvl.ilvl;
}

bool ListFormatOverrideLVL::overridesStartAt() const
{
    return m_lfolvl.fStartAt;
}

void ListFormatOverrideLVL::resetStartAtFlag()
{
    m_lfolvl.fStartAt = false;
}

bool ListFormatOverrideLVL::overridesFormat() const
{
    return m_lfolvl.fFormatting;
}

const ListLevel* ListFormatOverrideLVL::listLevel() const
{
    return m_level;
}

void ListFormatOverrideLVL::dump() const
{
    wvlog << "   ListFormatOverrideLVL::dump() ------------" << std::endl
          << "      iStartAt=" << m_lfolvl.iStartAt << " ilvl=" << static_cast<int>( m_lfolvl.ilvl ) << std::endl
          << "      fStartAt=" << static_cast<int>( m_lfolvl.fStartAt ) << " fFormatting="
          << static_cast<int>( m_lfolvl.fFormatting ) << std::endl
          << "   ListFormatOverrideLVL::dump() done" << std::endl;
}


ListFormatOverride::ListFormatOverride( AbstractOLEStreamReader* tableStream ) : m_lfo( tableStream, false )
{
#ifdef WV2_DEBUG_LIST_READING
    wvlog << "   ListFormatOverride:ListFormatOverride() ######" << std::endl
          << "      lsid=" << m_lfo.lsid << " clfolvl=" << static_cast<int>( m_lfo.clfolvl ) << std::endl;
#endif
}

ListFormatOverride::ListFormatOverride( S32 lsid )
{
    m_lfo.lsid = lsid;
}

ListFormatOverride::~ListFormatOverride()
{
    std::for_each( m_lfoLevels.begin(), m_lfoLevels.end(), Delete<ListFormatOverrideLVL>() );
}

S32 ListFormatOverride::lsid() const
{
    return m_lfo.lsid;
}

U8 ListFormatOverride::countOfLevels() const
{
    return m_lfo.clfolvl;
}

const ListFormatOverrideLVL* ListFormatOverride::overrideLVL( U8 level ) const
{
    std::vector<ListFormatOverrideLVL*>::const_iterator it = m_lfoLevels.begin();
    std::vector<ListFormatOverrideLVL*>::const_iterator end = m_lfoLevels.end();
    for ( ; it != end; ++it )
        if ( ( *it )->level() == level )
            return *it;
    return 0;
}

void ListFormatOverride::appendListFormatOverrideLVL( ListFormatOverrideLVL* listFormatOverrideLVL )
{
    m_lfoLevels.push_back( listFormatOverrideLVL );
}


ListText::ListText() : chp( 0 )
{
}

ListText::~ListText()
{
}


ListInfo::ListInfo( Word97::PAP& pap, ListInfoProvider& listInfoProvider ) :
    m_linkedIstd( istdNil ), m_restartingCounter( false ), m_numberFormat( 0 ),
    m_alignment( 0 ), m_isLegal( false ), m_notRestarted( false ), m_prev( false ),
    m_prevSpace( false ), m_isWord6( false ), m_followingChar( 0 ), m_lsid( 0 )
{
    if ( !listInfoProvider.setPAP( &pap ) )
        return;
    const ListLevel* const level = listInfoProvider.formattingListLevel();
    const ListData* const listData = listInfoProvider.m_currentLst;

    if ( listData ) {
        m_linkedIstd = listData->istdForLevel( pap.ilvl );
        m_restartingCounter = listData->restartingCounter();
        m_lsid = listData->lsid();
    }
    else
        wvlog << "Bug: The ListData is 0!!" << std::endl;

    m_startAt = listInfoProvider.startAt();

    if ( level ) {
        m_numberFormat = level->numberFormat();
        m_alignment = level->alignment();
        m_isLegal = level->isLegal();
        m_notRestarted = level->notRestarted();
        m_prev = level->prev();
        m_prevSpace = level->prevSpace();
        m_isWord6 = level->isWord6();
        m_text = listInfoProvider.text();
        m_followingChar = level->followingChar();
    }
    else
        wvlog << "Bug: The ListLevel is 0!!" << std::endl;
}

void ListInfo::dump() const
{
    wvlog << "ListInfo::dump() ------------------------------" << std::endl;
    wvlog << "   linkedIstd=" << m_linkedIstd << std::endl
          << "   restartingCounter=" << m_restartingCounter << " startAt=" << m_startAt.first << std::endl
          << "   startAtOverridden=" << m_startAt.second << std::endl
          << "   numberFormat=" << static_cast<int>( m_numberFormat ) << " alignment="
          << static_cast<int>( m_alignment ) << std::endl << "   isLegal=" << m_isLegal
          << " notRestarted=" << m_notRestarted << std::endl << "   prev=" << m_prev
          << " prevSpace=" << m_prevSpace << std::endl << "   isWord6=" << m_isWord6
          << " text= '";
    for ( int i = 0; i < m_text.text.length(); ++i )
        wvlog << "<" << static_cast<char>( m_text.text[ i ].low() ) << "/" << m_text.text[ i ].unicode() << ">";
    wvlog << "'" << std::endl
          << "   followingChar=" << static_cast<int>( m_followingChar ) << std::endl
          << "ListInfo::dump() done -------------------------" << std::endl;
}


ListInfoProvider::ListInfoProvider( const StyleSheet* styleSheet )
    : m_listNames( 0 ), m_pap( 0 ), m_styleSheet( styleSheet ), m_currentLfoLVL( 0 ),
      m_currentLst( 0 ), m_version( Word67 )
{
#ifdef WV2_DEBUG_LIST_READING
    wvlog << "ListInfoProvider::ListInfoProvider() ################################" << std::endl
          << " ---> pre-Word 8" << std::endl;
#endif
}

ListInfoProvider::ListInfoProvider( AbstractOLEStreamReader* tableStream, const Word97::FIB& fib, const StyleSheet* styleSheet ) :
    m_listNames( 0 ), m_pap( 0 ), m_styleSheet( styleSheet ), m_currentLfoLVL( 0 ), m_currentLst( 0 ), m_version( Word8 )
{
#ifdef WV2_DEBUG_LIST_READING
    wvlog << "ListInfoProvider::ListInfoProvider() ################################" << std::endl
          << "   fcPlcfLst=" << fib.fcPlcfLst << " lcbPlcfLst=" << fib.lcbPlcfLst << std::endl
          << "   fcPlfLfo=" << fib.fcPlfLfo << " lcbPlfLfo=" << fib.lcbPlfLfo << std::endl
          << "   fcSttbListNames=" << fib.fcSttbListNames << " lcbSttbListNames=" << fib.lcbSttbListNames << std::endl;
#endif

    tableStream->push();
    if ( fib.lcbPlcfLst != 0 ) {
        tableStream->seek( fib.fcPlcfLst, SEEK_SET );
        readListData( tableStream, fib.fcPlcfLst + fib.lcbPlcfLst );
    }
    if ( fib.lcbPlfLfo != 0 ) {
        if ( static_cast<U32>( tableStream->tell() ) != fib.fcPlfLfo ) {
            wvlog << "Found a \"hole\" within the table stream (list data): current="
                  << tableStream->tell() << " expected=" << fib.fcPlfLfo << std::endl;
            tableStream->seek( fib.fcPlfLfo, SEEK_SET );
        }
        readListFormatOverride( tableStream );
    }
    if ( fib.lcbSttbListNames != 0 ) {
        // Get rid of leading garbage. Take care, though, as the STTBF most likely starts
        // with 0xffff (extended character STTBF)
        while ( static_cast<U32>( tableStream->tell() ) < fib.fcSttbListNames &&
                tableStream->readU8() == 0xff ); // the ; is intended!

        // Check the position and warn about corrupt files
        if ( static_cast<U32>( tableStream->tell() ) != fib.fcSttbListNames ) {
            wvlog << "Found a \"hole\" within the table stream (list format override): current="
                  << tableStream->tell() << " expected=" << fib.fcSttbListNames << std::endl;
            tableStream->seek( fib.fcSttbListNames, SEEK_SET );
        }
        readListNames( tableStream );
    }
    tableStream->pop();

#ifdef WV2_DEBUG_LIST_READING
    wvlog << "ListInfoProvider::ListInfoProvider() done ###########################" << std::endl;
#endif
}

ListInfoProvider::~ListInfoProvider()
{
    delete m_listNames;
    std::for_each( m_listFormatOverride.begin(), m_listFormatOverride.end(), Delete<ListFormatOverride>() );
    std::for_each( m_listData.begin(), m_listData.end(), Delete<ListData>() );
}

bool ListInfoProvider::isValid( S16 ilfo, U8 nLvlAnm ) const
{
    if ( m_version == Word67 )
        return nLvlAnm != 0;
    else
        return ilfo == oldStyleIlfo || ( ilfo > 0 && ilfo <= static_cast<int>( m_listFormatOverride.size() ) );
}

bool ListInfoProvider::setPAP( Word97::PAP* pap )
{
#ifdef WV2_DEBUG_LIST_PROCESSING
    wvlog << "ListInfoProvider::setPAP(): nLvlAnm = " << static_cast<int>( pap->nLvlAnm )
          << " ilfo = " << pap->ilfo << std::endl;
#endif
    // Is it a list paragraph at all?
    if ( ( m_version == Word67 ? static_cast<S16>( pap->nLvlAnm ) : pap->ilfo ) < 1 )  {
        m_pap = 0;
        m_currentLfoLVL = 0;
        m_currentLst = 0;
        return false;
    }

    m_pap = pap;  // Don't we all love dangling pointers?
    if ( m_version == Word67 )
        convertCompatANLD();
    else {
        if ( m_listFormatOverride.size() < static_cast<unsigned int>( pap->ilfo ) ) { // 1-based index!
            // This might be an old-style pap, where a pap->ilfo of 2047 suggests to look at
            // the ANLD of that paragaph and to convert it
            if ( pap->ilfo == oldStyleIlfo )
                convertCompatANLD();
            else {
                wvlog << "Bug: ListInfoProvider::setWord97StylePAP -- out of bounds access (ilfo=" << pap->ilfo << ")" << std::endl;
                m_pap = 0;
                m_currentLfoLVL = 0;
                m_currentLst = 0;
                return false;
            }
        }
    }
    processOverride( m_listFormatOverride[ pap->ilfo - 1 ] );
    return true;
}

void ListInfoProvider::readListData( AbstractOLEStreamReader* tableStream, const U32 endOfLSTF )
{
    const U16 count = tableStream->readU16();
#ifdef WV2_DEBUG_LIST_READING
    wvlog << "ListInfoProvider::readListData(): count=" << count << std::endl;
#endif
    for ( int i = 0; i < count; ++i )
        m_listData.push_back( new ListData( tableStream ) );

    // Note that this is a bug in the spec, but it at least seems to be a "stable" bug ;)
    if ( static_cast<U32>( tableStream->tell() ) != endOfLSTF )
        wvlog << "Expected a different size of this plcflst! (expected: "
              << endOfLSTF << " position: " << tableStream->tell() << ")" << std::endl;

#ifdef WV2_DEBUG_LIST_READING
    wvlog << "ListInfoProvider::readListData(): 2nd step -- reading the LVLFs" << std::endl;
#endif
    // Now read in the ListLevels for each ListData
    // If fSimpleList is true we only have one level, else there are nine
    std::vector<ListData*>::const_iterator it = m_listData.begin();
    std::vector<ListData*>::const_iterator end = m_listData.end();
    for ( ; it != end; ++it ) {
        if ( ( *it )->isSimpleList() )
            ( *it )->appendListLevel( new ListLevel( tableStream ) );
        else
            for ( int i = 0; i < maxListLevels; ++i )
                ( *it )->appendListLevel( new ListLevel( tableStream ) );
    }
}

void ListInfoProvider::readListFormatOverride( AbstractOLEStreamReader* tableStream )
{
    const U32 count = tableStream->readU32();
#ifdef WV2_DEBUG_LIST_READING
    wvlog << "ListInfoProvider::readListFormatOverride(): count=" << count << std::endl;
#endif
    for ( U32 i = 0; i < count; ++i )
        m_listFormatOverride.push_back( new ListFormatOverride( tableStream ) );

    std::vector<ListFormatOverride*>::const_iterator it = m_listFormatOverride.begin();
    std::vector<ListFormatOverride*>::const_iterator end = m_listFormatOverride.end();
    for ( ; it != end; ++it ) {
        const U8 levelCount = ( *it )->countOfLevels();
        for ( int i = 0; i < levelCount; ++i ) {
            // Word seems to write 0xff pagging-bytes between LFO and LFOLVLs, also
            // between different LFOLVLs, get rid of it (Werner)
            eatLeading0xff( tableStream );
            ( *it )->appendListFormatOverrideLVL( new ListFormatOverrideLVL( tableStream ) );
        }
    }
}

void ListInfoProvider::readListNames( AbstractOLEStreamReader* tableStream )
{
#ifdef WV2_DEBUG_LIST_READING
    wvlog << "ListInfoProvider::readListNames()" << std::endl;
#endif
    m_listNames = new STTBF( usLid, tableStream, false );
#ifdef WV2_DEBUG_LIST_READING
    m_listNames->dumpStrings();
#endif
}

void ListInfoProvider::eatLeading0xff( AbstractOLEStreamReader* tableStream )
{
    while ( tableStream->readU8() == 0xff ); // semicolon intended ;)
    tableStream->seek( -1, SEEK_CUR ); // rewind the stream
}

void ListInfoProvider::processOverride( ListFormatOverride* lfo )
{
#ifdef WV2_DEBUG_LIST_PROCESSING
    wvlog << "   ListInfoProvider::processOverride()" << std::endl;
#endif
    bool appliedPapx = false;
    // It turns out we need a non-const pointer, as we have to reset the
    // startAt flag after the first paragraph
    m_currentLfoLVL = const_cast<ListFormatOverrideLVL*>( lfo->overrideLVL( m_pap->ilvl ) );

    if ( m_currentLfoLVL ) {
#ifdef WV2_DEBUG_LIST_PROCESSING
        wvlog << "      Level " << static_cast<int>( m_pap->ilvl ) << " found:" << std::endl;
        m_currentLfoLVL->dump();
#endif
        if ( m_currentLfoLVL->overridesFormat() && m_currentLfoLVL->listLevel() ) {
            m_currentLfoLVL->listLevel()->applyGrpprlPapx( m_pap );
            appliedPapx = true;
        }
    }
    m_currentLst = findLST( lfo->lsid() );

    if ( m_currentLst && !appliedPapx )
        m_currentLst->applyGrpprlPapx( m_pap );
}

void ListInfoProvider::convertCompatANLD()
{
    // We are creating a fake "unique" list ID via a CRC32 sum.
    // It should be rather unlikely that two different ANLDs
    // have the same lsid. In the case that the lsid is already
    // there we assume that we already hit that ANLD before and
    // just use its formatting information right away.
    // There's one problem with that approach: If you have a Word 97
    // document using some compat-lists and the lsid the CRC algorithm
    // creates is already there, we end up using the wrong formatting
    // information. I'm sure it's more likely to win in the lottery
    // than facing that situation, though :-)
    // Note that compat lists are always simple lists. (ilvl == 0)
    const S32 lsid = createFakeLSID( m_pap->anld );
    m_pap->ilvl = 0;
#ifdef WV2_DEBUG_LIST_PROCESSING
    wvlog << "   ListInfoProvider::convertCompatANLD" << std::endl
          << "      generated a unique lsid: " << lsid << std::endl;
#endif

    // Does it already exist?
    std::vector<ListData*>::const_iterator it = m_listData.begin();
    std::vector<ListData*>::const_iterator end = m_listData.end();
    for ( int i = 1; it != end; ++it, ++i ) // 1-based index!
        if ( ( *it )->lsid() == lsid ) {
            m_pap->ilfo = i;
            return;
        }

    // It's not there yet...
    // Fake a LFO structure (with 0 overridden levels and the lsid we generated)
    m_listFormatOverride.push_back( new ListFormatOverride( lsid ) );
    m_pap->ilfo = m_listFormatOverride.size();

    // And another fake, the LSTF
    ListData* listData = new ListData( lsid, m_pap->anld.fRestartHdn );
    listData->appendListLevel( new ListLevel( m_pap->anld ) );
    m_listData.push_back( listData );
}

ListData* ListInfoProvider::findLST( S32 lsid )
{
#ifdef WV2_DEBUG_LIST_PROCESSING
    wvlog << "   ListInfoProvider::findLST: lsid=" << lsid << std::endl;
#endif
    std::vector<ListData*>::const_iterator it = m_listData.begin();
    std::vector<ListData*>::const_iterator end = m_listData.end();
    for ( ; it != end; ++it )
        if ( ( *it )->lsid() == lsid )
            return *it;
    return 0;
}

const ListLevel* ListInfoProvider::formattingListLevel() const
{
    if ( m_currentLfoLVL && m_currentLfoLVL->overridesFormat() && m_currentLfoLVL->listLevel() )
        return m_currentLfoLVL->listLevel();
    return m_currentLst ? m_currentLst->listLevel( m_pap->ilvl ) : 0;
}

std::pair<S32, bool> ListInfoProvider::startAt()
{
    std::pair<S32, bool> start( 1, false );
    if ( m_currentLfoLVL && m_currentLfoLVL->overridesStartAt() ) {
        start.second = true;
        if ( m_currentLfoLVL->overridesFormat() && m_currentLfoLVL->listLevel() )
            start.first = m_currentLfoLVL->listLevel()->startAt();
        else
            start.first = m_currentLfoLVL->startAt();
        // Reset the startAt flag after the first paragraph (Word 97 spec, LFO parag.)
        m_currentLfoLVL->resetStartAtFlag();
    }
    else {
        const ListLevel* level = m_currentLst ? m_currentLst->listLevel( m_pap->ilvl ) : 0;
        if ( level )
            start.first = level->startAt();
    }
    return start;
}

ListText ListInfoProvider::text() const
{
    ListText ret;
    ret.text = formattingListLevel()->text();

    // Get the appropriate style for this paragraph
    const Style* style = m_styleSheet->styleByIndex( m_pap->istd );
    if ( !style ) {
        wvlog << "Bug: Huh, really obscure error, couldn't find the Style for the current PAP" << std::endl;
        ret.chp = new Word97::CHP;
    }
    else
        ret.chp = new Word97::CHP( style->chp() );

    formattingListLevel()->applyGrpprlChpx( ret.chp, style );
    return ret;
}
