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

#ifndef FOOTNOTES97_H
#define FOOTNOTES97_H

#include "global.h"
#include "functordata.h"

#include <vector>

namespace wvWare
{
    namespace Word97
    {
        struct FIB;
        struct FRD;
    }
    template<class T> class PLCF;
    template<class T> class PLCFIterator;
	class AbstractOLEStreamReader;
    struct FootnoteData;

    /**
     * @internal
     */
    class Footnotes97
    {
    public:
		Footnotes97( AbstractOLEStreamReader* tableStream, const Word97::FIB& fib );
        ~Footnotes97();

        /**
         * Get the FootnoteData for the footnote/endnote at @param globalCP.
         * The @param ok flag is true if a footnote/endnote has been found.
         * If @þaram ok is false no footnote/endnote has been found and the
         * returned FootnoteData structure is invalid.
         */
        FootnoteData footnote( U32 globalCP, bool& ok );

        /**
         * Returns the global CP of the next footnote reference,
         * 0xffffffff if none exists.
         */
        U32 nextFootnote() const;
        /**
         * Returns the global CP of the next endnote reference,
         * 0xffffffff if none exists.
         */
        U32 nextEndnote() const;

    private:
        Footnotes97( const Footnotes97& rhs );
        Footnotes97& operator=( const Footnotes97& rhs );

        // Ugly, but helps to avoid code duplication
		void init( U32 fcRef, U32 lcbRef, U32 fcTxt, U32 lcbTxt, AbstractOLEStreamReader* tableStream,
                   PLCF<Word97::FRD>** ref, PLCFIterator<Word97::FRD>** refIt,
                   std::vector<U32>& txt, std::vector<U32>::const_iterator& txtIt );

        PLCF<Word97::FRD>* m_footnoteRef;
        PLCFIterator<Word97::FRD>* m_footnoteRefIt;
        std::vector<U32> m_footnoteTxt;
        std::vector<U32>::const_iterator m_footnoteTxtIt;

        PLCF<Word97::FRD>* m_endnoteRef;
        PLCFIterator<Word97::FRD>* m_endnoteRefIt;
        std::vector<U32> m_endnoteTxt;
        std::vector<U32>::const_iterator m_endnoteTxtIt;
    };

} // namespace wvWare

#endif // FOOTNOTES97_H
