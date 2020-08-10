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

#ifndef PROPERTIES97_H
#define PROPERTIES97_H

#include "sharedptr.h"
#include "word_helper.h"
#include "word97_generated.h"

namespace wvWare
{
    class ParagraphProperties;
    namespace Word95
    {
        class PHE;
    }

    class Properties97
    {
    public:
		Properties97( AbstractOLEStreamReader* wordDocument, AbstractOLEStreamReader* table, const Word97::FIB &fib );
        ~Properties97();

        // StyleSheet ---------
        const Style* styleByIndex( U16 istd ) const;
        StyleSheet& styleSheet() const;

        // Document properties ---------
        const Word97::DOP& dop() const;

        // Section properties ---------
        SharedPtr<const Word97::SEP> sepForCP( U32 cp ) const;

        // Paragraph properties ---------
        // Determines the PAP state from the last full-save (ownership is transferred)
		ParagraphProperties* fullSavedPap( U32 fc, AbstractOLEStreamReader* dataStream );
        // Apply the latest changes recorded in the clxGrppl section of the piece table
        void applyClxGrpprl( const Word97::PCD* pcd, U32 fcClx, ParagraphProperties* properties );

        // Table properties ---------
        // Determines the TAP state from the last full-save (ownership is transferred)
		Word97::TAP* fullSavedTap( U32 fc, AbstractOLEStreamReader* dataStream );
        void applyClxGrpprl( const Word97::PCD* pcd, U32 fcClx, Word97::TAP* tap, const Style* style );

        // Character properties ---------
        // Determines the CHP state from the last full-save (ownership is transferred)
        U32 fullSavedChp( const U32 fc, Word97::CHP* chp, const Style* paragraphStyle );
        // Apply the latest changes recorded in the clxGrppl section of the piece table
        void applyClxGrpprl( const Word97::PCD* pcd, U32 fcClx, Word97::CHP* chp, const Style* style );

    private:
        Properties97( const Properties97& rhs );
        Properties97& operator=( const Properties97& rhs );

        // This should have been called "applyClxGrpprl" too, but VC7 doesn't like that
        template<class P> void applyClxGrpprlImpl( const Word97::PCD* pcd, U32 fcClx, P* properties, const Style* style );

        void fillBinTable( PLCF<Word97::BTE>* bte, U16 cpnBte );

        const WordVersion m_version;
		AbstractOLEStreamReader* m_wordDocument; // doesn't belong to us, be careful
		AbstractOLEStreamReader* m_table; // doesn't belong to us, be careful

        StyleSheet* m_stylesheet;
        Word97::DOP m_dop;
        PLCF<Word97::SED>* m_plcfsed;     // section descr. table
        PLCF<Word97::BTE>* m_plcfbtePapx; // PAPX FKP page numbers
        PLCF<Word97::BTE>* m_plcfbteChpx; // CHPX FKP page numbers

        typedef FKP< BX<Word97::PHE> > PAPXFKP_t;
        typedef FKP< BX<Word95::PHE> > PAPXFKP95_t;
        typedef FKPIterator< BX<Word97::PHE> > PAPXFKPIterator;
        PAPXFKP_t* m_papxFkp;  // Currently cached PAPX FKP

        typedef FKP<CHPFKP_BX> CHPXFKP_t;
        typedef FKPIterator<CHPFKP_BX> CHPXFKPIterator;
        CHPXFKP_t* m_chpxFkp;  // Currently cached CHPX FKP
};

} // namespace wvWare

#endif // PROPERTIES97_H
