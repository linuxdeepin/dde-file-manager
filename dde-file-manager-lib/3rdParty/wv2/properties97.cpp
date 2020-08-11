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

#include "properties97.h"
#include "word97_helper.h"
#include "word95_generated.h"
#include "convert.h"
#include "styles.h"
#include "paragraphproperties.h"


namespace wvWare
{
    FKP< BX<Word97::PHE> >* convertFKP( const FKP< BX<Word95::PHE> >& old )
    {
        FKP< BX<Word97::PHE> >* ret( new FKP< BX<Word97::PHE> > );
        // The unchanged fields first...
        ret->m_crun = old.m_crun;
        ret->m_rgfc = new U32[ old.m_crun + 1 ];
        memcpy( ret->m_rgfc, old.m_rgfc, ( old.m_crun + 1 ) * sizeof( U32 ) );
        ret->m_internalOffset = old.m_internalOffset;
        const U16 length = 511 - old.m_internalOffset;
        ret->m_fkp = new U8[ length ];
        memcpy( ret->m_fkp, old.m_fkp, length );

        // Convert the Offset array
        ret->m_rgb = new BX<Word97::PHE>[ old.m_crun ];
        for ( int i = 0; i < old.m_crun; ++i ) {
            ret->m_rgb[ i ].offset = old.m_rgb[ i ].offset;
            ret->m_rgb[ i ].phe = Word95::toWord97( old.m_rgb[ i ].phe );
        }
        return ret;
    }
} // namespace

using namespace wvWare;


Properties97::Properties97( AbstractOLEStreamReader* wordDocument, AbstractOLEStreamReader* table, const Word97::FIB &fib ) :
    m_version( fib.nFib < Word8nFib ? Word67 : Word8 ), m_wordDocument( wordDocument ), m_table( table ),
    m_stylesheet( 0 ), m_plcfsed( 0 ), m_plcfbtePapx( 0 ), m_plcfbteChpx( 0 ), m_papxFkp( 0 ), m_chpxFkp( 0 )
{
    // First create the whole stylesheet information.
    m_stylesheet = new StyleSheet( m_table, fib.fcStshf, fib.lcbStshf );

    // Read the DOP
    m_table->seek( fib.fcDop );
    if ( m_version == Word8 )
        m_dop.read( m_table, false );
    else
        m_dop = Word95::toWord97( Word95::DOP( m_table, false ) );

    if ( m_table->tell() != static_cast<S32>( fib.fcDop + fib.lcbDop ) )
        wvlog << "Warning: DOP has a different size than expected." << std::endl;

    // Read the PLCF SED. The Word95::SED is different, but the differing
    // fields are unused and of the same size, so what :-)
    m_table->seek( fib.fcPlcfsed );
    m_plcfsed = new PLCF<Word97::SED>( fib.lcbPlcfsed, m_table );

    if ( fib.lcbClx != 0 ) {
        // Read the PAPX and CHPX BTE PLCFs (to locate the appropriate FKPs)
        m_table->seek( fib.fcPlcfbtePapx );
        if ( m_version == Word8 )
            m_plcfbtePapx = new PLCF<Word97::BTE>( fib.lcbPlcfbtePapx, m_table );
        else
            m_plcfbtePapx = convertPLCF<Word95::BTE, Word97::BTE>( PLCF<Word95::BTE>( fib.lcbPlcfbtePapx, m_table ) );
        if ( fib.cpnBtePap != 0 && fib.cpnBtePap != m_plcfbtePapx->count() )
            wvlog << "Error: The PAP piece table is incomplete! (Should be " << fib.cpnBtePap << ")" << std::endl;

        m_table->seek( fib.fcPlcfbteChpx );
        if ( m_version == Word8 )
            m_plcfbteChpx = new PLCF<Word97::BTE>( fib.lcbPlcfbteChpx, m_table );
        else
            m_plcfbteChpx = convertPLCF<Word95::BTE, Word97::BTE>( PLCF<Word95::BTE>( fib.lcbPlcfbteChpx, m_table ) );
        if ( fib.cpnBteChp != 0 && fib.cpnBteChp != m_plcfbteChpx->count() )
            wvlog << "Error: The CHP piece table is incomplete! (Should be " << fib.cpnBteChp << ")" << std::endl;
    }
    else {
        // Read the PAPX and CHPX BTE PLCFs (to locate the appropriate FKPs) from a non-complex file
        m_table->seek( fib.fcPlcfbtePapx );
        m_plcfbtePapx = convertPLCF<Word95::BTE, Word97::BTE>( PLCF<Word95::BTE>( fib.lcbPlcfbtePapx, m_table ) );
        if ( fib.cpnBtePap != m_plcfbtePapx->count() )
            fillBinTable( m_plcfbtePapx, fib.cpnBtePap );

        m_table->seek( fib.fcPlcfbteChpx );
        m_plcfbteChpx = convertPLCF<Word95::BTE, Word97::BTE>( PLCF<Word95::BTE>( fib.lcbPlcfbteChpx, m_table ) );
        if ( fib.cpnBteChp != m_plcfbteChpx->count() )
            fillBinTable( m_plcfbteChpx, fib.cpnBteChp );
    }


}

Properties97::~Properties97()
{
    delete m_chpxFkp;
    delete m_papxFkp;
    delete m_plcfbteChpx;
    delete m_plcfbtePapx;
    delete m_plcfsed;
    delete m_stylesheet;
}

const Style* Properties97::styleByIndex( U16 istd ) const
{
    return m_stylesheet->styleByIndex( istd );
}

StyleSheet& Properties97::styleSheet() const
{
    return *m_stylesheet;
}

const Word97::DOP& Properties97::dop() const
{
    return m_dop;
}

SharedPtr<const Word97::SEP> Properties97::sepForCP( U32 cp ) const
{
    // Did we find a page break? If we don't have any PLCFSED entries it has to be one
    if ( m_plcfsed->isEmpty() )
        return SharedPtr<const Word97::SEP>( 0 );

    // The documentation says that it's a page break if this PLCF doesn't have an entry
    // for this CP -- at least I read it that way
    PLCFIterator<Word97::SED> it( *m_plcfsed );
    while ( it.current() && it.currentLim() <= cp )
        ++it;

    if ( it.currentStart() == cp ) {
        Word97::SED* sed = it.current();
        Word97::SEP* sep = new Word97::SEP;

        if ( !sed || sed->fcSepx == 0xffffffff )
            return SharedPtr<const Word97::SEP>( sep );

        m_wordDocument->push();
        m_wordDocument->seek( sed->fcSepx );
        const U16 count = m_wordDocument->readU16();
        U8* grpprl = new U8[ count ];
        m_wordDocument->read( grpprl, count );

        sep->apply( grpprl, count, 0, 0, m_version );

        delete [] grpprl;
        m_wordDocument->pop();
        return SharedPtr<const Word97::SEP>( sep );
    }
    return SharedPtr<const Word97::SEP>( 0 );
}

ParagraphProperties* Properties97::fullSavedPap( U32 fc, AbstractOLEStreamReader* dataStream  )
{
    // Step 1: Search the correct FKP entry in the PLCFBTE
    PLCFIterator<Word97::BTE> it( *m_plcfbtePapx );
    while ( it.current() && it.currentLim() <= fc )
        ++it;

    if ( !it.current() ) {
        wvlog << "Bug: PAPX BTE screwed" << std::endl;
        return new ParagraphProperties;
    }

    // Step 2: Check the cache if we already have the correct FKP
    if ( m_papxFkp ) {
        PAPXFKPIterator fkpit( *m_papxFkp );
        if ( fkpit.currentStart() != it.currentStart() ) {
            delete m_papxFkp;
            m_papxFkp = 0;
        }
    }

    // Step 3: Get the new FKP, if necessary
    if ( !m_papxFkp ) {
        m_wordDocument->push();
        m_wordDocument->seek( it.current()->pn << 9, SEEK_SET );  // 512 byte pages ( << 9 )
        if ( m_version == Word8 )
            m_papxFkp = new PAPXFKP_t( m_wordDocument, false );
        else
            m_papxFkp = convertFKP( PAPXFKP95_t( m_wordDocument, false ) );
        m_wordDocument->pop();
    }

    // Step 4: Get the right entry within our FKP
    PAPXFKPIterator fkpit( *m_papxFkp );
    while ( !fkpit.atEnd() && fkpit.currentLim() <= fc )
        ++fkpit;

    // Step 5: Now that we are at the correct place let's apply the PAPX grpprl
    ParagraphProperties *properties = Word97::initPAPFromStyle( fkpit.current(), m_stylesheet, dataStream, m_version );

    // Step 6: Copy the PHE from the BX in the FKP to restore the state of the PAP during
    //         the last full-save of the document
    properties->pap().phe = fkpit.currentOffset().phe;

    return properties;
}

void Properties97::applyClxGrpprl( const Word97::PCD* pcd, U32 fcClx, ParagraphProperties* properties )
{
    applyClxGrpprlImpl<Word97::PAP>( pcd, fcClx, &properties->pap(), m_stylesheet->styleByIndex( properties->pap().istd ) );
}

Word97::TAP* Properties97::fullSavedTap( U32 fc, AbstractOLEStreamReader* dataStream )
{
    // This method is quite similar to fullSavedPap, but a template solution would suck :}
    // Maybe I'll clean that up later

    // Step 1: Search the correct FKP entry in the PLCFBTE
    PLCFIterator<Word97::BTE> it( *m_plcfbtePapx );
    while ( it.current() && it.currentLim() <= fc )
        ++it;

    if ( !it.current() ) {
        wvlog << "Bug: TAPX BTE screwed" << std::endl;
        return new Word97::TAP;
    }

    // Step 2: Check the cache if we already have the correct FKP
    if ( m_papxFkp ) {
        PAPXFKPIterator fkpit( *m_papxFkp );
        if ( fkpit.currentStart() != it.currentStart() ) {
            delete m_papxFkp;
            m_papxFkp = 0;
        }
    }

    // Step 3: Get the new FKP, if necessary
    if ( !m_papxFkp ) {
        m_wordDocument->push();
        m_wordDocument->seek( it.current()->pn << 9, SEEK_SET );  // 512 byte pages ( << 9 )
        if ( m_version == Word8 )
            m_papxFkp = new PAPXFKP_t( m_wordDocument, false );
        else
            m_papxFkp = convertFKP( PAPXFKP95_t( m_wordDocument, false ) );
        m_wordDocument->pop();
    }

    // Step 4: Get the right entry within our FKP
    PAPXFKPIterator fkpit( *m_papxFkp );
    while ( !fkpit.atEnd() && fkpit.currentLim() <= fc )
        ++fkpit;

    // Step 5: Now that we are at the correct place let's apply the PAPX grpprl
    //         to our TAP
    return Word97::initTAP( fkpit.current(), dataStream, m_version );
}

void Properties97::applyClxGrpprl( const Word97::PCD* pcd, U32 fcClx, Word97::TAP* tap, const Style* style )
{
    applyClxGrpprlImpl<Word97::TAP>( pcd, fcClx, tap, style );
}

U32 Properties97::fullSavedChp( const U32 fc, Word97::CHP* chp, const Style* paragraphStyle )
{
    // Before we start with the plain FKP algorithm like above we have to apply any
    // CHPX found in the style entry for the CHP, unless it's istdNormalChar
    U16 oldIstd = chp->istd; // remember the old istd, see below
    if ( chp->istd != 10 ) {
        wvlog << "Applying a character style" << std::endl;
        const Style* style = m_stylesheet->styleByIndex( chp->istd );
        if ( style && style->type() == Style::sgcChp ) {
            const UPECHPX& upechpx( style->upechpx() );
            chp->apply( upechpx.grpprl, upechpx.cb, paragraphStyle, 0, m_version );
        }
        else
            wvlog << "Couldn't find the character style with istd " << chp->istd << std::endl;
    }

    // Step 1: Search the correct FKP entry in the PLCFBTE
    PLCFIterator<Word97::BTE> it( *m_plcfbteChpx );
    while ( it.current() && it.currentLim() <= fc )
        ++it;

    if ( !it.current() ) {
        wvlog << "Bug: CHPX BTE screwed (backing out by faking properties)" << std::endl;
        it.toFirst();
    }

    // Step 2: Check the cache if we already have the correct FKP
    if ( m_chpxFkp ) {
        CHPXFKPIterator fkpit( *m_chpxFkp );
        if ( fkpit.currentStart() != it.currentStart() ) {
            delete m_chpxFkp;
            m_chpxFkp = 0;
        }
    }

    // Step 3: Get the new FKP, if necessary
    if ( !m_chpxFkp ) {
        m_wordDocument->push();
        m_wordDocument->seek( it.current()->pn << 9, SEEK_SET );  // 512 byte pages ( << 9 )
        m_chpxFkp = new CHPXFKP_t( m_wordDocument, false );
        m_wordDocument->pop();
    }

    // Step 4: Get the right entry within our FKP
    CHPXFKPIterator fkpit( *m_chpxFkp );
    while ( !fkpit.atEnd() && fkpit.currentLim() <= fc )
        ++fkpit;

    // Step 5: Now that we are at the correct place let's apply the CHPX grpprl
    chp->applyExceptions( fkpit.current(), paragraphStyle, 0, m_version );

    // In case the applied exceptions changed the istd it seems we have to apply
    // the new character style. The footnote handling indicates that this is right,
    // but the specification doesn't talk about that. I'm not sure if the character
    // style mess from above is need at all, but this simple if() doesn't hurt (Werner)
    if ( chp->istd != oldIstd && chp->istd != 10 ) {
        //wvlog << "Applying a character style (after applying the FKP sprms)" << std::endl;
        const Style* style = m_stylesheet->styleByIndex( chp->istd );
        if ( style && style->type() == Style::sgcChp ) {
            const UPECHPX& upechpx( style->upechpx() );
            chp->apply( upechpx.grpprl, upechpx.cb, paragraphStyle, 0, m_version );
        }
        else
            wvlog << "Couldn't find the character style with istd " << chp->istd << std::endl;
    }
    return fkpit.currentLim() - fc;
}

void Properties97::applyClxGrpprl( const Word97::PCD* pcd, U32 fcClx, Word97::CHP* chp, const Style* style )
{
    applyClxGrpprlImpl<Word97::CHP>( pcd, fcClx, chp, style );
}

template<class P>
void Properties97::applyClxGrpprlImpl( const Word97::PCD* pcd, U32 fcClx, P* properties, const Style* style )
{
    if ( !pcd ) {
        wvlog << "Huh? This can't have happended, right?" << std::endl;
        return;
    }

    if ( pcd->prm.fComplex != 0 ) {
        U16 igrpprl = pcd->prm.toPRM2().igrpprl;
        //wvlog << "############# igrpprl: " << igrpprl << std::endl;
        m_table->push();
        m_table->seek( fcClx );
        U8 blockType = m_table->readU8();

        while ( blockType == wvWare::clxtGrpprl && igrpprl > 0 ) {
            U16 size = m_table->readU16();
            //wvlog << "Skipping a clxtGrpprl (size=" << size << ")" << std::endl;
            m_table->seek( size, SEEK_CUR );
            blockType = m_table->readU8();
            --igrpprl;
        }

        if ( blockType == wvWare::clxtGrpprl ) {
            U16 size = m_table->readU16();
            //wvlog << "Found the right clxtGrpprl (size=" << size << ")" << std::endl;
            U8 *grpprl = new U8[ size ];
            m_table->read( grpprl, size );
            properties->apply( grpprl, size, style, 0, m_version ); // dataStream shouldn't be necessary in a clx
            delete [] grpprl;
        }
        m_table->pop();
    }
    else {
        U16 sprm = toLittleEndian( Word97::SPRM::unzippedOpCode( pcd->prm.isprm ) ); // force LE order
        if ( sprm != 0 ) {
            //wvlog << "CHPX/PAPX/TAPX ###### compressed: " << pcd->prm.isprm << " Uncompressed sprm: " << sprm
            //      << " data: " << ( int )pcd->prm.val << std::endl;
            U8 grpprl[ 3 ];
            grpprl[ 0 ] = static_cast<U8>( sprm & 0x00ff );
            grpprl[ 1 ] = static_cast<U8>( ( sprm & 0xff00 ) >> 8 );
            grpprl[ 2 ] = pcd->prm.val;
            properties->apply( grpprl, 3, style, 0, Word8 ); // dataStream shouldn't be necessary in a clx
        }
    }
}

void Properties97::fillBinTable( PLCF<Word97::BTE>* bte, U16 cpnBte )
{
    U16 pnLast = 0;
    PLCFIterator<Word97::BTE> it( *bte );
    for ( ; it.current(); ++it )
        if ( it.current()->pn > pnLast )
            pnLast = it.current()->pn;

    m_wordDocument->push();
    cpnBte -= bte->count();
    while ( cpnBte > 0 ) {
        Word97::BTE* tmp( new Word97::BTE );
        tmp->pn = ++pnLast;
        m_wordDocument->seek( pnLast << 9, SEEK_SET );
        bte->insert( m_wordDocument->readU32(), tmp );
        --cpnBte;
    }
    m_wordDocument->pop();
}
