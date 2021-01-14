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

#ifndef HANDLERS_H
#define HANDLERS_H

#include "global.h"
#include "sharedptr.h"
#include "functordata.h"

namespace wvWare {

    class Parser9x;
    struct UChar;
    class UString;
    template<class ParserT, typename Data> class Functor;
    typedef Functor<Parser9x, HeaderData> HeaderFunctor;
    typedef Functor<Parser9x, TableRowData> TableRowFunctor;
    typedef Functor<Parser9x, FootnoteData> FootnoteFunctor;

    /**
     * This class allows to replace the character values of some
     * characters in a Word document, like non-breaking hyphens,...
     * during the processing. As we have to look at every single
     * character anyway, we can as well do single-character replacement
     * in case it's necessary. That way you don't have to scan all the
     * text again in the consumer filter.
     *
     * We use U8 instead of U16 as all interesting Word characters are
     * within 7bit ASCII range, the names of the methods should be
     * descriptive enough.
     */
    class InlineReplacementHandler
    {
    public:
        virtual ~InlineReplacementHandler();

        virtual U8 tab();
        virtual U8 hardLineBreak();
        virtual U8 columnBreak();
        virtual U8 nonBreakingHyphen();
        virtual U8 nonRequiredHyphen();
        virtual U8 nonBreakingSpace();
    };


    /**
     * The SubDocumentHandler is more or less a state-callback class. It informs
     * the consumer filter about changes of subdocuments like body, header,
     * footnote,...
     * This class is needed as we reuse one @ref TextHandler object for all the
     * text we find, no matter whether it's inside the main body or within a textbox.
     */
    class WV2_DLLEXPORT SubDocumentHandler
    {
    public:
        virtual ~SubDocumentHandler();

        /**
         * This method is called as soon as you call @ref Parser::parse. It indicates
         * the start of the body text (the main document text-flow).
         */
        virtual void bodyStart();
        /**
         * This method is called when all characters of the main body text
         * are processed.
         */
        virtual void bodyEnd();

        /**
         * Every time you invoke a @ref FoonoteFunctor this method will be called.
         * Note that @ref FoonoteFunctor is also emitted when we find endnotes.
         */
        virtual void footnoteStart();
        /**
         * Once the footnote characters are processed this method is called.
         */
        virtual void footnoteEnd();

        /**
         * For every section in a Word document a @ref HeaderFunctor is emitted.
         * Invoking the functor triggers the parsing of all headers inside the
         * section, this method is called to indicate that.
         */
        virtual void headersStart();
        /**
         * This method is called once all headers and footers of the current
         * section are processed.
         */
        virtual void headersEnd();
        /**
         * There are up to 6 headers/footers for every section. This method informs
         * you about the begin of a new header or footer of the given type.
         *
         * The default header/footer is the *Odd one. If you also get any *Even
         * header or footer the document's left and right pages have different
         * headers/footers. In case you get a *First header or footer the first
         * page of this section has a different header or footer.
         *
         * @param type The type of the header or footer. Always emitted in
         *             the order of definition in the enum
         */
        virtual void headerStart( HeaderData::Type type );
        /**
         * The end of the current header or footer.
         */
        virtual void headerEnd();
    };


    namespace Word97
    {
        struct TAP;
    }

    /**
     * The TableHandler class is the interface for all table related callbacks.
     * It informs about about starts and ends of table cells and rows.
     * Only invoking a table row functor triggers the parsing of the given table
     * row, and results in table(Row|Cell)(Start|End) callbacks.
     *
     * Word doesn't store information about table boundaries internally, but
     * it's quite easy to find out the start/end of a table, as all table
     * row functors are emitted in a row (pun intended). This means that if
     * you get a sequence of table row functors and then suddenly a
     * paragraphStart callback, you know that the table has ended.
     */
    class WV2_DLLEXPORT TableHandler
    {
    public:
        virtual ~TableHandler();

        /**
         * Indicates the start of a table row with the passed properties.
         */
        virtual void tableRowStart( SharedPtr<const Word97::TAP> tap );
        /**
         * Indicates the end of a table row.
         */
        virtual void tableRowEnd();
        /**
         * This method is invoked every time we start processing a new cell.
         */
        virtual void tableCellStart();
        /**
         * This method is invoked every time we reach a cell end.
         */
        virtual void tableCellEnd();
    };


    class ParagraphProperties;
    struct FLD;
    namespace Word97
    {
        struct CHP;
        struct SEP;
    }

    /**
     * The TextHandler class is the main worker in this filter. It tells you
     * about sections, paragraphs, and characters inside the document. If
     * you want to have a fast filter, try to make those methods fast, they
     * will get called very oftern.
     */
    class WV2_DLLEXPORT TextHandler
    {
    public:
        virtual ~TextHandler();

        //////////////////////////////////////////////////////////////////////
        // Section related callbacks...
        /**
         * Denotes the start of a section.
         * The section properties are passed in the @p sep argument.
         */
        virtual void sectionStart( SharedPtr<const Word97::SEP> sep );
        virtual void sectionEnd();
        /**
         * A page break within a section.
         */
        virtual void pageBreak();
        /**
         * Emitted when we found headers or footers. The default implementation
         * simply invokes the functor. This function is called right after the
         * start of a new section.
         */
        virtual void headersFound( const HeaderFunctor& parseHeaders );

        //////////////////////////////////////////////////////////////////////
        // Paragraph related callbacks...
        /**
         * Denotes the start of a paragraph.
         * The paragraph properties are passed in the @p paragraphProperties argument.
         */
        virtual void paragraphStart( SharedPtr<const ParagraphProperties> paragraphProperties );
        virtual void paragraphEnd();

        /**
         * runOfText is the main worker in this API. It provides a chunk of text with
         * the properties passed. A run of text inside a Word document may encompass
         * an entire document, but in our implementation we chop those runs of text into
         * smaller pieces.
         * A run of text will never cross paragraph boundaries, even if the paragraph
         * properties of the two or more containing paragraphs are the same. It would
         * probably be slightly faster to do that, but IMHO it's just not worth the
         * pain this would cause.
         * @param text The text of this run, UString holds it as UCS-2 host order string.
         * @param chp The character properties attached to this run of text
         */
        virtual void runOfText( const UString& text, SharedPtr<const Word97::CHP> chp );

        //////////////////////////////////////////////////////////////////////
        // Special characters...
        /**
         * The enum only contains characters which are "easy" to handle (as they don't
         * need any further information). All the other fSpec characters are/will be
         * handled via functors. The values match the ones found in the specification.
         *
         * The current list is just a first draft, I have no idea how many of them will
         * be moved out because they're more complex as expected.
         */
        enum SpecialCharacter { CurrentPageNumber = 0, LineNumber = 6,
                                AbbreviatedDate = 10, TimeHMS = 11,
                                CurrentSectionNumber = 12, AbbreviatedDayOfWeek = 14,
                                DayOfWeek = 15, DayShort = 16, HourCurrentTime = 22,
                                HourCurrentTimeTwoDigits = 23, MinuteCurrentTime = 24,
                                MinuteCurrentTimeTwoDigits = 25, SecondsCurrentTime = 26,
                                AMPMCurrentTime = 27, CurrentTimeHMSOld = 28,
                                DateM = 29, DateShort = 30, MonthShort = 33,
                                YearLong = 34, YearShort = 35,
                                AbbreviatedMonth = 36, MonthLong = 37,
                                CurrentTimeHMS = 38, DateLong = 39 };

        /**
         * Very special characters (bad, bad name) are the ones which need additional
         * information from the file (i.e. the plain "put the current date there" isn't sufficent).
         */
        enum VerySpecialCharacter { Picture = 1, FootnoteAuto = 2, FieldBegin = 19, FieldSeparator = 20,
                                    FieldEnd = 21, FieldEscapeChar = 92 };

        /**
         * This method passes the simple cases of special characters we find. More complex ones
         * will get their own callback.
         */
        virtual void specialCharacter( SpecialCharacter character, SharedPtr<const Word97::CHP> chp );

        /**
         * The parser found a footnote. The passed functor will trigger the parsing of this
         * footnote/endnote, the default implementation just emits the passed character
         * (unless it's an auto-numbered footnote, we won't emit ASCII 2 by default) with
         * runOfText (that it doesn't get lost if someone doesn't override this method) and
         * invokes the functor.
         */
        virtual void footnoteFound( FootnoteData::Type type, UChar character,
                                    SharedPtr<const Word97::CHP> chp, const FootnoteFunctor& parseFootnote );
        /**
         * This callback will get triggered when parsing a auto-numbered footnote.
         * The passed CHP is the character formatting information provided for the
         * "number"
         */
        virtual void footnoteAutoNumber( SharedPtr<const Word97::CHP> chp );

        /**
         * This callback indicates the start of a field. Fields consist of two parts, separated
         * by a field separator.
         * @param fld Describes the type of the field for live fields. May be 0!
         * @param chp The character properties of the field start (ASCII 19)
         */
        virtual void fieldStart( const FLD* fld, SharedPtr<const Word97::CHP> chp );
        /**
         * This callback separates the two parts of a field. The first part contains control
         * codes and keywords, the second part is the field result.
         * @param fld Describes the type of the field for live fields. May be 0!
         * @param chp The character properties of the field separator (ASCII 20)
         */
        virtual void fieldSeparator( const FLD* fld, SharedPtr<const Word97::CHP> chp );
        /**
         * The end of the field result is indicated by this callback, fields may be nested up
         * to 20 levels, so take care :-)
         * @param fld Describes the type of the field for live fields. May be 0!
         * @param chp The character properties of the field end (ASCII 21)
         */
        virtual void fieldEnd( const FLD* fld, SharedPtr<const Word97::CHP> chp );

        /**
         * This method is called every time we find a table row. The default
         * implementation invokes the functor, which triggers the parsing
         * process for the given table row.
         * @param tap the table row properties. Those are the same as the
         * ones you'll get when invoking the functor, but by having them here,
         * you can do some preprocessing on the whole table first.
         */
        virtual void tableRowFound( const TableRowFunctor& tableRow, SharedPtr<const Word97::TAP> tap );
    };

} // namespace wvWare

#endif // HANDLERS_H
