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

#ifndef PARSER9X_H
#define PARSER9X_H

#include "parser.h"
#include "word97_generated.h"

#include <string>
#include <list>
#include <stack>

namespace wvWare
{

    // Word97 so far. Is that different in Word95?
    const unsigned char CELL_MARK = 7;
    const unsigned char ROW_MARK = 7;
    const unsigned char TAB = 9;
    const unsigned char HARD_LINE_BREAK = 11;
    const unsigned char PAGE_BREAK = 12;
    const unsigned char SECTION_MARK = 12;
    const unsigned char PARAGRAPH_MARK = 13;
    const unsigned char COLUMN_BREAK = 14;
    const unsigned char FIELD_BEGIN_MARK = 19;
    const unsigned char FIELD_SEPARATOR = 20;
    const unsigned char FIELD_END_MARK = 21;
    const unsigned char NON_BREAKING_HYPHEN = 30;
    const unsigned char NON_REQUIRED_HYPHEN = 31;
    const unsigned char SPACE = 32;
    const unsigned char BREAKING_HYPHEN = 45;
    const unsigned char NON_BREAKING_SPACE = 160;
    const unsigned char FIELD_ESCAPE_CHAR = '\\';
    const unsigned char FORMULA_MARK = '\\';

    // Special chars (fSpec==1)
    const unsigned char SPEC_CURRENT_PAGE_NUMBER = 0;
    const unsigned char SPEC_PICTURE = 1;
    const unsigned char SPEC_AUTONUM_FOOTNOTE_REF = 2;
    const unsigned char SPEC_FOOTNOTE_SEPARATOR = 3;
    const unsigned char SPEC_FOOTNOTE_CONTINUATION = 4;
    const unsigned char SPEC_ANNOTATION_REF = 5;
    const unsigned char SPEC_LINE_NUMBER = 6;
    const unsigned char SPEC_HAND_ANNOTATION_PIC = 7;
    const unsigned char SPEC_DRAWN_OBJECT = 8;
    const unsigned char SPEC_ABBREV_DATE = 10;
    const unsigned char SPEC_TIME_HMS = 11;
    const unsigned char SPEC_CURRENT_SECTION_NUMBER = 12;
    const unsigned char SPEC_ABBREV_WEEKDAY = 14;
    const unsigned char SPEC_WEEKDAY = 15;
    const unsigned char SPEC_DAY_SHORT = 16;
    const unsigned char SPEC_CURRENT_HOUR = 22;
    const unsigned char SPEC_CURRENT_HOUR_TWODIG = 23;
    const unsigned char SPEC_CURRENT_MINUTE = 24;
    const unsigned char SPEC_CURRENT_MINUTE_TWODIG = 25;
    const unsigned char SPEC_CURRENT_SECONDS = 26;
    const unsigned char SPEC_CURRENT_AMPM = 27;
    const unsigned char SPEC_CURRENT_TIME_HMS = 28;
    const unsigned char SPEC_DATE_M = 29;
    const unsigned char SPEC_DATE_SHORT = 30;
    const unsigned char SPEC_MONTH_SHORT = 33;
    const unsigned char SPEC_YEAR_LONG = 34;
    const unsigned char SPEC_YEAR_SHORT = 35;
    const unsigned char SPEC_MONTH_ABBREV = 36;
    const unsigned char SPEC_MONTH_LONG = 37;
    const unsigned char SPEC_CURRENT_TIME_HM = 38;
    const unsigned char SPEC_DATE_LONG = 39;
    const unsigned char SPEC_MERGE_HELPER = 41;


    class Properties97;
    class ListInfoProvider;
    class FontCollection;
    class TextConverter;
    class Fields;
    class Headers;
    class Footnotes97;
    template<class T> class PLCF;

    // Helper structures for the Functor-based approach
    struct HeaderData;
    struct FootnoteData;
    struct TableRowData;

    /**
     * This class should contain all the common functionality shared
     * among the Word9[5|7] parsers.
     */
    class Parser9x : public Parser
    {
    public:
		Parser9x( AbstractOLEStorage* storage, AbstractOLEStreamReader* wordDocument, const Word97::FIB& fib );
        virtual ~Parser9x();

        /**
         * The main parsing method
         */
        virtual bool parse();

        virtual const Word97::FIB& fib() const;
        virtual const Word97::DOP& dop() const;

        /**
         * Get the font family name structure for a given ftc.
         */
        virtual const Word97::FFN& font( S16 ftc ) const;

        /**
         * Get the associated strings (author, title,...).
         * Not cached.
         */
        virtual AssociatedStrings associatedStrings();

        virtual const StyleSheet& styleSheet() const;

        // This part of the public API is only visible to the Functor classes,
        // as the "outside world" only sees the public API of Parser. The Functors
        // allow to delay the parsing of certain text inside the file (e.g. headers)
        // and trigger parsing at any point (as long as the parser exists).
        //
        // In case you want to add a new method here, please obey the following guidelines:
        //    - Executing the method mustn't change the state of the parser (i.e. save and
        //      restore the state!)
        //    - Be very careful, these calls can possibly be triggered at any time
        void parseHeaders( const HeaderData& data );
        void parseFootnote( const FootnoteData& data );
        void parseTableRow( const TableRowData& data );

    protected:
        // First all variables which don't change their state during
        // the parsing process. We don't have to save and restore those.
        const Word97::FIB m_fib;

		AbstractOLEStreamReader* m_table; // table stream ('WordDocument' for Word 6+95 and
                                  // the real table stream for Word 97+)
		AbstractOLEStreamReader* m_data;  // data stream (if any, most of the time 0)

        Properties97* m_properties;
        Headers* m_headers;

        // From here on we have all variables which change their state depending
        // on the parsed content. These variables have to be saved and restored
        // to make the parsing code reentrant.

    private:
        // Don't copy or assign us
        Parser9x( const Parser9x& rhs );
        Parser9x& operator=( const Parser9x& rhs );

        // Uniquely represents a position inside a complex file. Used to map a CP to a Position
        struct Position
        {
            // Start position
            Position( U32 p, U32 o ) : piece( p ), offset( o ) {}
            // Constructs a Position from a CP
            Position( U32 cp, const PLCF<Word97::PCD>* plcfpcd );

            U32 piece;    // The piece number (0-based index)
            U32 offset;   // The CP offset within the piece
        };

        // Represents a chunk of text. This is a part of a (or a whole) paragraph
        // contained in one text piece. A paragraph consists of at least one Chunk.
        // We don't store the paragraph/section mark, and in case only the paragraph
        // mark sits in a different piece than the rest of the paragraph we just store
        // an empty string for this chunk.
        struct Chunk
        {
            Chunk( const UString& text, const Position& position, U32 startFC, bool isUnicode ) :
                m_text( text ), m_position( position ), m_startFC( startFC ), m_isUnicode( isUnicode ) {}

            UString m_text;
            Position m_position;
            U32 m_startFC;
            bool m_isUnicode;
        };
        // Represents a paragraph consisting of at least one Chunk. Right now it's only
        // a typedef, maybe we need more than that later on
        typedef std::list<Chunk> Paragraph;

        // We have to keep track of the current parsing mode (e.g. are we skimming tables
        // or are we parsing them?)
        enum ParsingMode { Default, Table };

        // "Callbacks" for the 95/97 parsers
        // ##### TODO

        // Private helper methods
        std::string tableStream() const;
        void init();
        bool readPieceTable();
        void fakePieceTable();

        bool parseBody();

        // Expects m_remainingChars to be set correctly, changes the state of m_wordDocument,...
        void parseHelper( Position startPos );
        template<typename String> void processPiece( String* string, U32 fc, U32 limit, const Position& position );
        // These helper methods are a cheap trick to "configure" parts of the template code by
        // plain old overloading. It's just a matter of compressed vs. real unicode (1 vs. 2 bytes)
        UString processPieceStringHelper( XCHAR* string, unsigned int start, unsigned int index ) const;
        UString processPieceStringHelper( U8* string, unsigned int start, unsigned int index ) const;
        // Processes the current contents of the Paragraph structure and clears it when it's done
        void processParagraph( U32 fc );
        void processChunk( const Chunk& chunk, SharedPtr<const Word97::CHP> chp,
                           U32 length, U32 index, U32 currentStart );
        void processRun( const Chunk& chunk, SharedPtr<const Word97::CHP> chp,
                         U32 length, U32 index, U32 currentStart );

        void processSpecialCharacter( UChar character, U32 globalCP, SharedPtr<const Word97::CHP> chp );
        void processFootnote( UChar character, U32 globalCP, SharedPtr<const Word97::CHP> chp );

        // Helper methods to gather and emit the information needed for the functors
        void emitHeaderData( SharedPtr<const Word97::SEP> sep );
        void emitPictureData( SharedPtr<const Word97::CHP> chp );

        void parseHeader( const HeaderData& data, unsigned char mask );

        void saveState( U32 newRemainingChars, SubDocument newSubDocument, ParsingMode newParsingMode = Default );
        void restoreState();

        // Maps the global CP (as found in the piece table) to the local CP
        // coordinate space of the corresponding sub document
        U32 toLocalCP( U32 globalCP ) const;
        // Calculates the real FC and tells us whether it was unicode or not
        inline void realFC( U32& fc, bool& unicode ) const;
        // Helper method to use std::accumulate in the table handling code
        static int accumulativeLength( int len, const Chunk& chunk );

        // Private variables, no access needed in 95/97 code
        // First all variables which don't change their state during
        // the parsing process. We don't have to save and restore those.
        ListInfoProvider* m_lists;
        TextConverter* m_textconverter;
        Fields* m_fields;
        Footnotes97* m_footnotes;
        FontCollection* m_fonts;

        PLCF<Word97::PCD>* m_plcfpcd;     // piece table

        // From here on we have all variables which change their state depending
        // on the parsed content. These variables have to be saved and restored
        // to make the parsing code reentrant.
        Position* m_tableRowStart;      // If != 0 this represents the start of a table row
        U32 m_tableRowLength;           // Lenght of the table row (in characters). Only valid
        bool m_cellMarkFound;           // if m_tableRowStart != 0
        int m_remainingCells;           // The number of remaining cells for the processed row

        Paragraph* m_currentParagraph;

        U32 m_remainingChars;
        U32 m_sectionNumber;

        // Keeps track of the current sub document
        SubDocument m_subDocument;

        // We have to behave differently, depending whether we are parsing
        // a table or the "main" text, as we skim the table first
        ParsingMode m_parsingMode;

        // Needed to have reentrant parsing methods (to make the functor approach work)
        struct ParsingState
        {
            ParsingState( Position* tableRowS, U32 tableRowL, int remCells, Paragraph* parag,
                          U32 remChars, U32 sectionNum, SubDocument subD, ParsingMode mode ) :
                tableRowStart( tableRowS ), tableRowLength( tableRowL ), remainingCells( remCells ),
                paragraph( parag ), remainingChars( remChars ), sectionNumber( sectionNum ),
                subDocument( subD ), parsingMode( mode ) {}

            Position* tableRowStart;
            U32 tableRowLength;
            int remainingCells;
            Paragraph* paragraph;
            U32 remainingChars;
            U32 sectionNumber;   // not strictly necessary, but doesn't hurt
            SubDocument subDocument;
            ParsingMode parsingMode;
        };

        std::stack<ParsingState> oldParsingStates;
    };

    inline void Parser9x::realFC( U32& fc, bool& unicode ) const
    {
        if ( fc & 0x40000000 ) {
            fc = ( fc & 0xbfffffff ) >> 1;
            unicode = false;
        }
        else
            unicode = m_fib.nFib >= Word8nFib;
    }

} // namespace wvWare

#endif // PARSER9X_H
