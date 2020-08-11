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

#ifndef LISTS_H
#define LISTS_H

#include <vector>
#include <utility>
#include "global.h"
#include "ustring.h"
#include "sharedptr.h"

namespace wvWare
{
    class STTBF;
	class AbstractOLEStreamReader;
    class StyleSheet;
    class ListLevel;
    class ListData;
    class ListFormatOverride;
    class ListFormatOverrideLVL;
    class ListInfoProvider;
    namespace Word97
    {
        struct FIB;
        struct PAP;
        struct CHP;
    }

    /**
     * Please refer to the documentation of the ListInfo::text method.
     */
    struct ListText
    {
        ListText();
        ~ListText();

        UString text;
        SharedPtr<Word97::CHP> chp;
    };


    /**
     * This class provides an abstraction for the list information in
     * Word6 and Word97 (they are totally different).
     *
     * I'm providing a slim API here and there would be some more information
     * inside the internal structures. Please speak up if you're missing
     * something, don't try to hack around this artificial limitation (Werner)
     */
    class ListInfo
    {
    public:
        ListInfo( Word97::PAP& pap, ListInfoProvider& listInfoProvider );

        /**
         * The istd linked to the current list/level, istdNil (4095) if none (LSFT::rgistd)
         */
        U16 linkedIstd() const { return m_linkedIstd; }
        /**
         * Returns whether the list counter should be restarted in new sections. (LSFT::fRestartHdn)
         */
        bool restartingCounter() const { return m_restartingCounter; }

        /**
         * The list starts at this value at the current level (LVLF::iStartAt)
         */
        S32 startAt() const { return m_startAt.first; }
        /**
         * This flag tells you whether some list format override set a new start value.
         * If this method returns true you have to restart the counter (initialized
         * with the startAt value)
         */
        bool startAtOverridden() const { return m_startAt.second; }
        /**
         * Number format code (see ANLD) (LVLF::nfc)
         */
        U8 numberFormat() const { return m_numberFormat; }
        /**
         * Alignment of the list number (LVLF::jc)
         */
        U8 alignment() const { return m_alignment; }
        /**
         * (LVLF::fLegal)
         */
        bool isLegal() const { return m_isLegal; }
        /**
         * (LVLF::fNoRestart)
         */
        bool notRestarted() const { return m_notRestarted; }
        /**
         * (LVLF::fPrev)
         */
        bool prev() const { return m_prev; }
        /**
         * (LVLF::fPrevSpace)
         */
        bool prevSpace() const { return m_prevSpace; }
        /**
         * (LVLF::fWord6)
         */
        bool isWord6() const { return m_isWord6; }

        /**
         * The most important method, returning the text template and the
         * corresponding CHP.
         *
         * The returned string contains place holders for the real list
         * counter text. The place holders are the values 0...8, representing
         * the corresponding list levels (pap->ilvl). To illustrate that,
         * consider the following example (<0>, <1>,... represent the ASCII
         * values 0, 1,...):
         *     "<0>.<1>.<2>)"
         * The <0> should be replaced with the current counter value of level 0,
         * then we should display a '.', <1> should be the counter value of level 1,
         * and so forth.
         *
         * The CHP provides the character formatting properties; information about
         * the alignment and optional spaces/tabs after the counter text is
         * also available here (alignment, followingChar,...)
         */
        const ListText& text() const { return m_text; }

        /**
         * The character following the list number (LVLF::ixchFollow)
         */
        U8 followingChar() const { return m_followingChar; }

        /**
         * In order to help users to detect when a new list starts
         * we also provide access to the (internal) unique ID of a list.
         * Returns 0 if it hasn't been initailized.
         */
        S32 lsid() const { return m_lsid; }

        /**
         * Debugging...
         */
        void dump() const;

    private:
        ListInfo& operator=( const ListInfo& rhs );

        U16 m_linkedIstd;
        bool m_restartingCounter;
        std::pair<S32, bool> m_startAt;
        U8 m_numberFormat;
        U8 m_alignment;
        bool m_isLegal;
        bool m_notRestarted;
        bool m_prev;
        bool m_prevSpace;
        bool m_isWord6;
        ListText m_text;
        U8 m_followingChar;
        S32 m_lsid;
    };


    /**
     * An internal class managing the list information read from the file.
     * Word versions before 8 don't have any explicit list information in
     * the table stream, it's just a part of the PAP (and it's way less
     * powerful). An object of this class will act as proxy for that
     * information in this case.
     */
    class ListInfoProvider
    {
        friend class ListInfo;
    public:
        /**
         * This constructor assumes that this is a list info provider
         * for a Word 6 document. It will just act as a proxy for the information
         * inside pap.anld. To make that work you have to set the ANLD structure
         * every time the PAP changes, using setWord6StylePAP()
         */
        ListInfoProvider( const StyleSheet* styleSheet );
        /**
         * This constructor reads the structures from the table stream of a
         * Word 97 document.
         */
		ListInfoProvider( AbstractOLEStreamReader* tableStream, const Word97::FIB& fib, const StyleSheet* styleSheet );
        ~ListInfoProvider();

        /**
         * Returns true if the passed ilfo is in a valid range.
         */
        bool isValid( S16 ilfo, U8 nLvlAnm ) const;

    private:
        ListInfoProvider( const ListInfoProvider& rhs );
        ListInfoProvider& operator=( const ListInfoProvider& rhs );

        /**
         * @internal
         * This function tries to get the list information (if any) from the
         * given PAP. As the list info handling is completely different in Word6
         * and in Word97 we have one additional level of indirection here (to
         * keep the user of the library from fighting multiple versions).
         * In case there's no list information for that paragraph it returns
         * false.
         *
         * Take care, these methods modify the passed PAP structure!!
         */
        bool setPAP( Word97::PAP* pap );

		void readListData( AbstractOLEStreamReader* tableStream, const U32 endOfLSTF );
		void readListFormatOverride( AbstractOLEStreamReader* tableStream );
		void readListNames( AbstractOLEStreamReader* tableStream );
		void eatLeading0xff( AbstractOLEStreamReader* tableStream );

        void processOverride( ListFormatOverride* lfo );
        void convertCompatANLD();
        ListData* findLST( S32 lsid );

        const ListLevel* formattingListLevel() const;
        std::pair<S32, bool> startAt();
        ListText text() const;

        std::vector<ListData*> m_listData;
        std::vector<ListFormatOverride*> m_listFormatOverride;
        STTBF* m_listNames;

        Word97::PAP* m_pap; // we don't own that one!
        const StyleSheet* const m_styleSheet; // needed to determine the CHP

        ListFormatOverrideLVL* m_currentLfoLVL;
        const ListData* m_currentLst;

        // We have to keep track of the version, as Word radically changed
        // it's way to handle lists from version 7 to 8.
        const WordVersion m_version;
    };

} // namespace wvWare

#endif // LISTS_H
