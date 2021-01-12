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

#ifndef STYLES_H
#define STYLES_H

#include "word97_generated.h"

namespace wvWare
{

class AbstractOLEStreamReader;

namespace Word97
{

/**
 * STyle Definition (STD)
 */
struct STD
{
    /**
     * Creates an empty STD structure and sets the defaults
     */
    STD();
    /**
     * Simply calls read(...)
     */
	STD( U16 baseSize, U16 totalSize, AbstractOLEStreamReader* stream, bool preservePos = false );
    /**
     * Attention: This struct allocates memory on the heap
     */
    STD( const STD& rhs );
    ~STD();

    STD& operator=( const STD& rhs );

    /**
     * This method reads the STD structure from the stream.
     * If  preservePos is true we push/pop the position of
     * the stream to save the state. If it's false the state
     * of stream will be changed!
     */
	bool read( U16 baseSize, U16 totalSize, AbstractOLEStreamReader* stream, bool preservePos = false );

    /**
     * Set all the fields to the inital value (default is 0)
     */
    void clear();

    // Data
    /**
     * invariant style identifier
     */
    U16 sti:12;

    /**
     * spare field for any temporary use, always reset back to zero!
     */
    U16 fScratch:1;

    /**
     * PHEs of all text with this style are wrong
     */
    U16 fInvalHeight:1;

    /**
     * UPEs have been generated
     */
    U16 fHasUpe:1;

    /**
     * std has been mass-copied; if unused at save time, style should be deleted
     */
    U16 fMassCopy:1;

    /**
     * style type code
     */
    U16 sgc:4;

    /**
     * base style
     */
    U16 istdBase:12;

    /**
     * # of UPXs (and UPEs)
     */
    U16 cupx:4;

    /**
     * next style
     */
    U16 istdNext:12;

    /**
     * offset to end of upx's, start of upe's
     */
    U16 bchUpe;

    /**
     * auto redefine style when appropriate
     */
    U16 fAutoRedef:1;

    /**
     * hidden from UI?
     */
    U16 fHidden:1;

    /**
     * unused bits
     */
    U16 unused8_3:14;

    /**
     * sub-names are separated by chDelimStyle
     */
    UString xstzName;

    U8* grupx;

    // Internal, for bookkeeping
    U16 grupxLen;

private:
    void clearInternal();
	void readStyleName( U16 baseSize, AbstractOLEStreamReader* stream );
}; // STD

bool operator==( const STD& lhs, const STD& rhs );
bool operator!=( const STD& lhs, const STD& rhs );

}  // namespace Word97


class StyleSheet;
class ParagraphProperties;
class ListInfoProvider;

// The structure to hold the UPE for character styles.
struct UPECHPX
{
    UPECHPX() : istd( 0 ), cb( 0 ), grpprl( 0 ) {}
    ~UPECHPX() { delete [] grpprl; }

    U16 istd;
    U8 cb;
    U8* grpprl;

private:
    UPECHPX( const UPECHPX& rhs );
    UPECHPX& operator=( const UPECHPX& rhs );
};

/**
 * This class represents one single style.
 */
// It would maybe be worth to optimize the construction a bit, like creating
// the PAP and the CHP on copying from the base style, and not before (TODO)
class Style
{
public:
    enum StyleType { sgcUnknown = 0, sgcPara = 1, sgcChp = 2 };

	Style( U16 baseSize, AbstractOLEStreamReader* tableStream, U16* ftc );
    ~Style();

    /**
     * The stylesheet can have "empty" slots
     */
    bool isEmpty() const { return m_isEmpty; }
    /**
     * Did we already unwrap the style?
     */
    bool isWrapped() const { return m_isWrapped; }

    /**
     * Unwrap the style and create a valid PAP/CHP
     */
    void unwrapStyle( const StyleSheet& stylesheet, WordVersion version );

    /**
     * Get the (unique?) sti of that style
     */
    U16 sti() const;

    /**
     * Get the type of the style (paragraph/character style)
     */
    StyleType type() const;

    /**
     * Style name
     */
    UString name() const;

    /**
     * Id of following style
     */
    U16 followingStyle() const;

    /*
     * For paragraph styles only
     */
    const ParagraphProperties& paragraphProperties() const;
    /*
     * For paragraph styles only
     */
    const Word97::CHP& chp() const;
    /*
     * For character styles only
     */
    const UPECHPX& upechpx() const;

private:
    Style( const Style& rhs );
    Style& operator=( const Style& rhs );

    // This helper method merges two CHPX structures to one
    // (needed for character styles)
    void mergeUpechpx( const Style* parentStyle, WordVersion version );

    bool m_isEmpty;
    bool m_isWrapped;
    Word97::STD* m_std;

    mutable ParagraphProperties *m_properties; // "mutable" in case someone goes mad
    mutable Word97::CHP *m_chp; //  with the styles. We have to create a default style
    mutable UPECHPX *m_upechpx; // to avoid crashes and still have to keep ownership!
};


/**
 * This class holds all the styles this Word document
 * defines. You can query styles by ID (sti) or index (istd)
 */
class StyleSheet
{
public:
	StyleSheet( AbstractOLEStreamReader* tableStream, U32 fcStshf, U32 lcbStshf );
    ~StyleSheet();

    /**
     * Return the number of styles.
     */
    unsigned int size() const;
    const Style* styleByIndex( U16 istd ) const;

    const Style* styleByID( U16 sti ) const;

    U16 indexByID( U16 sti, bool& ok ) const;

private:
    StyleSheet( const StyleSheet& rhs );
    StyleSheet& operator=( const StyleSheet& rhs );

    Word97::STSHI m_stsh;
    std::vector<Style*> m_styles;
};

}  // namespace wvWare

#endif // STYLES_H
