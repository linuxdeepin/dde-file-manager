/* This file is part of the wvWare 2 project
   Copyright (C) 2001 Werner Trobin <trobin@kde.org>

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

// This code is generated from the Microsoft HTML specification of the
// WinWord format. Do NOT edit this code, but fix the spec or the script
// generating the sources.
// For information about the script and the "hidden features" please read
// the comments at the begin of the script.

// If you find bugs or strange behavior please contact Werner Trobin
// <trobin@kde.org>

#ifndef WORD95_GENERATED_H
#define WORD95_GENERATED_H

#include "global.h"
#include "sharedptr.h"
#include "utilitiesrename.h"

namespace wvWare {

class AbstractOLEStreamReader;
class StyleSheet;
class Style;

namespace Word95 {


/**
 * Date and Time (internal date format) (DTTM)
 */
struct DTTM {
    /**
     * Creates an empty DTTM structure and sets the defaults
     */
    DTTM();
    /**
     * Simply calls read(...)
     */
	DTTM(AbstractOLEStreamReader *stream, bool preservePos=false);
    /**
     * Simply calls readPtr(...)
     */
    DTTM(const U8 *ptr);

    /**
     * This method reads the DTTM structure from the stream.
     * If  preservePos is true we push/pop the position of
     * the stream to save the state. If it's false the state
     * of stream will be changed!
     */
	bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * This method reads the struct from a pointer
     */
    void readPtr(const U8 *ptr);

    /**
     * Set all the fields to the inital value (default is 0)
     */
    void clear();

    /**
     * Dumps all fields of this structure (for debugging)
     */
    void dump() const;

    /**
     * Converts the data structure to a string (for debugging)
     */
    std::string toString() const;

    // Data
    /**
     * minutes (0-59)
     */
    U16 mint:6;

    /**
     * hours (0-23)
     */
    U16 hr:5;

    /**
     * days of month (1-31)
     */
    U16 dom:5;

    /**
     * months (1-12)
     */
    U16 mon:4;

    /**
     * years (1900-2411)-1900
     */
    U16 yr:9;

    /**
     * weekday
     * Sunday=0
     * Monday=1
     * Tuesday=2
     * Wednesday=3
     * Thursday=4
     * Friday=5
     * Saturday=6
     */
    U16 wdy:3;

}; // DTTM

bool operator==(const DTTM &lhs, const DTTM &rhs);
bool operator!=(const DTTM &lhs, const DTTM &rhs);


/**
 * Property Modifier(variant 2) (PRM2)
 */
struct PRM2 {
    /**
     * Creates an empty PRM2 structure and sets the defaults
     */
    PRM2();
    /**
     * Simply calls read(...)
     */
	PRM2(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * This method reads the PRM2 structure from the stream.
     * If  preservePos is true we push/pop the position of
     * the stream to save the state. If it's false the state
     * of stream will be changed!
     */
	bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * Set all the fields to the inital value (default is 0)
     */
    void clear();

    // Data
    /**
     * set to 1 for variant 2
     */
    U16 fComplex:1;

    /**
     * index to a grpprl stored in CLX portion of file.
     */
    U16 igrpprl:15;

}; // PRM2

bool operator==(const PRM2 &lhs, const PRM2 &rhs);
bool operator!=(const PRM2 &lhs, const PRM2 &rhs);


/**
 * Property Modifier(variant 1) (PRM)
 */
struct PRM {
    /**
     * Creates an empty PRM structure and sets the defaults
     */
    PRM();
    /**
     * Simply calls read(...)
     */
	PRM(AbstractOLEStreamReader *stream, bool preservePos=false);
    /**
     * Simply calls readPtr(...)
     */
    PRM(const U8 *ptr);

    /**
     * This method reads the PRM structure from the stream.
     * If  preservePos is true we push/pop the position of
     * the stream to save the state. If it's false the state
     * of stream will be changed!
     */
	bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * This method reads the struct from a pointer
     */
    void readPtr(const U8 *ptr);

    /**
     * Set all the fields to the inital value (default is 0)
     */
    void clear();

    /**
     * This method returns a PRM2 created from the current PRM
     */
    PRM2 toPRM2() const;

    // Size of the structure
    static const unsigned int sizeOf;

    // Data
    /**
     * set to 0 for variant 1
     */
    U8 fComplex:1;

    /**
     * sprm opcode
     */
    U8 sprm:7;

    /**
     * sprm's second byte if necessary
     */
    U8 val;

}; // PRM

bool operator==(const PRM &lhs, const PRM &rhs);
bool operator!=(const PRM &lhs, const PRM &rhs);


/**
 * Shading Descriptor (SHD)
 */
struct SHD {
    /**
     * Creates an empty SHD structure and sets the defaults
     */
    SHD();
    /**
     * Simply calls read(...)
     */
	SHD(AbstractOLEStreamReader *stream, bool preservePos=false);
    /**
     * Simply calls readPtr(...)
     */
    SHD(const U8 *ptr);

    /**
     * This method reads the SHD structure from the stream.
     * If  preservePos is true we push/pop the position of
     * the stream to save the state. If it's false the state
     * of stream will be changed!
     */
	bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * This method reads the struct from a pointer
     */
    void readPtr(const U8 *ptr);

    /**
     * Set all the fields to the inital value (default is 0)
     */
    void clear();

    /**
     * Dumps all fields of this structure (for debugging)
     */
    void dump() const;

    /**
     * Converts the data structure to a string (for debugging)
     */
    std::string toString() const;

    // Data
    /**
     * foreground color (see chp.ico)
     */
    U16 icoFore:5;

    /**
     * background color (see chp.ico)
     */
    U16 icoBack:5;

    /**
     * shading pattern (see ipat table below)
     * 0 Automatic
     * 1 Solid
     * 2 5 Percent
     * 3 10 Percent
     * 4 20 Percent
     * 5 25 Percent
     * 6 30 Percent
     * 7 40 Percent
     * 8 50 Percent
     * 9 60 Percent
     * 10 70 Percent
     * 11 75 Percent
     * 12 80 Percent
     * 13 90 Percent
     * 14 Dark Horizontal
     * 15 Dark Vertical
     * 16 Dark Forward Diagonal
     * 17 Dark Backward Diagonal
     * 18 Dark Cross
     * 19 Dark Diagonal Cross
     * 20 Horizontal
     * 21 Vertical
     * 22 Forward Diagonal
     * 23 Backward Diagonal
     * 24 Cross
     * 25 Diagonal Cross
     * 35 2.5 Percent
     * 36 7.5 Percent
     * 37 12.5 Percent
     * 38 15 Percent
     * 39 17.5 Percent
     * 40 22.5 Percent
     * 41 27.5 Percent
     * 42 32.5 Percent
     * 43 35 Percent
     * 44 37.5 Percent
     * 45 42.5 Percent
     * 46 45 Percent
     * 47 47.5 Percent
     * 48 52.5 Percent
     * 49 55 Percent
     * 50 57.5 Percent
     * 51 62.5 Percent
     * 52 65 Percent
     * 53 67.5 Percent
     * 54 72.5 Percent
     * 55 77.5 Percent
     * 56 82.5 Percent
     * 57 85 Percent
     * 58 87.5 Percent
     * 59 92.5 Percent
     * 60 95 Percent
     * 61 97.5 Percent
     * 62 97 Percent
     */
    U16 ipat:6;

}; // SHD

bool operator==(const SHD &lhs, const SHD &rhs);
bool operator!=(const SHD &lhs, const SHD &rhs);


/**
 * Paragraph Height (PHE)
 */
struct PHE {
    /**
     * Creates an empty PHE structure and sets the defaults
     */
    PHE();
    /**
     * Simply calls read(...)
     */
	PHE(AbstractOLEStreamReader *stream, bool preservePos=false);
    /**
     * Simply calls readPtr(...)
     */
    PHE(const U8 *ptr);

    /**
     * This method reads the PHE structure from the stream.
     * If  preservePos is true we push/pop the position of
     * the stream to save the state. If it's false the state
     * of stream will be changed!
     */
	bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * This method reads the struct from a pointer
     */
	void readPtr(const U8 *ptr);

    /**
     * Set all the fields to the inital value (default is 0)
     */
    void clear();

    /**
     * Dumps all fields of this structure (for debugging)
     */
    void dump() const;

    /**
     * Converts the data structure to a string (for debugging)
     */
    std::string toString() const;

    // Size of the structure
    static const unsigned int sizeOf;

    // Data
    /**
     * reserved
     */
    U16 fSpare:1;

    /**
     * phe entry is invalid when == 1
     */
    U16 fUnk:1;

    /**
     * when 1, total height of paragraph is known but lines in paragraph have
     * different heights.
     */
    U16 fDiffLines:1;

    /**
     * reserved
     */
    U16 unused0_3:5;

    /**
     * when fDiffLines is 0 is number of lines in paragraph
     */
    U16 clMac:8;

    /**
     * width of lines in paragraph
     */
    U16 dxaCol;

    /**
     * When fDiffLines is 0, this is the height of every line in paragraph.in pixels (dylLine).
     * When fDiffLines is 1, this is the total height in pixels of the paragraph (dylHeight).
     * dylHeight and dylLine overlap (shaheed).
     */
    U16 dylLine_dylHeight;

}; // PHE

bool operator==(const PHE &lhs, const PHE &rhs);
bool operator!=(const PHE &lhs, const PHE &rhs);


/**
 * Border Code (BRC)
 */
struct BRC {
    /**
     * Creates an empty BRC structure and sets the defaults
     */
    BRC();
    /**
     * Simply calls read(...)
     */
	BRC(AbstractOLEStreamReader *stream, bool preservePos=false);
    /**
     * Simply calls readPtr(...)
     */
    BRC(const U8 *ptr);

    /**
     * This method reads the BRC structure from the stream.
     * If  preservePos is true we push/pop the position of
     * the stream to save the state. If it's false the state
     * of stream will be changed!
     */
	bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * This method reads the struct from a pointer
     */
	void readPtr(const U8 *ptr);

    /**
     * Set all the fields to the inital value (default is 0)
     */
    void clear();

    /**
     * Dumps all fields of this structure (for debugging)
     */
    void dump() const;

    /**
     * Converts the data structure to a string (for debugging)
     */
    std::string toString() const;

    // Size of the structure
    static const unsigned int sizeOf;

    // Data
    /**
     * When dxpLineWidth is 0, 1, 2, 3, 4, or 5, this field is the width of
     * a single line of border in units of 0.75 points.Each line in the border
     * is this wide (e.g. a double border is three lines).Must be nonzero when
     * brcType is nonzero.When dxpLineWidth is 6, it means that the border line
     * is dotted.When dxpLineWidth is 7, it means the border line is dashed.
     */
    U16 dxpLineWidth:3;

    /**
     * border type code
     * 0 none
     * 1 single
     * 2 thick
     * 3 double
     */
    U16 brcType:2;

    /**
     * when 1, border is drawn with shadow. Must be 0 when BRC is a substructure
     * of the TC
     */
    U16 fShadow:1;

    /**
     * color code (see chp.ico)
     */
    U16 ico:5;

    /**
     * width of space to maintain between border and text within border. Must
     * be 0 when BRC is a substructure of the TC.Stored in points for Windows.
     */
    U16 dxpSpace:5;

}; // BRC

bool operator==(const BRC &lhs, const BRC &rhs);
bool operator!=(const BRC &lhs, const BRC &rhs);


/**
 * Table Autoformat Look sPecifier (TLP)
 */
struct TLP {
    /**
     * Creates an empty TLP structure and sets the defaults
     */
    TLP();
    /**
     * Simply calls read(...)
     */
	TLP(AbstractOLEStreamReader *stream, bool preservePos=false);
    /**
     * Simply calls readPtr(...)
     */
    TLP(const U8 *ptr);

    /**
     * This method reads the TLP structure from the stream.
     * If  preservePos is true we push/pop the position of
     * the stream to save the state. If it's false the state
     * of stream will be changed!
     */
	bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * This method reads the struct from a pointer
     */
	void readPtr(const U8 *ptr);

    /**
     * Set all the fields to the inital value (default is 0)
     */
    void clear();

    /**
     * Dumps all fields of this structure (for debugging)
     */
    void dump() const;

    /**
     * Converts the data structure to a string (for debugging)
     */
    std::string toString() const;

    // Data
    /**
     * index to Word's table of table looks
     */
    U16 itl;

    /**
     * when ==1, use the border properties from the selected table look
     */
    U16 fBorders:1;

    /**
     * when ==1, use the shading properties from the selected table look
     */
    U16 fShading:1;

    /**
     * when ==1, use the font from the selected table look
     */
    U16 fFont:1;

    /**
     * when ==1, use the color from the selected table look
     */
    U16 fColor:1;

    /**
     * when ==1, do best fit from the selected table look
     */
    U16 fBestFit:1;

    /**
     * when ==1, apply properties from the selected table look to the header
     * rows in the table
     */
    U16 fHdrRows:1;

    /**
     * when ==1, apply properties from the selected table look to the last
     * row in the table
     */
    U16 fLastRow:1;

    /**
     * when ==1, apply properties from the selected table look to the header
     * columns ofthe table
     */
    U16 fHdrCols:1;

    /**
     * when ==1, apply properties from the selected table look to the last
     * column ofthe table
     */
    U16 fLastCol:1;

    /**
     * unused
     */
    U16 unused2_9:7;

}; // TLP

bool operator==(const TLP &lhs, const TLP &rhs);
bool operator!=(const TLP &lhs, const TLP &rhs);


/**
 * Table Cell Descriptors (TC)
 */
struct TC {
    /**
     * Creates an empty TC structure and sets the defaults
     */
    TC();
    /**
     * Simply calls read(...)
     */
	TC(AbstractOLEStreamReader *stream, bool preservePos=false);
    /**
     * Simply calls readPtr(...)
     */
    TC(const U8 *ptr);

    /**
     * This method reads the TC structure from the stream.
     * If  preservePos is true we push/pop the position of
     * the stream to save the state. If it's false the state
     * of stream will be changed!
     */
	bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * This method reads the struct from a pointer
     */
	void readPtr(const U8 *ptr);

    /**
     * Set all the fields to the inital value (default is 0)
     */
    void clear();

    /**
     * Dumps all fields of this structure (for debugging)
     */
    void dump() const;

    /**
     * Converts the data structure to a string (for debugging)
     */
    std::string toString() const;

    // Size of the structure
    static const unsigned int sizeOf;

    // Data
    /**
     * set to 1 when cell is first cell of a range of cells that have been
     * merged. When a cell is merged, the display areas of the merged cells are
     * consolidated and the text within the cells is interpreted as belonging
     * to one text stream for purposes of calculating line breaks.
     */
    U16 fFirstMerged:1;

    /**
     * set to 1 when cell has been merged with preceding cell.
     */
    U16 fMerged:1;

    /**
     * reserved
     */
    U16 fUnused:14;

    /**
     * specification of the top border of a table cell
     */
    BRC brcTop;

    /**
     * specification of left border of table row
     */
    BRC brcLeft;

    /**
     * specification of bottom border of table row
     */
    BRC brcBottom;

    /**
     * specification f right border of table row.
     */
    BRC brcRight;

}; // TC

bool operator==(const TC &lhs, const TC &rhs);
bool operator!=(const TC &lhs, const TC &rhs);


/**
 * Drawing Primitive Header (Word) (DPHEAD)
 */
struct DPHEAD {
    /**
     * Creates an empty DPHEAD structure and sets the defaults
     */
    DPHEAD();
    /**
     * Simply calls read(...)
     */
	DPHEAD(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * This method reads the DPHEAD structure from the stream.
     * If  preservePos is true we push/pop the position of
     * the stream to save the state. If it's false the state
     * of stream will be changed!
     */
	bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * Set all the fields to the inital value (default is 0)
     */
    void clear();

    // Data
    /**
     * Drawn Primitive KindREVIEW davebu
     * 0x0000 = start of grouping of primitives (DO)
     * 0x0001 = line (DPLINE)
     * 0x0002 = textbox (DPTXBX)
     * 0x0003 = rectangle (DPRECT)
     * 0x0004 = arc (DPARC)
     * 0x0005 = ellipse (DPELLIPSE)
     * 0x0006 = polyline (DPPOLYLINE)
     * 0x0007 = callout textbox (DPCALLOUT)
     * 0x0008 = end of grouping of primitives
     * 0x0009 = sample primitve holding default values (DPSAMPLE)
     */
    U16 dpk;

    /**
     * size (count of bytes) of this DP
     */
    U16 cb;

    /**
     * These 2 points describe the rectangle enclosing this DP relative to
     * the origin of the DO
     */
    U16 xa;

    U16 ya;

    U16 dxa;

    U16 dya;

}; // DPHEAD

bool operator==(const DPHEAD &lhs, const DPHEAD &rhs);
bool operator!=(const DPHEAD &lhs, const DPHEAD &rhs);


/**
 * DP data for a textbox (DPTXBX)
 */
struct DPTXBX {
    /**
     * Creates an empty DPTXBX structure and sets the defaults
     */
    DPTXBX();
    /**
     * Simply calls read(...)
     */
	DPTXBX(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * This method reads the DPTXBX structure from the stream.
     * If  preservePos is true we push/pop the position of
     * the stream to save the state. If it's false the state
     * of stream will be changed!
     */
	bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * Set all the fields to the inital value (default is 0)
     */
    void clear();

    // Data
    /**
     * Common header for a drawing primitive
     */
    DPHEAD dphead;

    /**
     * LiNe Property Color -- RGB color value
     */
    U32 lnpc;

    /**
     * line property weight in twips
     */
    U16 lnpw;

    /**
     * line property style. See description in DPLINE.
     */
    U16 lnps;

    /**
     * FiLl Property Color ForeGround -- RGB color value
     */
    U32 dlpcFg;

    /**
     * FiLl Property Color BackGround -- RGB color value
     */
    U32 dlpcBg;

    /**
     * FiLl Property Pattern. REVIEW davebu
     */
    U16 flpp;

    /**
     * Shadow Property Intensity
     */
    U16 shdwpi;

    /**
     * x offset of shadow
     */
    U16 xaOffset;

    /**
     * y offset of shadow
     */
    U16 yaOffset;

    /**
     * 1 if the textbox has rounded corners
     */
    U16 fRoundCorners:1;

    /**
     * REVIEW davebu
     */
    U16 zaShape:15;

    /**
     * REVIEW davebu
     */
    U16 dzaInternalMargin;

}; // DPTXBX

bool operator==(const DPTXBX &lhs, const DPTXBX &rhs);
bool operator!=(const DPTXBX &lhs, const DPTXBX &rhs);


/**
 * DP data for a polyline (DPPOLYLINE)
 */
struct DPPOLYLINE {
    /**
     * Creates an empty DPPOLYLINE structure and sets the defaults
     */
    DPPOLYLINE();
    /**
     * Simply calls read(...)
     */
	DPPOLYLINE(AbstractOLEStreamReader *stream, bool preservePos=false);
    /**
     * Attention: This struct allocates memory on the heap
     */
    DPPOLYLINE(const DPPOLYLINE &rhs);
    ~DPPOLYLINE();

    DPPOLYLINE &operator=(const DPPOLYLINE &rhs);

    /**
     * This method reads the DPPOLYLINE structure from the stream.
     * If  preservePos is true we push/pop the position of
     * the stream to save the state. If it's false the state
     * of stream will be changed!
     */
	bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * Set all the fields to the inital value (default is 0)
     */
    void clear();

    // Data
    /**
     * Common header for a drawing primitive
     */
    DPHEAD dphead;

    /**
     * LiNe Property Color -- RGB color value
     */
    U32 lnpc;

    /**
     * line property weight in twips
     */
    U16 lnpw;

    /**
     * line property style. See description in DPLINE.
     */
    U16 lnps;

    /**
     * FiLl Property Color ForeGround -- RGB color value
     */
    U32 dlpcFg;

    /**
     * FiLl Property Color BackGround -- RGB color value
     */
    U32 dlpcBg;

    /**
     * FiLl Property Pattern. REVIEW davebu
     */
    U16 flpp;

    /**
     * Start EndPoint Property Style
     * 0 None
     * 1 Hollow
     * 2 Filled
     */
    U16 eppsStart:2;

    /**
     * Start EndPoint Property Weight
     */
    U16 eppwStart:2;

    /**
     * Start EndPoint Property length
     */
    U16 epplStart:2;

    U16 unused30_6:10;

    /**
     * End EndPoint Property Style
     */
    U16 eppsEnd:2;

    /**
     * End EndPoint Property Weight
     */
    U16 eppwEnd:2;

    /**
     * End EndPoint Property length
     */
    U16 epplEnd:2;

    U16 unused32_6:10;

    /**
     * Shadow Property Intensity
     */
    U16 shdwpi;

    /**
     * x offset of shadow
     */
    U16 xaOffset;

    /**
     * y offset of shadow
     */
    U16 yaOffset;

    /**
     * 1 if this is a polygon
     */
    U16 fPolygon:1;

    /**
     * count of points
     */
    U16 cpt:15;

    /**
     * These are the endpoints of the first line.
     */
    U16 xaFirst;

    U16 yaFirst;

    U16 xaEnd;

    U16 yaEnd;

    /**
     * An array of xa,ya pairs for the remaining points
     */
    U16 *rgpta;   //    U16 rgpta[];

private:
    void clearInternal();

}; // DPPOLYLINE

bool operator==(const DPPOLYLINE &lhs, const DPPOLYLINE &rhs);
bool operator!=(const DPPOLYLINE &lhs, const DPPOLYLINE &rhs);


/**
 * Table Properties (TAP)
 */
struct TAP : public Shared {
    /**
     * Creates an empty TAP structure and sets the defaults
     */
    TAP();
    /**
     * Simply calls read(...)
     */
	TAP(AbstractOLEStreamReader *stream, bool preservePos=false);
    /**
     * Attention: This struct allocates memory on the heap
     */
    TAP(const TAP &rhs);
    ~TAP();

    TAP &operator=(const TAP &rhs);

    /**
     * This method reads the TAP structure from the stream.
     * If  preservePos is true we push/pop the position of
     * the stream to save the state. If it's false the state
     * of stream will be changed!
     */
	bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * Set all the fields to the inital value (default is 0)
     */
    void clear();

    /**
     * Dumps all fields of this structure (for debugging)
     */
    void dump() const;

    /**
     * Converts the data structure to a string (for debugging)
     */
    std::string toString() const;

    // Data
    /**
     * justification code. specifies how table row should be justified within
     * its column.
     * 0 left justify
     * 1center
     * 2right justify
     * 3left and right justify
     */
    U16 jc;

    /**
     * measures half of the white space that will be maintained between textin
     * adjacent columns of a table row. A dxaGapHalf width of white space will
     * be maintained on both sides of a column boundary.
     */
    U16 dxaGapHalf;

    /**
     * when greater than 0. guarantees that the height of the table will be
     * at least dyaRowHeight high. When less than 0, guarantees that the height
     * of the table will be exactly absolute value of dyaRowHeight high.When 0,table
     * will be given a height large enough to representall of the text in all
     * of the cells of the table.
     */
    U16 dyaRowHeight;

    /**
     * when 1, table row may not be split across page bounds
     */
    U8 fCantSplit;

    /**
     * when 1, table row is to be used as the header of the table
     */
    U8 fTableHeader;

    /**
     * table look specifier (see TLP definition)
     */
    TLP tlp;

    /**
     * used internally by Word
     */
    U16 fCaFull:1;

    /**
     * used internally by Word
     */
    U16 fFirstRow:1;

    /**
     * used internally by Word
     */
    U16 fLastRow:1;

    /**
     * used internally by Word
     */
    U16 fOutline:1;

    /**
     * reserved
     */
    U16 unused12_4:12;

    /**
     * count of cells defined for this row. ItcMac must be >= 0 and less than
     * or equal to 32.
     */
    U16 itcMac;

    /**
     * used internally by Word
     */
    U16 dxaAdjust;

    /**
     * rgdxaCenter[0] is the left boundary of cell 0 measured relative to
     * margin.. rgdxaCenter[tap.itcMac - 1] is left boundary of last cell. rgdxaCenter[tap.itcMac]
     * is right boundary of last cell.
     */
    U16 *rgdxaCenter;   //    U16 rgdxaCenter[itcMac + 1];

    /**
     * array of table cell descriptors
     */
    TC *rgtc;   //    TC rgtc[itcMac];

    /**
     * array of cell shades
     */
    SHD *rgshd;   //    SHD rgshd[itcMac];

    /**
     * array of border defaults for cells
     */
    BRC rgbrcTable[6];

private:
    void clearInternal();

}; // TAP

bool operator==(const TAP &lhs, const TAP &rhs);
bool operator!=(const TAP &lhs, const TAP &rhs);


/**
 * Tab Descriptor (TBD)
 */
/* This structure has been commented out because we can't handle it correctly
 * Please don't try to fix it here in this file, but rather copy this broken
 * structure definition and fix it in some auxilliary file. If you want to
 * include that aux. file here, please change the template file.
 */
//struct TBD {
//    /**
//     * Creates an empty TBD structure and sets the defaults
//     */
//    TBD();
//    /**
//     * Simply calls read(...)
//     */
//    TBD(AbstractOLEStreamReader *stream, bool preservePos=false);

//    /**
//     * This method reads the TBD structure from the stream.
//     * If  preservePos is true we push/pop the position of
//     * the stream to save the state. If it's false the state
//     * of stream will be changed!
//     */
//    bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

//    /**
//     * Same as reading :)
//     */
//    //bool write(OLEStreamWriter *stream, bool preservePos=false) const;

//    /**
//     * Set all the fields to the inital value (default is 0)
//     */
//    void clear();

//    // Data
//    /**
//     * justification code
//     * 0 left tab
//     * 1 centered tab
//     * 2 right tab
//     * 3 decimal tab
//     * 4 bar
//     */
//    U8 jc:3;

//    /**
//     * tab leader code
//     * 0 no leader
//     * 1 dotted leader
//     * 2 hyphenated leader
//     * 3 single line leader
//     * 4 heavy line leader
//     */
//    U8 tlc:3;

//    /**
//     * reserved
//     */
//    U8 unused0_6:2;

//}; // TBD

//bool operator==(const TBD &lhs, const TBD &rhs);
//bool operator!=(const TBD &lhs, const TBD &rhs);


/**
 * Autonumbered List Data Descriptor (ANLD)
 */
struct ANLD {
    /**
     * Creates an empty ANLD structure and sets the defaults
     */
    ANLD();
    /**
     * Simply calls read(...)
     */
	ANLD(AbstractOLEStreamReader *stream, bool preservePos=false);
    /**
     * Simply calls readPtr(...)
     */
    ANLD(const U8 *ptr);

    /**
     * This method reads the ANLD structure from the stream.
     * If  preservePos is true we push/pop the position of
     * the stream to save the state. If it's false the state
     * of stream will be changed!
     */
	bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * This method reads the struct from a pointer
     */
    void readPtr(const U8 *ptr);

    /**
     * Set all the fields to the inital value (default is 0)
     */
    void clear();

    /**
     * Dumps all fields of this structure (for debugging)
     */
    void dump() const;

    /**
     * Converts the data structure to a string (for debugging)
     */
    std::string toString() const;

    // Data
    /**
     * number format code
     * 0 Arabic numbering
     * 1 Upper case Roman
     * 2 Lower case Roman
     * 3 Upper case Letter
     * 4 Lower case letter
     * 5 Ordinal
     */
    U8 nfc;

    /**
     * offset into anld.rgch that is the limit of the text that will be displayed
     * as the prefix of the autonumber text
     */
    U8 cxchTextBefore;

    /**
     * anld.cxchTextBefore will be the beginning offset of the text in the
     * anld.rgchthat will be displayed as the suffix of an autonumber. The sum
     * of anld.cxchTextBefore + anld.cxchTextAfter will be the limit of the autonumber
     * suffix in anld.rgch
     */
    U8 cxchTextAfter;

    /**
     * justification code
     * 0 left justify
     * 1 center
     * 2 right justify
     * 3 left and right justify
     */
    U8 jc:2;

    /**
     * when ==1, number generated will include previous levels (used for legal
     * numbering)
     */
    U8 fPrev:1;

    /**
     * when ==1, number will be displayed using a hanging indent
     */
    U8 fHang:1;

    /**
     * when ==1, boldness of number will be determined by anld.fBold.
     */
    U8 fSetBold:1;

    /**
     * when ==1, italicness of number will be determined by anld.fItalic
     */
    U8 fSetItalic:1;

    /**
     * when ==1, anld.fSmallCaps will determine whether number will be displayed
     * in small caps or not.
     */
    U8 fSetSmallCaps:1;

    /**
     * when ==1, anld.fCaps will determine whether number will be displayed
     * capitalized or not
     */
    U8 fSetCaps:1;

    /**
     * when ==1, anld.fStrike will determine whether the number will be displayed
     * using strikethrough or not.
     */
    U8 fSetStrike:1;

    /**
     * when ==1, anld.kul will determine the underlining state of the autonumber.
     */
    U8 fSetKul:1;

    /**
     * when ==1, autonumber will be displayed with a single prefixing space
     * character
     */
    U8 fPrevSpace:1;

    /**
     * determines boldness of autonumber when anld.fSetBold == 1.
     */
    U8 fBold:1;

    /**
     * determines italicness of autonumber when anld.fSetItalic == 1.
     */
    U8 fItalic:1;

    /**
     * determines whether autonumber will be displayed using small caps when
     * anld.fSetSmallCaps == 1.
     */
    U8 fSmallCaps:1;

    /**
     * determines whether autonumber will be displayed using caps when anld.fSetCaps
     * == 1.
     */
    U8 fCaps:1;

    /**
     * determines whether autonumber will be displayed using caps when anld.fSetStrike
     * == 1.
     */
    U8 fStrike:1;

    /**
     * determines whether autonumber will be displayed with underlining when
     * anld.fSetKul == 1.
     */
    U8 kul:3;

    /**
     * color of autonumber
     */
    U8 ico:5;

    /**
     * font code of autonumber
     */
    S16 ftc;

    /**
     * font half point size (or 0=auto)
     */
    U16 hps;

    /**
     * starting value (0 to 65535)
     */
    U16 iStartAt;

    /**
     * width of prefix text (same as indent)
     */
    U16 dxaIndent;

    /**
     * minimum space between number and paragraph
     */
    U16 dxaSpace;

    /**
     * number only 1 item per table cell
     */
    U8 fNumber1;

    /**
     * number across cells in table rows(instead of down)
     */
    U8 fNumberAcross;

    /**
     * restart heading number on section boundary
     */
    U8 fRestartHdn;

    /**
     * unused( should be 0)
     */
    U8 fSpareX;

    /**
     * characters displayed before/after autonumber
     */
    U8 rgchAnld[32];

}; // ANLD

bool operator==(const ANLD &lhs, const ANLD &rhs);
bool operator!=(const ANLD &lhs, const ANLD &rhs);


/**
 * Autonumber Level Descriptor (ANLV)
 */
struct ANLV {
    /**
     * Creates an empty ANLV structure and sets the defaults
     */
    ANLV();
    /**
     * Simply calls read(...)
     */
	ANLV(AbstractOLEStreamReader *stream, bool preservePos=false);
    /**
     * Simply calls readPtr(...)
     */
    ANLV(const U8 *ptr);

    /**
     * This method reads the ANLV structure from the stream.
     * If  preservePos is true we push/pop the position of
     * the stream to save the state. If it's false the state
     * of stream will be changed!
     */
	bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * This method reads the struct from a pointer
     */
	void readPtr(const U8 *ptr);

    /**
     * Set all the fields to the inital value (default is 0)
     */
    void clear();

    /**
     * Dumps all fields of this structure (for debugging)
     */
    void dump() const;

    /**
     * Converts the data structure to a string (for debugging)
     */
    std::string toString() const;

    // Size of the structure
    static const unsigned int sizeOf;

    // Data
    /**
     * number format code
     * 0 Arabic numbering
     * 1 Upper case Roman
     * 2 Lower case Roman
     * 3 Upper case Letter
     * 4 Lower case letter
     * 5 Ordinal
     */
    U8 nfc;

    /**
     * offset into anld.rgch that is the limit of the text that will be displayed
     * as the prefix of the autonumber text
     */
    U8 cxchTextBefore;

    /**
     * anld.cxchTextBefore will be the beginning offset of the text in the
     * anld.rgch that will be displayed as the suffix of an autonumber. The sum
     * of anld.cxchTextBefore + anld.cxchTextAfter will be the limit of the autonumber
     * suffix in anld.rgch
     */
    U8 cxchTextAfter;

    /**
     * justification code
     * 0 left justify
     * 1 center
     * 2 right justify
     * 3 left and right justify
     */
    U8 jc:2;

    /**
     * when ==1, number generated will include previous levels (used for legal
     * numbering)
     */
    U8 fPrev:1;

    /**
     * when ==1, number will be displayed using a hanging indent
     */
    U8 fHang:1;

    /**
     * when ==1, boldness of number will be determined by anld.fBold.
     */
    U8 fSetBold:1;

    /**
     * when ==1, italicness of number will be determined by anld.fItalic
     */
    U8 fSetItalic:1;

    /**
     * when ==1, anld.fSmallCaps will determine whether number will be displayed
     * in small caps or not.
     */
    U8 fSetSmallCaps:1;

    /**
     * when ==1, anld.fCaps will determine whether number will be displayed
     * capitalized or not
     */
    U8 fSetCaps:1;

    /**
     * when ==1, anld.fStrike will determine whether the number will be displayed
     * using strikethrough or not.
     */
    U8 fSetStrike:1;

    /**
     * when ==1, anld.kul will determine the underlining state of the autonumber.
     */
    U8 fSetKul:1;

    /**
     * when ==1, autonumber will be displayed with a single prefixing space
     * character
     */
    U8 fPrevSpace:1;

    /**
     * determines boldness of autonumber when anld.fSetBold == 1.
     */
    U8 fBold:1;

    /**
     * determines italicness of autonumber when anld.fSetItalic == 1.
     */
    U8 fItalic:1;

    /**
     * determines whether autonumber will be displayed using small caps when
     * anld.fSetSmallCaps == 1.
     */
    U8 fSmallCaps:1;

    /**
     * determines whether autonumber will be displayed using caps when anld.fSetCaps
     * == 1.
     */
    U8 fCaps:1;

    /**
     * determines whether autonumber will be displayed using caps when anld.fSetStrike
     * == 1.
     */
    U8 fStrike:1;

    /**
     * determines whetherautonumber will be displayed with underlining when
     * anld.fSetKul == 1.
     */
    U8 kul:3;

    /**
     * color of autonumber
     */
    U8 ico:5;

    /**
     * font code of autonumber
     */
    S16 ftc;

    /**
     * font half point size (or 0=auto)
     */
    U16 hps;

    /**
     * starting value (0 to 65535)
     */
    U16 iStartAt;

    /**
     * width of prefix text (same as indent)
     */
    U16 dxaIndent;

    /**
     * minimum space between number and paragraph
     */
    U16 dxaSpace;

}; // ANLV

bool operator==(const ANLV &lhs, const ANLV &rhs);
bool operator!=(const ANLV &lhs, const ANLV &rhs);


/**
 * BooKmark First descriptor (BKF)
 */
struct BKF {
    /**
     * Creates an empty BKF structure and sets the defaults
     */
    BKF();
    /**
     * Simply calls read(...)
     */
	BKF(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * This method reads the BKF structure from the stream.
     * If  preservePos is true we push/pop the position of
     * the stream to save the state. If it's false the state
     * of stream will be changed!
     */
	bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * Set all the fields to the inital value (default is 0)
     */
    void clear();

    // Data
    /**
     * index to BKL entry in plcfbkl that describes the ending position of
     * this bookmark in the CP stream.
     */
    S16 ibkl;

    /**
     * when bkf.fCol is 1, this is the index to the first column of a table
     * column bookmark.
     */
    U16 itcFirst:7;

    /**
     * when 1, this indicates that this bookmark is marking the range of a
     * Macintosh Publisher section.
     */
    U16 fPub:1;

    /**
     * when bkf.fCol is 1, this is the index to limit column of a table column
     * bookmark.
     */
    U16 itcLim:7;

    /**
     * when 1, this bookmark marks a range of columns in a table specified
     * by [bkf.itcFirst, bkf.itcLim).
     */
    U16 fCol:1;

}; // BKF

bool operator==(const BKF &lhs, const BKF &rhs);
bool operator!=(const BKF &lhs, const BKF &rhs);


/**
 * BooKmark Lim descriptor (BKL)
 */
struct BKL {
    /**
     * Creates an empty BKL structure and sets the defaults
     */
    BKL();
    /**
     * Simply calls read(...)
     */
	BKL(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * This method reads the BKL structure from the stream.
     * If  preservePos is true we push/pop the position of
     * the stream to save the state. If it's false the state
     * of stream will be changed!
     */
	bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * Set all the fields to the inital value (default is 0)
     */
    void clear();

    // Data
    /**
     * index to BKF entry in plcfbkf that
     */
    S16 ibkf;

}; // BKL

bool operator==(const BKL &lhs, const BKL &rhs);
bool operator!=(const BKL &lhs, const BKL &rhs);


/**
 * Border Code for Windows Word 1.0 (BRC10)
 */
struct BRC10 {
    /**
     * Creates an empty BRC10 structure and sets the defaults
     */
    BRC10();
    /**
     * Simply calls read(...)
     */
	BRC10(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * This method reads the BRC10 structure from the stream.
     * If  preservePos is true we push/pop the position of
     * the stream to save the state. If it's false the state
     * of stream will be changed!
     */
	bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * Set all the fields to the inital value (default is 0)
     */
    void clear();

    // Data
    /**
     * width of second line of border in pixels
     */
    U16 dxpLine2Width:3;

    /**
     * distance to maintain between both lines of borderin pixels
     */
    U16 dxpSpaceBetween:3;

    /**
     * width of first border line in pixels
     */
    U16 dxpLine1Width:3;

    /**
     * width of space to maintain between border and text within border. Must
     * be 0 when BRC is a substructure of the TC.
     */
    U16 dxpSpace:5;

    /**
     * when 1, border is drawn with shadow. Must be 0 when BRC10 is a substructure
     * of the TC.
     */
    U16 fShadow:1;

    /**
     * reserved
     */
    U16 fSpare:1;

}; // BRC10

bool operator==(const BRC10 &lhs, const BRC10 &rhs);
bool operator!=(const BRC10 &lhs, const BRC10 &rhs);


/**
 * Bin Table Entry (BTE)
 */
struct BTE {
    /**
     * Creates an empty BTE structure and sets the defaults
     */
    BTE();
    /**
     * Simply calls read(...)
     */
	BTE(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * This method reads the BTE structure from the stream.
     * If  preservePos is true we push/pop the position of
     * the stream to save the state. If it's false the state
     * of stream will be changed!
     */
	bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * Set all the fields to the inital value (default is 0)
     */
    void clear();

    // Size of the structure
    static const unsigned int sizeOf;

    // Data
    /**
     * Page Number for FKP
     */
    U16 pn;

}; // BTE

bool operator==(const BTE &lhs, const BTE &rhs);
bool operator!=(const BTE &lhs, const BTE &rhs);


/**
 * Character Properties (CHP)
 */
struct CHP : public Shared {
    /**
     * Creates an empty CHP structure and sets the defaults
     */
    CHP();
    /**
     * Simply calls read(...)
     */
	CHP(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * This method reads the CHP structure from the stream.
     * If  preservePos is true we push/pop the position of
     * the stream to save the state. If it's false the state
     * of stream will be changed!
     */
	bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * Set all the fields to the inital value (default is 0)
     */
    void clear();

    /**
     * Dumps all fields of this structure (for debugging)
     */
    void dump() const;

    /**
     * Converts the data structure to a string (for debugging)
     */
    std::string toString() const;

    // Data
    /**
     * text is bold when 1 , and not bold when 0.
     */
    U8 fBold:1;

    /**
     * italic when 1, not italic when 0
     */
    U8 fItalic:1;

    /**
     * when 1, text has been deleted and will be displayed with strikethrus
     * when revision marked text is to displayed
     */
    U8 fRMarkDel:1;

    /**
     * outlined when 1, not outlined when 0
     */
    U8 fOutline:1;

    /**
     * &lt;needs work>
     */
    U8 fFldVanish:1;

    /**
     * displayed with small caps when 1, no small caps when 0
     */
    U8 fSmallCaps:1;

    /**
     * displayed with caps when 1, no caps when 0
     */
    U8 fCaps:1;

    U8 fVanish:1;

    /**
     * when 1, text is newly typed since the last time revision marks have
     * been accepted and will be displayed with an underline when revision marked
     * text is to be displayed
     */
    U8 fRMark:1;

    /**
     * character is a Word special character when 1, not a special character
     * when 0
     */
    U8 fSpec:1;

    /**
     * displayed with strikethrough when 1, no strikethroughwhen 0
     */
    U8 fStrike:1;

    /**
     * embedded object when 1, not an embedded object when 0
     */
    U8 fObj:1;

    /**
     * character is drawn with a shdow when 1; drawn without shadow when 0
     */
    U8 fShadow:1;

    /**
     * character is displayed in lower case when 1. No case transformation
     * is performed when 0. This field may be set to 1 only when chp.fSmallCaps
     * is 1.
     */
    U8 fLowerCase:1;

    /**
     * when 1, chp.fcPic points to an FFDATA the data structure binary data
     * used by Word to describe a form field. chp.fData may only be 1 when chp.fSpec
     * is also 1 and the special character in the document stream that has this
     * property is a chPicture (0x01).
     */
    U8 fData:1;

    /**
     * when 1, chp.lTagObj specifies a particular object in the object stream
     * that specifies the particular OLE object in the stream that should be displayed
     * when the chPicture fSpec character that is tagged with the fOle2 is encountered.
     * chp.fOle2 may only be 1 when chp.fSpec is also 1 and the special character
     * in the document stream that has this property is a chPicture (0x01).
     */
    U8 fOle2:1;

    /**
     * Reserved
     */
    U16 unused2;

    /**
     * font code. The ftc is an index into the rgffn structure. The rgffn
     * entry indexed by ftc describes the font that will be used to display the
     * run of text described by the CHP.
     */
    U16 ftc;

    /**
     * font size in half points
     */
    U16 hps;

    /**
     * space following each character in the run expressed in twip units.
     */
    U16 dxaSpace;

    /**
     * superscript/subscript indices
     * 0 means no super/subscripting
     * 1 means text in run is superscrpted
     * 2 means text in run is subscripted
     */
    U8 iss:3;

    /**
     * reserved
     */
    U8 unused10_3:3;

    /**
     * used by Word internally, not stored in file
     */
    U8 fSysVanish:1;

    /**
     * reserved
     */
    U8 unused10_7:1;

    /**
     * color of text:
     * 0 Auto
     * 1 Black
     * 2 Blue
     * 3 Cyan
     * 4 Green
     * 5 Magenta
     * 6 Red
     * 7 Yellow
     * 8 White
     * 9 DkBlue
     * 10 DkCyan
     * 11 DkGreen
     * 12 DkMagenta
     * 13 DkRed
     * 14 DkYellow
     * 15 DkGray
     * 16 LtGray
     */
    U8 ico:5;

    /**
     * underline code:
     * 0 none
     * 1 single
     * 2 by word
     * 3 double
     * 4 dotted
     * 5 hidden
     */
    U8 kul:3;

    /**
     * super/subscript position in half points; positive means text is raised;
     * negative means text is lowered.
     */
    S16 hpsPos;

    /**
     * Language Name Language ID
     * 0x0401 Arabic
     * 0x0402 Bulgarian
     * 0x0403 Catalan
     * 0x0404 Traditional Chinese
     * 0x0804 Simplified Chinese
     * 0x0405 Czech
     * 0x0406 Danish
     * 0x0407 German
     * 0x0807 Swiss German
     * 0x0408 Greek
     * 0x0409 U.S. English
     * 0x0809 U.K. English
     * 0x0c09 Australian English
     * 0x040a Castilian Spanish
     * 0x080a Mexican Spanish
     * 0x040b Finnish
     * 0x040c French
     * 0x080c Belgian French
     * 0x0c0c Canadian French
     * 0x100c Swiss French
     * 0x040d Hebrew
     * 0x040e Hungarian
     * 0x040f Icelandic
     * 0x0410 Italian
     * 0x0810 Swiss Italian
     * 0x0411 Japanese
     * 0x0412 Korean
     * 0x0413 Dutch
     * 0x0813 Belgian Dutch
     * 0x0414 Norwegian - Bokmal
     * 0x0814 Norwegian - Nynorsk
     * 0x0415 Polish
     * 0x0416 Brazilian Portuguese
     * 0x0816 Portuguese
     * 0x0417 Rhaeto-Romanic
     * 0x0418 Romanian
     * 0x0419 Russian
     * 0x041a Croato-Serbian (Latin)
     * 0x081a Serbo-Croatian (Cyrillic)
     * 0x041b Slovak
     * 0x041c Albanian
     * 0x041d Swedish
     * 0x041e Thai
     * 0x041f Turkish
     * 0x0420 Urdu
     * 0x0421 Bahasa
     */
    U16 lid;

    /**
     * offset in document stream pointing to beginning of a picture when character
     * is a picture character (character is 0x01 and chp.fSpec is 1)
     * offset in document stream pointing to beginning of a picture when character
     * is an OLE1 object character (character is 0x20 and chp.fSpec is 1, chp.fOle2
     * is 0)
     * long word tag that identifies an OLE2 object in the object stream when
     * the character is an OLE2 object character. (character is 0x01 and chp.fSpec
     * is 1, chp.fOle2 is 1)
     */
    U32 fcPic_fcObj_lTagObj;

    /**
     * index to author IDs stored in hsttbfRMark. used when text in run was
     * newly typed or deleted when revision marking was enabled
     */
    U16 ibstRMark;

    /**
     * Date/time at which this run of text was entered/modified by the author.
     * (Only recorded whenrevision marking is on.)
     */
    DTTM dttmRMark;

    /**
     * reserved
     */
    U16 unused26;

    /**
     * index to character style descriptor in the stylesheet that tags this
     * run of text When istd is istdNormalChar (10 decimal), characters in run
     * are not affected by a character style. If chp.istd contains any other value,
     * chpx of the specified character style are applied to CHP for this run before
     * any other exceptional properties are applied.
     */
    U16 istd;

    /**
     * when chp.fSpec is 1 and the character recorded for the run in the document
     * stream is chSymbol (0x28), chp.ftcSym identifies the font code of the symbol
     * font that will be used to display the symbol character recorded in chp.chSym.
     * Just like chp.ftc, chp.ftcSym is an index into the rgffn structure.
     */
    U16 ftcSym;

    /**
     * when chp.fSpec is 1 and the character recorded for the run in the document
     * stream is chSymbol (0x28), the character stored chp.chSym will be displayed
     * using the font specified in chp.ftcSym.
     */
    U8 chSym;

    /**
     * when 1, the character set used to interpret the characters recorded
     * in the run identified by chp.chse is different from the native character
     * set for this document which is stored in fib.chse.
     */
    U8 fChsDiff;

    /**
     * an index to strings displayed as reasons for actions taken by Word?s
     * AutoFormat code
     */
    U16 idslRMReason;

    /**
     * hyphenation rule
     * 0 No hyphenation
     * 1Normal hyphenation
     * 2Add letter before hyphen
     * 3Change letter before hyphen
     * 4Delete letter before hyphen
     * 5Change letter after hyphen
     * 6Delete letter before the hyphen and change the letter preceding the
     * deleted character
     */
    U8 ysr;

    /**
     * the character that will be used to add or changea letter when chp.ysr
     * is 2,3, 5 or 6
     */
    U8 chYsr;

    /**
     * extended character set id
     * 0 characters in run should be interpreted using the ANSI set used by
     * Windows
     * 256 characters in run should be interpreted using the Macintosh character
     * set.
     */
    U16 chse;

    /**
     * kerning distance for characters in run recorded in half points
     */
    U16 hpsKern;

}; // CHP

bool operator==(const CHP &lhs, const CHP &rhs);
bool operator!=(const CHP &lhs, const CHP &rhs);


/**
 * Character Property Exceptions (CHPX)
 */
/* This structure has been commented out because we can't handle it correctly
 * Please don't try to fix it here in this file, but rather copy this broken
 * structure definition and fix it in some auxilliary file. If you want to
 * include that aux. file here, please change the template file.
 */
//struct CHPX {
//    /**
//     * Creates an empty CHPX structure and sets the defaults
//     */
//    CHPX();
//    /**
//     * Simply calls read(...)
//     */
//    CHPX(AbstractOLEStreamReader *stream, bool preservePos=false);
//    /**
//     * Attention: This struct allocates memory on the heap
//     */
//    CHPX(const CHPX &rhs);
//    ~CHPX();

//    CHPX &operator=(const CHPX &rhs);

//    /**
//     * This method reads the CHPX structure from the stream.
//     * If  preservePos is true we push/pop the position of
//     * the stream to save the state. If it's false the state
//     * of stream will be changed!
//     */
//    bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

//    /**
//     * Same as reading :)
//     */
//    //bool write(OLEStreamWriter *stream, bool preservePos=false) const;

//    /**
//     * Set all the fields to the inital value (default is 0)
//     */
//    void clear();

//    // Data
//    /**
//     * count of bytes of following data in CHPX.
//     */
//    U8 cb;

//    /**
//     * a list of the sprms that encode the differences between CHP for a run
//     * of text and the CHP generated by the paragraph and character styles that
//     * tag the run.
//     */
//    U8 *grpprl;   //    U8 grpprl[cb];

//private:
//    void clearInternal();

//}; // CHPX

//bool operator==(const CHPX &lhs, const CHPX &rhs);
//bool operator!=(const CHPX &lhs, const CHPX &rhs);


/**
 * Formatted Disk Page for CHPXs (CHPXFKP)
 */
/* This structure has been commented out because we can't handle it correctly
 * Please don't try to fix it here in this file, but rather copy this broken
 * structure definition and fix it in some auxilliary file. If you want to
 * include that aux. file here, please change the template file.
 */
//struct CHPXFKP {
//    /**
//     * Creates an empty CHPXFKP structure and sets the defaults
//     */
//    CHPXFKP();
//    /**
//     * Simply calls read(...)
//     */
//    CHPXFKP(AbstractOLEStreamReader *stream, bool preservePos=false);
//    /**
//     * Attention: This struct allocates memory on the heap
//     */
//    CHPXFKP(const CHPXFKP &rhs);
//    ~CHPXFKP();

//    CHPXFKP &operator=(const CHPXFKP &rhs);

//    /**
//     * This method reads the CHPXFKP structure from the stream.
//     * If  preservePos is true we push/pop the position of
//     * the stream to save the state. If it's false the state
//     * of stream will be changed!
//     */
//    bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

//    /**
//     * Same as reading :)
//     */
//    //bool write(OLEStreamWriter *stream, bool preservePos=false) const;

//    /**
//     * Set all the fields to the inital value (default is 0)
//     */
//    void clear();

//    // Data
//    /**
//     * Array of FCs. Each FC is the limit FC of a run of exception text.
//     */
//    U32 *rgfc;   //    U32 rgfc[];

//    /**
//     * an array of bytes where each byte is the word offset of aCHPX. If the
//     * byte stored is 0,there is no difference between run's character properties
//     * and the style's character properties
//     */
//    U8 *rgb;   //    U8 rgb[];

//    /**
//     * As new runs/paragraphs are recorded in the FKP,unused space is reduced
//     * by 5 if CHPX is already recorded and is reduced by5+sizeof(CHPX) if property
//     * is not already recorded.
//     */
//    U8 *unusedSpace;   //    U8 unusedSpace[];

//    /**
//     * grpchpx consists of all of the CHPXs stored in FKP concatenated end
//     * to end. Each CHPXis prefixed with a count of bytes which records its length.
//     */
//    U8 *grpchpx;   //    U8 grpchpx[];

//    /**
//     * count of runs for CHPX FKP,
//     */
//    U8 crun;

//private:
//    void clearInternal();

//}; // CHPXFKP

//bool operator==(const CHPXFKP &lhs, const CHPXFKP &rhs);
//bool operator!=(const CHPXFKP &lhs, const CHPXFKP &rhs);


/**
 * Drop Cap Specifier (DCS)
 */
struct DCS {
    /**
     * Creates an empty DCS structure and sets the defaults
     */
    DCS();
    /**
     * Simply calls read(...)
     */
	DCS(AbstractOLEStreamReader *stream, bool preservePos=false);
    /**
     * Simply calls readPtr(...)
     */
    DCS(const U8 *ptr);

    /**
     * This method reads the DCS structure from the stream.
     * If  preservePos is true we push/pop the position of
     * the stream to save the state. If it's false the state
     * of stream will be changed!
     */
	bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * This method reads the struct from a pointer
     */
    void readPtr(const U8 *ptr);

    /**
     * Set all the fields to the inital value (default is 0)
     */
    void clear();

    /**
     * Dumps all fields of this structure (for debugging)
     */
    void dump() const;

    /**
     * Converts the data structure to a string (for debugging)
     */
    std::string toString() const;

    // Data
    /**
     * default value 0
     * drop cap type
     * 0 no drop cap
     * 1 normal drop cap
     * 2 drop cap in margin
     */
    U8 fdct:3;

    /**
     * count of lines to drop
     */
    U8 lines:5;

    /**
     * reserved
     */
    U8 unused1;

}; // DCS

bool operator==(const DCS &lhs, const DCS &rhs);
bool operator!=(const DCS &lhs, const DCS &rhs);


/**
 * Drawing Object (Word) (DO)
 */
struct DO {
    /**
     * Creates an empty DO structure and sets the defaults
     */
    DO();
    /**
     * Simply calls read(...)
     */
	DO(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * This method reads the DO structure from the stream.
     * If  preservePos is true we push/pop the position of
     * the stream to save the state. If it's false the state
     * of stream will be changed!
     */
	bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * Set all the fields to the inital value (default is 0)
     */
    void clear();

    // Data
    /**
     * FC pointing to drawing object data
     */
    U32 fc;

    /**
     * Drawn Object Kind, currently this is always 0
     */
    U16 dok;

    /**
     * size (count of bytes) of the entire DO
     */
    U16 cb;

    /**
     * x position relative to anchor CP
     */
    U8 bx;

    /**
     * y position relative to anchor CP
     */
    U8 by;

    /**
     * height of DO
     */
    U16 dhgt;

    /**
     * 1 if the DO anchor is locked
     */
    U16 fAnchorLock:1;

    U16 unused8:15;

    /**
     * variable length array of drawing primitives
     */
    U8 rgdp;

}; // DO

bool operator==(const DO &lhs, const DO &rhs);
bool operator!=(const DO &lhs, const DO &rhs);


/**
 * Document Properties (DOP)
 */
struct DOP {
    /**
     * Creates an empty DOP structure and sets the defaults
     */
    DOP();
    /**
     * Simply calls read(...)
     */
	DOP(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * This method reads the DOP structure from the stream.
     * If  preservePos is true we push/pop the position of
     * the stream to save the state. If it's false the state
     * of stream will be changed!
     */
	bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * Set all the fields to the inital value (default is 0)
     */
    void clear();

    // Data
    /**
     * 1 when facing pages should be printed (default 0)
     */
    U16 fFacingPages:1;

    /**
     * 1 when widow control is in effect. 0 when widow control disabled. (default
     * 1)
     */
    U16 fWidowControl:1;

    /**
     * 1 when doc is a main doc for Print Merge Helper, 0 when not; default=0
     */
    U16 fPMHMainDoc:1;

    /**
     * Default line suppression storage
     * 0= form letter line suppression
     * 1= no line suppression
     * default=0
     */
    U16 grfSuppression:2;

    /**
     * footnote position code
     * 0 print as endnotes
     * 1 print at bottom of page
     * 2 print immediately beneath text
     * (default 1)
     */
    U16 fpc:2;

    /**
     * unused (default 0)
     */
    U16 unused0_7:1;

    /**
     * specification of document headers and footers. See explanation under
     * Headers and Footers topic. (default 0)
     */
    U16 grpfIhdt:8;

    /**
     * restart index for footnote
     * 0 don't restart note numbering
     * 1 restart for each section
     * 2 restart for each page
     * (default 0)
     */
    U16 rncFtn:2;

    /**
     * initial footnote number for document (default 1)
     */
    U16 nFtn:14;

    /**
     * when 1, indicates that information in the hplcpad should be refreshed
     * since outline has been dirtied
     */
    U8 fOutlineDirtySave:1;

    /**
     * reserved
     */
    U8 unused4_1:7;

    /**
     * when 1, Word believes all pictures recorded in the document were created
     * on a Macintosh
     */
    U8 fOnlyMacPics:1;

    /**
     * when 1, Word believes all pictures recorded in the document were created
     * in Windows
     */
    U8 fOnlyWinPics:1;

    /**
     * when 1, document was created as a print merge labels document
     */
    U8 fLabelDoc:1;

    /**
     * when 1, Word is allowed to hyphenate words that are capitalized. When
     * 0, capitalized may not be hyphenated
     */
    U8 fHyphCapitals:1;

    /**
     * when 1, Word will hyphenate newly typed text as a background task
     */
    U8 fAutoHyphen:1;

    U8 fFormNoFields:1;

    /**
     * when 1, Word will merge styles from its template
     */
    U8 fLinkStyles:1;

    /**
     * when 1, Word will mark revisions as the document is edited
     */
    U8 fRevMarking:1;

    /**
     * always make backup when document saved when 1.
     */
    U8 fBackup:1;

    U8 fExactCWords:1;

    U8 fPagHidden:1;

    U8 fPagResults:1;

    /**
     * when 1, annotations are locked for editing
     */
    U8 fLockAtn:1;

    /**
     * swap margins on left/right pages when 1.
     */
    U8 fMirrorMargins:1;

    /**
     * user has recommended that this doc be opened read-only when 1
     */
    U8 fReadOnlyRecommended:1;

    /**
     * when 1, use TrueType fonts by default (flag obeyed only when doc was
     * created by WinWord 2.x)
     */
    U8 fDfltTrueType:1;

    /**
     * when 1, file created with SUPPRESSTOPSPACING=YES in win.ini. (flag
     * obeyed only when doc was created by WinWord 2.x).
     */
    U8 fPagSuppressTopSpacing:1;

    /**
     * when 1, document is protected from edit operations
     */
    U8 fProtEnabled:1;

    /**
     * when 1, restrict selections to occur only within form fields
     */
    U8 fDispFormFldSel:1;

    /**
     * when 1, show revision markings on screen
     */
    U8 fRMView:1;

    /**
     * when 1, print revision marks when document is printed
     */
    U8 fRMPrint:1;

    U8 fWriteReservation:1;

    /**
     * when 1, the current revision marking state is locked
     */
    U8 fLockRev:1;

    /**
     * when 1, document contains embedded True Type fonts
     */
    U8 fEmbedFonts:1;

    /**
     * compatability option: when 1, don?t add automatic tab stops for hanging
     * indent
     */
    U16 copts_fNoTabForInd:1;

    /**
     * compatability option: when 1, don?t add extra space for raised or lowered
     * characters
     */
    U16 copts_fNoSpaceRaiseLower:1;

    /**
     * compatability option: when 1, supress the paragraph Space Before and
     * Space After options after a page break
     */
    U16 copts_fSuppressSpbfAfterPageBreak:1;

    /**
     * compatability option: when 1, wrap trailing spaces at the end of a
     * line to the next line
     */
    U16 copts_fWrapTrailSpaces:1;

    /**
     * compatability option: when 1, print colors as black on non-color printers
     */
    U16 copts_fMapPrintTextColor:1;

    /**
     * compatability option: when 1, don?t balance columns for Continuous
     * Section starts
     */
    U16 copts_fNoColumnBalance:1;

    U16 copts_fConvMailMergeEsc:1;

    /**
     * compatability option: when 1, supress extra line spacing at top of
     * page
     */
    U16 copts_fSupressTopSpacing:1;

    /**
     * compatability option: when 1, combine table borders like Word 5.x for
     * the Macintosh
     */
    U16 copts_fOrigWordTableRules:1;

    /**
     * compatability option: when 1, don?t blank area between metafile pictures
     */
    U16 copts_fTransparentMetafiles:1;

    /**
     * compatability option: when 1, show hard page or column breaks in frames
     */
    U16 copts_fShowBreaksInFrames:1;

    /**
     * compatability option: when 1, swap left and right pages on odd facing
     * pages
     */
    U16 copts_fSwapBordersFacingPgs:1;

    /**
     * reserved
     */
    U16 unused8_12:4;

    /**
     * (default 720 twips) default tab width
     */
    U16 dxaTab;

    U16 wSpare;

    /**
     * width of hyphenation hot zone measured in twips
     */
    U16 dxaHotZ;

    /**
     * number of lines allowed to have consecutive hyphens
     */
    U16 cConsecHypLim;

    /**
     * reserved
     */
    U16 wSpare2;

    /**
     * date and time document was created
     */
    DTTM dttmCreated;

    /**
     * date and time document was last revised
     */
    DTTM dttmRevised;

    /**
     * date and time document was last printed
     */
    DTTM dttmLastPrint;

    /**
     * number of times document has been revised since its creation
     */
    U16 nRevision;

    /**
     * time document was last edited
     */
    U32 tmEdited;

    /**
     * count of words tallied by last Word Count execution
     */
    U32 cWords;

    /**
     * count of characters tallied by last Word Count execution
     */
    U32 cCh;

    /**
     * count of pages tallied by last Word Count execution
     */
    U16 cPg;

    /**
     * count of paragraphs tallied by last Word Count execution
     */
    U32 cParas;

    /**
     * restart endnote number code
     * 0 don't restart endnote numbering
     * 1 restart for each section
     * 2 restart for each page
     */
    U16 rncEdn:2;

    /**
     * beginning endnote number
     */
    U16 nEdn:14;

    /**
     * endnote position code
     * 0 display endnotes at end of section
     * 3 display endnotes at end of document
     */
    U16 epc:2;

    /**
     * number format code for auto footnotes
     * 0 Arabic numbering
     * 1 Upper case Roman
     * 2 Lower case Roman
     * 3 Upper case Letter
     * 4 Lower case letter
     * 5 Ordinal
     */
    U16 nfcFtnRef:4;

    /**
     * number format code for auto endnotes
     * 0 Arabic numbering
     * 1 Upper case Roman
     * 2 Lower case Roman
     * 3 Upper case Letter
     * 4 Lower case letter
     * 5 Ordinal
     */
    U16 nfcEdnRef:4;

    /**
     * only print data inside of form fields
     */
    U16 fPrintFormData:1;

    /**
     * only save document data that is inside of a form field.
     */
    U16 fSaveFormData:1;

    /**
     * shade form fields
     */
    U16 fShadeFormData:1;

    /**
     * reserved
     */
    U16 unused54_13:2;

    /**
     * when 1, include footnotes and endnotes in word count
     */
    U16 fWCFtnEdn:1;

    /**
     * count of lines tallied by last Word Count operation
     */
    U32 cLines;

    /**
     * count of words in footnotes and endnotes tallied by last Word Count
     * operation
     */
    U32 cWordsFtnEnd;

    /**
     * count of characters in footnotes and endnotes tallied by last Word
     * Count operation
     */
    U32 cChFtnEdn;

    /**
     * count of pages in footnotes and endnotes tallied by last Word Count
     * operation
     */
    U16 cPgFtnEdn;

    /**
     * count of paragraphs in footnotes and endnotes tallied by last Word
     * Count operation
     */
    U32 cParasFtnEdn;

    /**
     * count of paragraphs in footnotes and endnotes tallied by last Word
     * Count operation
     */
    U32 cLinesFtnEdn;

    /**
     * document protection password key, only valid if dop.fProtEnabled, dop.fLockAtn
     * or dop.fLockRev are 1.
     */
    U32 lKeyProtDoc;

    /**
     * document view kind
     * 0 Normal view
     * 1 Outline view
     * 2 Page View
     */
    U16 wvkSaved:3;

    U16 wScaleSaved:9;

    U16 zkSaved:2;

    U16 unused82_14:2;

}; // DOP

bool operator==(const DOP &lhs, const DOP &rhs);
bool operator!=(const DOP &lhs, const DOP &rhs);


/**
 * DP data for an arc (DPARC)
 */
struct DPARC {
    /**
     * Creates an empty DPARC structure and sets the defaults
     */
    DPARC();
    /**
     * Simply calls read(...)
     */
	DPARC(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * This method reads the DPARC structure from the stream.
     * If  preservePos is true we push/pop the position of
     * the stream to save the state. If it's false the state
     * of stream will be changed!
     */
	bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * Set all the fields to the inital value (default is 0)
     */
    void clear();

    // Data
    /**
     * Common header for a drawing primitive
     */
    DPHEAD dphead;

    /**
     * LiNe Property Color -- RGB color value
     */
    U32 lnpc;

    /**
     * line property weight in twips
     */
    U16 lnpw;

    /**
     * line property style. See description in DPLINE.
     */
    U16 lnps;

    /**
     * FiLl Property Color ForeGround -- RGB color value
     */
    U32 dlpcFg;

    /**
     * FiLl Property Color BackGround -- RGB color value
     */
    U32 dlpcBg;

    /**
     * FiLl Property Pattern. REVIEW davebu
     */
    U16 flpp;

    /**
     * Shadow Property Intensity
     */
    U16 shdwpi;

    /**
     * x offset of shadow
     */
    U16 xaOffset;

    /**
     * y offset of shadow
     */
    U16 yaOffset;

    /**
     * REVIEW davebu
     */
    U16 fLeft:8;

    /**
     * REVIEW davebu
     */
    U16 fUp:8;

}; // DPARC

bool operator==(const DPARC &lhs, const DPARC &rhs);
bool operator!=(const DPARC &lhs, const DPARC &rhs);


/**
 * DP data for a callout textbox (DPCALLOUT)
 */
struct DPCALLOUT {
    /**
     * Creates an empty DPCALLOUT structure and sets the defaults
     */
    DPCALLOUT();
    /**
     * Simply calls read(...)
     */
	DPCALLOUT(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * This method reads the DPCALLOUT structure from the stream.
     * If  preservePos is true we push/pop the position of
     * the stream to save the state. If it's false the state
     * of stream will be changed!
     */
	bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * Set all the fields to the inital value (default is 0)
     */
    void clear();

    // Data
    /**
     * Common header for a drawing primitive
     */
    DPHEAD dphead;

    /**
     * REVIEW davebu flags
     */
    U16 unused12;

    /**
     * REVIEW davebu
     */
    U16 dzaOffset;

    /**
     * REVIEW davebu
     */
    U16 dzaDescent;

    /**
     * REVIEW davebu
     */
    U16 dzaLength;

    /**
     * DP for a textbox
     */
    DPTXBX dptxbx;

    /**
     * DP for a polyline
     */
    DPPOLYLINE dpPolyLine;

}; // DPCALLOUT

bool operator==(const DPCALLOUT &lhs, const DPCALLOUT &rhs);
bool operator!=(const DPCALLOUT &lhs, const DPCALLOUT &rhs);


/**
 * DP data for an ellipse (DPELLIPSE)
 */
struct DPELLIPSE {
    /**
     * Creates an empty DPELLIPSE structure and sets the defaults
     */
    DPELLIPSE();
    /**
     * Simply calls read(...)
     */
	DPELLIPSE(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * This method reads the DPELLIPSE structure from the stream.
     * If  preservePos is true we push/pop the position of
     * the stream to save the state. If it's false the state
     * of stream will be changed!
     */
	bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * Set all the fields to the inital value (default is 0)
     */
    void clear();

    // Data
    /**
     * Common header for a drawing primitive
     */
    DPHEAD dphead;

    /**
     * LiNe Property Color -- RGB color value
     */
    U32 lnpc;

    /**
     * line property weight in twips
     */
    U16 lnpw;

    /**
     * line property style. See description in DPLINE.
     */
    U16 lnps;

    /**
     * FiLl Property Color ForeGround -- RGB color value
     */
    U32 dlpcFg;

    /**
     * FiLl Property Color BackGround -- RGB color value
     */
    U32 dlpcBg;

    /**
     * FiLl Property Pattern. REVIEW davebu
     */
    U16 flpp;

    /**
     * Shadow Property Intensity
     */
    U16 shdwpi;

    /**
     * x offset of shadow
     */
    U16 xaOffset;

    /**
     * y offset of shadow
     */
    U16 yaOffset;

}; // DPELLIPSE

bool operator==(const DPELLIPSE &lhs, const DPELLIPSE &rhs);
bool operator!=(const DPELLIPSE &lhs, const DPELLIPSE &rhs);


/**
 * DP data for a line (DPLINE)
 */
struct DPLINE {
    /**
     * Creates an empty DPLINE structure and sets the defaults
     */
    DPLINE();
    /**
     * Simply calls read(...)
     */
	DPLINE(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * This method reads the DPLINE structure from the stream.
     * If  preservePos is true we push/pop the position of
     * the stream to save the state. If it's false the state
     * of stream will be changed!
     */
	bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * Set all the fields to the inital value (default is 0)
     */
    void clear();

    // Data
    /**
     * Common header for a drawing primitive
     */
    DPHEAD dphead;

    /**
     * starting point for line
     */
    U16 xaStart;

    U16 yaStart;

    /**
     * ending point for line
     */
    U16 xaEnd;

    U16 yaEnd;

    /**
     * LiNe Property Color -- RGB color value
     */
    U32 lnpc;

    /**
     * line property weight in twips
     */
    U16 lnpw;

    /**
     * line property style
     * 0 Solid
     * 1 Dashed
     * 2 Dotted
     * 3 Dash Dot
     * 4 Dash Dot Dot
     * 5 Hollow
     */
    U16 lnps;

    /**
     * Start EndPoint Property Style 0 None
     * 1 Hollow
     * 2 Filled
     */
    U16 eppsStart:2;

    /**
     * Start EndPoint Property Weight
     */
    U16 eppwStart:2;

    /**
     * Start EndPoint Property length
     */
    U16 epplStart:2;

    U16 unused24_6:10;

    /**
     * End EndPoint Property Style
     */
    U16 eppsEnd:2;

    /**
     * End EndPoint Property Weight
     */
    U16 eppwEnd:2;

    /**
     * End EndPoint Property length
     */
    U16 epplEnd:2;

    U16 unused26_6:10;

    /**
     * Shadow Property Intensity REVIEW davebu
     */
    U16 shdwpi;

    /**
     * x offset of shadow
     */
    U16 xaOffset;

    /**
     * y offset of shadow
     */
    U16 yaOffset;

}; // DPLINE

bool operator==(const DPLINE &lhs, const DPLINE &rhs);
bool operator!=(const DPLINE &lhs, const DPLINE &rhs);


/**
 * DP data for a rectangle (DPRECT)
 */
struct DPRECT {
    /**
     * Creates an empty DPRECT structure and sets the defaults
     */
    DPRECT();
    /**
     * Simply calls read(...)
     */
	DPRECT(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * This method reads the DPRECT structure from the stream.
     * If  preservePos is true we push/pop the position of
     * the stream to save the state. If it's false the state
     * of stream will be changed!
     */
	bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * Set all the fields to the inital value (default is 0)
     */
    void clear();

    // Data
    /**
     * Common header for a drawing primitive
     */
    DPHEAD dphead;

    /**
     * LiNe Property Color -- RGB color value
     */
    U32 lnpc;

    /**
     * line property weight in twips
     */
    U16 lnpw;

    /**
     * line property style. See description in DPLINE.
     */
    U16 lnps;

    /**
     * FiLl Property Color ForeGround -- RGB color value
     */
    U32 dlpcFg;

    /**
     * FiLl Property Color BackGround -- RGB color value
     */
    U32 dlpcBg;

    /**
     * FiLl Property Pattern. REVIEW davebu
     */
    U16 flpp;

    /**
     * Shadow Property Intensity
     */
    U16 shdwpi;

    /**
     * x offset of shadow
     */
    U16 xaOffset;

    /**
     * y offset of shadow
     */
    U16 yaOffset;

    /**
     * 1 if the textbox has rounded corners
     */
    U16 fRoundCorners:1;

    /**
     * REVIEW davebu
     */
    U16 zaShape:15;

}; // DPRECT

bool operator==(const DPRECT &lhs, const DPRECT &rhs);
bool operator!=(const DPRECT &lhs, const DPRECT &rhs);


/**
 * DP data for a sample primitive holding default values (DPSAMPLE)
 */
struct DPSAMPLE {
    /**
     * Creates an empty DPSAMPLE structure and sets the defaults
     */
    DPSAMPLE();
    /**
     * Simply calls read(...)
     */
	DPSAMPLE(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * This method reads the DPSAMPLE structure from the stream.
     * If  preservePos is true we push/pop the position of
     * the stream to save the state. If it's false the state
     * of stream will be changed!
     */
	bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * Set all the fields to the inital value (default is 0)
     */
    void clear();

    // Data
    /**
     * Common header for a drawing primitive
     */
    DPHEAD dphead;

    /**
     * LiNe Property Color -- RGB color value
     */
    U32 lnpc;

    /**
     * line property weight in twips
     */
    U16 lnpw;

    /**
     * line property style. See description in DPLINE.
     */
    U16 lnps;

    /**
     * FiLl Property Color ForeGround -- RGB color value
     */
    U32 dlpcFg;

    /**
     * FiLl Property Color BackGround -- RGB color value
     */
    U32 dlpcBg;

    /**
     * FiLl Property Pattern. REVIEW davebu
     */
    U16 flpp;

    /**
     * Start EndPoint Property Style
     * 0 None
     * 1 Hollow
     * 2 Filled
     */
    U16 eppsStart:2;

    /**
     * Start EndPoint Property Weight
     */
    U16 eppwStart:2;

    /**
     * Start EndPoint Property length
     */
    U16 epplStart:2;

    U16 unused30_6:10;

    /**
     * End EndPoint Property Style
     */
    U16 eppsEnd:2;

    /**
     * End EndPoint Property Weight
     */
    U16 eppwEnd:2;

    /**
     * End EndPoint Property length
     */
    U16 epplEnd:2;

    U16 unused32_6:10;

    /**
     * Shadow Property Intensity
     */
    U16 shdwpi;

    /**
     * x offset of shadow
     */
    U16 xaOffset;

    /**
     * y offset of shadow
     */
    U16 yaOffset;

    U16 unused40;

    /**
     * REVIEW davebu
     */
    U16 dzaOffset;

    /**
     * REVIEW davebu
     */
    U16 dzaDescent;

    /**
     * REVIEW davebu
     */
    U16 dzaLength;

    /**
     * 1 if the textbox has rounded corners
     */
    U16 fRoundCorners:1;

    /**
     * REVIEW davebu
     */
    U16 zaShape:15;

    /**
     * REVIEW davebu
     */
    U16 dzaInternalMargin;

}; // DPSAMPLE

bool operator==(const DPSAMPLE &lhs, const DPSAMPLE &rhs);
bool operator!=(const DPSAMPLE &lhs, const DPSAMPLE &rhs);


/**
 * File Drawn Object Address (Word) (FDOA)
 */
struct FDOA {
    /**
     * Creates an empty FDOA structure and sets the defaults
     */
    FDOA();
    /**
     * Simply calls read(...)
     */
	FDOA(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * This method reads the FDOA structure from the stream.
     * If  preservePos is true we push/pop the position of
     * the stream to save the state. If it's false the state
     * of stream will be changed!
     */
	bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * Set all the fields to the inital value (default is 0)
     */
    void clear();

    // Data
    /**
     * FC pointing to drawing object data
     */
    U32 fc;

    /**
     * count of textboxes in the drawing object
     */
    U16 ctxbx;

}; // FDOA

bool operator==(const FDOA &lhs, const FDOA &rhs);
bool operator!=(const FDOA &lhs, const FDOA &rhs);


/**
 * Font Family Name (FFN)
 */
/* This structure has been commented out because we can't handle it correctly
 * Please don't try to fix it here in this file, but rather copy this broken
 * structure definition and fix it in some auxilliary file. If you want to
 * include that aux. file here, please change the template file.
 */
//struct FFN {
//    /**
//     * Creates an empty FFN structure and sets the defaults
//     */
//    FFN();
//    /**
//     * Simply calls read(...)
//     */
//    FFN(AbstractOLEStreamReader *stream, bool preservePos=false);
//    /**
//     * Attention: This struct allocates memory on the heap
//     */
//    FFN(const FFN &rhs);
//    ~FFN();

//    FFN &operator=(const FFN &rhs);

//    /**
//     * This method reads the FFN structure from the stream.
//     * If  preservePos is true we push/pop the position of
//     * the stream to save the state. If it's false the state
//     * of stream will be changed!
//     */
//    bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

//    /**
//     * Same as reading :)
//     */
//    //bool write(OLEStreamWriter *stream, bool preservePos=false) const;

//    /**
//     * Set all the fields to the inital value (default is 0)
//     */
//    void clear();

//    // Data
//    /**
//     * total length of FFN - 1.
//     */
//    U8 cbFfnM1;

//    /**
//     * pitch request
//     */
//    U8 prq:2;

//    /**
//     * when 1, font is a TrueType font
//     */
//    U8 fTrueType:1;

//    /**
//     * reserved
//     */
//    U8 unused1_3:1;

//    /**
//     * font family id
//     */
//    U8 ff:3;

//    /**
//     * reserved
//     */
//    U8 unused1_7:1;

//    /**
//     * base weight of font
//     */
//    U16 wWeight;

//    /**
//     * character set identifier
//     */
//    U8 chs;

//    /**
//     * index into ffn.szFfn to the name of the alternate font
//     */
//    U8 ibszAlt;

//    /**
//     * zero terminated string that records name of font. Possibly followed
//     * by a second sz which records the name of an alternate font to use if the
//     * first named font does not exist on this system. Maximal size of szFfn is
//     * 65 characters.
//     */
//    U8 *szFfn;   //    U8 szFfn[];

//private:
//    void clearInternal();

//}; // FFN

//bool operator==(const FFN &lhs, const FFN &rhs);
//bool operator!=(const FFN &lhs, const FFN &rhs);


/**
 * File Information Block (Windows Word) (FIB)
 */
struct FIB {
    /**
     * Creates an empty FIB structure and sets the defaults
     */
    FIB();
    /**
     * Simply calls read(...)
     */
	FIB(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * This method reads the FIB structure from the stream.
     * If  preservePos is true we push/pop the position of
     * the stream to save the state. If it's false the state
     * of stream will be changed!
     */
	bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * Set all the fields to the inital value (default is 0)
     */
    void clear();

    // Data
    /**
     * magic number
     */
    U16 wIdent;

    /**
     * FIB version written
     */
    U16 nFib;

    /**
     * product version written by
     */
    U16 nProduct;

    /**
     * language stamp---localized version;
     * <p>In pre-WinWord2.0 files this value was the nLocale.If value is &lt;
     * 999, then it is the nLocale, otherwise it is the lid.
     */
    U16 lid;

    U16 pnNext;

    U16 fDot:1;

    U16 fGlsy:1;

    /**
     * when 1, file is in complex, fast-saved format.
     */
    U16 fComplex:1;

    /**
     * file contains 1 or more pictures
     */
    U16 fHasPic:1;

    /**
     * count of times file was quicksaved
     */
    U16 cQuickSaves:4;

    /**
     * 1 if file is encrypted, 0 if not
     */
    U16 fEncrypted:1;

    /**
     * reserved
     */
    U16 unused10_9:1;

    /**
     * =1 when user has recommended that file be read read-only
     */
    U16 fReadOnlyRecommended:1;

    /**
     * =1, when file owner has made the file write reserved
     */
    U16 fWriteReservation:1;

    /**
     * =1, when using extended character set in file
     */
    U16 fExtChar:1;

    /**
     * unused
     */
    U16 unused10_13:3;

    U16 nFibBack;

    /**
     * file encrypted key, only valid if fEncrypted.
     */
    U32 lKey;

    /**
     * environment in which file was created
     * 0 created by Win Word
     * 1 created by Mac Word
     */
    U8 envr;

    /**
     * reserved
     */
    U8 unused19;

    /**
     * default extended character set id for text in document stream. (overidden
     * by chp.chse)
     * 0 by default characters in doc stream should be interpreted using the
     * ANSI character set used by Windows
     * 256 characters in doc stream should be interpreted using the Macintosh
     * character set.
     */
    U16 chse;

    /**
     * default extended character set id for text in internal data structures
     * 0 by default characters in doc stream should be interpreted using the
     * ANSI character set used by Windows
     * 256 characters in doc stream should be interpreted using the Macintosh
     * character set.
     */
    U16 chseTables;

    /**
     * file offset of first character of text. In non-complexfiles a CP can
     * be transformed into an FC by the following transformation: fc = cp + fib.fcMin.
     */
    U32 fcMin;

    /**
     * file offset of last character of text in document text stream+ 1
     */
    U32 fcMac;

    /**
     * file offset of last byte written to file + 1.
     */
    U32 cbMac;

    /**
     * reserved
     */
    U32 fcSpare0;

    /**
     * reserved
     */
    U32 fcSpare1;

    /**
     * reserved
     */
    U32 fcSpare2;

    /**
     * reserved
     */
    U32 fcSpare3;

    /**
     * length of main document text stream
     */
    U32 ccpText;

    /**
     * length of footnote subdocument text stream
     */
    U32 ccpFtn;

    /**
     * length of header subdocument text stream
     */
    U32 ccpHdd;

    /**
     * length of macro subdocument text stream
     */
    U32 ccpMcr;

    /**
     * length of annotation subdocument text stream
     */
    U32 ccpAtn;

    /**
     * length of endnote subdocument text stream
     */
    U32 ccpEdn;

    /**
     * length of textbox subdocument text stream
     */
    U32 ccpTxbx;

    /**
     * length of header textbox subdocument text stream
     * <p>Note: when ccpFtn == 0 and ccpHdr == 0 and ccpMcr== 0 and ccpAtn ==
     * 0 and ccpEdn ==0 and ccpTxbx == 0 and ccpHdrTxbx == 0, then fib.fcMac =
     * fib.fcMin+ fib.ccpText. If either ccpFtn != 0 or ccpHdd != 0or ccpMcr !=
     * 0or ccpAtn != 0 or ccpEdn !=0 or ccpTxbx != 0 or ccpHdrTxbx == 0, then
     * fib.fcMac = fib.fcMin + fib.ccpText + fib.ccpFtn + fib.ccpHdd+ fib.ccpMcr+
     * fib.ccpAtn + fib.ccpEdn + fib.ccpTxbx + fib.ccpHdrTxbx + 1. The single
     * characterstored beginning at file position fib.fcMac - 1 must always be
     * a CRcharacter (ASCII 13).
     */
    U32 ccpHdrTxbx;

    /**
     * reserved
     */
    U32 ccpSpare2;

    /**
     * file offset of original allocation for STSH in file. During fast save
     * Word will attempt to reuse this allocation if STSH is small enough to fit.
     */
    U32 fcStshfOrig;

    /**
     * count of bytes of original STSH allocation
     */
    U32 lcbStshfOrig;

    /**
     * file offset of STSH in file.
     */
    U32 fcStshf;

    /**
     * count of bytes of current STSH allocation
     */
    U32 lcbStshf;

    /**
     * file offset of footnote reference PLC. CPs in PLC are relative to main
     * document text stream and give location of footnote references. The structure
     * stored in this plc, called the FRD (footnote reference descriptor) is two
     * byte long.
     */
    U32 fcPlcffndRef;

    /**
     * count of bytes of footnote reference PLC. == 0 if no footnotes defined
     * in document.
     */
    U32 lcbPlcffndRef;

    /**
     * file offset of footnote text PLC. CPs in PLC are relative to footnote
     * subdocument text stream and give location of beginnings of footnote text
     * for correspondings references recorded in plcffndRef. No structure is stored
     * in this plc. There will just be n+1 FC entries in this PLC when there are
     * n footnotes
     */
    U32 fcPlcffndTxt;

    /**
     * count of bytes of footnote text PLC. == 0 if no footnotes defined in
     * document
     */
    U32 lcbPlcffndTxt;

    /**
     * file offset of annotation reference PLC. The CPs recorded in this PLC
     * give the offset of annotation references in the main document.
     */
    U32 fcPlcfandRef;

    /**
     * count of bytes of annotation reference PLC.
     */
    U32 lcbPlcfandRef;

    /**
     * file offset of annotation text PLC. The Cps recorded in this PLC give
     * the offset of the annotation text in the annotation sub document corresponding
     * to the references stored in the plcfandRef. There is a 1 to 1 correspondence
     * between entries recorded in the plcfandTxt and the plcfandRef.
     */
    U32 fcPlcfandTxt;

    /**
     * count of bytes of the annotation text PLC
     */
    U32 lcbPlcfandTxt;

    /**
     * file offset of section descriptor PLC. CPs in PLC are relative to main
     * document. The length of the SED is 12 bytes.
     */
    U32 fcPlcfsed;

    /**
     * count of bytes of section descriptor PLC.
     */
    U32 lcbPlcfsed;

    /**
     * file offset of paragraph descriptor PLCfor main document which is used
     * by Word's Outline view. CPs in PLC are relative to main document. The length
     * of the PGD is 8 bytes.
     */
    U32 fcPlcfpad;

    /**
     * count of bytes of paragraph descriptor PLC. ==0 if file was never viewed
     * in Outline view. Should not be written by third party creators of Word
     * files.
     */
    U32 lcbPlcfpad;

    /**
     * file offset of PLC of paragraph heights. CPs in PLC are relative to
     * main document text stream. Only written for fies in complex format. Should
     * not be written by third party creators of Word files. The PHE is 6 bytes
     * long.
     */
    U32 fcPlcfphe;

    /**
     * count of bytes of paragraph height PLC. ==0 when file is non-complex.
     */
    U32 lcbPlcfphe;

    /**
     * file offset of glossary string table. This table consists of pascal
     * style strings (strings stored prefixed with a length byte) concatenated
     * one after another.
     */
    U32 fcSttbfglsy;

    /**
     * count of bytes of glossary string table.
     * == 0 for non-glossary documents.
     * !=0 for glossary documents.
     */
    U32 lcbSttbfglsy;

    /**
     * file offset of glossary PLC. CPs in PLC are relative to main document
     * and mark the beginnings of glossary entries and are in 1-1 correspondence
     * with entries of sttbfglsy. No structure is stored in this PLC. There will
     * be n+1 FC entries in this PLC when there are n glossary entries.
     */
    U32 fcPlcfglsy;

    /**
     * count of bytes of glossary PLC.
     * == 0 for non-glossary documents.
     * !=0 for glossary documents.
     */
    U32 lcbPlcfglsy;

    /**
     * byte offset of header PLC. CPs are relative to header subdocument and
     * mark the beginnings of individual headers in the header subdoc. No structure
     * is stored in this PLC. There will be n+1 FC entries in this PLC when there
     * are n headers stored for the document.
     */
    U32 fcPlcfhdd;

    /**
     * count of bytes of header PLC. == 0 if document contains no headers
     */
    U32 lcbPlcfhdd;

    /**
     * file offset of character property bin table.PLC. FCs in PLC are file
     * offsets. Describes text of main document and all subdocuments. The BTE
     * is 2 bytes long.
     */
    U32 fcPlcfbteChpx;

    /**
     * count of bytes of character property bin table PLC.
     */
    U32 lcbPlcfbteChpx;

    /**
     * file offset of paragraph property bin table.PLC. FCs in PLC are file
     * offsets. Describes text of main document and all subdocuments. The BTE
     * is 2 bytes long.
     */
    U32 fcPlcfbtePapx;

    /**
     * count of bytes of paragraph property bin table PLC.
     */
    U32 lcbPlcfbtePapx;

    /**
     * file offset of PLC reserved for private use. The SEA is 6 bytes long.
     */
    U32 fcPlcfsea;

    /**
     * count of bytes of private use PLC.
     */
    U32 lcbPlcfsea;

    /**
     * file offset of font information STTBF. The nth entry in the STTBF describes
     * the font that will be displayed when the chp.ftc for text is equal to n.
     * See the FFN file structure definition.
     */
    U32 fcSttbfffn;

    /**
     * count of bytes in sttbfffn.
     */
    U32 lcbSttbfffn;

    /**
     * offset in doc stream to the PLC of field positions in the main document.
     * The Cps point to the beginning CP of a field, the CP offield separator
     * character inside a field and the ending CP of the field. A field may be
     * nested within another field. 20 levels of field nesting are allowed.
     */
    U32 fcPlcffldMom;

    U32 lcbPlcffldMom;

    /**
     * offset in doc stream to the PLC of field positions in the header subdocument.
     */
    U32 fcPlcffldHdr;

    U32 lcbPlcffldHdr;

    /**
     * offset in doc stream to the PLC of field positions in the footnote
     * subdocument.
     */
    U32 fcPlcffldFtn;

    U32 lcbPlcffldFtn;

    /**
     * offset in doc stream to the PLC of field positions in the annotation
     * subdocument.
     */
    U32 fcPlcffldAtn;

    U32 lcbPlcffldAtn;

    /**
     * offset in doc stream to the PLC of field positions in the macro subdocument.
     */
    U32 fcPlcffldMcr;

    U32 lcbPlcffldMcr;

    /**
     * offset in document stream of the STTBF that records bookmark names
     * in the main document
     */
    U32 fcSttbfbkmk;

    U32 lcbSttbfbkmk;

    /**
     * offset in document stream of the PLCF that records the beginning CP
     * offsets of bookmarks in the main document. See BKF structure definition
     */
    U32 fcPlcfbkf;

    U32 lcbPlcfbkf;

    /**
     * offset in document stream of the PLCF that records the ending CP offsets
     * of bookmarks recorded in the main document. See the BKL structure definition.
     */
    U32 fcPlcfbkl;

    U32 lcbPlcfbkl;

    U32 fcCmds;

    U32 lcbCmds;

    U32 fcPlcmcr;

    U32 lcbPlcmcr;

    U32 fcSttbfmcr;

    U32 lcbSttbfmcr;

    /**
     * file offset of the printer driver information (names of drivers, port
     * etc...)
     */
    U32 fcPrDrvr;

    /**
     * count of bytes of the printer driver information (names of drivers,
     * port etc...)
     */
    U32 lcbPrDrvr;

    /**
     * file offset of the print environment in portrait mode.
     */
    U32 fcPrEnvPort;

    /**
     * count of bytes of the print environment in portrait mode.
     */
    U32 lcbPrEnvPort;

    /**
     * file offset of the print environment in landscape mode.
     */
    U32 fcPrEnvLand;

    /**
     * count of bytes of the print environment in landscape mode.
     */
    U32 lcbPrEnvLand;

    /**
     * file offset of Window Save State data structure. WSS contains dimensions
     * of document's main text window and the last selection made by Word user.
     */
    U32 fcWss;

    /**
     * count of bytes of WSS. ==0 if unable to store the window state. Should
     * not be written by third party creators of Word files.
     */
    U32 lcbWss;

    /**
     * file offset of document property data structure.
     */
    U32 fcDop;

    /**
     * count of bytes of document properties.
     */
    U32 lcbDop;

    /**
     * offset to STTBF of associated strings. The strings in this table specify
     * document summary info and the paths to special documents related to this
     * document. See documentation of the STTBFASSOC.
     */
    U32 fcSttbfAssoc;

    U32 lcbSttbfAssoc;

    /**
     * file of offset of beginning of information for complex files. Consists
     * of an encoding of all of the prms quoted by the document followed by the
     * plcpcd (piece table) for the document.
     */
    U32 fcClx;

    /**
     * count of bytes of complex file information. == 0 if file is non-complex.
     */
    U32 lcbClx;

    /**
     * file offset of page descriptor PLC for footnote subdocument. CPs in
     * PLC are relative to footnote subdocument. Should not be written by third
     * party creators of Word files.
     */
    U32 fcPlcfpgdFtn;

    /**
     * count of bytes of page descriptor PLC for footnote subdocument. ==0
     * if document has not been paginated. The length of the PGD is 8 bytes.
     */
    U32 lcbPlcfpgdFtn;

    /**
     * file offset of the name of the original file.fcAutosaveSource and cbAutosaveSource
     * should both be 0 if autosave is off.
     */
    U32 fcAutosaveSource;

    /**
     * count of bytes of the name of the original file.
     */
    U32 lcbAutosaveSource;

    /**
     * group of strings recording the names of the owners of annotations stored
     * in the document
     */
    U32 fcGrpStAtnOwners;

    /**
     * count of bytes of the group of strings
     */
    U32 lcbGrpStAtnOwners;

    /**
     * file offset of the sttbf that records names of bookmarks in the annotation
     * subdocument
     */
    U32 fcSttbfAtnbkmk;

    /**
     * length in bytes of the sttbf that records names of bookmarks in the
     * annotation subdocument
     */
    U32 lcbSttbfAtnbkmk;

    U16 wSpare4Fib;

    /**
     * the page number of the lowest numbered page in the document that records
     * CHPX FKP information
     */
    U16 pnChpFirst;

    /**
     * the page number of the lowest numbered page in the document that records
     * PAPX FKP information
     */
    U16 pnPapFirst;

    /**
     * count of CHPX FKPs recorded in file. In non-complexfiles if the number
     * of entries in the plcfbteChpxis less than this, the plcfbteChpxis incomplete.
     */
    U16 cpnBteChp;

    /**
     * count of PAPX FKPs recorded in file. In non-complexfiles if the number
     * of entries in the plcfbtePapxis less than this, the plcfbtePapxis incomplete.
     */
    U16 cpnBtePap;

    /**
     * file offset of theFDOA (drawn object) PLC for main document. ==0 if
     * document has no drawn objects. The length of the FDOA is 6 bytes.
     */
    U32 fcPlcfdoaMom;

    /**
     * length in bytes of the FDOA PLC of the main document
     */
    U32 lcbPlcfdoaMom;

    /**
     * file offset of theFDOA (drawn object) PLC for the header document.
     * ==0 if document has no drawn objects. The length of the FDOA is 6 bytes.
     */
    U32 fcPlcfdoaHdr;

    /**
     * length in bytes of the FDOA PLC of the header document
     */
    U32 lcbPlcfdoaHdr;

    U32 fcUnused1;

    U32 lcbUnused1;

    U32 fcUnused2;

    U32 lcbUnused2;

    /**
     * file offset of BKF (bookmark first) PLC of the annotation subdocument
     */
    U32 fcPlcfAtnbkf;

    /**
     * length in bytes of BKF (bookmark first) PLC of the annotation subdocument
     */
    U32 lcbPlcfAtnbkf;

    /**
     * file offset of BKL (bookmark last) PLC of the annotation subdocument
     */
    U32 fcPlcfAtnbkl;

    /**
     * length in bytes of BKL (bookmark first) PLC of the annotation subdocument
     */
    U32 lcbPlcfAtnbkl;

    /**
     * file offset of PMS (Print Merge State) information block
     */
    U32 fcPms;

    /**
     * length in bytes of PMS
     */
    U32 lcbPms;

    /**
     * file offset of form field Sttbf which contains strings used in form
     * field dropdown controls
     */
    U32 fcFormFldSttbf;

    /**
     * length in bytes of form field Sttbf
     */
    U32 lcbFormFldSttbf;

    /**
     * file offset of PlcfendRef which points to endnote references in the
     * main document stream
     */
    U32 fcPlcfendRef;

    U32 lcbPlcfendRef;

    /**
     * file offset of PlcfendRef which points to endnote textin the endnote
     * document stream which corresponds with the plcfendRef
     */
    U32 fcPlcfendTxt;

    U32 lcbPlcfendTxt;

    /**
     * offset to PLCF of field positions in the endnote subdoc
     */
    U32 fcPlcffldEdn;

    U32 lcbPlcffldEdn;

    /**
     * offset to PLCF of page boundaries in the endnote subdoc.
     */
    U32 fcPlcfpgdEdn;

    U32 lcbPlcfpgdEdn;

    U32 fcUnused3;

    U32 lcbUnused3;

    /**
     * offset to STTBF that records the author abbreviations for authors who
     * have made revisions in the document.
     */
    U32 fcSttbfRMark;

    U32 lcbSttbfRMark;

    /**
     * offset to STTBF that records caption titles used in the document.
     */
    U32 fcSttbfCaption;

    U32 lcbSttbfCaption;

    U32 fcSttbfAutoCaption;

    U32 lcbSttbfAutoCaption;

    /**
     * offset to PLCF that describes the boundaries of contributing documents
     * in a master document
     */
    U32 fcPlcfwkb;

    U32 lcbPlcfwkb;

    U32 fcUnused4;

    U32 lcbUnused4;

    /**
     * offset in doc stream of PLCF that records the beginning CP in the text
     * box subdoc of the text of individual text box entries
     */
    U32 fcPlcftxbxTxt;

    U32 lcbPlcftxbxTxt;

    /**
     * offset in doc stream of the PLCF that records field boundaries recorded
     * in the textbox subdoc.
     */
    U32 fcPlcffldTxbx;

    U32 lcbPlcffldTxbx;

    /**
     * offset in doc stream of PLCF that records the beginning CP in the header
     * text box subdoc of the text of individual header text box entries
     */
    U32 fcPlcfHdrtxbxTxt;

    U32 lcbPlcfHdrtxbxTxt;

    /**
     * offset in doc stream of the PLCF that records field boundaries recorded
     * in the header textbox subdoc.
     */
    U32 fcPlcffldHdrTxbx;

    U32 lcbPlcffldHdrTxbx;

    /**
     * Macro User storage
     */
    U32 fcStwUser;

    U32 lcbStwUser;

    U32 fcSttbttmbd;

    U32 lcbSttbttmbd;

    U32 fcUnused;

    U32 lcbUnused;

    U32 fcPgdMother;

    U32 lcbPgdMother;

    U32 fcBkdMother;

    U32 lcbBkdMother;

    U32 fcPgdFtn;

    U32 lcbPgdFtn;

    U32 fcBkdFtn;

    U32 lcbBkdFtn;

    U32 fcPgdEdn;

    U32 lcbPgdEdn;

    U32 fcBkdEdn;

    U32 lcbBkdEdn;

    U32 fcSttbfIntlFld;

    U32 lcbSttbfIntlFld;

    U32 fcRouteSlip;

    U32 lcbRouteSlip;

    U32 fcSttbSavedBy;

    U32 lcbSttbSavedBy;

    U32 fcSttbFnm;

    U32 lcbSttbFnm;

}; // FIB

bool operator==(const FIB &lhs, const FIB &rhs);
bool operator!=(const FIB &lhs, const FIB &rhs);


/**
 * Field Descriptor (FLD)
 */
/* This structure has been commented out because we can't handle it correctly
 * Please don't try to fix it here in this file, but rather copy this broken
 * structure definition and fix it in some auxilliary file. If you want to
 * include that aux. file here, please change the template file.
 */
//struct FLD {
//    /**
//     * Creates an empty FLD structure and sets the defaults
//     */
//    FLD();
//    /**
//     * Simply calls read(...)
//     */
//    FLD(AbstractOLEStreamReader *stream, bool preservePos=false);

//    /**
//     * This method reads the FLD structure from the stream.
//     * If  preservePos is true we push/pop the position of
//     * the stream to save the state. If it's false the state
//     * of stream will be changed!
//     */
//    bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

//    /**
//     * Same as reading :)
//     */
//    //bool write(OLEStreamWriter *stream, bool preservePos=false) const;

//    /**
//     * Set all the fields to the inital value (default is 0)
//     */
//    void clear();

//    // Data
//    /**
//     * type of field boundary the FLD describes.
//     * 19 field begin mark
//     * 20 field separator
//     * 21 field end mark
//     */
//    U8 ch;

//}; // FLD

//bool operator==(const FLD &lhs, const FLD &rhs);
//bool operator!=(const FLD &lhs, const FLD &rhs);


/**
 * Line Spacing Descriptor (LSPD)
 */
struct LSPD {
    /**
     * Creates an empty LSPD structure and sets the defaults
     */
    LSPD();
    /**
     * Simply calls read(...)
     */
	LSPD(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * This method reads the LSPD structure from the stream.
     * If  preservePos is true we push/pop the position of
     * the stream to save the state. If it's false the state
     * of stream will be changed!
     */
	bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * Set all the fields to the inital value (default is 0)
     */
    void clear();

    /**
     * Dumps all fields of this structure (for debugging)
     */
    void dump() const;

    /**
     * Converts the data structure to a string (for debugging)
     */
    std::string toString() const;

    // Data
    /**
     * see description of sprmPDyaLine in the Sprm Definitions sectionfor
     * description of the meaning of dyaLine and fMultLinespace fields
     */
    U16 dyaLine;

    U16 fMultLinespace;

}; // LSPD

bool operator==(const LSPD &lhs, const LSPD &rhs);
bool operator!=(const LSPD &lhs, const LSPD &rhs);


/**
 * Window's (METAFILEPICT)
 */
struct METAFILEPICT {
    /**
     * Creates an empty METAFILEPICT structure and sets the defaults
     */
    METAFILEPICT();
    /**
     * Simply calls read(...)
     */
	METAFILEPICT(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * This method reads the METAFILEPICT structure from the stream.
     * If  preservePos is true we push/pop the position of
     * the stream to save the state. If it's false the state
     * of stream will be changed!
     */
	bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * Set all the fields to the inital value (default is 0)
     */
    void clear();

    // Data
    /**
     * Specifies the mapping mode in which the picture is drawn.
     */
    U16 mm;

    /**
     * Specifies the size of the metafile picture for all modes except the
     * MM_ISOTROPIC and MM_ANISOTROPIC modes. (For more information about these
     * modes, see the yExt member.) The x-extent specifies the width of the rectangle
     * within which the picture is drawn. The coordinates are in units that correspond
     * to the mapping mode.
     */
    U16 xExt;

    /**
     * Specifies the size of the metafile picture for all modes except the
     * MM_ISOTROPIC and MM_ANISOTROPIC modes. The y-extent specifies the height
     * of the rectangle within which the picture is drawn. The coordinates are
     * in units that correspond to the mapping mode.
     * <p>For MM_ISOTROPIC and MM_ANISOTROPIC modes, which can be scaled, the
     * xExt and yExt members contain an optional suggested size in MM_HIMETRIC
     * units.
     * <p>For MM_ANISOTROPIC pictures, xExt and yExt can be zero when no suggested
     * size is supplied. For MM_ISOTROPIC pictures, an aspect ratio must be supplied
     * even when no suggested size is given. (If a suggested size is given, the
     * aspect ratio is implied by the size.) To give an aspect ratio without implying
     * a suggested size, set xExt and yExt to negative values whose ratio is the
     * appropriate aspect ratio. The magnitude of the negative xExt and yExt values
     * is ignored; only the ratio is used.
     */
    U16 yExt;

    /**
     * Identifies a memory metafile.
     */
    U16 hMF;

}; // METAFILEPICT

bool operator==(const METAFILEPICT &lhs, const METAFILEPICT &rhs);
bool operator!=(const METAFILEPICT &lhs, const METAFILEPICT &rhs);


/**
 * Embedded Object Properties (OBJHEADER)
 */
struct OBJHEADER {
    /**
     * Creates an empty OBJHEADER structure and sets the defaults
     */
    OBJHEADER();
    /**
     * Simply calls read(...)
     */
	OBJHEADER(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * This method reads the OBJHEADER structure from the stream.
     * If  preservePos is true we push/pop the position of
     * the stream to save the state. If it's false the state
     * of stream will be changed!
     */
	bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * Set all the fields to the inital value (default is 0)
     */
    void clear();

    // Data
    /**
     * length of object (including this header)
     */
    U32 lcb;

    /**
     * length of this header (for future use)
     */
    U16 cbHeader;

    /**
     * index to clipboard format of object
     */
    U16 icf;

}; // OBJHEADER

bool operator==(const OBJHEADER &lhs, const OBJHEADER &rhs);
bool operator!=(const OBJHEADER &lhs, const OBJHEADER &rhs);


/**
 * Outline LiST Data (OLST)
 */
struct OLST {
    /**
     * Creates an empty OLST structure and sets the defaults
     */
    OLST();
    /**
     * Simply calls read(...)
     */
	OLST(AbstractOLEStreamReader *stream, bool preservePos=false);
    /**
     * Simply calls readPtr(...)
     */
    OLST(const U8 *ptr);

    /**
     * This method reads the OLST structure from the stream.
     * If  preservePos is true we push/pop the position of
     * the stream to save the state. If it's false the state
     * of stream will be changed!
     */
	bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * This method reads the struct from a pointer
     */
	void readPtr(const U8 *ptr);

    /**
     * Set all the fields to the inital value (default is 0)
     */
    void clear();

    /**
     * Dumps all fields of this structure (for debugging)
     */
    void dump() const;

    /**
     * Converts the data structure to a string (for debugging)
     */
    std::string toString() const;

    // Size of the structure
    static const unsigned int sizeOf;

    // Data
    /**
     * an array of 9 ANLV structures describing how heading numbers should
     * be displayed for each of Word?s 9 outline heading levels
     */
    ANLV rganlv[9];

    /**
     * when ==1, restart heading on section break
     */
    U8 fRestartHdr;

    /**
     * reserved
     */
    U8 fSpareOlst2;

    /**
     * reserved
     */
    U8 fSpareOlst3;

    /**
     * reserved
     */
    U8 fSpareOlst4;

    /**
     * text before/after number
     */
    U8 rgch[64];

}; // OLST

bool operator==(const OLST &lhs, const OLST &rhs);
bool operator!=(const OLST &lhs, const OLST &rhs);


/**
 * Paragraph Properties (PAP)
 */
struct PAP : public Shared {
    /**
     * Creates an empty PAP structure and sets the defaults
     */
    PAP();
    /**
     * Simply calls read(...)
     */
	PAP(AbstractOLEStreamReader *stream, bool preservePos=false);
    /**
     * Attention: This struct allocates memory on the heap
     */
    PAP(const PAP &rhs);
    ~PAP();

    PAP &operator=(const PAP &rhs);

    /**
     * This method reads the PAP structure from the stream.
     * If  preservePos is true we push/pop the position of
     * the stream to save the state. If it's false the state
     * of stream will be changed!
     */
	bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * Set all the fields to the inital value (default is 0)
     */
    void clear();

    /**
     * Dumps all fields of this structure (for debugging)
     */
    void dump() const;

    /**
     * Converts the data structure to a string (for debugging)
     */
    std::string toString() const;

    // Data
    /**
     * index to style descriptor . This is an index to an STD in the STSH
     * structure
     */
    U16 istd;

    /**
     * justification code 0left justify
     * 1center
     * 2right justify
     * 3left and right justify
     */
    U8 jc;

    /**
     * keep entire paragraph on one page if possible
     */
    U8 fKeep;

    /**
     * keep paragraph on same page with next paragraph if possible
     */
    U8 fKeepFollow;

    /**
     * start this paragraph on new page
     */
    U8 fPageBreakBefore;

    U8 fBrLnAbove:1;

    U8 fBrLnBelow:1;

    /**
     * reserved
     */
    U8 fUnused:2;

    /**
     * vertical position code. Specifies coordinate frame to use when paragraphs
     * are absolutely positioned.
     * 0 vertical position coordinates are relative to margin
     * 1 coordinates are relative to page
     * 2 coordinates are relative to text.This means: relative to where the
     * next non-APO text would have been placed if this APO did not exist.
     */
    U8 pcVert:2;

    /**
     * horizontal position code. Specifies coordinate frame to use when paragraphs
     * are absolutely positioned.
     * 0 horiz. position coordinates are relative to column.
     * 1 coordinates are relative to margin
     * 2 coordinates are relative to page
     */
    U8 pcHorz:2;

    /**
     * rectangle border codes (the brcp and brcl fields have been superceded
     * by the newly defined brcLeft, brcTop, etc. fields. They remain in the PAP
     * for compatibility with MacWord 3.0)
     * 0 none
     * 1 border above
     * 2 border below
     * 15 box around
     * 16 bar to left of paragraph
     */
    U8 brcp;

    /**
     * border line style
     * 0 single
     * 1 thick
     * 2 double
     * 3 shadow
     */
    U8 brcl;

    /**
     * reserved
     */
    U8 unused9;

    /**
     * auto list numbering level (0 = nothing)
     */
    U8 nLvlAnm;

    /**
     * no line numbering for this para. (makes this an exception to the section
     * property of line numbering)
     */
    U8 fNoLnn;

    /**
     * when 1, paragraph is a side by side paragraph
     */
    U8 fSideBySide;

    /**
     * indent from right margin (signed).
     */
    S16 dxaRight;

    /**
     * indent from left margin (signed)
     */
    S16 dxaLeft;

    /**
     * first line indent; signed number relative to dxaLeft
     */
    S16 dxaLeft1;

    /**
     * line spacing descriptor
     */
    LSPD lspd;

    /**
     * vertical spacing before paragraph (unsigned)
     */
    U16 dyaBefore;

    /**
     * vertical spacing after paragraph (unsigned)
     */
    U16 dyaAfter;

    /**
     * height of current paragraph.
     */
    PHE phe;

    /**
     * when 1, text in paragraph may be auto hyphenated
     */
    U8 fAutoHyph;

    /**
     * when 1, Word will prevent widowed lines in this paragraph from being
     * placed at the beginning of a page
     */
    U8 fWidowControl;

    /**
     * when 1, paragraph is contained in a table row
     */
    U8 fInTable;

    /**
     * when 1, paragraph consists only of the row mark special character and
     * marks the end of a table row.
     */
    U8 fTtp;

    /**
     * used internally by Word
     */
    U16 ptap;

    /**
     * when positive, is the horizontal distance from the reference frame
     * specified by pap.pcHorz. 0 means paragraph is positioned at the left with
     * respect to the refence frame specified by pcHorz. Certain negative values
     * have special meaning:
     * -4 paragraph centered horizontally within reference frame
     * -8 paragraph adjusted right within reference frame
     * -12 paragraph placed immediately inside of reference frame
     * -16 paragraph placed immediately outside of reference frame
     */
    S16 dxaAbs;

    /**
     * when positive, is the vertical distance from the reference frame specified
     * by pap.pcVert. 0 means paragraph's y-position is unconstrained. . Certain
     * negative values have special meaning:
     * -4 paragraph is placed at top of reference frame
     * -8 paragraph is centered vertically within reference frame
     * -12 paragraph is placed at bottom of reference frame.
     */
    S16 dyaAbs;

    /**
     * when not == 0, paragraph is constrained to be dxaWidth wide, independent
     * of current margin or column setings.
     */
    U16 dxaWidth;

    /**
     * specification for border above paragraph
     */
    BRC brcTop;

    /**
     * specification for border to the left of paragraph
     */
    BRC brcLeft;

    /**
     * specification for border below paragraph
     */
    BRC brcBottom;

    /**
     * specification for border to the right of paragraph
     */
    BRC brcRight;

    /**
     * specification of border to place between conforming paragraphs. Two
     * paragraphs conform when both have borders, their brcLeft and brcRight matches,
     * their widths are the same, theyboth belong to tables or both do not, and
     * have the same absolute positioning props.
     */
    BRC brcBetween;

    /**
     * specification of border to place on outside of text when facing pages
     * are to be displayed.
     */
    BRC brcBar;

    /**
     * horizontal distance to be maintained between an absolutely positioned
     * paragraph and any non-absolute positioned text
     */
    U16 dxaFromText;

    /**
     * vertical distance to be maintained between an absolutely positioned
     * paragraph and any non-absolute positioned text
     */
    U16 dyaFromText;

    /**
     * Wrap Code for absolute objects
     */
    U8 wr;

    /**
     * when 1, paragraph may not be editted
     */
    U8 fLocked;

    /**
     * height of abs obj; 0 == Auto
     */
    U16 dyaHeight:15;

    /**
     * 0 = Exact, 1 = At Least
     */
    U16 fMinHeight:1;

    /**
     * shading
     */
    SHD shd;

    /**
     * drop cap specifier (see DCS definition)
     */
    DCS dcs;

    /**
     * autonumber list descriptor (see ANLD definition)
     */
    ANLD anld;

    /**
     * number of tabs stops defined for paragraph. Must be >= 0 and &lt;=
     * 50.
     */
    U16 itbdMac;

    /**
     * array of positions of itbdMac tab stops. itbdMax == 50
     */
    U16 *rgdxaTab;   //    U16 rgdxaTab[itbdMac];

    /**
     * array of itbdMac tab descriptors
     */
    U8 *rgtbd;   //    U8 rgtbd[itbdMac];

private:
    void clearInternal();

}; // PAP

bool operator==(const PAP &lhs, const PAP &rhs);
bool operator!=(const PAP &lhs, const PAP &rhs);


/**
 * Paragraph Property Exceptions (PAPX)
 */
/* This structure has been commented out because we can't handle it correctly
 * Please don't try to fix it here in this file, but rather copy this broken
 * structure definition and fix it in some auxilliary file. If you want to
 * include that aux. file here, please change the template file.
 */
//struct PAPX {
//    /**
//     * Creates an empty PAPX structure and sets the defaults
//     */
//    PAPX();
//    /**
//     * Simply calls read(...)
//     */
//    PAPX(AbstractOLEStreamReader *stream, bool preservePos=false);
//    /**
//     * Attention: This struct allocates memory on the heap
//     */
//    PAPX(const PAPX &rhs);
//    ~PAPX();

//    PAPX &operator=(const PAPX &rhs);

//    /**
//     * This method reads the PAPX structure from the stream.
//     * If  preservePos is true we push/pop the position of
//     * the stream to save the state. If it's false the state
//     * of stream will be changed!
//     */
//    bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

//    /**
//     * Same as reading :)
//     */
//    //bool write(OLEStreamWriter *stream, bool preservePos=false) const;

//    /**
//     * Set all the fields to the inital value (default is 0)
//     */
//    void clear();

//    // Data
//    /**
//     * count of words of following data in PAPX. The first byte of a PAPX
//     * is a count of words when PAPX is stored in an FKP. Count of words is used
//     * because PAPX in an FKP can contain paragraph and table sprms.
//     */
//    U8 cw;

//    /**
//     * count of bytes of following data in PAPX. The first byte of a PAPX
//     * is a count of bytes when a PAPX is stored in a STSH. Count of bytes is
//     * used because only paragraph sprms are stored in a STSH PAPX.
//     */
//    U8 cb;

//    /**
//     * index to style descriiptor of the style from which the paragraph inherits
//     * its paragraph and character properties
//     */
//    U8 istd;

//    /**
//     * a list of the sprms that encode the differences between PAP for a paragraph
//     * and the PAP for the style used. When a paragraph bound is also the end
//     * of a table row, the PAPX also contains a list of table sprms which express
//     * the difference of table row's TAP from an empty TAP that has been cleared
//     * to zeros. The table sprms are recorded in the list after all of the paragraph
//     * sprms.See Sprms definitions for list of sprms that are used in PAPXs.
//     */
//    U8 *grpprl;   //    U8 grpprl[];

//private:
//    void clearInternal();

//}; // PAPX

//bool operator==(const PAPX &lhs, const PAPX &rhs);
//bool operator!=(const PAPX &lhs, const PAPX &rhs);


/**
 * Formatted Disk Page for PAPXs (PAPXFKP)
 */
/* This structure has been commented out because we can't handle it correctly
 * Please don't try to fix it here in this file, but rather copy this broken
 * structure definition and fix it in some auxilliary file. If you want to
 * include that aux. file here, please change the template file.
 */
//struct PAPXFKP {
//    /**
//     * Creates an empty PAPXFKP structure and sets the defaults
//     */
//    PAPXFKP();
//    /**
//     * Simply calls read(...)
//     */
//    PAPXFKP(AbstractOLEStreamReader *stream, bool preservePos=false);
//    /**
//     * Attention: This struct allocates memory on the heap
//     */
//    PAPXFKP(const PAPXFKP &rhs);
//    ~PAPXFKP();

//    PAPXFKP &operator=(const PAPXFKP &rhs);

//    /**
//     * This method reads the PAPXFKP structure from the stream.
//     * If  preservePos is true we push/pop the position of
//     * the stream to save the state. If it's false the state
//     * of stream will be changed!
//     */
//    bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

//    /**
//     * Same as reading :)
//     */
//    //bool write(OLEStreamWriter *stream, bool preservePos=false) const;

//    /**
//     * Set all the fields to the inital value (default is 0)
//     */
//    void clear();

//    // Data
//    /**
//     * Each FC is the limit FC of a paragraph (ie. points to the next character
//     * past an end of paragraph mark). There will be fkp.crun+1 recorded in the
//     * FKP.
//     */
//    FC *rgfc;   //    FC rgfc[fkp.crun+1];

//    /**
//     * an array of the BX data structure. The ith BX entry in the array describes
//     * the paragraph beginning at fkp.rgfc[i]. The BX is a seven byte data structure.
//     * The first byte of each BX is the word offset of thePAPX recorded for the
//     * paragraph corresponding to this BX. ..If the byte stored is 0, this represents
//     * a 1 line paragraph 15 pixels high with Normal style (stc == 0) whose column
//     * width is 7980 dxas.
//     * <p>The last six bytes of the BX is a PHE structure which stores the current
//     * paragraph height for the paragraph corresponding to the BX. If a plcfphe
//     * has an entry that maps to the FC for this paragraph, that entry?s PHE overides
//     * the PHE stored in the FKP.
//     */
//    BX *rgbx;   //    BX rgbx[fkp.crun];

//    /**
//     * As new runs/paragraphs are recorded in the FKP,unused space is reduced
//     * by 11 if CHPX/PAPX is already recorded and is reduced by11+sizeof(PAPX)
//     * if property is not already recorded.
//     */
//    U8 *unusedSpace;   //    U8 unusedSpace[];

//    /**
//     * grppapx consists of all of the PAPXs stored in FKP concatenated end
//     * to end. Each PAPX begins with a count of words which records its length
//     * padded to a word boundary.
//     */
//    U8 *grppapx;   //    U8 grppapx[];

//    /**
//     * count of paragraphs for PAPX FKP.
//     */
//    U8 crun;

//private:
//    void clearInternal();

//}; // PAPXFKP

//bool operator==(const PAPXFKP &lhs, const PAPXFKP &rhs);
//bool operator!=(const PAPXFKP &lhs, const PAPXFKP &rhs);


/**
 * Piece Descriptor (PCD)
 */
struct PCD {
    /**
     * Creates an empty PCD structure and sets the defaults
     */
    PCD();
    /**
     * Simply calls read(...)
     */
	PCD(AbstractOLEStreamReader *stream, bool preservePos=false);
    /**
     * Simply calls readPtr(...)
     */
    PCD(const U8 *ptr);

    /**
     * This method reads the PCD structure from the stream.
     * If  preservePos is true we push/pop the position of
     * the stream to save the state. If it's false the state
     * of stream will be changed!
     */
	bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * This method reads the struct from a pointer
     */
    void readPtr(const U8 *ptr);

    /**
     * Set all the fields to the inital value (default is 0)
     */
    void clear();

    // Size of the structure
    static const unsigned int sizeOf;

    // Data
    /**
     * when 1, means that piece contains no end of paragraph marks.
     */
    U16 fNoParaLast:1;

    /**
     * used internally by Word
     */
    U16 fPaphNil:1;

    /**
     * used internally by Word
     */
    U16 fCopied:1;

    U16 unused0_3:5;

    /**
     * used internally by Word
     */
    U16 fn:8;

    /**
     * file offset of beginning of piece. The size of the ithpiece can be
     * determined by subtracting rgcp[i] of the containing plcfpcd from its rgcp[i+1].
     */
    U32 fc;

    /**
     * contains either a single sprm or else an index number of the grpprl
     * which contains the sprms that modify the properties of the piece.
     */
    PRM prm;

}; // PCD

bool operator==(const PCD &lhs, const PCD &rhs);
bool operator!=(const PCD &lhs, const PCD &rhs);


/**
 * Page Descriptor (PGD)
 */
struct PGD {
    /**
     * Creates an empty PGD structure and sets the defaults
     */
    PGD();
    /**
     * Simply calls read(...)
     */
	PGD(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * This method reads the PGD structure from the stream.
     * If  preservePos is true we push/pop the position of
     * the stream to save the state. If it's false the state
     * of stream will be changed!
     */
	bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * Set all the fields to the inital value (default is 0)
     */
    void clear();

    // Data
    U16 unused0_0:5;

    /**
     * redefine fEmptyPage and fAllFtn. true when blank page or footnote only
     * page
     */
    U16 fGhost:2;

    U16 unused0_7:9;

    /**
     * 1 only when footnote is continued from previous page
     */
    U16 fContinue:1;

    /**
     * 1 when page is dirty (ie. pagination cannot be trusted)
     */
    U16 fUnk:1;

    /**
     * 1 when right hand side page
     */
    U16 fRight:1;

    /**
     * 1 when page number must be reset to 1.
     */
    U16 fPgnRestart:1;

    /**
     * 1 when section break forced page to be empty.
     */
    U16 fEmptyPage:1;

    /**
     * 1 when page contains nothing but footnotes
     */
    U16 fAllFtn:1;

    U16 fColOnly:1;

    U16 fTableBreaks:1;

    U16 fMarked:1;

    U16 fColumnBreaks:1;

    U16 fTableHeader:1;

    U16 fNewPage:1;

    /**
     * section break code
     */
    U16 bkc:4;

    /**
     * line number of first line, -1 if no line numbering
     */
    U16 lnn;

    /**
     * page number as printed
     */
    U16 pgn;

}; // PGD

bool operator==(const PGD &lhs, const PGD &rhs);
bool operator!=(const PGD &lhs, const PGD &rhs);


/**
 * Picture Descriptor (PICF)
 */
struct PICF : public Shared {
    /**
     * Creates an empty PICF structure and sets the defaults
     */
    PICF();
    /**
     * Simply calls read(...)
     */
	PICF(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * This method reads the PICF structure from the stream.
     * If  preservePos is true we push/pop the position of
     * the stream to save the state. If it's false the state
     * of stream will be changed!
     */
	bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * Set all the fields to the inital value (default is 0)
     */
    void clear();

    // Data
    /**
     * number of bytes in the PIC structure plus size of following picture
     * data which may be a Window's metafile, a bitmap, or the filename of a TIFF
     * file.
     */
    U32 lcb;

    /**
     * number of bytes in the PIC (to allow for future expansion).
     */
    U16 cbHeader;

    /**
     * If a Windows metafiles is stored immediatelly followingthe PIC structure,
     * the mfp is a Window's METAFILEPICT structure. When the data immediately
     * following the PIC is aTIFF filename, mfp.mm == 98. If a bitmap is stored
     * after the pic,mfp.mm == 99
     * When the PIC describes a bitmap, mfp.xExt is the width of the bitmap
     * in pixels and mfp.yExt is the height of the bitmap in pixels..
     */
    METAFILEPICT mfp;

    /**
     * Window's bitmap structure when PIC describes a BITMAP. rect for window
     * origin and extents whenmetafile is stored -- ignored if 0
     */
    U8 bm_rcWinMF[14];

    /**
     * horizontalmeasurement in twips of therectangle the picture should be
     * imaged within.
     */
    U16 dxaGoal;

    /**
     * verticalmeasurement in twips of therectangle the picture should be
     * imaged within. when scaling bitmaps, dxaGoal and dyaGoal may be ignored
     * if the operation would cause the bitmap to shrink or grow by anon -power-of-two
     * factor
     */
    U16 dyaGoal;

    /**
     * horizontal scaling factor supplied by user expressedin .001% units.
     */
    U16 mx;

    /**
     * vertical scaling factor supplied by user expressed in .001% units.
     * for all of the Crop values, a positive measurement means the specified
     * border has been moved inward from its original setting and a negative measurement
     * means the borderhas been moved outward from its original setting.
     */
    U16 my;

    /**
     * the amount the picture has been cropped on the left in twips.
     */
    U16 dxaCropLeft;

    /**
     * the amount the picture has been cropped on the top in twips.
     */
    U16 dyaCropTop;

    /**
     * the amount the picture has been cropped on the right in twips.
     */
    U16 dxaCropRight;

    /**
     * the amount the picture has been cropped on the bottom in twips.
     */
    U16 dyaCropBottom;

    /**
     * Obsolete, superseded by brcTop, etc.In WinWord 1.x, it was the type
     * of border to place around picture
     * 0 single
     * 1 thick
     * 2 double
     * 3 shadow
     */
    U16 brcl:4;

    /**
     * picture consists of a single frame
     */
    U16 fFrameEmpty:1;

    /**
     * ==1, when picture is just a bitmap
     */
    U16 fBitmap:1;

    /**
     * ==1, when picture is an active OLE object
     */
    U16 fDrawHatch:1;

    /**
     * ==1, when picture is just an error message
     */
    U16 fError:1;

    /**
     * bits per pixel
     * 0 unknown
     * 1 monochrome
     * 4
     */
    U16 bpp:8;

    /**
     * specification for border above picture
     */
    BRC brcTop;

    /**
     * specification for border to the left of picture
     */
    BRC brcLeft;

    /**
     * specification for border below picture
     */
    BRC brcBottom;

    /**
     * specification for border to the right of picture
     */
    BRC brcRight;

    /**
     * horizontal offset of hand annotation origin
     */
    U16 dxaOrigin;

    /**
     * vertical offset of hand annotation origin
     */
    U16 dyaOrigin;

}; // PICF

bool operator==(const PICF &lhs, const PICF &rhs);
bool operator!=(const PICF &lhs, const PICF &rhs);


/**
 * Plex of CPs stored in File (PLCF)
 */
/* This structure has been commented out because we can't handle it correctly
 * Please don't try to fix it here in this file, but rather copy this broken
 * structure definition and fix it in some auxilliary file. If you want to
 * include that aux. file here, please change the template file.
 */
//struct PLCF {
//    /**
//     * Creates an empty PLCF structure and sets the defaults
//     */
//    PLCF();
//    /**
//     * Simply calls read(...)
//     */
//    PLCF(AbstractOLEStreamReader *stream, bool preservePos=false);
//    /**
//     * Attention: This struct allocates memory on the heap
//     */
//    PLCF(const PLCF &rhs);
//    ~PLCF();

//    PLCF &operator=(const PLCF &rhs);

//    /**
//     * This method reads the PLCF structure from the stream.
//     * If  preservePos is true we push/pop the position of
//     * the stream to save the state. If it's false the state
//     * of stream will be changed!
//     */
//    bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

//    /**
//     * Same as reading :)
//     */
//    //bool write(OLEStreamWriter *stream, bool preservePos=false) const;

//    /**
//     * Set all the fields to the inital value (default is 0)
//     */
//    void clear();

//    // Data
//    /**
//     * given that the size of PLCF is cb and the size of the structure stored
//     * in plc is cbStruct, then the number of structure instances stored in PLCF,
//     * iMac is given by (cb -4)/(4 + cbStruct) The number of FCs stored in the
//     * PLCF will be iMac + 1.
//     */
//    FC *rgfc;   //    FC rgfc[];

//    struct *rgstruct;   //    struct rgstruct[];

//private:
//    void clearInternal();

//}; // PLCF

//bool operator==(const PLCF &lhs, const PLCF &rhs);
//bool operator!=(const PLCF &lhs, const PLCF &rhs);


/**
 * Section Descriptor (SED)
 */
struct SED {
    /**
     * Creates an empty SED structure and sets the defaults
     */
    SED();
    /**
     * Simply calls read(...)
     */
	SED(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * This method reads the SED structure from the stream.
     * If  preservePos is true we push/pop the position of
     * the stream to save the state. If it's false the state
     * of stream will be changed!
     */
	bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * Set all the fields to the inital value (default is 0)
     */
    void clear();

    // Size of the structure
    static const unsigned int sizeOf;

    // Data
    /**
     * runtime flag, indicates whether orientation should be changed before
     * printing. 0 indicates no change, 1 indicates orientation change.
     */
    U16 fSwap:1;

    /**
     * used internally by Windows Word
     */
    U16 fUnk:1;

    /**
     * used internally by Windows Word
     */
    U16 fn:14;

    /**
     * file offset to beginning of SEPX stored for section. If sed.fcSepx==
     * 0xFFFFFFFF, the section properties for the section are equal to the standard
     * SEP (see SEP definition).
     */
    U32 fcSepx;

    /**
     * used internally by Windows Word
     */
    U16 fnMpr;

    /**
     * points to offset in FC space where the Macintosh Print Record for a
     * document created on a Mac will be stored
     */
    U32 fcMpr;

}; // SED

bool operator==(const SED &lhs, const SED &rhs);
bool operator!=(const SED &lhs, const SED &rhs);


/**
 * Section Properties (SEP)
 */
struct SEP : public Shared {
    /**
     * Creates an empty SEP structure and sets the defaults
     */
    SEP();
    /**
     * Simply calls read(...)
     */
	SEP(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * This method reads the SEP structure from the stream.
     * If  preservePos is true we push/pop the position of
     * the stream to save the state. If it's false the state
     * of stream will be changed!
     */
	bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * Set all the fields to the inital value (default is 0)
     */
    void clear();

    /**
     * Dumps all fields of this structure (for debugging)
     */
    void dump() const;

    /**
     * Converts the data structure to a string (for debugging)
     */
    std::string toString() const;

    // Data
    /**
     * break code:
     * 0 No break
     * 1 New column
     * 2 New page
     * 3 Even page
     * 4 Odd page
     */
    U8 bkc;

    /**
     * set to 1 when a title page is to be displayed
     */
    U8 fTitlePage;

    /**
     * number of columns in section - 1.
     */
    U16 ccolM1;

    /**
     * distance that will be maintained between columns
     */
    U16 dxaColumns;

    /**
     * only for Mac compatability, used only during open, when 1, sep.dxaPgn
     * and sep.dyaPgn are valid page number locations
     */
    U8 fAutoPgn;

    /**
     * page number format code:
     * 0Arabic numbering
     * 1 Upper case Roman
     * 2 Lower case Roman
     * 3 Upper case Letter
     * 4 Lower case letter
     * 5 Ordinal
     */
    U8 nfcPgn;

    /**
     * user specified starting page number.
     */
    U16 pgnStart;

    /**
     * set to 1, when a section in a locked document is unlocked
     */
    U8 fUnlocked;

    /**
     * chapter number separator for page numbers
     */
    U8 cnsPgn;

    /**
     * set to 1 when page numbering should be restarted at the beginning of
     * this section
     */
    U8 fPgnRestart;

    /**
     * when 1, footnotes placed at end of section. When 0, footnotes are placed
     * at bottom of page.
     */
    U8 fEndNote;

    /**
     * line numbering code:
     * 0 Per page
     * 1 Restart
     * 2 Continue
     */
    U8 lnc;

    /**
     * specification of which headers and footers are included in this section.
     * See explanation inHeaders and Footers topic.
     */
    U8 grpfIhdt;

    /**
     * if 0, no line numbering, otherwise this is the line number modulus
     * (e.g. if nLnnMod is 5, line numbers appear on line 5, 10, etc.)
     */
    U16 nLnnMod;

    /**
     * distance of
     */
    U16 dxaLnn;

    /**
     * y position of top header measured from top edge of page.
     */
    U16 dyaHdrTop;

    /**
     * y position of top header measured from top edge of page.
     */
    U16 dyaHdrBottom;

    /**
     * when fAutoPgn ==1, gives the x position of auto page number on page
     * in twips (for Mac compatabilty only)
     */
    U16 dxaPgn;

    /**
     * when fAutoPgn ==1, gives the y position of auto page number on page
     * in twips (for Mac compatabilty only)
     */
    U16 dyaPgn;

    /**
     * when ==1, draw vertical lines between columns
     */
    U8 fLBetween;

    /**
     * vertical justification code
     * 0 top justified
     * 1 centered
     * 2 fully justified vertically
     * 3 bottom justified
     */
    U8 vjc;

    /**
     * beginning line number for section
     */
    U16 lnnMin;

    /**
     * orientation of pages in that section.set to 0 when portrait, 1 when
     * landscape
     */
    U8 dmOrientPage;

    /**
     * heading number level for page number
     */
    U8 iHeadingPgn;

    /**
     * width of page default value is 12240 twips
     */
    U16 xaPage;

    /**
     * height of page default value is 15840 twips
     */
    U16 yaPage;

    /**
     * left margin default value is 1800 twips
     */
    U16 dxaLeft;

    /**
     * right margin default value is 1800 twips
     */
    U16 dxaRight;

    /**
     * top margin default value is 1440 twips
     */
    U16 dyaTop;

    /**
     * bottom margin default value is 1440 twips
     */
    U16 dyaBottom;

    /**
     * gutter width default value is 0 twips
     */
    U16 dzaGutter;

    /**
     * bin number supplied from windows printer driver indicating which bin
     * the first page of section will be printed.
     */
    U16 dmBinFirst;

    /**
     * bin number supplied from windows printer driver indicating which bin
     * the pages other than the first page of section will be printed.
     */
    U16 dmBinOther;

    /**
     * dmPaper code for form selected by user
     */
    U16 dmPaperReq;

    /**
     * when == 1, columns are evenly spaced. Default value is 1.
     */
    U8 fEvenlySpaced;

    /**
     * reserved
     */
    U8 unused55;

    /**
     * used internally by Word
     */
    U16 dxaColumnWidth;

    /**
     * array of 89 Xas that determine bounds of irregular width columns
     */
    U16 rgdxaColumnWidthSpacing[89];

    /**
     * multilevel autonumbering list data (see OLST definition)
     */
    OLST olstAnm;

}; // SEP

bool operator==(const SEP &lhs, const SEP &rhs);
bool operator!=(const SEP &lhs, const SEP &rhs);


/**
 * Section Property Exceptions (SEPX)
 */
struct SEPX {
    /**
     * Creates an empty SEPX structure and sets the defaults
     */
    SEPX();
    /**
     * Simply calls read(...)
     */
	SEPX(AbstractOLEStreamReader *stream, bool preservePos=false);
    /**
     * Attention: This struct allocates memory on the heap
     */
    SEPX(const SEPX &rhs);
    ~SEPX();

    SEPX &operator=(const SEPX &rhs);

    /**
     * This method reads the SEPX structure from the stream.
     * If  preservePos is true we push/pop the position of
     * the stream to save the state. If it's false the state
     * of stream will be changed!
     */
	bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * Set all the fields to the inital value (default is 0)
     */
    void clear();

    // Data
    /**
     * count of bytes in remainder of SEPX.
     */
    U8 cb;

    /**
     * list of sprms that encodes the differences between the properties of
     * a section and Word's default section properties.
     */
    U8 *grpprl;   //    U8 grpprl[];

private:
    void clearInternal();

}; // SEPX

bool operator==(const SEPX &lhs, const SEPX &rhs);
bool operator!=(const SEPX &lhs, const SEPX &rhs);


/**
 * STyleSHeet Information (STSHI)
 */
struct STSHI {
    /**
     * Creates an empty STSHI structure and sets the defaults
     */
    STSHI();
    /**
     * Simply calls read(...)
     */
	STSHI(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * This method reads the STSHI structure from the stream.
     * If  preservePos is true we push/pop the position of
     * the stream to save the state. If it's false the state
     * of stream will be changed!
     */
	bool read(AbstractOLEStreamReader *stream, bool preservePos=false);

    /**
     * Set all the fields to the inital value (default is 0)
     */
    void clear();

    // Size of the structure
    static const unsigned int sizeOf;

    // Data
    /**
     * Count of styles in stylesheet
     */
    U16 cstd;

    /**
     * Length of STD Base as stored in a file
     */
    U16 cbSTDBaseInFile;

    /**
     * Are built-in stylenames stored?
     */
    U16 fStdStylenamesWritten:1;

    /**
     * Spare flags
     */
    U16 unused4_2:15;

    /**
     * Max sti known when this file was written
     */
    U16 stiMaxWhenSaved;

    /**
     * How many fixed-index istds are there?
     */
    U16 istdMaxFixedWhenSaved;

    /**
     * Current version of built-in stylenames
     */
    U16 nVerBuiltInNamesWhenSaved;

    /**
     * ftc used by StandardChpStsh for this document
     */
    U16 ftcStandardChpStsh;

}; // STSHI

bool operator==(const STSHI &lhs, const STSHI &rhs);
bool operator!=(const STSHI &lhs, const STSHI &rhs);



} // namespace Word95

} // namespace wvWare

#endif // WORD95_GENERATED_H
