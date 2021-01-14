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
// generating the sources. If you want to add some additional code, some
// includes or any other stuff, please add it to the template file!
// For information about the script and the "hidden features" please read
// the comments at the begin of the script.

// If you find bugs or strange behavior please contact Werner Trobin
// <trobin@kde.org>

#include "word97_generated.h"
#include "olestream.h"
#include <string.h>  // memset(), memcpy()
#include "wvlog.h"

namespace wvWare {

namespace Word97 {

// FFN implementation, located in template-Word97.cpp
FFN::FFN()
{
    clearInternal();
}

FFN::FFN(AbstractOLEStreamReader *stream, Version version, bool preservePos)
{
    clearInternal();
    read(stream, version, preservePos);
}

bool FFN::read(AbstractOLEStreamReader *stream, Version version, bool preservePos)
{

    U8 shifterU8;

    if (preservePos)
        stream->push();

    cbFfnM1 = stream->readU8();
    shifterU8 = stream->readU8();
    prq = shifterU8;
    shifterU8 >>= 2;
    fTrueType = shifterU8;
    shifterU8 >>= 1;
    unused1_3 = shifterU8;
    shifterU8 >>= 1;
    ff = shifterU8;
    shifterU8 >>= 3;
    unused1_7 = shifterU8;
    wWeight = stream->readS16();
    chs = stream->readU8();
    ixchSzAlt = stream->readU8();

    U8 remainingSize = cbFfnM1 - 5;

    if ( version == Word97 ) {
        for (int _i = 0; _i < (10); ++_i)
            panose[_i] = stream->readU8();
        for (int _i = 0; _i < (24); ++_i)
            fs[_i] = stream->readU8();
        remainingSize -= 34;

        // Remaining size in bytes -> shorts
        remainingSize /= 2;
        XCHAR *string = new XCHAR[ remainingSize ];
        for ( int i = 0; i < remainingSize; ++i )
            string[ i ] = stream->readU16();
        if ( ixchSzAlt == 0 )
            xszFfn = UString( reinterpret_cast<const wvWare::UChar *>( string ), remainingSize - 1 );
        else {
            xszFfn = UString( reinterpret_cast<const wvWare::UChar *>( string ), ixchSzAlt - 1 );
            xszFfnAlt = UString( reinterpret_cast<const wvWare::UChar *>( &string[ ixchSzAlt ] ), remainingSize - 1 - ixchSzAlt );
        }
        delete [] string;
    } else {
        U8 *string = new U8[ remainingSize ];
        stream->read( string, remainingSize );
        // ###### Assume plain latin1 strings, maybe we'll have to use a textconverter here...
        if ( ixchSzAlt == 0 )
            xszFfn = UString( reinterpret_cast<char *>( string ) );
        else {
            xszFfn = UString( reinterpret_cast<char *>( string ) ); // The strings are 0-terminated, according to the SPEC
            xszFfnAlt = UString( reinterpret_cast<char *>( &string[ ixchSzAlt ] ) );
        }
        delete [] string;
    }

    if (preservePos)
        stream->pop();
    return true;
}

void FFN::clear()
{
    clearInternal();
}

void FFN::clearInternal()
{
    cbFfnM1 = 0;
    prq = 0;
    fTrueType = 0;
    unused1_3 = 0;
    ff = 0;
    unused1_7 = 0;
    wWeight = 0;
    chs = 0;
    ixchSzAlt = 0;
    for (int _i = 0; _i < (10); ++_i)
        panose[_i] = 0;
    for (int _i = 0; _i < (24); ++_i)
        fs[_i] = 0;
    xszFfn = UString::null;
    xszFfnAlt = UString::null;
}

// There can be only one tab at a given position, no matter what the other options are
bool operator==( const TabDescriptor &lhs, const TabDescriptor &rhs )
{
    return lhs.dxaTab == rhs.dxaTab;
}
bool operator!=( const TabDescriptor &lhs, const TabDescriptor &rhs )
{
    return lhs.dxaTab != rhs.dxaTab;
}
bool operator<( const TabDescriptor &lhs, const TabDescriptor &rhs )
{
    return lhs.dxaTab < rhs.dxaTab;
}
bool operator>( const TabDescriptor &lhs, const TabDescriptor &rhs )
{
    return lhs.dxaTab > rhs.dxaTab;
}


// DTTM implementation

const unsigned int DTTM::sizeOf = 4;

DTTM::DTTM()
{
    clear();
}

DTTM::DTTM(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

DTTM::DTTM(const U8 *ptr)
{
    clear();
    readPtr(ptr);
}

bool DTTM::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    U16 shifterU16;

    if (preservePos)
        stream->push();

    shifterU16 = stream->readU16();
    mint = shifterU16;
    shifterU16 >>= 6;
    hr = shifterU16;
    shifterU16 >>= 5;
    dom = shifterU16;
    shifterU16 = stream->readU16();
    mon = shifterU16;
    shifterU16 >>= 4;
    yr = shifterU16;
    shifterU16 >>= 9;
    wdy = shifterU16;

    if (preservePos)
        stream->pop();
    return true;
}

void DTTM::readPtr(const U8 *ptr)
{

    U16 shifterU16;

    shifterU16 = readU16(ptr);
    ptr += sizeof(U16);
    mint = shifterU16;
    shifterU16 >>= 6;
    hr = shifterU16;
    shifterU16 >>= 5;
    dom = shifterU16;
    shifterU16 = readU16(ptr);
    ptr += sizeof(U16);
    mon = shifterU16;
    shifterU16 >>= 4;
    yr = shifterU16;
    shifterU16 >>= 9;
    wdy = shifterU16;
}

void DTTM::clear()
{
    mint = 0;
    hr = 0;
    dom = 0;
    mon = 0;
    yr = 0;
    wdy = 0;
}

void DTTM::dump() const
{
    wvlog << "Dumping DTTM:" << std::endl;
    wvlog << toString().c_str() << std::endl;
    wvlog << "\nDumping DTTM done." << std::endl;
}

std::string DTTM::toString() const
{
    std::string s( "DTTM:" );
    s += "\nmint=";
    s += uint2string( mint );
    s += "\nhr=";
    s += uint2string( hr );
    s += "\ndom=";
    s += uint2string( dom );
    s += "\nmon=";
    s += uint2string( mon );
    s += "\nyr=";
    s += uint2string( yr );
    s += "\nwdy=";
    s += uint2string( wdy );
    s += "\nDTTM Done.";
    return s;
}

bool operator==(const DTTM &lhs, const DTTM &rhs)
{

    return lhs.mint == rhs.mint &&
           lhs.hr == rhs.hr &&
           lhs.dom == rhs.dom &&
           lhs.mon == rhs.mon &&
           lhs.yr == rhs.yr &&
           lhs.wdy == rhs.wdy;
}

bool operator!=(const DTTM &lhs, const DTTM &rhs)
{
    return !(lhs == rhs);
}


// DOPTYPOGRAPHY implementation

DOPTYPOGRAPHY::DOPTYPOGRAPHY()
{
    clear();
}

DOPTYPOGRAPHY::DOPTYPOGRAPHY(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

bool DOPTYPOGRAPHY::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    U16 shifterU16;

    if (preservePos)
        stream->push();

    shifterU16 = stream->readU16();
    fKerningPunct = shifterU16;
    shifterU16 >>= 1;
    iJustification = shifterU16;
    shifterU16 >>= 2;
    iLevelOfKinsoku = shifterU16;
    shifterU16 >>= 2;
    f2on1 = shifterU16;
    shifterU16 >>= 1;
    unused0_6 = shifterU16;
    cchFollowingPunct = stream->readS16();
    cchLeadingPunct = stream->readS16();
    for (int _i = 0; _i < (101); ++_i)
        rgxchFPunct[_i] = stream->readU16();
    for (int _i = 0; _i < (51); ++_i)
        rgxchLPunct[_i] = stream->readU16();

    if (preservePos)
        stream->pop();
    return true;
}

void DOPTYPOGRAPHY::clear()
{
    fKerningPunct = 0;
    iJustification = 0;
    iLevelOfKinsoku = 0;
    f2on1 = 0;
    unused0_6 = 0;
    cchFollowingPunct = 0;
    cchLeadingPunct = 0;
    for (int _i = 0; _i < (101); ++_i)
        rgxchFPunct[_i] = 0;
    for (int _i = 0; _i < (51); ++_i)
        rgxchLPunct[_i] = 0;
}

bool operator==(const DOPTYPOGRAPHY &lhs, const DOPTYPOGRAPHY &rhs)
{

    for (int _i = 0; _i < (101); ++_i) {
        if (lhs.rgxchFPunct[_i] != rhs.rgxchFPunct[_i])
            return false;
    }

    for (int _i = 0; _i < (51); ++_i) {
        if (lhs.rgxchLPunct[_i] != rhs.rgxchLPunct[_i])
            return false;
    }

    return lhs.fKerningPunct == rhs.fKerningPunct &&
           lhs.iJustification == rhs.iJustification &&
           lhs.iLevelOfKinsoku == rhs.iLevelOfKinsoku &&
           lhs.f2on1 == rhs.f2on1 &&
           lhs.unused0_6 == rhs.unused0_6 &&
           lhs.cchFollowingPunct == rhs.cchFollowingPunct &&
           lhs.cchLeadingPunct == rhs.cchLeadingPunct;
}

bool operator!=(const DOPTYPOGRAPHY &lhs, const DOPTYPOGRAPHY &rhs)
{
    return !(lhs == rhs);
}


// PRM2 implementation

PRM2::PRM2()
{
    clear();
}

PRM2::PRM2(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

bool PRM2::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    U16 shifterU16;

    if (preservePos)
        stream->push();

    shifterU16 = stream->readU16();
    fComplex = shifterU16;
    shifterU16 >>= 1;
    igrpprl = shifterU16;

    if (preservePos)
        stream->pop();
    return true;
}

void PRM2::clear()
{
    fComplex = 0;
    igrpprl = 0;
}

bool operator==(const PRM2 &lhs, const PRM2 &rhs)
{

    return lhs.fComplex == rhs.fComplex &&
           lhs.igrpprl == rhs.igrpprl;
}

bool operator!=(const PRM2 &lhs, const PRM2 &rhs)
{
    return !(lhs == rhs);
}


// PRM implementation

const unsigned int PRM::sizeOf = 2;

PRM::PRM()
{
    clear();
}

PRM::PRM(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

PRM::PRM(const U8 *ptr)
{
    clear();
    readPtr(ptr);
}

bool PRM::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    U16 shifterU16;

    if (preservePos)
        stream->push();

    shifterU16 = stream->readU16();
    fComplex = shifterU16;
    shifterU16 >>= 1;
    isprm = shifterU16;
    shifterU16 >>= 7;
    val = shifterU16;

    if (preservePos)
        stream->pop();
    return true;
}

void PRM::readPtr(const U8 *ptr)
{

    U16 shifterU16;

    shifterU16 = readU16(ptr);
    ptr += sizeof(U16);
    fComplex = shifterU16;
    shifterU16 >>= 1;
    isprm = shifterU16;
    shifterU16 >>= 7;
    val = shifterU16;
}

void PRM::clear()
{
    fComplex = 0;
    isprm = 0;
    val = 0;
}

bool operator==(const PRM &lhs, const PRM &rhs)
{

    return lhs.fComplex == rhs.fComplex &&
           lhs.isprm == rhs.isprm &&
           lhs.val == rhs.val;
}

bool operator!=(const PRM &lhs, const PRM &rhs)
{
    return !(lhs == rhs);
}


// SHD implementation

const unsigned int SHD::sizeOf = 2;

SHD::SHD()
{
    clear();
}

SHD::SHD(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

SHD::SHD(const U8 *ptr)
{
    clear();
    readPtr(ptr);
}

bool SHD::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    U16 shifterU16;

    if (preservePos)
        stream->push();

    shifterU16 = stream->readU16();
    icoFore = shifterU16;
    shifterU16 >>= 5;
    icoBack = shifterU16;
    shifterU16 >>= 5;
    ipat = shifterU16;

    if (preservePos)
        stream->pop();
    return true;
}

void SHD::readPtr(const U8 *ptr)
{

    U16 shifterU16;

    shifterU16 = readU16(ptr);
    ptr += sizeof(U16);
    icoFore = shifterU16;
    shifterU16 >>= 5;
    icoBack = shifterU16;
    shifterU16 >>= 5;
    ipat = shifterU16;
}

void SHD::clear()
{
    icoFore = 0;
    icoBack = 0;
    ipat = 0;
}

void SHD::dump() const
{
    wvlog << "Dumping SHD:" << std::endl;
    wvlog << toString().c_str() << std::endl;
    wvlog << "\nDumping SHD done." << std::endl;
}

std::string SHD::toString() const
{
    std::string s( "SHD:" );
    s += "\nicoFore=";
    s += uint2string( icoFore );
    s += "\nicoBack=";
    s += uint2string( icoBack );
    s += "\nipat=";
    s += uint2string( ipat );
    s += "\nSHD Done.";
    return s;
}

bool operator==(const SHD &lhs, const SHD &rhs)
{

    return lhs.icoFore == rhs.icoFore &&
           lhs.icoBack == rhs.icoBack &&
           lhs.ipat == rhs.ipat;
}

bool operator!=(const SHD &lhs, const SHD &rhs)
{
    return !(lhs == rhs);
}


// PHE implementation

const unsigned int PHE::sizeOf = 12;

PHE::PHE()
{
    clear();
}

PHE::PHE(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

PHE::PHE(const U8 *ptr)
{
    clear();
    readPtr(ptr);
}

bool PHE::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    U16 shifterU16;

    if (preservePos)
        stream->push();

    shifterU16 = stream->readU16();
    fSpare = shifterU16;
    shifterU16 >>= 1;
    fUnk = shifterU16;
    shifterU16 >>= 1;
    fDiffLines = shifterU16;
    shifterU16 >>= 1;
    unused0_3 = shifterU16;
    shifterU16 >>= 5;
    clMac = shifterU16;
    unused2 = stream->readU16();
    dxaCol = stream->readS32();
    dym = stream->readS32();

    if (preservePos)
        stream->pop();
    return true;
}

void PHE::readPtr(const U8 *ptr)
{

    U16 shifterU16;

    shifterU16 = readU16(ptr);
    ptr += sizeof(U16);
    fSpare = shifterU16;
    shifterU16 >>= 1;
    fUnk = shifterU16;
    shifterU16 >>= 1;
    fDiffLines = shifterU16;
    shifterU16 >>= 1;
    unused0_3 = shifterU16;
    shifterU16 >>= 5;
    clMac = shifterU16;
    unused2 = readU16(ptr);
    ptr += sizeof(U16);
    dxaCol = readS32(ptr);
    ptr += sizeof(S32);
    dym = readS32(ptr);
    ptr += sizeof(S32);
}

void PHE::clear()
{
    fSpare = 0;
    fUnk = 0;
    fDiffLines = 0;
    unused0_3 = 0;
    clMac = 0;
    unused2 = 0;
    dxaCol = 0;
    dym = 0;
}

void PHE::dump() const
{
    wvlog << "Dumping PHE:" << std::endl;
    wvlog << toString().c_str() << std::endl;
    wvlog << "\nDumping PHE done." << std::endl;
}

std::string PHE::toString() const
{
    std::string s( "PHE:" );
    s += "\nfSpare=";
    s += uint2string( fSpare );
    s += "\nfUnk=";
    s += uint2string( fUnk );
    s += "\nfDiffLines=";
    s += uint2string( fDiffLines );
    s += "\nunused0_3=";
    s += uint2string( unused0_3 );
    s += "\nclMac=";
    s += uint2string( clMac );
    s += "\nunused2=";
    s += uint2string( unused2 );
    s += "\ndxaCol=";
    s += int2string( dxaCol );
    s += "\ndym=";
    s += int2string( dym );
    s += "\nPHE Done.";
    return s;
}

bool operator==(const PHE &lhs, const PHE &rhs)
{

    return lhs.fSpare == rhs.fSpare &&
           lhs.fUnk == rhs.fUnk &&
           lhs.fDiffLines == rhs.fDiffLines &&
           lhs.unused0_3 == rhs.unused0_3 &&
           lhs.clMac == rhs.clMac &&
           lhs.unused2 == rhs.unused2 &&
           lhs.dxaCol == rhs.dxaCol &&
           lhs.dym == rhs.dym;
}

bool operator!=(const PHE &lhs, const PHE &rhs)
{
    return !(lhs == rhs);
}


// BRC implementation

const unsigned int BRC::sizeOf = 4;

BRC::BRC()
{
    clear();
}

BRC::BRC(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

BRC::BRC(const U8 *ptr)
{
    clear();
    readPtr(ptr);
}

bool BRC::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    U16 shifterU16;

    if (preservePos)
        stream->push();

    shifterU16 = stream->readU16();
    dptLineWidth = shifterU16;
    shifterU16 >>= 8;
    brcType = shifterU16;
    shifterU16 = stream->readU16();
    ico = shifterU16;
    shifterU16 >>= 8;
    dptSpace = shifterU16;
    shifterU16 >>= 5;
    fShadow = shifterU16;
    shifterU16 >>= 1;
    fFrame = shifterU16;
    shifterU16 >>= 1;
    unused2_15 = shifterU16;

    if (preservePos)
        stream->pop();
    return true;
}

void BRC::readPtr(const U8 *ptr)
{

    U16 shifterU16;

    shifterU16 = readU16(ptr);
    ptr += sizeof(U16);
    dptLineWidth = shifterU16;
    shifterU16 >>= 8;
    brcType = shifterU16;
    shifterU16 = readU16(ptr);
    ptr += sizeof(U16);
    ico = shifterU16;
    shifterU16 >>= 8;
    dptSpace = shifterU16;
    shifterU16 >>= 5;
    fShadow = shifterU16;
    shifterU16 >>= 1;
    fFrame = shifterU16;
    shifterU16 >>= 1;
    unused2_15 = shifterU16;
}

void BRC::clear()
{
    dptLineWidth = 0;
    brcType = 0;
    ico = 0;
    dptSpace = 0;
    fShadow = 0;
    fFrame = 0;
    unused2_15 = 0;
}

void BRC::dump() const
{
    wvlog << "Dumping BRC:" << std::endl;
    wvlog << toString().c_str() << std::endl;
    wvlog << "\nDumping BRC done." << std::endl;
}

std::string BRC::toString() const
{
    std::string s( "BRC:" );
    s += "\ndptLineWidth=";
    s += uint2string( dptLineWidth );
    s += "\nbrcType=";
    s += uint2string( brcType );
    s += "\nico=";
    s += uint2string( ico );
    s += "\ndptSpace=";
    s += uint2string( dptSpace );
    s += "\nfShadow=";
    s += uint2string( fShadow );
    s += "\nfFrame=";
    s += uint2string( fFrame );
    s += "\nunused2_15=";
    s += uint2string( unused2_15 );
    s += "\nBRC Done.";
    return s;
}

bool operator==(const BRC &lhs, const BRC &rhs)
{

    return lhs.dptLineWidth == rhs.dptLineWidth &&
           lhs.brcType == rhs.brcType &&
           lhs.ico == rhs.ico &&
           lhs.dptSpace == rhs.dptSpace &&
           lhs.fShadow == rhs.fShadow &&
           lhs.fFrame == rhs.fFrame &&
           lhs.unused2_15 == rhs.unused2_15;
}

bool operator!=(const BRC &lhs, const BRC &rhs)
{
    return !(lhs == rhs);
}


// TLP implementation

TLP::TLP()
{
    clear();
}

TLP::TLP(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

TLP::TLP(const U8 *ptr)
{
    clear();
    readPtr(ptr);
}

bool TLP::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    U16 shifterU16;

    if (preservePos)
        stream->push();

    itl = stream->readS16();
    shifterU16 = stream->readU16();
    fBorders = shifterU16;
    shifterU16 >>= 1;
    fShading = shifterU16;
    shifterU16 >>= 1;
    fFont = shifterU16;
    shifterU16 >>= 1;
    fColor = shifterU16;
    shifterU16 >>= 1;
    fBestFit = shifterU16;
    shifterU16 >>= 1;
    fHdrRows = shifterU16;
    shifterU16 >>= 1;
    fLastRow = shifterU16;
    shifterU16 >>= 1;
    fHdrCols = shifterU16;
    shifterU16 >>= 1;
    fLastCol = shifterU16;
    shifterU16 >>= 1;
    unused2_9 = shifterU16;

    if (preservePos)
        stream->pop();
    return true;
}

void TLP::readPtr(const U8 *ptr)
{

    U16 shifterU16;

    itl = readS16(ptr);
    ptr += sizeof(S16);
    shifterU16 = readU16(ptr);
    ptr += sizeof(U16);
    fBorders = shifterU16;
    shifterU16 >>= 1;
    fShading = shifterU16;
    shifterU16 >>= 1;
    fFont = shifterU16;
    shifterU16 >>= 1;
    fColor = shifterU16;
    shifterU16 >>= 1;
    fBestFit = shifterU16;
    shifterU16 >>= 1;
    fHdrRows = shifterU16;
    shifterU16 >>= 1;
    fLastRow = shifterU16;
    shifterU16 >>= 1;
    fHdrCols = shifterU16;
    shifterU16 >>= 1;
    fLastCol = shifterU16;
    shifterU16 >>= 1;
    unused2_9 = shifterU16;
}

void TLP::clear()
{
    itl = 0;
    fBorders = 0;
    fShading = 0;
    fFont = 0;
    fColor = 0;
    fBestFit = 0;
    fHdrRows = 0;
    fLastRow = 0;
    fHdrCols = 0;
    fLastCol = 0;
    unused2_9 = 0;
}

void TLP::dump() const
{
    wvlog << "Dumping TLP:" << std::endl;
    wvlog << toString().c_str() << std::endl;
    wvlog << "\nDumping TLP done." << std::endl;
}

std::string TLP::toString() const
{
    std::string s( "TLP:" );
    s += "\nitl=";
    s += int2string( itl );
    s += "\nfBorders=";
    s += uint2string( fBorders );
    s += "\nfShading=";
    s += uint2string( fShading );
    s += "\nfFont=";
    s += uint2string( fFont );
    s += "\nfColor=";
    s += uint2string( fColor );
    s += "\nfBestFit=";
    s += uint2string( fBestFit );
    s += "\nfHdrRows=";
    s += uint2string( fHdrRows );
    s += "\nfLastRow=";
    s += uint2string( fLastRow );
    s += "\nfHdrCols=";
    s += uint2string( fHdrCols );
    s += "\nfLastCol=";
    s += uint2string( fLastCol );
    s += "\nunused2_9=";
    s += uint2string( unused2_9 );
    s += "\nTLP Done.";
    return s;
}

bool operator==(const TLP &lhs, const TLP &rhs)
{

    return lhs.itl == rhs.itl &&
           lhs.fBorders == rhs.fBorders &&
           lhs.fShading == rhs.fShading &&
           lhs.fFont == rhs.fFont &&
           lhs.fColor == rhs.fColor &&
           lhs.fBestFit == rhs.fBestFit &&
           lhs.fHdrRows == rhs.fHdrRows &&
           lhs.fLastRow == rhs.fLastRow &&
           lhs.fHdrCols == rhs.fHdrCols &&
           lhs.fLastCol == rhs.fLastCol &&
           lhs.unused2_9 == rhs.unused2_9;
}

bool operator!=(const TLP &lhs, const TLP &rhs)
{
    return !(lhs == rhs);
}


// TC implementation

const unsigned int TC::sizeOf = 20;

TC::TC()
{
    clear();
}

TC::TC(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

TC::TC(const U8 *ptr)
{
    clear();
    readPtr(ptr);
}

bool TC::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    U16 shifterU16;

    if (preservePos)
        stream->push();

    shifterU16 = stream->readU16();
    fFirstMerged = shifterU16;
    shifterU16 >>= 1;
    fMerged = shifterU16;
    shifterU16 >>= 1;
    fVertical = shifterU16;
    shifterU16 >>= 1;
    fBackward = shifterU16;
    shifterU16 >>= 1;
    fRotateFont = shifterU16;
    shifterU16 >>= 1;
    fVertMerge = shifterU16;
    shifterU16 >>= 1;
    fVertRestart = shifterU16;
    shifterU16 >>= 1;
    vertAlign = shifterU16;
    shifterU16 >>= 2;
    fUnused = shifterU16;
    wUnused = stream->readU16();
    brcTop.read(stream, false);
    brcLeft.read(stream, false);
    brcBottom.read(stream, false);
    brcRight.read(stream, false);

    if (preservePos)
        stream->pop();
    return true;
}

void TC::readPtr(const U8 *ptr)
{

    U16 shifterU16;

    shifterU16 = readU16(ptr);
    ptr += sizeof(U16);
    fFirstMerged = shifterU16;
    shifterU16 >>= 1;
    fMerged = shifterU16;
    shifterU16 >>= 1;
    fVertical = shifterU16;
    shifterU16 >>= 1;
    fBackward = shifterU16;
    shifterU16 >>= 1;
    fRotateFont = shifterU16;
    shifterU16 >>= 1;
    fVertMerge = shifterU16;
    shifterU16 >>= 1;
    fVertRestart = shifterU16;
    shifterU16 >>= 1;
    vertAlign = shifterU16;
    shifterU16 >>= 2;
    fUnused = shifterU16;
    wUnused = readU16(ptr);
    ptr += sizeof(U16);
    brcTop.readPtr(ptr);
    ptr += BRC::sizeOf;
    brcLeft.readPtr(ptr);
    ptr += BRC::sizeOf;
    brcBottom.readPtr(ptr);
    ptr += BRC::sizeOf;
    brcRight.readPtr(ptr);
    ptr += BRC::sizeOf;
}

void TC::clear()
{
    fFirstMerged = 0;
    fMerged = 0;
    fVertical = 0;
    fBackward = 0;
    fRotateFont = 0;
    fVertMerge = 0;
    fVertRestart = 0;
    vertAlign = 0;
    fUnused = 0;
    wUnused = 0;
    brcTop.clear();
    brcLeft.clear();
    brcBottom.clear();
    brcRight.clear();
}

void TC::dump() const
{
    wvlog << "Dumping TC:" << std::endl;
    wvlog << toString().c_str() << std::endl;
    wvlog << "\nDumping TC done." << std::endl;
}

std::string TC::toString() const
{
    std::string s( "TC:" );
    s += "\nfFirstMerged=";
    s += uint2string( fFirstMerged );
    s += "\nfMerged=";
    s += uint2string( fMerged );
    s += "\nfVertical=";
    s += uint2string( fVertical );
    s += "\nfBackward=";
    s += uint2string( fBackward );
    s += "\nfRotateFont=";
    s += uint2string( fRotateFont );
    s += "\nfVertMerge=";
    s += uint2string( fVertMerge );
    s += "\nfVertRestart=";
    s += uint2string( fVertRestart );
    s += "\nvertAlign=";
    s += uint2string( vertAlign );
    s += "\nfUnused=";
    s += uint2string( fUnused );
    s += "\nwUnused=";
    s += uint2string( wUnused );
    s += "\nbrcTop=";
    s += "\n{" + brcTop.toString() + "}\n";
    s += "\nbrcLeft=";
    s += "\n{" + brcLeft.toString() + "}\n";
    s += "\nbrcBottom=";
    s += "\n{" + brcBottom.toString() + "}\n";
    s += "\nbrcRight=";
    s += "\n{" + brcRight.toString() + "}\n";
    s += "\nTC Done.";
    return s;
}

bool operator==(const TC &lhs, const TC &rhs)
{

    return lhs.fFirstMerged == rhs.fFirstMerged &&
           lhs.fMerged == rhs.fMerged &&
           lhs.fVertical == rhs.fVertical &&
           lhs.fBackward == rhs.fBackward &&
           lhs.fRotateFont == rhs.fRotateFont &&
           lhs.fVertMerge == rhs.fVertMerge &&
           lhs.fVertRestart == rhs.fVertRestart &&
           lhs.vertAlign == rhs.vertAlign &&
           lhs.fUnused == rhs.fUnused &&
           lhs.wUnused == rhs.wUnused &&
           lhs.brcTop == rhs.brcTop &&
           lhs.brcLeft == rhs.brcLeft &&
           lhs.brcBottom == rhs.brcBottom &&
           lhs.brcRight == rhs.brcRight;
}

bool operator!=(const TC &lhs, const TC &rhs)
{
    return !(lhs == rhs);
}


// TAP implementation

TAP::TAP() : Shared()
{
    clear();
}

TAP::TAP(AbstractOLEStreamReader *stream, bool preservePos) : Shared()
{
    clear();
    read(stream, preservePos);
}

bool TAP::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    U16 shifterU16;

    if (preservePos)
        stream->push();

    jc = stream->readS16();
    dxaGapHalf = stream->readS32();
    dyaRowHeight = stream->readS32();
    fCantSplit = stream->readU8();
    fTableHeader = stream->readU8();
    tlp.read(stream, false);
    lwHTMLProps = stream->readS32();
    shifterU16 = stream->readU16();
    fCaFull = shifterU16;
    shifterU16 >>= 1;
    fFirstRow = shifterU16;
    shifterU16 >>= 1;
    fLastRow = shifterU16;
    shifterU16 >>= 1;
    fOutline = shifterU16;
    shifterU16 >>= 1;
    unused20_12 = shifterU16;
    itcMac = stream->readS16();
    dxaAdjust = stream->readS32();
    dxaScale = stream->readS32();
    dxsInch = stream->readS32();
    // skipping the std::vector rgdxaCenter
    // skipping the std::vector rgdxaCenterPrint
    // skipping the std::vector rgtc
    // skipping the std::vector rgshd
    for (int _i = 0; _i < (6); ++_i)
        rgbrcTable[_i].read(stream, false);

    if (preservePos)
        stream->pop();
    return true;
}

void TAP::clear()
{
    jc = 0;
    dxaGapHalf = 0;
    dyaRowHeight = 0;
    fCantSplit = 0;
    fTableHeader = 0;
    tlp.clear();
    lwHTMLProps = 0;
    fCaFull = 0;
    fFirstRow = 0;
    fLastRow = 0;
    fOutline = 0;
    unused20_12 = 0;
    itcMac = 0;
    dxaAdjust = 0;
    dxaScale = 0;
    dxsInch = 0;
    rgdxaCenter.clear();
    rgdxaCenterPrint.clear();
    rgtc.clear();
    rgshd.clear();
    for (int _i = 0; _i < (6); ++_i)
        rgbrcTable[_i].clear();
}

void TAP::dump() const
{
    wvlog << "Dumping TAP:" << std::endl;
    wvlog << toString().c_str() << std::endl;
    wvlog << "\nDumping TAP done." << std::endl;
}

std::string TAP::toString() const
{
    std::string s( "TAP:" );
    s += "\njc=";
    s += int2string( jc );
    s += "\ndxaGapHalf=";
    s += int2string( dxaGapHalf );
    s += "\ndyaRowHeight=";
    s += int2string( dyaRowHeight );
    s += "\nfCantSplit=";
    s += uint2string( fCantSplit );
    s += "\nfTableHeader=";
    s += uint2string( fTableHeader );
    s += "\ntlp=";
    s += "\n{" + tlp.toString() + "}\n";
    s += "\nlwHTMLProps=";
    s += int2string( lwHTMLProps );
    s += "\nfCaFull=";
    s += uint2string( fCaFull );
    s += "\nfFirstRow=";
    s += uint2string( fFirstRow );
    s += "\nfLastRow=";
    s += uint2string( fLastRow );
    s += "\nfOutline=";
    s += uint2string( fOutline );
    s += "\nunused20_12=";
    s += uint2string( unused20_12 );
    s += "\nitcMac=";
    s += int2string( itcMac );
    s += "\ndxaAdjust=";
    s += int2string( dxaAdjust );
    s += "\ndxaScale=";
    s += int2string( dxaScale );
    s += "\ndxsInch=";
    s += int2string( dxsInch );
    s += "\nrgdxaCenter=";
    // skipping the std::vector rgdxaCenter
    s += "\nrgdxaCenterPrint=";
    // skipping the std::vector rgdxaCenterPrint
    s += "\nrgtc=";
    // skipping the std::vector rgtc
    s += "\nrgshd=";
    // skipping the std::vector rgshd
    for (int _i = 0; _i < (6); ++_i) {
        s += "\nrgbrcTable[" + int2string( _i ) + "]=";
        s += "\n{" + rgbrcTable[_i].toString() + "}\n";
    }
    s += "\nTAP Done.";
    return s;
}

bool operator==(const TAP &lhs, const TAP &rhs)
{

    for (int _i = 0; _i < (6); ++_i) {
        if (lhs.rgbrcTable[_i] != rhs.rgbrcTable[_i])
            return false;
    }

    return lhs.jc == rhs.jc &&
           lhs.dxaGapHalf == rhs.dxaGapHalf &&
           lhs.dyaRowHeight == rhs.dyaRowHeight &&
           lhs.fCantSplit == rhs.fCantSplit &&
           lhs.fTableHeader == rhs.fTableHeader &&
           lhs.tlp == rhs.tlp &&
           lhs.lwHTMLProps == rhs.lwHTMLProps &&
           lhs.fCaFull == rhs.fCaFull &&
           lhs.fFirstRow == rhs.fFirstRow &&
           lhs.fLastRow == rhs.fLastRow &&
           lhs.fOutline == rhs.fOutline &&
           lhs.unused20_12 == rhs.unused20_12 &&
           lhs.itcMac == rhs.itcMac &&
           lhs.dxaAdjust == rhs.dxaAdjust &&
           lhs.dxaScale == rhs.dxaScale &&
           lhs.dxsInch == rhs.dxsInch &&
           lhs.rgdxaCenter == rhs.rgdxaCenter &&
           lhs.rgdxaCenterPrint == rhs.rgdxaCenterPrint &&
           lhs.rgtc == rhs.rgtc &&
           lhs.rgshd == rhs.rgshd;
}

bool operator!=(const TAP &lhs, const TAP &rhs)
{
    return !(lhs == rhs);
}


// ANLD implementation

ANLD::ANLD()
{
    clear();
}

ANLD::ANLD(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

ANLD::ANLD(const U8 *ptr)
{
    clear();
    readPtr(ptr);
}

bool ANLD::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    U8 shifterU8;

    if (preservePos)
        stream->push();

    nfc = stream->readU8();
    cxchTextBefore = stream->readU8();
    cxchTextAfter = stream->readU8();
    shifterU8 = stream->readU8();
    jc = shifterU8;
    shifterU8 >>= 2;
    fPrev = shifterU8;
    shifterU8 >>= 1;
    fHang = shifterU8;
    shifterU8 >>= 1;
    fSetBold = shifterU8;
    shifterU8 >>= 1;
    fSetItalic = shifterU8;
    shifterU8 >>= 1;
    fSetSmallCaps = shifterU8;
    shifterU8 >>= 1;
    fSetCaps = shifterU8;
    shifterU8 = stream->readU8();
    fSetStrike = shifterU8;
    shifterU8 >>= 1;
    fSetKul = shifterU8;
    shifterU8 >>= 1;
    fPrevSpace = shifterU8;
    shifterU8 >>= 1;
    fBold = shifterU8;
    shifterU8 >>= 1;
    fItalic = shifterU8;
    shifterU8 >>= 1;
    fSmallCaps = shifterU8;
    shifterU8 >>= 1;
    fCaps = shifterU8;
    shifterU8 >>= 1;
    fStrike = shifterU8;
    shifterU8 = stream->readU8();
    kul = shifterU8;
    shifterU8 >>= 3;
    ico = shifterU8;
    ftc = stream->readS16();
    hps = stream->readU16();
    iStartAt = stream->readU16();
    dxaIndent = stream->readU16();
    dxaSpace = stream->readU16();
    fNumber1 = stream->readU8();
    fNumberAcross = stream->readU8();
    fRestartHdn = stream->readU8();
    fSpareX = stream->readU8();
    for (int _i = 0; _i < (32); ++_i)
        rgxch[_i] = stream->readU16();

    if (preservePos)
        stream->pop();
    return true;
}

void ANLD::readPtr(const U8 *ptr)
{

    U8 shifterU8;

    nfc = readU8(ptr);
    ptr += sizeof(U8);
    cxchTextBefore = readU8(ptr);
    ptr += sizeof(U8);
    cxchTextAfter = readU8(ptr);
    ptr += sizeof(U8);
    shifterU8 = readU8(ptr);
    ptr += sizeof(U8);
    jc = shifterU8;
    shifterU8 >>= 2;
    fPrev = shifterU8;
    shifterU8 >>= 1;
    fHang = shifterU8;
    shifterU8 >>= 1;
    fSetBold = shifterU8;
    shifterU8 >>= 1;
    fSetItalic = shifterU8;
    shifterU8 >>= 1;
    fSetSmallCaps = shifterU8;
    shifterU8 >>= 1;
    fSetCaps = shifterU8;
    shifterU8 = readU8(ptr);
    ptr += sizeof(U8);
    fSetStrike = shifterU8;
    shifterU8 >>= 1;
    fSetKul = shifterU8;
    shifterU8 >>= 1;
    fPrevSpace = shifterU8;
    shifterU8 >>= 1;
    fBold = shifterU8;
    shifterU8 >>= 1;
    fItalic = shifterU8;
    shifterU8 >>= 1;
    fSmallCaps = shifterU8;
    shifterU8 >>= 1;
    fCaps = shifterU8;
    shifterU8 >>= 1;
    fStrike = shifterU8;
    shifterU8 = readU8(ptr);
    ptr += sizeof(U8);
    kul = shifterU8;
    shifterU8 >>= 3;
    ico = shifterU8;
    ftc = readS16(ptr);
    ptr += sizeof(S16);
    hps = readU16(ptr);
    ptr += sizeof(U16);
    iStartAt = readU16(ptr);
    ptr += sizeof(U16);
    dxaIndent = readU16(ptr);
    ptr += sizeof(U16);
    dxaSpace = readU16(ptr);
    ptr += sizeof(U16);
    fNumber1 = readU8(ptr);
    ptr += sizeof(U8);
    fNumberAcross = readU8(ptr);
    ptr += sizeof(U8);
    fRestartHdn = readU8(ptr);
    ptr += sizeof(U8);
    fSpareX = readU8(ptr);
    ptr += sizeof(U8);
    for (int _i = 0; _i < (32); ++_i) {
        rgxch[_i] = readU16(ptr);
        ptr += sizeof(U16);
    }
}

void ANLD::clear()
{
    nfc = 0;
    cxchTextBefore = 0;
    cxchTextAfter = 0;
    jc = 0;
    fPrev = 0;
    fHang = 0;
    fSetBold = 0;
    fSetItalic = 0;
    fSetSmallCaps = 0;
    fSetCaps = 0;
    fSetStrike = 0;
    fSetKul = 0;
    fPrevSpace = 0;
    fBold = 0;
    fItalic = 0;
    fSmallCaps = 0;
    fCaps = 0;
    fStrike = 0;
    kul = 0;
    ico = 0;
    ftc = 0;
    hps = 0;
    iStartAt = 0;
    dxaIndent = 0;
    dxaSpace = 0;
    fNumber1 = 0;
    fNumberAcross = 0;
    fRestartHdn = 0;
    fSpareX = 0;
    for (int _i = 0; _i < (32); ++_i)
        rgxch[_i] = 0;
}

void ANLD::dump() const
{
    wvlog << "Dumping ANLD:" << std::endl;
    wvlog << toString().c_str() << std::endl;
    wvlog << "\nDumping ANLD done." << std::endl;
}

std::string ANLD::toString() const
{
    std::string s( "ANLD:" );
    s += "\nnfc=";
    s += uint2string( nfc );
    s += "\ncxchTextBefore=";
    s += uint2string( cxchTextBefore );
    s += "\ncxchTextAfter=";
    s += uint2string( cxchTextAfter );
    s += "\njc=";
    s += uint2string( jc );
    s += "\nfPrev=";
    s += uint2string( fPrev );
    s += "\nfHang=";
    s += uint2string( fHang );
    s += "\nfSetBold=";
    s += uint2string( fSetBold );
    s += "\nfSetItalic=";
    s += uint2string( fSetItalic );
    s += "\nfSetSmallCaps=";
    s += uint2string( fSetSmallCaps );
    s += "\nfSetCaps=";
    s += uint2string( fSetCaps );
    s += "\nfSetStrike=";
    s += uint2string( fSetStrike );
    s += "\nfSetKul=";
    s += uint2string( fSetKul );
    s += "\nfPrevSpace=";
    s += uint2string( fPrevSpace );
    s += "\nfBold=";
    s += uint2string( fBold );
    s += "\nfItalic=";
    s += uint2string( fItalic );
    s += "\nfSmallCaps=";
    s += uint2string( fSmallCaps );
    s += "\nfCaps=";
    s += uint2string( fCaps );
    s += "\nfStrike=";
    s += uint2string( fStrike );
    s += "\nkul=";
    s += uint2string( kul );
    s += "\nico=";
    s += uint2string( ico );
    s += "\nftc=";
    s += int2string( ftc );
    s += "\nhps=";
    s += uint2string( hps );
    s += "\niStartAt=";
    s += uint2string( iStartAt );
    s += "\ndxaIndent=";
    s += uint2string( dxaIndent );
    s += "\ndxaSpace=";
    s += uint2string( dxaSpace );
    s += "\nfNumber1=";
    s += uint2string( fNumber1 );
    s += "\nfNumberAcross=";
    s += uint2string( fNumberAcross );
    s += "\nfRestartHdn=";
    s += uint2string( fRestartHdn );
    s += "\nfSpareX=";
    s += uint2string( fSpareX );
    for (int _i = 0; _i < (32); ++_i) {
        s += "\nrgxch[" + int2string( _i ) + "]=";
        s += uint2string( rgxch[_i] );
    }
    s += "\nANLD Done.";
    return s;
}

bool operator==(const ANLD &lhs, const ANLD &rhs)
{

    for (int _i = 0; _i < (32); ++_i) {
        if (lhs.rgxch[_i] != rhs.rgxch[_i])
            return false;
    }

    return lhs.nfc == rhs.nfc &&
           lhs.cxchTextBefore == rhs.cxchTextBefore &&
           lhs.cxchTextAfter == rhs.cxchTextAfter &&
           lhs.jc == rhs.jc &&
           lhs.fPrev == rhs.fPrev &&
           lhs.fHang == rhs.fHang &&
           lhs.fSetBold == rhs.fSetBold &&
           lhs.fSetItalic == rhs.fSetItalic &&
           lhs.fSetSmallCaps == rhs.fSetSmallCaps &&
           lhs.fSetCaps == rhs.fSetCaps &&
           lhs.fSetStrike == rhs.fSetStrike &&
           lhs.fSetKul == rhs.fSetKul &&
           lhs.fPrevSpace == rhs.fPrevSpace &&
           lhs.fBold == rhs.fBold &&
           lhs.fItalic == rhs.fItalic &&
           lhs.fSmallCaps == rhs.fSmallCaps &&
           lhs.fCaps == rhs.fCaps &&
           lhs.fStrike == rhs.fStrike &&
           lhs.kul == rhs.kul &&
           lhs.ico == rhs.ico &&
           lhs.ftc == rhs.ftc &&
           lhs.hps == rhs.hps &&
           lhs.iStartAt == rhs.iStartAt &&
           lhs.dxaIndent == rhs.dxaIndent &&
           lhs.dxaSpace == rhs.dxaSpace &&
           lhs.fNumber1 == rhs.fNumber1 &&
           lhs.fNumberAcross == rhs.fNumberAcross &&
           lhs.fRestartHdn == rhs.fRestartHdn &&
           lhs.fSpareX == rhs.fSpareX;
}

bool operator!=(const ANLD &lhs, const ANLD &rhs)
{
    return !(lhs == rhs);
}


// ANLV implementation

const unsigned int ANLV::sizeOf = 16;

ANLV::ANLV()
{
    clear();
}

ANLV::ANLV(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

ANLV::ANLV(const U8 *ptr)
{
    clear();
    readPtr(ptr);
}

bool ANLV::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    U8 shifterU8;

    if (preservePos)
        stream->push();

    nfc = stream->readU8();
    cxchTextBefore = stream->readU8();
    cxchTextAfter = stream->readU8();
    shifterU8 = stream->readU8();
    jc = shifterU8;
    shifterU8 >>= 2;
    fPrev = shifterU8;
    shifterU8 >>= 1;
    fHang = shifterU8;
    shifterU8 >>= 1;
    fSetBold = shifterU8;
    shifterU8 >>= 1;
    fSetItalic = shifterU8;
    shifterU8 >>= 1;
    fSetSmallCaps = shifterU8;
    shifterU8 >>= 1;
    fSetCaps = shifterU8;
    shifterU8 = stream->readU8();
    fSetStrike = shifterU8;
    shifterU8 >>= 1;
    fSetKul = shifterU8;
    shifterU8 >>= 1;
    fPrevSpace = shifterU8;
    shifterU8 >>= 1;
    fBold = shifterU8;
    shifterU8 >>= 1;
    fItalic = shifterU8;
    shifterU8 >>= 1;
    fSmallCaps = shifterU8;
    shifterU8 >>= 1;
    fCaps = shifterU8;
    shifterU8 >>= 1;
    fStrike = shifterU8;
    shifterU8 = stream->readU8();
    kul = shifterU8;
    shifterU8 >>= 3;
    ico = shifterU8;
    ftc = stream->readS16();
    hps = stream->readU16();
    iStartAt = stream->readU16();
    dxaIndent = stream->readU16();
    dxaSpace = stream->readU16();

    if (preservePos)
        stream->pop();
    return true;
}

void ANLV::readPtr(const U8 *ptr)
{

    U8 shifterU8;

    nfc = readU8(ptr);
    ptr += sizeof(U8);
    cxchTextBefore = readU8(ptr);
    ptr += sizeof(U8);
    cxchTextAfter = readU8(ptr);
    ptr += sizeof(U8);
    shifterU8 = readU8(ptr);
    ptr += sizeof(U8);
    jc = shifterU8;
    shifterU8 >>= 2;
    fPrev = shifterU8;
    shifterU8 >>= 1;
    fHang = shifterU8;
    shifterU8 >>= 1;
    fSetBold = shifterU8;
    shifterU8 >>= 1;
    fSetItalic = shifterU8;
    shifterU8 >>= 1;
    fSetSmallCaps = shifterU8;
    shifterU8 >>= 1;
    fSetCaps = shifterU8;
    shifterU8 = readU8(ptr);
    ptr += sizeof(U8);
    fSetStrike = shifterU8;
    shifterU8 >>= 1;
    fSetKul = shifterU8;
    shifterU8 >>= 1;
    fPrevSpace = shifterU8;
    shifterU8 >>= 1;
    fBold = shifterU8;
    shifterU8 >>= 1;
    fItalic = shifterU8;
    shifterU8 >>= 1;
    fSmallCaps = shifterU8;
    shifterU8 >>= 1;
    fCaps = shifterU8;
    shifterU8 >>= 1;
    fStrike = shifterU8;
    shifterU8 = readU8(ptr);
    ptr += sizeof(U8);
    kul = shifterU8;
    shifterU8 >>= 3;
    ico = shifterU8;
    ftc = readS16(ptr);
    ptr += sizeof(S16);
    hps = readU16(ptr);
    ptr += sizeof(U16);
    iStartAt = readU16(ptr);
    ptr += sizeof(U16);
    dxaIndent = readU16(ptr);
    ptr += sizeof(U16);
    dxaSpace = readU16(ptr);
    ptr += sizeof(U16);
}

void ANLV::clear()
{
    nfc = 0;
    cxchTextBefore = 0;
    cxchTextAfter = 0;
    jc = 0;
    fPrev = 0;
    fHang = 0;
    fSetBold = 0;
    fSetItalic = 0;
    fSetSmallCaps = 0;
    fSetCaps = 0;
    fSetStrike = 0;
    fSetKul = 0;
    fPrevSpace = 0;
    fBold = 0;
    fItalic = 0;
    fSmallCaps = 0;
    fCaps = 0;
    fStrike = 0;
    kul = 0;
    ico = 0;
    ftc = 0;
    hps = 0;
    iStartAt = 0;
    dxaIndent = 0;
    dxaSpace = 0;
}

void ANLV::dump() const
{
    wvlog << "Dumping ANLV:" << std::endl;
    wvlog << toString().c_str() << std::endl;
    wvlog << "\nDumping ANLV done." << std::endl;
}

std::string ANLV::toString() const
{
    std::string s( "ANLV:" );
    s += "\nnfc=";
    s += uint2string( nfc );
    s += "\ncxchTextBefore=";
    s += uint2string( cxchTextBefore );
    s += "\ncxchTextAfter=";
    s += uint2string( cxchTextAfter );
    s += "\njc=";
    s += uint2string( jc );
    s += "\nfPrev=";
    s += uint2string( fPrev );
    s += "\nfHang=";
    s += uint2string( fHang );
    s += "\nfSetBold=";
    s += uint2string( fSetBold );
    s += "\nfSetItalic=";
    s += uint2string( fSetItalic );
    s += "\nfSetSmallCaps=";
    s += uint2string( fSetSmallCaps );
    s += "\nfSetCaps=";
    s += uint2string( fSetCaps );
    s += "\nfSetStrike=";
    s += uint2string( fSetStrike );
    s += "\nfSetKul=";
    s += uint2string( fSetKul );
    s += "\nfPrevSpace=";
    s += uint2string( fPrevSpace );
    s += "\nfBold=";
    s += uint2string( fBold );
    s += "\nfItalic=";
    s += uint2string( fItalic );
    s += "\nfSmallCaps=";
    s += uint2string( fSmallCaps );
    s += "\nfCaps=";
    s += uint2string( fCaps );
    s += "\nfStrike=";
    s += uint2string( fStrike );
    s += "\nkul=";
    s += uint2string( kul );
    s += "\nico=";
    s += uint2string( ico );
    s += "\nftc=";
    s += int2string( ftc );
    s += "\nhps=";
    s += uint2string( hps );
    s += "\niStartAt=";
    s += uint2string( iStartAt );
    s += "\ndxaIndent=";
    s += uint2string( dxaIndent );
    s += "\ndxaSpace=";
    s += uint2string( dxaSpace );
    s += "\nANLV Done.";
    return s;
}

bool operator==(const ANLV &lhs, const ANLV &rhs)
{

    return lhs.nfc == rhs.nfc &&
           lhs.cxchTextBefore == rhs.cxchTextBefore &&
           lhs.cxchTextAfter == rhs.cxchTextAfter &&
           lhs.jc == rhs.jc &&
           lhs.fPrev == rhs.fPrev &&
           lhs.fHang == rhs.fHang &&
           lhs.fSetBold == rhs.fSetBold &&
           lhs.fSetItalic == rhs.fSetItalic &&
           lhs.fSetSmallCaps == rhs.fSetSmallCaps &&
           lhs.fSetCaps == rhs.fSetCaps &&
           lhs.fSetStrike == rhs.fSetStrike &&
           lhs.fSetKul == rhs.fSetKul &&
           lhs.fPrevSpace == rhs.fPrevSpace &&
           lhs.fBold == rhs.fBold &&
           lhs.fItalic == rhs.fItalic &&
           lhs.fSmallCaps == rhs.fSmallCaps &&
           lhs.fCaps == rhs.fCaps &&
           lhs.fStrike == rhs.fStrike &&
           lhs.kul == rhs.kul &&
           lhs.ico == rhs.ico &&
           lhs.ftc == rhs.ftc &&
           lhs.hps == rhs.hps &&
           lhs.iStartAt == rhs.iStartAt &&
           lhs.dxaIndent == rhs.dxaIndent &&
           lhs.dxaSpace == rhs.dxaSpace;
}

bool operator!=(const ANLV &lhs, const ANLV &rhs)
{
    return !(lhs == rhs);
}


// ASUMY implementation

ASUMY::ASUMY()
{
    clear();
}

ASUMY::ASUMY(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

bool ASUMY::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    if (preservePos)
        stream->push();

    lLevel = stream->readS32();

    if (preservePos)
        stream->pop();
    return true;
}

void ASUMY::clear()
{
    lLevel = 0;
}

bool operator==(const ASUMY &lhs, const ASUMY &rhs)
{

    return lhs.lLevel == rhs.lLevel;
}

bool operator!=(const ASUMY &lhs, const ASUMY &rhs)
{
    return !(lhs == rhs);
}


// ASUMYI implementation

ASUMYI::ASUMYI()
{
    clear();
}

ASUMYI::ASUMYI(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

bool ASUMYI::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    U16 shifterU16;

    if (preservePos)
        stream->push();

    shifterU16 = stream->readU16();
    fValid = shifterU16;
    shifterU16 >>= 1;
    fView = shifterU16;
    shifterU16 >>= 1;
    iViewBy = shifterU16;
    shifterU16 >>= 2;
    fUpdateProps = shifterU16;
    shifterU16 >>= 1;
    unused0_5 = shifterU16;
    wDlgLevel = stream->readS16();
    lHighestLevel = stream->readS32();
    lCurrentLevel = stream->readS32();

    if (preservePos)
        stream->pop();
    return true;
}

void ASUMYI::clear()
{
    fValid = 0;
    fView = 0;
    iViewBy = 0;
    fUpdateProps = 0;
    unused0_5 = 0;
    wDlgLevel = 0;
    lHighestLevel = 0;
    lCurrentLevel = 0;
}

bool operator==(const ASUMYI &lhs, const ASUMYI &rhs)
{

    return lhs.fValid == rhs.fValid &&
           lhs.fView == rhs.fView &&
           lhs.iViewBy == rhs.iViewBy &&
           lhs.fUpdateProps == rhs.fUpdateProps &&
           lhs.unused0_5 == rhs.unused0_5 &&
           lhs.wDlgLevel == rhs.wDlgLevel &&
           lhs.lHighestLevel == rhs.lHighestLevel &&
           lhs.lCurrentLevel == rhs.lCurrentLevel;
}

bool operator!=(const ASUMYI &lhs, const ASUMYI &rhs)
{
    return !(lhs == rhs);
}


// ATRD implementation

ATRD::ATRD()
{
    clear();
}

ATRD::ATRD(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

bool ATRD::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    U16 shifterU16;

    if (preservePos)
        stream->push();

    for (int _i = 0; _i < (10); ++_i)
        xstUsrInitl[_i] = stream->readU16();
    ibst = stream->readS16();
    shifterU16 = stream->readU16();
    ak = shifterU16;
    shifterU16 >>= 2;
    unused22_2 = shifterU16;
    grfbmc = stream->readU16();
    lTagBkmk = stream->readS32();

    if (preservePos)
        stream->pop();
    return true;
}

void ATRD::clear()
{
    for (int _i = 0; _i < (10); ++_i)
        xstUsrInitl[_i] = 0;
    ibst = 0;
    ak = 0;
    unused22_2 = 0;
    grfbmc = 0;
    lTagBkmk = 0;
}

bool operator==(const ATRD &lhs, const ATRD &rhs)
{

    for (int _i = 0; _i < (10); ++_i) {
        if (lhs.xstUsrInitl[_i] != rhs.xstUsrInitl[_i])
            return false;
    }

    return lhs.ibst == rhs.ibst &&
           lhs.ak == rhs.ak &&
           lhs.unused22_2 == rhs.unused22_2 &&
           lhs.grfbmc == rhs.grfbmc &&
           lhs.lTagBkmk == rhs.lTagBkmk;
}

bool operator!=(const ATRD &lhs, const ATRD &rhs)
{
    return !(lhs == rhs);
}


// BKD implementation

BKD::BKD()
{
    clear();
}

BKD::BKD(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

bool BKD::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    U16 shifterU16;

    if (preservePos)
        stream->push();

    ipgd = stream->readS16();
    dcpDepend = stream->readS16();
    shifterU16 = stream->readU16();
    icol = shifterU16;
    shifterU16 >>= 8;
    fTableBreak = shifterU16;
    shifterU16 >>= 1;
    fColumnBreak = shifterU16;
    shifterU16 >>= 1;
    fMarked = shifterU16;
    shifterU16 >>= 1;
    fUnk = shifterU16;
    shifterU16 >>= 1;
    fTextOverflow = shifterU16;
    shifterU16 >>= 1;
    unused4_13 = shifterU16;

    if (preservePos)
        stream->pop();
    return true;
}

void BKD::clear()
{
    ipgd = 0;
    dcpDepend = 0;
    icol = 0;
    fTableBreak = 0;
    fColumnBreak = 0;
    fMarked = 0;
    fUnk = 0;
    fTextOverflow = 0;
    unused4_13 = 0;
}

bool operator==(const BKD &lhs, const BKD &rhs)
{

    return lhs.ipgd == rhs.ipgd &&
           lhs.dcpDepend == rhs.dcpDepend &&
           lhs.icol == rhs.icol &&
           lhs.fTableBreak == rhs.fTableBreak &&
           lhs.fColumnBreak == rhs.fColumnBreak &&
           lhs.fMarked == rhs.fMarked &&
           lhs.fUnk == rhs.fUnk &&
           lhs.fTextOverflow == rhs.fTextOverflow &&
           lhs.unused4_13 == rhs.unused4_13;
}

bool operator!=(const BKD &lhs, const BKD &rhs)
{
    return !(lhs == rhs);
}


// BKF implementation

BKF::BKF()
{
    clear();
}

BKF::BKF(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

bool BKF::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    U16 shifterU16;

    if (preservePos)
        stream->push();

    ibkl = stream->readS16();
    shifterU16 = stream->readU16();
    itcFirst = shifterU16;
    shifterU16 >>= 7;
    fPub = shifterU16;
    shifterU16 >>= 1;
    itcLim = shifterU16;
    shifterU16 >>= 7;
    fCol = shifterU16;

    if (preservePos)
        stream->pop();
    return true;
}

void BKF::clear()
{
    ibkl = 0;
    itcFirst = 0;
    fPub = 0;
    itcLim = 0;
    fCol = 0;
}

bool operator==(const BKF &lhs, const BKF &rhs)
{

    return lhs.ibkl == rhs.ibkl &&
           lhs.itcFirst == rhs.itcFirst &&
           lhs.fPub == rhs.fPub &&
           lhs.itcLim == rhs.itcLim &&
           lhs.fCol == rhs.fCol;
}

bool operator!=(const BKF &lhs, const BKF &rhs)
{
    return !(lhs == rhs);
}


// BKL implementation

BKL::BKL()
{
    clear();
}

BKL::BKL(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

bool BKL::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    if (preservePos)
        stream->push();

    ibkf = stream->readS16();

    if (preservePos)
        stream->pop();
    return true;
}

void BKL::clear()
{
    ibkf = 0;
}

bool operator==(const BKL &lhs, const BKL &rhs)
{

    return lhs.ibkf == rhs.ibkf;
}

bool operator!=(const BKL &lhs, const BKL &rhs)
{
    return !(lhs == rhs);
}


// BRC10 implementation

BRC10::BRC10()
{
    clear();
}

BRC10::BRC10(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

bool BRC10::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    U16 shifterU16;

    if (preservePos)
        stream->push();

    shifterU16 = stream->readU16();
    dxpLine2Width = shifterU16;
    shifterU16 >>= 3;
    dxpSpaceBetween = shifterU16;
    shifterU16 >>= 3;
    dxpLine1Width = shifterU16;
    shifterU16 >>= 3;
    dxpSpace = shifterU16;
    shifterU16 >>= 5;
    fShadow = shifterU16;
    shifterU16 >>= 1;
    fSpare = shifterU16;

    if (preservePos)
        stream->pop();
    return true;
}

void BRC10::clear()
{
    dxpLine2Width = 0;
    dxpSpaceBetween = 0;
    dxpLine1Width = 0;
    dxpSpace = 0;
    fShadow = 0;
    fSpare = 0;
}

bool operator==(const BRC10 &lhs, const BRC10 &rhs)
{

    return lhs.dxpLine2Width == rhs.dxpLine2Width &&
           lhs.dxpSpaceBetween == rhs.dxpSpaceBetween &&
           lhs.dxpLine1Width == rhs.dxpLine1Width &&
           lhs.dxpSpace == rhs.dxpSpace &&
           lhs.fShadow == rhs.fShadow &&
           lhs.fSpare == rhs.fSpare;
}

bool operator!=(const BRC10 &lhs, const BRC10 &rhs)
{
    return !(lhs == rhs);
}


// BTE implementation

const unsigned int BTE::sizeOf = 4;

BTE::BTE()
{
    clear();
}

BTE::BTE(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

bool BTE::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    if (preservePos)
        stream->push();

    pn = stream->readU32();

    if (preservePos)
        stream->pop();
    return true;
}

void BTE::clear()
{
    pn = 0;
}

bool operator==(const BTE &lhs, const BTE &rhs)
{

    return lhs.pn == rhs.pn;
}

bool operator!=(const BTE &lhs, const BTE &rhs)
{
    return !(lhs == rhs);
}


// CHP implementation

CHP::CHP() : Shared()
{
    clear();
}

CHP::CHP(AbstractOLEStreamReader *stream, bool preservePos) : Shared()
{
    clear();
    read(stream, preservePos);
}

bool CHP::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    U8 shifterU8;
    U16 shifterU16;

    if (preservePos)
        stream->push();

    shifterU8 = stream->readU8();
    fBold = shifterU8;
    shifterU8 >>= 1;
    fItalic = shifterU8;
    shifterU8 >>= 1;
    fRMarkDel = shifterU8;
    shifterU8 >>= 1;
    fOutline = shifterU8;
    shifterU8 >>= 1;
    fFldVanish = shifterU8;
    shifterU8 >>= 1;
    fSmallCaps = shifterU8;
    shifterU8 >>= 1;
    fCaps = shifterU8;
    shifterU8 >>= 1;
    fVanish = shifterU8;
    shifterU8 = stream->readU8();
    fRMark = shifterU8;
    shifterU8 >>= 1;
    fSpec = shifterU8;
    shifterU8 >>= 1;
    fStrike = shifterU8;
    shifterU8 >>= 1;
    fObj = shifterU8;
    shifterU8 >>= 1;
    fShadow = shifterU8;
    shifterU8 >>= 1;
    fLowerCase = shifterU8;
    shifterU8 >>= 1;
    fData = shifterU8;
    shifterU8 >>= 1;
    fOle2 = shifterU8;
    shifterU16 = stream->readU16();
    fEmboss = shifterU16;
    shifterU16 >>= 1;
    fImprint = shifterU16;
    shifterU16 >>= 1;
    fDStrike = shifterU16;
    shifterU16 >>= 1;
    fUsePgsuSettings = shifterU16;
    shifterU16 >>= 1;
    unused2_4 = shifterU16;
    unused4 = stream->readS32();
    ftc = stream->readS16();
    ftcAscii = stream->readS16();
    ftcFE = stream->readS16();
    ftcOther = stream->readS16();
    hps = stream->readU16();
    dxaSpace = stream->readS32();
    shifterU8 = stream->readU8();
    iss = shifterU8;
    shifterU8 >>= 3;
    kul = shifterU8;
    shifterU8 >>= 4;
    fSpecSymbol = shifterU8;
    shifterU8 = stream->readU8();
    ico = shifterU8;
    shifterU8 >>= 5;
    unused23_5 = shifterU8;
    shifterU8 >>= 1;
    fSysVanish = shifterU8;
    shifterU8 >>= 1;
    hpScript = shifterU8;
    hpsPos = stream->readS16();
    lid = stream->readU16();
    lidDefault = stream->readU16();
    lidFE = stream->readU16();
    idct = stream->readU8();
    idctHint = stream->readU8();
    wCharScale = stream->readU16();
    fcPic_fcObj_lTagObj = stream->readS32();
    ibstRMark = stream->readS16();
    ibstRMarkDel = stream->readS16();
    dttmRMark.read(stream, false);
    dttmRMarkDel.read(stream, false);
    unused52 = stream->readS16();
    istd = stream->readU16();
    ftcSym = stream->readS16();
    xchSym = stream->readU16();
    idslRMReason = stream->readS16();
    idslRMReasonDel = stream->readS16();
    ysr = stream->readU8();
    chYsr = stream->readU8();
    chse = stream->readU16();
    hpsKern = stream->readU16();
    shifterU16 = stream->readU16();
    icoHighlight = shifterU16;
    shifterU16 >>= 5;
    fHighlight = shifterU16;
    shifterU16 >>= 1;
    kcd = shifterU16;
    shifterU16 >>= 3;
    fNavHighlight = shifterU16;
    shifterU16 >>= 1;
    fChsDiff = shifterU16;
    shifterU16 >>= 1;
    fMacChs = shifterU16;
    shifterU16 >>= 1;
    fFtcAsciSym = shifterU16;
    shifterU16 >>= 1;
    reserved_3 = shifterU16;
    fPropMark = stream->readU16();
    ibstPropRMark = stream->readS16();
    dttmPropRMark.read(stream, false);
    sfxtText = stream->readU8();
    unused81 = stream->readU8();
    unused82 = stream->readU8();
    unused83 = stream->readU16();
    unused85 = stream->readS16();
    unused87 = stream->readU32();
    fDispFldRMark = stream->readS8();
    ibstDispFldRMark = stream->readS16();
    dttmDispFldRMark = stream->readU32();
    for (int _i = 0; _i < (16); ++_i)
        xstDispFldRMark[_i] = stream->readU16();
    shd.read(stream, false);
    brc.read(stream, false);

    if (preservePos)
        stream->pop();
    return true;
}

void CHP::clear()
{
    fBold = 0;
    fItalic = 0;
    fRMarkDel = 0;
    fOutline = 0;
    fFldVanish = 0;
    fSmallCaps = 0;
    fCaps = 0;
    fVanish = 0;
    fRMark = 0;
    fSpec = 0;
    fStrike = 0;
    fObj = 0;
    fShadow = 0;
    fLowerCase = 0;
    fData = 0;
    fOle2 = 0;
    fEmboss = 0;
    fImprint = 0;
    fDStrike = 0;
    fUsePgsuSettings = 0;
    unused2_4 = 0;
    unused4 = 0;
    ftc = 0;
    ftcAscii = 0;
    ftcFE = 0;
    ftcOther = 0;
    hps = 20;
    dxaSpace = 0;
    iss = 0;
    kul = 0;
    fSpecSymbol = 0;
    ico = 0;
    unused23_5 = 0;
    fSysVanish = 0;
    hpScript = 0;
    hpsPos = 0;
    lid = 0x0400;
    lidDefault = 0x0400;
    lidFE = 0x0400;
    idct = 0;
    idctHint = 0;
    wCharScale = 100;
    fcPic_fcObj_lTagObj = -1;
    ibstRMark = 0;
    ibstRMarkDel = 0;
    dttmRMark.clear();
    dttmRMarkDel.clear();
    unused52 = 0;
    istd = 10;
    ftcSym = 0;
    xchSym = 0;
    idslRMReason = 0;
    idslRMReasonDel = 0;
    ysr = 0;
    chYsr = 0;
    chse = 0;
    hpsKern = 0;
    icoHighlight = 0;
    fHighlight = 0;
    kcd = 0;
    fNavHighlight = 0;
    fChsDiff = 0;
    fMacChs = 0;
    fFtcAsciSym = 0;
    reserved_3 = 0;
    fPropMark = 0;
    ibstPropRMark = 0;
    dttmPropRMark.clear();
    sfxtText = 0;
    unused81 = 0;
    unused82 = 0;
    unused83 = 0;
    unused85 = 0;
    unused87 = 0;
    fDispFldRMark = 0;
    ibstDispFldRMark = 0;
    dttmDispFldRMark = 0;
    for (int _i = 0; _i < (16); ++_i)
        xstDispFldRMark[_i] = 0;
    shd.clear();
    brc.clear();
}

void CHP::dump() const
{
    wvlog << "Dumping CHP:" << std::endl;
    wvlog << toString().c_str() << std::endl;
    wvlog << "\nDumping CHP done." << std::endl;
}

std::string CHP::toString() const
{
    std::string s( "CHP:" );
    s += "\nfBold=";
    s += uint2string( fBold );
    s += "\nfItalic=";
    s += uint2string( fItalic );
    s += "\nfRMarkDel=";
    s += uint2string( fRMarkDel );
    s += "\nfOutline=";
    s += uint2string( fOutline );
    s += "\nfFldVanish=";
    s += uint2string( fFldVanish );
    s += "\nfSmallCaps=";
    s += uint2string( fSmallCaps );
    s += "\nfCaps=";
    s += uint2string( fCaps );
    s += "\nfVanish=";
    s += uint2string( fVanish );
    s += "\nfRMark=";
    s += uint2string( fRMark );
    s += "\nfSpec=";
    s += uint2string( fSpec );
    s += "\nfStrike=";
    s += uint2string( fStrike );
    s += "\nfObj=";
    s += uint2string( fObj );
    s += "\nfShadow=";
    s += uint2string( fShadow );
    s += "\nfLowerCase=";
    s += uint2string( fLowerCase );
    s += "\nfData=";
    s += uint2string( fData );
    s += "\nfOle2=";
    s += uint2string( fOle2 );
    s += "\nfEmboss=";
    s += uint2string( fEmboss );
    s += "\nfImprint=";
    s += uint2string( fImprint );
    s += "\nfDStrike=";
    s += uint2string( fDStrike );
    s += "\nfUsePgsuSettings=";
    s += uint2string( fUsePgsuSettings );
    s += "\nunused2_4=";
    s += uint2string( unused2_4 );
    s += "\nunused4=";
    s += int2string( unused4 );
    s += "\nftc=";
    s += int2string( ftc );
    s += "\nftcAscii=";
    s += int2string( ftcAscii );
    s += "\nftcFE=";
    s += int2string( ftcFE );
    s += "\nftcOther=";
    s += int2string( ftcOther );
    s += "\nhps=";
    s += uint2string( hps );
    s += "\ndxaSpace=";
    s += int2string( dxaSpace );
    s += "\niss=";
    s += uint2string( iss );
    s += "\nkul=";
    s += uint2string( kul );
    s += "\nfSpecSymbol=";
    s += uint2string( fSpecSymbol );
    s += "\nico=";
    s += uint2string( ico );
    s += "\nunused23_5=";
    s += uint2string( unused23_5 );
    s += "\nfSysVanish=";
    s += uint2string( fSysVanish );
    s += "\nhpScript=";
    s += uint2string( hpScript );
    s += "\nhpsPos=";
    s += int2string( hpsPos );
    s += "\nlid=";
    s += uint2string( lid );
    s += "\nlidDefault=";
    s += uint2string( lidDefault );
    s += "\nlidFE=";
    s += uint2string( lidFE );
    s += "\nidct=";
    s += uint2string( idct );
    s += "\nidctHint=";
    s += uint2string( idctHint );
    s += "\nwCharScale=";
    s += uint2string( wCharScale );
    s += "\nfcPic_fcObj_lTagObj=";
    s += int2string( fcPic_fcObj_lTagObj );
    s += "\nibstRMark=";
    s += int2string( ibstRMark );
    s += "\nibstRMarkDel=";
    s += int2string( ibstRMarkDel );
    s += "\ndttmRMark=";
    s += "\n{" + dttmRMark.toString() + "}\n";
    s += "\ndttmRMarkDel=";
    s += "\n{" + dttmRMarkDel.toString() + "}\n";
    s += "\nunused52=";
    s += int2string( unused52 );
    s += "\nistd=";
    s += uint2string( istd );
    s += "\nftcSym=";
    s += int2string( ftcSym );
    s += "\nxchSym=";
    s += uint2string( xchSym );
    s += "\nidslRMReason=";
    s += int2string( idslRMReason );
    s += "\nidslRMReasonDel=";
    s += int2string( idslRMReasonDel );
    s += "\nysr=";
    s += uint2string( ysr );
    s += "\nchYsr=";
    s += uint2string( chYsr );
    s += "\nchse=";
    s += uint2string( chse );
    s += "\nhpsKern=";
    s += uint2string( hpsKern );
    s += "\nicoHighlight=";
    s += uint2string( icoHighlight );
    s += "\nfHighlight=";
    s += uint2string( fHighlight );
    s += "\nkcd=";
    s += uint2string( kcd );
    s += "\nfNavHighlight=";
    s += uint2string( fNavHighlight );
    s += "\nfChsDiff=";
    s += uint2string( fChsDiff );
    s += "\nfMacChs=";
    s += uint2string( fMacChs );
    s += "\nfFtcAsciSym=";
    s += uint2string( fFtcAsciSym );
    s += "\nreserved_3=";
    s += uint2string( reserved_3 );
    s += "\nfPropMark=";
    s += uint2string( fPropMark );
    s += "\nibstPropRMark=";
    s += int2string( ibstPropRMark );
    s += "\ndttmPropRMark=";
    s += "\n{" + dttmPropRMark.toString() + "}\n";
    s += "\nsfxtText=";
    s += uint2string( sfxtText );
    s += "\nunused81=";
    s += uint2string( unused81 );
    s += "\nunused82=";
    s += uint2string( unused82 );
    s += "\nunused83=";
    s += uint2string( unused83 );
    s += "\nunused85=";
    s += int2string( unused85 );
    s += "\nunused87=";
    s += uint2string( unused87 );
    s += "\nfDispFldRMark=";
    s += int2string( fDispFldRMark );
    s += "\nibstDispFldRMark=";
    s += int2string( ibstDispFldRMark );
    s += "\ndttmDispFldRMark=";
    s += uint2string( dttmDispFldRMark );
    for (int _i = 0; _i < (16); ++_i) {
        s += "\nxstDispFldRMark[" + int2string( _i ) + "]=";
        s += uint2string( xstDispFldRMark[_i] );
    }
    s += "\nshd=";
    s += "\n{" + shd.toString() + "}\n";
    s += "\nbrc=";
    s += "\n{" + brc.toString() + "}\n";
    s += "\nCHP Done.";
    return s;
}

bool operator==(const CHP &lhs, const CHP &rhs)
{

    for (int _i = 0; _i < (16); ++_i) {
        if (lhs.xstDispFldRMark[_i] != rhs.xstDispFldRMark[_i])
            return false;
    }

    return lhs.fBold == rhs.fBold &&
           lhs.fItalic == rhs.fItalic &&
           lhs.fRMarkDel == rhs.fRMarkDel &&
           lhs.fOutline == rhs.fOutline &&
           lhs.fFldVanish == rhs.fFldVanish &&
           lhs.fSmallCaps == rhs.fSmallCaps &&
           lhs.fCaps == rhs.fCaps &&
           lhs.fVanish == rhs.fVanish &&
           lhs.fRMark == rhs.fRMark &&
           lhs.fSpec == rhs.fSpec &&
           lhs.fStrike == rhs.fStrike &&
           lhs.fObj == rhs.fObj &&
           lhs.fShadow == rhs.fShadow &&
           lhs.fLowerCase == rhs.fLowerCase &&
           lhs.fData == rhs.fData &&
           lhs.fOle2 == rhs.fOle2 &&
           lhs.fEmboss == rhs.fEmboss &&
           lhs.fImprint == rhs.fImprint &&
           lhs.fDStrike == rhs.fDStrike &&
           lhs.fUsePgsuSettings == rhs.fUsePgsuSettings &&
           lhs.unused2_4 == rhs.unused2_4 &&
           lhs.unused4 == rhs.unused4 &&
           lhs.ftc == rhs.ftc &&
           lhs.ftcAscii == rhs.ftcAscii &&
           lhs.ftcFE == rhs.ftcFE &&
           lhs.ftcOther == rhs.ftcOther &&
           lhs.hps == rhs.hps &&
           lhs.dxaSpace == rhs.dxaSpace &&
           lhs.iss == rhs.iss &&
           lhs.kul == rhs.kul &&
           lhs.fSpecSymbol == rhs.fSpecSymbol &&
           lhs.ico == rhs.ico &&
           lhs.unused23_5 == rhs.unused23_5 &&
           lhs.fSysVanish == rhs.fSysVanish &&
           lhs.hpScript == rhs.hpScript &&
           lhs.hpsPos == rhs.hpsPos &&
           lhs.lid == rhs.lid &&
           lhs.lidDefault == rhs.lidDefault &&
           lhs.lidFE == rhs.lidFE &&
           lhs.idct == rhs.idct &&
           lhs.idctHint == rhs.idctHint &&
           lhs.wCharScale == rhs.wCharScale &&
           lhs.fcPic_fcObj_lTagObj == rhs.fcPic_fcObj_lTagObj &&
           lhs.ibstRMark == rhs.ibstRMark &&
           lhs.ibstRMarkDel == rhs.ibstRMarkDel &&
           lhs.dttmRMark == rhs.dttmRMark &&
           lhs.dttmRMarkDel == rhs.dttmRMarkDel &&
           lhs.unused52 == rhs.unused52 &&
           lhs.istd == rhs.istd &&
           lhs.ftcSym == rhs.ftcSym &&
           lhs.xchSym == rhs.xchSym &&
           lhs.idslRMReason == rhs.idslRMReason &&
           lhs.idslRMReasonDel == rhs.idslRMReasonDel &&
           lhs.ysr == rhs.ysr &&
           lhs.chYsr == rhs.chYsr &&
           lhs.chse == rhs.chse &&
           lhs.hpsKern == rhs.hpsKern &&
           lhs.icoHighlight == rhs.icoHighlight &&
           lhs.fHighlight == rhs.fHighlight &&
           lhs.kcd == rhs.kcd &&
           lhs.fNavHighlight == rhs.fNavHighlight &&
           lhs.fChsDiff == rhs.fChsDiff &&
           lhs.fMacChs == rhs.fMacChs &&
           lhs.fFtcAsciSym == rhs.fFtcAsciSym &&
           lhs.reserved_3 == rhs.reserved_3 &&
           lhs.fPropMark == rhs.fPropMark &&
           lhs.ibstPropRMark == rhs.ibstPropRMark &&
           lhs.dttmPropRMark == rhs.dttmPropRMark &&
           lhs.sfxtText == rhs.sfxtText &&
           lhs.unused81 == rhs.unused81 &&
           lhs.unused82 == rhs.unused82 &&
           lhs.unused83 == rhs.unused83 &&
           lhs.unused85 == rhs.unused85 &&
           lhs.unused87 == rhs.unused87 &&
           lhs.fDispFldRMark == rhs.fDispFldRMark &&
           lhs.ibstDispFldRMark == rhs.ibstDispFldRMark &&
           lhs.dttmDispFldRMark == rhs.dttmDispFldRMark &&
           lhs.shd == rhs.shd &&
           lhs.brc == rhs.brc;
}

bool operator!=(const CHP &lhs, const CHP &rhs)
{
    return !(lhs == rhs);
}


// DCS implementation

DCS::DCS()
{
    clear();
}

DCS::DCS(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

DCS::DCS(const U8 *ptr)
{
    clear();
    readPtr(ptr);
}

bool DCS::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    U8 shifterU8;

    if (preservePos)
        stream->push();

    shifterU8 = stream->readU8();
    fdct = shifterU8;
    shifterU8 >>= 3;
    lines = shifterU8;
    unused1 = stream->readU8();

    if (preservePos)
        stream->pop();
    return true;
}

void DCS::readPtr(const U8 *ptr)
{

    U8 shifterU8;

    shifterU8 = readU8(ptr);
    ptr += sizeof(U8);
    fdct = shifterU8;
    shifterU8 >>= 3;
    lines = shifterU8;
    unused1 = readU8(ptr);
    ptr += sizeof(U8);
}

void DCS::clear()
{
    fdct = 0;
    lines = 0;
    unused1 = 0;
}

void DCS::dump() const
{
    wvlog << "Dumping DCS:" << std::endl;
    wvlog << toString().c_str() << std::endl;
    wvlog << "\nDumping DCS done." << std::endl;
}

std::string DCS::toString() const
{
    std::string s( "DCS:" );
    s += "\nfdct=";
    s += uint2string( fdct );
    s += "\nlines=";
    s += uint2string( lines );
    s += "\nunused1=";
    s += uint2string( unused1 );
    s += "\nDCS Done.";
    return s;
}

bool operator==(const DCS &lhs, const DCS &rhs)
{

    return lhs.fdct == rhs.fdct &&
           lhs.lines == rhs.lines &&
           lhs.unused1 == rhs.unused1;
}

bool operator!=(const DCS &lhs, const DCS &rhs)
{
    return !(lhs == rhs);
}


// DOGRID implementation

DOGRID::DOGRID()
{
    clear();
}

DOGRID::DOGRID(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

bool DOGRID::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    U16 shifterU16;

    if (preservePos)
        stream->push();

    xaGrid = stream->readS16();
    yaGrid = stream->readS16();
    dxaGrid = stream->readS16();
    dyaGrid = stream->readS16();
    shifterU16 = stream->readU16();
    dyGridDisplay = shifterU16;
    shifterU16 >>= 7;
    fTurnItOff = shifterU16;
    shifterU16 >>= 1;
    dxGridDisplay = shifterU16;
    shifterU16 >>= 7;
    fFollowMargins = shifterU16;

    if (preservePos)
        stream->pop();
    return true;
}

void DOGRID::clear()
{
    xaGrid = 0;
    yaGrid = 0;
    dxaGrid = 0;
    dyaGrid = 0;
    dyGridDisplay = 0;
    fTurnItOff = 0;
    dxGridDisplay = 0;
    fFollowMargins = 0;
}

bool operator==(const DOGRID &lhs, const DOGRID &rhs)
{

    return lhs.xaGrid == rhs.xaGrid &&
           lhs.yaGrid == rhs.yaGrid &&
           lhs.dxaGrid == rhs.dxaGrid &&
           lhs.dyaGrid == rhs.dyaGrid &&
           lhs.dyGridDisplay == rhs.dyGridDisplay &&
           lhs.fTurnItOff == rhs.fTurnItOff &&
           lhs.dxGridDisplay == rhs.dxGridDisplay &&
           lhs.fFollowMargins == rhs.fFollowMargins;
}

bool operator!=(const DOGRID &lhs, const DOGRID &rhs)
{
    return !(lhs == rhs);
}


// DOP implementation

DOP::DOP()
{
    clear();
}

DOP::DOP(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

bool DOP::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    U8 shifterU8;
    U16 shifterU16;
    U32 shifterU32;

    if (preservePos)
        stream->push();

    shifterU16 = stream->readU16();
    fFacingPages = shifterU16;
    shifterU16 >>= 1;
    fWidowControl = shifterU16;
    shifterU16 >>= 1;
    fPMHMainDoc = shifterU16;
    shifterU16 >>= 1;
    grfSuppression = shifterU16;
    shifterU16 >>= 2;
    fpc = shifterU16;
    shifterU16 >>= 2;
    unused0_7 = shifterU16;
    shifterU16 >>= 1;
    grpfIhdt = shifterU16;
    shifterU16 = stream->readU16();
    rncFtn = shifterU16;
    shifterU16 >>= 2;
    nFtn = shifterU16;
    shifterU8 = stream->readU8();
    fOutlineDirtySave = shifterU8;
    shifterU8 >>= 1;
    unused4_1 = shifterU8;
    shifterU8 = stream->readU8();
    fOnlyMacPics = shifterU8;
    shifterU8 >>= 1;
    fOnlyWinPics = shifterU8;
    shifterU8 >>= 1;
    fLabelDoc = shifterU8;
    shifterU8 >>= 1;
    fHyphCapitals = shifterU8;
    shifterU8 >>= 1;
    fAutoHyphen = shifterU8;
    shifterU8 >>= 1;
    fFormNoFields = shifterU8;
    shifterU8 >>= 1;
    fLinkStyles = shifterU8;
    shifterU8 >>= 1;
    fRevMarking = shifterU8;
    shifterU8 = stream->readU8();
    fBackup = shifterU8;
    shifterU8 >>= 1;
    fExactCWords = shifterU8;
    shifterU8 >>= 1;
    fPagHidden = shifterU8;
    shifterU8 >>= 1;
    fPagResults = shifterU8;
    shifterU8 >>= 1;
    fLockAtn = shifterU8;
    shifterU8 >>= 1;
    fMirrorMargins = shifterU8;
    shifterU8 >>= 1;
    unused6_6 = shifterU8;
    shifterU8 >>= 1;
    fDfltTrueType = shifterU8;
    shifterU8 = stream->readU8();
    fPagSuppressTopSpacing = shifterU8;
    shifterU8 >>= 1;
    fProtEnabled = shifterU8;
    shifterU8 >>= 1;
    fDispFormFldSel = shifterU8;
    shifterU8 >>= 1;
    fRMView = shifterU8;
    shifterU8 >>= 1;
    fRMPrint = shifterU8;
    shifterU8 >>= 1;
    unused7_5 = shifterU8;
    shifterU8 >>= 1;
    fLockRev = shifterU8;
    shifterU8 >>= 1;
    fEmbedFonts = shifterU8;
    shifterU16 = stream->readU16();
    copts_fNoTabForInd = shifterU16;
    shifterU16 >>= 1;
    copts_fNoSpaceRaiseLower = shifterU16;
    shifterU16 >>= 1;
    copts_fSuppressSpbfAfterPageBreak = shifterU16;
    shifterU16 >>= 1;
    copts_fWrapTrailSpaces = shifterU16;
    shifterU16 >>= 1;
    copts_fMapPrintTextColor = shifterU16;
    shifterU16 >>= 1;
    copts_fNoColumnBalance = shifterU16;
    shifterU16 >>= 1;
    copts_fConvMailMergeEsc = shifterU16;
    shifterU16 >>= 1;
    copts_fSupressTopSpacing = shifterU16;
    shifterU16 >>= 1;
    copts_fOrigWordTableRules = shifterU16;
    shifterU16 >>= 1;
    copts_fTransparentMetafiles = shifterU16;
    shifterU16 >>= 1;
    copts_fShowBreaksInFrames = shifterU16;
    shifterU16 >>= 1;
    copts_fSwapBordersFacingPgs = shifterU16;
    shifterU16 >>= 1;
    unused8_12 = shifterU16;
    dxaTab = stream->readU16();
    wSpare = stream->readU16();
    dxaHotZ = stream->readU16();
    cConsecHypLim = stream->readU16();
    wSpare2 = stream->readU16();
    dttmCreated.read(stream, false);
    dttmRevised.read(stream, false);
    dttmLastPrint.read(stream, false);
    nRevision = stream->readS16();
    tmEdited = stream->readS32();
    cWords = stream->readS32();
    cCh = stream->readS32();
    cPg = stream->readS16();
    cParas = stream->readS32();
    shifterU16 = stream->readU16();
    rncEdn = shifterU16;
    shifterU16 >>= 2;
    nEdn = shifterU16;
    shifterU16 = stream->readU16();
    epc = shifterU16;
    shifterU16 >>= 2;
    nfcFtnRef = shifterU16;
    shifterU16 >>= 4;
    nfcEdnRef = shifterU16;
    shifterU16 >>= 4;
    fPrintFormData = shifterU16;
    shifterU16 >>= 1;
    fSaveFormData = shifterU16;
    shifterU16 >>= 1;
    fShadeFormData = shifterU16;
    shifterU16 >>= 1;
    unused54_13 = shifterU16;
    shifterU16 >>= 2;
    fWCFtnEdn = shifterU16;
    cLines = stream->readS32();
    cWordsFtnEnd = stream->readS32();
    cChFtnEdn = stream->readS32();
    cPgFtnEdn = stream->readS16();
    cParasFtnEdn = stream->readS32();
    cLinesFtnEdn = stream->readS32();
    lKeyProtDoc = stream->readS32();
    shifterU16 = stream->readU16();
    wvkSaved = shifterU16;
    shifterU16 >>= 3;
    wScaleSaved = shifterU16;
    shifterU16 >>= 9;
    zkSaved = shifterU16;
    shifterU16 >>= 2;
    fRotateFontW6 = shifterU16;
    shifterU16 >>= 1;
    iGutterPos = shifterU16;
    shifterU32 = stream->readU32();
    fNoTabForInd = shifterU32;
    shifterU32 >>= 1;
    fNoSpaceRaiseLower = shifterU32;
    shifterU32 >>= 1;
    fSupressSpbfAfterPageBreak = shifterU32;
    shifterU32 >>= 1;
    fWrapTrailSpaces = shifterU32;
    shifterU32 >>= 1;
    fMapPrintTextColor = shifterU32;
    shifterU32 >>= 1;
    fNoColumnBalance = shifterU32;
    shifterU32 >>= 1;
    fConvMailMergeEsc = shifterU32;
    shifterU32 >>= 1;
    fSupressTopSpacing = shifterU32;
    shifterU32 >>= 1;
    fOrigWordTableRules = shifterU32;
    shifterU32 >>= 1;
    fTransparentMetafiles = shifterU32;
    shifterU32 >>= 1;
    fShowBreaksInFrames = shifterU32;
    shifterU32 >>= 1;
    fSwapBordersFacingPgs = shifterU32;
    shifterU32 >>= 1;
    unused84_12 = shifterU32;
    shifterU32 >>= 4;
    fSuppressTopSpacingMac5 = shifterU32;
    shifterU32 >>= 1;
    fTruncDxaExpand = shifterU32;
    shifterU32 >>= 1;
    fPrintBodyBeforeHdr = shifterU32;
    shifterU32 >>= 1;
    fNoLeading = shifterU32;
    shifterU32 >>= 1;
    unused84_20 = shifterU32;
    shifterU32 >>= 1;
    fMWSmallCaps = shifterU32;
    shifterU32 >>= 1;
    unused84_22 = shifterU32;
    adt = stream->readU16();
    doptypography.read(stream, false);
    dogrid.read(stream, false);
    shifterU16 = stream->readU16();
    reserved = shifterU16;
    shifterU16 >>= 1;
    lvl = shifterU16;
    shifterU16 >>= 4;
    fGramAllDone = shifterU16;
    shifterU16 >>= 1;
    fGramAllClean = shifterU16;
    shifterU16 >>= 1;
    fSubsetFonts = shifterU16;
    shifterU16 >>= 1;
    fHideLastVersion = shifterU16;
    shifterU16 >>= 1;
    fHtmlDoc = shifterU16;
    shifterU16 >>= 1;
    unused410_11 = shifterU16;
    shifterU16 >>= 1;
    fSnapBorder = shifterU16;
    shifterU16 >>= 1;
    fIncludeHeader = shifterU16;
    shifterU16 >>= 1;
    fIncludeFooter = shifterU16;
    shifterU16 >>= 1;
    fForcePageSizePag = shifterU16;
    shifterU16 >>= 1;
    fMinFontSizePag = shifterU16;
    shifterU16 = stream->readU16();
    fHaveVersions = shifterU16;
    shifterU16 >>= 1;
    fAutoVersion = shifterU16;
    shifterU16 >>= 1;
    unused412_2 = shifterU16;
    asumyi.read(stream, false);
    cChWS = stream->readS32();
    cChWSFtnEdn = stream->readS32();
    grfDocEvents = stream->readS32();
    shifterU32 = stream->readU32();
    fVirusPrompted = shifterU32;
    shifterU32 >>= 1;
    fVirusLoadSafe = shifterU32;
    shifterU32 >>= 1;
    KeyVirusSession30 = shifterU32;
    for (int _i = 0; _i < (30); ++_i)
        Spare[_i] = stream->readU8();
    unused472 = stream->readU32();
    unused476 = stream->readU32();
    cDBC = stream->readS32();
    cDBCFtnEdn = stream->readS32();
    unused488 = stream->readU32();
    nfcFtnRef2 = stream->readS16();
    nfcEdnRef2 = stream->readS16();
    hpsZoonFontPag = stream->readS16();
    dywDispPag = stream->readS16();

    if (preservePos)
        stream->pop();
    return true;
}

void DOP::clear()
{
    fFacingPages = 0;
    fWidowControl = 1;
    fPMHMainDoc = 0;
    grfSuppression = 0;
    fpc = 1;
    unused0_7 = 0;
    grpfIhdt = 0;
    rncFtn = 0;
    nFtn = 1;
    fOutlineDirtySave = 0;
    unused4_1 = 0;
    fOnlyMacPics = 0;
    fOnlyWinPics = 0;
    fLabelDoc = 0;
    fHyphCapitals = 0;
    fAutoHyphen = 0;
    fFormNoFields = 0;
    fLinkStyles = 0;
    fRevMarking = 0;
    fBackup = 0;
    fExactCWords = 0;
    fPagHidden = 0;
    fPagResults = 0;
    fLockAtn = 0;
    fMirrorMargins = 0;
    unused6_6 = 0;
    fDfltTrueType = 0;
    fPagSuppressTopSpacing = 0;
    fProtEnabled = 0;
    fDispFormFldSel = 0;
    fRMView = 0;
    fRMPrint = 0;
    unused7_5 = 0;
    fLockRev = 0;
    fEmbedFonts = 0;
    copts_fNoTabForInd = 0;
    copts_fNoSpaceRaiseLower = 0;
    copts_fSuppressSpbfAfterPageBreak = 0;
    copts_fWrapTrailSpaces = 0;
    copts_fMapPrintTextColor = 0;
    copts_fNoColumnBalance = 0;
    copts_fConvMailMergeEsc = 0;
    copts_fSupressTopSpacing = 0;
    copts_fOrigWordTableRules = 0;
    copts_fTransparentMetafiles = 0;
    copts_fShowBreaksInFrames = 0;
    copts_fSwapBordersFacingPgs = 0;
    unused8_12 = 0;
    dxaTab = 720;
    wSpare = 0;
    dxaHotZ = 0;
    cConsecHypLim = 0;
    wSpare2 = 0;
    dttmCreated.clear();
    dttmRevised.clear();
    dttmLastPrint.clear();
    nRevision = 0;
    tmEdited = 0;
    cWords = 0;
    cCh = 0;
    cPg = 0;
    cParas = 0;
    rncEdn = 0;
    nEdn = 0;
    epc = 0;
    nfcFtnRef = 0;
    nfcEdnRef = 0;
    fPrintFormData = 0;
    fSaveFormData = 0;
    fShadeFormData = 0;
    unused54_13 = 0;
    fWCFtnEdn = 0;
    cLines = 0;
    cWordsFtnEnd = 0;
    cChFtnEdn = 0;
    cPgFtnEdn = 0;
    cParasFtnEdn = 0;
    cLinesFtnEdn = 0;
    lKeyProtDoc = 0;
    wvkSaved = 0;
    wScaleSaved = 0;
    zkSaved = 0;
    fRotateFontW6 = 0;
    iGutterPos = 0;
    fNoTabForInd = 0;
    fNoSpaceRaiseLower = 0;
    fSupressSpbfAfterPageBreak = 0;
    fWrapTrailSpaces = 0;
    fMapPrintTextColor = 0;
    fNoColumnBalance = 0;
    fConvMailMergeEsc = 0;
    fSupressTopSpacing = 0;
    fOrigWordTableRules = 0;
    fTransparentMetafiles = 0;
    fShowBreaksInFrames = 0;
    fSwapBordersFacingPgs = 0;
    unused84_12 = 0;
    fSuppressTopSpacingMac5 = 0;
    fTruncDxaExpand = 0;
    fPrintBodyBeforeHdr = 0;
    fNoLeading = 0;
    unused84_20 = 0;
    fMWSmallCaps = 0;
    unused84_22 = 0;
    adt = 0;
    doptypography.clear();
    dogrid.clear();
    reserved = 0;
    lvl = 0;
    fGramAllDone = 0;
    fGramAllClean = 0;
    fSubsetFonts = 0;
    fHideLastVersion = 0;
    fHtmlDoc = 0;
    unused410_11 = 0;
    fSnapBorder = 0;
    fIncludeHeader = 0;
    fIncludeFooter = 0;
    fForcePageSizePag = 0;
    fMinFontSizePag = 0;
    fHaveVersions = 0;
    fAutoVersion = 0;
    unused412_2 = 0;
    asumyi.clear();
    cChWS = 0;
    cChWSFtnEdn = 0;
    grfDocEvents = 0;
    fVirusPrompted = 0;
    fVirusLoadSafe = 0;
    KeyVirusSession30 = 0;
    for (int _i = 0; _i < (30); ++_i)
        Spare[_i] = 0;
    unused472 = 0;
    unused476 = 0;
    cDBC = 0;
    cDBCFtnEdn = 0;
    unused488 = 0;
    nfcFtnRef2 = 0;
    nfcEdnRef2 = 0;
    hpsZoonFontPag = 0;
    dywDispPag = 0;
}

bool operator==(const DOP &lhs, const DOP &rhs)
{

    for (int _i = 0; _i < (30); ++_i) {
        if (lhs.Spare[_i] != rhs.Spare[_i])
            return false;
    }

    return lhs.fFacingPages == rhs.fFacingPages &&
           lhs.fWidowControl == rhs.fWidowControl &&
           lhs.fPMHMainDoc == rhs.fPMHMainDoc &&
           lhs.grfSuppression == rhs.grfSuppression &&
           lhs.fpc == rhs.fpc &&
           lhs.unused0_7 == rhs.unused0_7 &&
           lhs.grpfIhdt == rhs.grpfIhdt &&
           lhs.rncFtn == rhs.rncFtn &&
           lhs.nFtn == rhs.nFtn &&
           lhs.fOutlineDirtySave == rhs.fOutlineDirtySave &&
           lhs.unused4_1 == rhs.unused4_1 &&
           lhs.fOnlyMacPics == rhs.fOnlyMacPics &&
           lhs.fOnlyWinPics == rhs.fOnlyWinPics &&
           lhs.fLabelDoc == rhs.fLabelDoc &&
           lhs.fHyphCapitals == rhs.fHyphCapitals &&
           lhs.fAutoHyphen == rhs.fAutoHyphen &&
           lhs.fFormNoFields == rhs.fFormNoFields &&
           lhs.fLinkStyles == rhs.fLinkStyles &&
           lhs.fRevMarking == rhs.fRevMarking &&
           lhs.fBackup == rhs.fBackup &&
           lhs.fExactCWords == rhs.fExactCWords &&
           lhs.fPagHidden == rhs.fPagHidden &&
           lhs.fPagResults == rhs.fPagResults &&
           lhs.fLockAtn == rhs.fLockAtn &&
           lhs.fMirrorMargins == rhs.fMirrorMargins &&
           lhs.unused6_6 == rhs.unused6_6 &&
           lhs.fDfltTrueType == rhs.fDfltTrueType &&
           lhs.fPagSuppressTopSpacing == rhs.fPagSuppressTopSpacing &&
           lhs.fProtEnabled == rhs.fProtEnabled &&
           lhs.fDispFormFldSel == rhs.fDispFormFldSel &&
           lhs.fRMView == rhs.fRMView &&
           lhs.fRMPrint == rhs.fRMPrint &&
           lhs.unused7_5 == rhs.unused7_5 &&
           lhs.fLockRev == rhs.fLockRev &&
           lhs.fEmbedFonts == rhs.fEmbedFonts &&
           lhs.copts_fNoTabForInd == rhs.copts_fNoTabForInd &&
           lhs.copts_fNoSpaceRaiseLower == rhs.copts_fNoSpaceRaiseLower &&
           lhs.copts_fSuppressSpbfAfterPageBreak == rhs.copts_fSuppressSpbfAfterPageBreak &&
           lhs.copts_fWrapTrailSpaces == rhs.copts_fWrapTrailSpaces &&
           lhs.copts_fMapPrintTextColor == rhs.copts_fMapPrintTextColor &&
           lhs.copts_fNoColumnBalance == rhs.copts_fNoColumnBalance &&
           lhs.copts_fConvMailMergeEsc == rhs.copts_fConvMailMergeEsc &&
           lhs.copts_fSupressTopSpacing == rhs.copts_fSupressTopSpacing &&
           lhs.copts_fOrigWordTableRules == rhs.copts_fOrigWordTableRules &&
           lhs.copts_fTransparentMetafiles == rhs.copts_fTransparentMetafiles &&
           lhs.copts_fShowBreaksInFrames == rhs.copts_fShowBreaksInFrames &&
           lhs.copts_fSwapBordersFacingPgs == rhs.copts_fSwapBordersFacingPgs &&
           lhs.unused8_12 == rhs.unused8_12 &&
           lhs.dxaTab == rhs.dxaTab &&
           lhs.wSpare == rhs.wSpare &&
           lhs.dxaHotZ == rhs.dxaHotZ &&
           lhs.cConsecHypLim == rhs.cConsecHypLim &&
           lhs.wSpare2 == rhs.wSpare2 &&
           lhs.dttmCreated == rhs.dttmCreated &&
           lhs.dttmRevised == rhs.dttmRevised &&
           lhs.dttmLastPrint == rhs.dttmLastPrint &&
           lhs.nRevision == rhs.nRevision &&
           lhs.tmEdited == rhs.tmEdited &&
           lhs.cWords == rhs.cWords &&
           lhs.cCh == rhs.cCh &&
           lhs.cPg == rhs.cPg &&
           lhs.cParas == rhs.cParas &&
           lhs.rncEdn == rhs.rncEdn &&
           lhs.nEdn == rhs.nEdn &&
           lhs.epc == rhs.epc &&
           lhs.nfcFtnRef == rhs.nfcFtnRef &&
           lhs.nfcEdnRef == rhs.nfcEdnRef &&
           lhs.fPrintFormData == rhs.fPrintFormData &&
           lhs.fSaveFormData == rhs.fSaveFormData &&
           lhs.fShadeFormData == rhs.fShadeFormData &&
           lhs.unused54_13 == rhs.unused54_13 &&
           lhs.fWCFtnEdn == rhs.fWCFtnEdn &&
           lhs.cLines == rhs.cLines &&
           lhs.cWordsFtnEnd == rhs.cWordsFtnEnd &&
           lhs.cChFtnEdn == rhs.cChFtnEdn &&
           lhs.cPgFtnEdn == rhs.cPgFtnEdn &&
           lhs.cParasFtnEdn == rhs.cParasFtnEdn &&
           lhs.cLinesFtnEdn == rhs.cLinesFtnEdn &&
           lhs.lKeyProtDoc == rhs.lKeyProtDoc &&
           lhs.wvkSaved == rhs.wvkSaved &&
           lhs.wScaleSaved == rhs.wScaleSaved &&
           lhs.zkSaved == rhs.zkSaved &&
           lhs.fRotateFontW6 == rhs.fRotateFontW6 &&
           lhs.iGutterPos == rhs.iGutterPos &&
           lhs.fNoTabForInd == rhs.fNoTabForInd &&
           lhs.fNoSpaceRaiseLower == rhs.fNoSpaceRaiseLower &&
           lhs.fSupressSpbfAfterPageBreak == rhs.fSupressSpbfAfterPageBreak &&
           lhs.fWrapTrailSpaces == rhs.fWrapTrailSpaces &&
           lhs.fMapPrintTextColor == rhs.fMapPrintTextColor &&
           lhs.fNoColumnBalance == rhs.fNoColumnBalance &&
           lhs.fConvMailMergeEsc == rhs.fConvMailMergeEsc &&
           lhs.fSupressTopSpacing == rhs.fSupressTopSpacing &&
           lhs.fOrigWordTableRules == rhs.fOrigWordTableRules &&
           lhs.fTransparentMetafiles == rhs.fTransparentMetafiles &&
           lhs.fShowBreaksInFrames == rhs.fShowBreaksInFrames &&
           lhs.fSwapBordersFacingPgs == rhs.fSwapBordersFacingPgs &&
           lhs.unused84_12 == rhs.unused84_12 &&
           lhs.fSuppressTopSpacingMac5 == rhs.fSuppressTopSpacingMac5 &&
           lhs.fTruncDxaExpand == rhs.fTruncDxaExpand &&
           lhs.fPrintBodyBeforeHdr == rhs.fPrintBodyBeforeHdr &&
           lhs.fNoLeading == rhs.fNoLeading &&
           lhs.unused84_20 == rhs.unused84_20 &&
           lhs.fMWSmallCaps == rhs.fMWSmallCaps &&
           lhs.unused84_22 == rhs.unused84_22 &&
           lhs.adt == rhs.adt &&
           lhs.doptypography == rhs.doptypography &&
           lhs.dogrid == rhs.dogrid &&
           lhs.reserved == rhs.reserved &&
           lhs.lvl == rhs.lvl &&
           lhs.fGramAllDone == rhs.fGramAllDone &&
           lhs.fGramAllClean == rhs.fGramAllClean &&
           lhs.fSubsetFonts == rhs.fSubsetFonts &&
           lhs.fHideLastVersion == rhs.fHideLastVersion &&
           lhs.fHtmlDoc == rhs.fHtmlDoc &&
           lhs.unused410_11 == rhs.unused410_11 &&
           lhs.fSnapBorder == rhs.fSnapBorder &&
           lhs.fIncludeHeader == rhs.fIncludeHeader &&
           lhs.fIncludeFooter == rhs.fIncludeFooter &&
           lhs.fForcePageSizePag == rhs.fForcePageSizePag &&
           lhs.fMinFontSizePag == rhs.fMinFontSizePag &&
           lhs.fHaveVersions == rhs.fHaveVersions &&
           lhs.fAutoVersion == rhs.fAutoVersion &&
           lhs.unused412_2 == rhs.unused412_2 &&
           lhs.asumyi == rhs.asumyi &&
           lhs.cChWS == rhs.cChWS &&
           lhs.cChWSFtnEdn == rhs.cChWSFtnEdn &&
           lhs.grfDocEvents == rhs.grfDocEvents &&
           lhs.fVirusPrompted == rhs.fVirusPrompted &&
           lhs.fVirusLoadSafe == rhs.fVirusLoadSafe &&
           lhs.KeyVirusSession30 == rhs.KeyVirusSession30 &&
           lhs.unused472 == rhs.unused472 &&
           lhs.unused476 == rhs.unused476 &&
           lhs.cDBC == rhs.cDBC &&
           lhs.cDBCFtnEdn == rhs.cDBCFtnEdn &&
           lhs.unused488 == rhs.unused488 &&
           lhs.nfcFtnRef2 == rhs.nfcFtnRef2 &&
           lhs.nfcEdnRef2 == rhs.nfcEdnRef2 &&
           lhs.hpsZoonFontPag == rhs.hpsZoonFontPag &&
           lhs.dywDispPag == rhs.dywDispPag;
}

bool operator!=(const DOP &lhs, const DOP &rhs)
{
    return !(lhs == rhs);
}


// FIB implementation

FIB::FIB()
{
    clear();
}

FIB::FIB(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

bool FIB::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    U8 shifterU8;
    U16 shifterU16;

    if (preservePos)
        stream->push();

    wIdent = stream->readU16();
    nFib = stream->readU16();
    nProduct = stream->readU16();
    lid = stream->readU16();
    pnNext = stream->readS16();
    shifterU16 = stream->readU16();
    fDot = shifterU16;
    shifterU16 >>= 1;
    fGlsy = shifterU16;
    shifterU16 >>= 1;
    fComplex = shifterU16;
    shifterU16 >>= 1;
    fHasPic = shifterU16;
    shifterU16 >>= 1;
    cQuickSaves = shifterU16;
    shifterU16 >>= 4;
    fEncrypted = shifterU16;
    shifterU16 >>= 1;
    fWhichTblStm = shifterU16;
    shifterU16 >>= 1;
    fReadOnlyRecommended = shifterU16;
    shifterU16 >>= 1;
    fWriteReservation = shifterU16;
    shifterU16 >>= 1;
    fExtChar = shifterU16;
    shifterU16 >>= 1;
    fLoadOverride = shifterU16;
    shifterU16 >>= 1;
    fFarEast = shifterU16;
    shifterU16 >>= 1;
    fCrypto = shifterU16;
    nFibBack = stream->readU16();
    lKey = stream->readU32();
    envr = stream->readU8();
    shifterU8 = stream->readU8();
    fMac = shifterU8;
    shifterU8 >>= 1;
    fEmptySpecial = shifterU8;
    shifterU8 >>= 1;
    fLoadOverridePage = shifterU8;
    shifterU8 >>= 1;
    fFutureSavedUndo = shifterU8;
    shifterU8 >>= 1;
    fWord97Saved = shifterU8;
    shifterU8 >>= 1;
    fSpare0 = shifterU8;
    chs = stream->readU16();
    chsTables = stream->readU16();
    fcMin = stream->readU32();
    fcMac = stream->readU32();
    csw = stream->readU16();
    wMagicCreated = stream->readU16();
    wMagicRevised = stream->readU16();
    wMagicCreatedPrivate = stream->readU16();
    wMagicRevisedPrivate = stream->readU16();
    pnFbpChpFirst_W6 = stream->readU16();
    pnChpFirst_W6 = stream->readU16();
    cpnBteChp_W6 = stream->readU16();
    pnFbpPapFirst_W6 = stream->readU16();
    pnPapFirst_W6 = stream->readU16();
    cpnBtePap_W6 = stream->readU16();
    pnFbpLvcFirst_W6 = stream->readU16();
    pnLvcFirst_W6 = stream->readU16();
    cpnBteLvc_W6 = stream->readU16();
    lidFE = stream->readS16();
    clw = stream->readU16();
    cbMac = stream->readU32();
    lProductCreated = stream->readU32();
    lProductRevised = stream->readU32();
    ccpText = stream->readU32();
    ccpFtn = stream->readU32();
    ccpHdd = stream->readU32();
    ccpMcr = stream->readU32();
    ccpAtn = stream->readU32();
    ccpEdn = stream->readU32();
    ccpTxbx = stream->readU32();
    ccpHdrTxbx = stream->readU32();
    pnFbpChpFirst = stream->readU32();
    pnChpFirst = stream->readU32();
    cpnBteChp = stream->readU32();
    pnFbpPapFirst = stream->readU32();
    pnPapFirst = stream->readU32();
    cpnBtePap = stream->readU32();
    pnFbpLvcFirst = stream->readU32();
    pnLvcFirst = stream->readU32();
    cpnBteLvc = stream->readU32();
    fcIslandFirst = stream->readU32();
    fcIslandLim = stream->readU32();
    cfclcb = stream->readU16();
    fcStshfOrig = stream->readU32();
    lcbStshfOrig = stream->readU32();
    fcStshf = stream->readU32();
    lcbStshf = stream->readU32();
    fcPlcffndRef = stream->readU32();
    lcbPlcffndRef = stream->readU32();
    fcPlcffndTxt = stream->readU32();
    lcbPlcffndTxt = stream->readU32();
    fcPlcfandRef = stream->readU32();
    lcbPlcfandRef = stream->readU32();
    fcPlcfandTxt = stream->readU32();
    lcbPlcfandTxt = stream->readU32();
    fcPlcfsed = stream->readU32();
    lcbPlcfsed = stream->readU32();
    fcPlcfpad = stream->readU32();
    lcbPlcfpad = stream->readU32();
    fcPlcfphe = stream->readU32();
    lcbPlcfphe = stream->readU32();
    fcSttbfglsy = stream->readU32();
    lcbSttbfglsy = stream->readU32();
    fcPlcfglsy = stream->readU32();
    lcbPlcfglsy = stream->readU32();
    fcPlcfhdd = stream->readU32();
    lcbPlcfhdd = stream->readU32();
    fcPlcfbteChpx = stream->readU32();
    lcbPlcfbteChpx = stream->readU32();
    fcPlcfbtePapx = stream->readU32();
    lcbPlcfbtePapx = stream->readU32();
    fcPlcfsea = stream->readU32();
    lcbPlcfsea = stream->readU32();
    fcSttbfffn = stream->readU32();
    lcbSttbfffn = stream->readU32();
    fcPlcffldMom = stream->readU32();
    lcbPlcffldMom = stream->readU32();
    fcPlcffldHdr = stream->readU32();
    lcbPlcffldHdr = stream->readU32();
    fcPlcffldFtn = stream->readU32();
    lcbPlcffldFtn = stream->readU32();
    fcPlcffldAtn = stream->readU32();
    lcbPlcffldAtn = stream->readU32();
    fcPlcffldMcr = stream->readU32();
    lcbPlcffldMcr = stream->readU32();
    fcSttbfbkmk = stream->readU32();
    lcbSttbfbkmk = stream->readU32();
    fcPlcfbkf = stream->readU32();
    lcbPlcfbkf = stream->readU32();
    fcPlcfbkl = stream->readU32();
    lcbPlcfbkl = stream->readU32();
    fcCmds = stream->readU32();
    lcbCmds = stream->readU32();
    fcPlcmcr = stream->readU32();
    lcbPlcmcr = stream->readU32();
    fcSttbfmcr = stream->readU32();
    lcbSttbfmcr = stream->readU32();
    fcPrDrvr = stream->readU32();
    lcbPrDrvr = stream->readU32();
    fcPrEnvPort = stream->readU32();
    lcbPrEnvPort = stream->readU32();
    fcPrEnvLand = stream->readU32();
    lcbPrEnvLand = stream->readU32();
    fcWss = stream->readU32();
    lcbWss = stream->readU32();
    fcDop = stream->readU32();
    lcbDop = stream->readU32();
    fcSttbfAssoc = stream->readU32();
    lcbSttbfAssoc = stream->readU32();
    fcClx = stream->readU32();
    lcbClx = stream->readU32();
    fcPlcfpgdFtn = stream->readU32();
    lcbPlcfpgdFtn = stream->readU32();
    fcAutosaveSource = stream->readU32();
    lcbAutosaveSource = stream->readU32();
    fcGrpXstAtnOwners = stream->readU32();
    lcbGrpXstAtnOwners = stream->readU32();
    fcSttbfAtnbkmk = stream->readU32();
    lcbSttbfAtnbkmk = stream->readU32();
    fcPlcdoaMom = stream->readU32();
    lcbPlcdoaMom = stream->readU32();
    fcPlcdoaHdr = stream->readU32();
    lcbPlcdoaHdr = stream->readU32();
    fcPlcspaMom = stream->readU32();
    lcbPlcspaMom = stream->readU32();
    fcPlcspaHdr = stream->readU32();
    lcbPlcspaHdr = stream->readU32();
    fcPlcfAtnbkf = stream->readU32();
    lcbPlcfAtnbkf = stream->readU32();
    fcPlcfAtnbkl = stream->readU32();
    lcbPlcfAtnbkl = stream->readU32();
    fcPms = stream->readU32();
    lcbPms = stream->readU32();
    fcFormFldSttbf = stream->readU32();
    lcbFormFldSttbf = stream->readU32();
    fcPlcfendRef = stream->readU32();
    lcbPlcfendRef = stream->readU32();
    fcPlcfendTxt = stream->readU32();
    lcbPlcfendTxt = stream->readU32();
    fcPlcffldEdn = stream->readU32();
    lcbPlcffldEdn = stream->readU32();
    fcPlcfpgdEdn = stream->readU32();
    lcbPlcfpgdEdn = stream->readU32();
    fcDggInfo = stream->readU32();
    lcbDggInfo = stream->readU32();
    fcSttbfRMark = stream->readU32();
    lcbSttbfRMark = stream->readU32();
    fcSttbfCaption = stream->readU32();
    lcbSttbfCaption = stream->readU32();
    fcSttbfAutoCaption = stream->readU32();
    lcbSttbfAutoCaption = stream->readU32();
    fcPlcfwkb = stream->readU32();
    lcbPlcfwkb = stream->readU32();
    fcPlcfspl = stream->readU32();
    lcbPlcfspl = stream->readU32();
    fcPlcftxbxTxt = stream->readU32();
    lcbPlcftxbxTxt = stream->readU32();
    fcPlcffldTxbx = stream->readU32();
    lcbPlcffldTxbx = stream->readU32();
    fcPlcfHdrtxbxTxt = stream->readU32();
    lcbPlcfHdrtxbxTxt = stream->readU32();
    fcPlcffldHdrTxbx = stream->readU32();
    lcbPlcffldHdrTxbx = stream->readU32();
    fcStwUser = stream->readU32();
    lcbStwUser = stream->readU32();
    fcSttbttmbd = stream->readU32();
    lcbSttbttmbd = stream->readU32();
    fcUnused = stream->readU32();
    lcbUnused = stream->readU32();
    fcPgdMother = stream->readU32();
    lcbPgdMother = stream->readU32();
    fcBkdMother = stream->readU32();
    lcbBkdMother = stream->readU32();
    fcPgdFtn = stream->readU32();
    lcbPgdFtn = stream->readU32();
    fcBkdFtn = stream->readU32();
    lcbBkdFtn = stream->readU32();
    fcPgdEdn = stream->readU32();
    lcbPgdEdn = stream->readU32();
    fcBkdEdn = stream->readU32();
    lcbBkdEdn = stream->readU32();
    fcSttbfIntlFld = stream->readU32();
    lcbSttbfIntlFld = stream->readU32();
    fcRouteSlip = stream->readU32();
    lcbRouteSlip = stream->readU32();
    fcSttbSavedBy = stream->readU32();
    lcbSttbSavedBy = stream->readU32();
    fcSttbFnm = stream->readU32();
    lcbSttbFnm = stream->readU32();
    fcPlcfLst = stream->readU32();
    lcbPlcfLst = stream->readU32();
    fcPlfLfo = stream->readU32();
    lcbPlfLfo = stream->readU32();
    fcPlcftxbxBkd = stream->readU32();
    lcbPlcftxbxBkd = stream->readU32();
    fcPlcftxbxHdrBkd = stream->readU32();
    lcbPlcftxbxHdrBkd = stream->readU32();
    fcDocUndo = stream->readU32();
    lcbDocUndo = stream->readU32();
    fcRgbuse = stream->readU32();
    lcbRgbuse = stream->readU32();
    fcUsp = stream->readU32();
    lcbUsp = stream->readU32();
    fcUskf = stream->readU32();
    lcbUskf = stream->readU32();
    fcPlcupcRgbuse = stream->readU32();
    lcbPlcupcRgbuse = stream->readU32();
    fcPlcupcUsp = stream->readU32();
    lcbPlcupcUsp = stream->readU32();
    fcSttbGlsyStyle = stream->readU32();
    lcbSttbGlsyStyle = stream->readU32();
    fcPlgosl = stream->readU32();
    lcbPlgosl = stream->readU32();
    fcPlcocx = stream->readU32();
    lcbPlcocx = stream->readU32();
    fcPlcfbteLvc = stream->readU32();
    lcbPlcfbteLvc = stream->readU32();
    dwLowDateTime = stream->readU32();
    dwHighDateTime = stream->readU32();
    fcPlcflvc = stream->readU32();
    lcbPlcflvc = stream->readU32();
    fcPlcasumy = stream->readU32();
    lcbPlcasumy = stream->readU32();
    fcPlcfgram = stream->readU32();
    lcbPlcfgram = stream->readU32();
    fcSttbListNames = stream->readU32();
    lcbSttbListNames = stream->readU32();
    fcSttbfUssr = stream->readU32();
    lcbSttbfUssr = stream->readU32();

    if (preservePos)
        stream->pop();
    return true;
}

void FIB::clear()
{
    wIdent = 0;
    nFib = 0;
    nProduct = 0;
    lid = 0;
    pnNext = 0;
    fDot = 0;
    fGlsy = 0;
    fComplex = 0;
    fHasPic = 0;
    cQuickSaves = 0;
    fEncrypted = 0;
    fWhichTblStm = 0;
    fReadOnlyRecommended = 0;
    fWriteReservation = 0;
    fExtChar = 0;
    fLoadOverride = 0;
    fFarEast = 0;
    fCrypto = 0;
    nFibBack = 0;
    lKey = 0;
    envr = 0;
    fMac = 0;
    fEmptySpecial = 0;
    fLoadOverridePage = 0;
    fFutureSavedUndo = 0;
    fWord97Saved = 0;
    fSpare0 = 0;
    chs = 0;
    chsTables = 0;
    fcMin = 0;
    fcMac = 0;
    csw = 0;
    wMagicCreated = 0;
    wMagicRevised = 0;
    wMagicCreatedPrivate = 0;
    wMagicRevisedPrivate = 0;
    pnFbpChpFirst_W6 = 0;
    pnChpFirst_W6 = 0;
    cpnBteChp_W6 = 0;
    pnFbpPapFirst_W6 = 0;
    pnPapFirst_W6 = 0;
    cpnBtePap_W6 = 0;
    pnFbpLvcFirst_W6 = 0;
    pnLvcFirst_W6 = 0;
    cpnBteLvc_W6 = 0;
    lidFE = 0;
    clw = 0;
    cbMac = 0;
    lProductCreated = 0;
    lProductRevised = 0;
    ccpText = 0;
    ccpFtn = 0;
    ccpHdd = 0;
    ccpMcr = 0;
    ccpAtn = 0;
    ccpEdn = 0;
    ccpTxbx = 0;
    ccpHdrTxbx = 0;
    pnFbpChpFirst = 0;
    pnChpFirst = 0;
    cpnBteChp = 0;
    pnFbpPapFirst = 0;
    pnPapFirst = 0;
    cpnBtePap = 0;
    pnFbpLvcFirst = 0;
    pnLvcFirst = 0;
    cpnBteLvc = 0;
    fcIslandFirst = 0;
    fcIslandLim = 0;
    cfclcb = 0;
    fcStshfOrig = 0;
    lcbStshfOrig = 0;
    fcStshf = 0;
    lcbStshf = 0;
    fcPlcffndRef = 0;
    lcbPlcffndRef = 0;
    fcPlcffndTxt = 0;
    lcbPlcffndTxt = 0;
    fcPlcfandRef = 0;
    lcbPlcfandRef = 0;
    fcPlcfandTxt = 0;
    lcbPlcfandTxt = 0;
    fcPlcfsed = 0;
    lcbPlcfsed = 0;
    fcPlcfpad = 0;
    lcbPlcfpad = 0;
    fcPlcfphe = 0;
    lcbPlcfphe = 0;
    fcSttbfglsy = 0;
    lcbSttbfglsy = 0;
    fcPlcfglsy = 0;
    lcbPlcfglsy = 0;
    fcPlcfhdd = 0;
    lcbPlcfhdd = 0;
    fcPlcfbteChpx = 0;
    lcbPlcfbteChpx = 0;
    fcPlcfbtePapx = 0;
    lcbPlcfbtePapx = 0;
    fcPlcfsea = 0;
    lcbPlcfsea = 0;
    fcSttbfffn = 0;
    lcbSttbfffn = 0;
    fcPlcffldMom = 0;
    lcbPlcffldMom = 0;
    fcPlcffldHdr = 0;
    lcbPlcffldHdr = 0;
    fcPlcffldFtn = 0;
    lcbPlcffldFtn = 0;
    fcPlcffldAtn = 0;
    lcbPlcffldAtn = 0;
    fcPlcffldMcr = 0;
    lcbPlcffldMcr = 0;
    fcSttbfbkmk = 0;
    lcbSttbfbkmk = 0;
    fcPlcfbkf = 0;
    lcbPlcfbkf = 0;
    fcPlcfbkl = 0;
    lcbPlcfbkl = 0;
    fcCmds = 0;
    lcbCmds = 0;
    fcPlcmcr = 0;
    lcbPlcmcr = 0;
    fcSttbfmcr = 0;
    lcbSttbfmcr = 0;
    fcPrDrvr = 0;
    lcbPrDrvr = 0;
    fcPrEnvPort = 0;
    lcbPrEnvPort = 0;
    fcPrEnvLand = 0;
    lcbPrEnvLand = 0;
    fcWss = 0;
    lcbWss = 0;
    fcDop = 0;
    lcbDop = 0;
    fcSttbfAssoc = 0;
    lcbSttbfAssoc = 0;
    fcClx = 0;
    lcbClx = 0;
    fcPlcfpgdFtn = 0;
    lcbPlcfpgdFtn = 0;
    fcAutosaveSource = 0;
    lcbAutosaveSource = 0;
    fcGrpXstAtnOwners = 0;
    lcbGrpXstAtnOwners = 0;
    fcSttbfAtnbkmk = 0;
    lcbSttbfAtnbkmk = 0;
    fcPlcdoaMom = 0;
    lcbPlcdoaMom = 0;
    fcPlcdoaHdr = 0;
    lcbPlcdoaHdr = 0;
    fcPlcspaMom = 0;
    lcbPlcspaMom = 0;
    fcPlcspaHdr = 0;
    lcbPlcspaHdr = 0;
    fcPlcfAtnbkf = 0;
    lcbPlcfAtnbkf = 0;
    fcPlcfAtnbkl = 0;
    lcbPlcfAtnbkl = 0;
    fcPms = 0;
    lcbPms = 0;
    fcFormFldSttbf = 0;
    lcbFormFldSttbf = 0;
    fcPlcfendRef = 0;
    lcbPlcfendRef = 0;
    fcPlcfendTxt = 0;
    lcbPlcfendTxt = 0;
    fcPlcffldEdn = 0;
    lcbPlcffldEdn = 0;
    fcPlcfpgdEdn = 0;
    lcbPlcfpgdEdn = 0;
    fcDggInfo = 0;
    lcbDggInfo = 0;
    fcSttbfRMark = 0;
    lcbSttbfRMark = 0;
    fcSttbfCaption = 0;
    lcbSttbfCaption = 0;
    fcSttbfAutoCaption = 0;
    lcbSttbfAutoCaption = 0;
    fcPlcfwkb = 0;
    lcbPlcfwkb = 0;
    fcPlcfspl = 0;
    lcbPlcfspl = 0;
    fcPlcftxbxTxt = 0;
    lcbPlcftxbxTxt = 0;
    fcPlcffldTxbx = 0;
    lcbPlcffldTxbx = 0;
    fcPlcfHdrtxbxTxt = 0;
    lcbPlcfHdrtxbxTxt = 0;
    fcPlcffldHdrTxbx = 0;
    lcbPlcffldHdrTxbx = 0;
    fcStwUser = 0;
    lcbStwUser = 0;
    fcSttbttmbd = 0;
    lcbSttbttmbd = 0;
    fcUnused = 0;
    lcbUnused = 0;
    fcPgdMother = 0;
    lcbPgdMother = 0;
    fcBkdMother = 0;
    lcbBkdMother = 0;
    fcPgdFtn = 0;
    lcbPgdFtn = 0;
    fcBkdFtn = 0;
    lcbBkdFtn = 0;
    fcPgdEdn = 0;
    lcbPgdEdn = 0;
    fcBkdEdn = 0;
    lcbBkdEdn = 0;
    fcSttbfIntlFld = 0;
    lcbSttbfIntlFld = 0;
    fcRouteSlip = 0;
    lcbRouteSlip = 0;
    fcSttbSavedBy = 0;
    lcbSttbSavedBy = 0;
    fcSttbFnm = 0;
    lcbSttbFnm = 0;
    fcPlcfLst = 0;
    lcbPlcfLst = 0;
    fcPlfLfo = 0;
    lcbPlfLfo = 0;
    fcPlcftxbxBkd = 0;
    lcbPlcftxbxBkd = 0;
    fcPlcftxbxHdrBkd = 0;
    lcbPlcftxbxHdrBkd = 0;
    fcDocUndo = 0;
    lcbDocUndo = 0;
    fcRgbuse = 0;
    lcbRgbuse = 0;
    fcUsp = 0;
    lcbUsp = 0;
    fcUskf = 0;
    lcbUskf = 0;
    fcPlcupcRgbuse = 0;
    lcbPlcupcRgbuse = 0;
    fcPlcupcUsp = 0;
    lcbPlcupcUsp = 0;
    fcSttbGlsyStyle = 0;
    lcbSttbGlsyStyle = 0;
    fcPlgosl = 0;
    lcbPlgosl = 0;
    fcPlcocx = 0;
    lcbPlcocx = 0;
    fcPlcfbteLvc = 0;
    lcbPlcfbteLvc = 0;
    dwLowDateTime = 0;
    dwHighDateTime = 0;
    fcPlcflvc = 0;
    lcbPlcflvc = 0;
    fcPlcasumy = 0;
    lcbPlcasumy = 0;
    fcPlcfgram = 0;
    lcbPlcfgram = 0;
    fcSttbListNames = 0;
    lcbSttbListNames = 0;
    fcSttbfUssr = 0;
    lcbSttbfUssr = 0;
}

bool operator==(const FIB &lhs, const FIB &rhs)
{

    return lhs.wIdent == rhs.wIdent &&
           lhs.nFib == rhs.nFib &&
           lhs.nProduct == rhs.nProduct &&
           lhs.lid == rhs.lid &&
           lhs.pnNext == rhs.pnNext &&
           lhs.fDot == rhs.fDot &&
           lhs.fGlsy == rhs.fGlsy &&
           lhs.fComplex == rhs.fComplex &&
           lhs.fHasPic == rhs.fHasPic &&
           lhs.cQuickSaves == rhs.cQuickSaves &&
           lhs.fEncrypted == rhs.fEncrypted &&
           lhs.fWhichTblStm == rhs.fWhichTblStm &&
           lhs.fReadOnlyRecommended == rhs.fReadOnlyRecommended &&
           lhs.fWriteReservation == rhs.fWriteReservation &&
           lhs.fExtChar == rhs.fExtChar &&
           lhs.fLoadOverride == rhs.fLoadOverride &&
           lhs.fFarEast == rhs.fFarEast &&
           lhs.fCrypto == rhs.fCrypto &&
           lhs.nFibBack == rhs.nFibBack &&
           lhs.lKey == rhs.lKey &&
           lhs.envr == rhs.envr &&
           lhs.fMac == rhs.fMac &&
           lhs.fEmptySpecial == rhs.fEmptySpecial &&
           lhs.fLoadOverridePage == rhs.fLoadOverridePage &&
           lhs.fFutureSavedUndo == rhs.fFutureSavedUndo &&
           lhs.fWord97Saved == rhs.fWord97Saved &&
           lhs.fSpare0 == rhs.fSpare0 &&
           lhs.chs == rhs.chs &&
           lhs.chsTables == rhs.chsTables &&
           lhs.fcMin == rhs.fcMin &&
           lhs.fcMac == rhs.fcMac &&
           lhs.csw == rhs.csw &&
           lhs.wMagicCreated == rhs.wMagicCreated &&
           lhs.wMagicRevised == rhs.wMagicRevised &&
           lhs.wMagicCreatedPrivate == rhs.wMagicCreatedPrivate &&
           lhs.wMagicRevisedPrivate == rhs.wMagicRevisedPrivate &&
           lhs.pnFbpChpFirst_W6 == rhs.pnFbpChpFirst_W6 &&
           lhs.pnChpFirst_W6 == rhs.pnChpFirst_W6 &&
           lhs.cpnBteChp_W6 == rhs.cpnBteChp_W6 &&
           lhs.pnFbpPapFirst_W6 == rhs.pnFbpPapFirst_W6 &&
           lhs.pnPapFirst_W6 == rhs.pnPapFirst_W6 &&
           lhs.cpnBtePap_W6 == rhs.cpnBtePap_W6 &&
           lhs.pnFbpLvcFirst_W6 == rhs.pnFbpLvcFirst_W6 &&
           lhs.pnLvcFirst_W6 == rhs.pnLvcFirst_W6 &&
           lhs.cpnBteLvc_W6 == rhs.cpnBteLvc_W6 &&
           lhs.lidFE == rhs.lidFE &&
           lhs.clw == rhs.clw &&
           lhs.cbMac == rhs.cbMac &&
           lhs.lProductCreated == rhs.lProductCreated &&
           lhs.lProductRevised == rhs.lProductRevised &&
           lhs.ccpText == rhs.ccpText &&
           lhs.ccpFtn == rhs.ccpFtn &&
           lhs.ccpHdd == rhs.ccpHdd &&
           lhs.ccpMcr == rhs.ccpMcr &&
           lhs.ccpAtn == rhs.ccpAtn &&
           lhs.ccpEdn == rhs.ccpEdn &&
           lhs.ccpTxbx == rhs.ccpTxbx &&
           lhs.ccpHdrTxbx == rhs.ccpHdrTxbx &&
           lhs.pnFbpChpFirst == rhs.pnFbpChpFirst &&
           lhs.pnChpFirst == rhs.pnChpFirst &&
           lhs.cpnBteChp == rhs.cpnBteChp &&
           lhs.pnFbpPapFirst == rhs.pnFbpPapFirst &&
           lhs.pnPapFirst == rhs.pnPapFirst &&
           lhs.cpnBtePap == rhs.cpnBtePap &&
           lhs.pnFbpLvcFirst == rhs.pnFbpLvcFirst &&
           lhs.pnLvcFirst == rhs.pnLvcFirst &&
           lhs.cpnBteLvc == rhs.cpnBteLvc &&
           lhs.fcIslandFirst == rhs.fcIslandFirst &&
           lhs.fcIslandLim == rhs.fcIslandLim &&
           lhs.cfclcb == rhs.cfclcb &&
           lhs.fcStshfOrig == rhs.fcStshfOrig &&
           lhs.lcbStshfOrig == rhs.lcbStshfOrig &&
           lhs.fcStshf == rhs.fcStshf &&
           lhs.lcbStshf == rhs.lcbStshf &&
           lhs.fcPlcffndRef == rhs.fcPlcffndRef &&
           lhs.lcbPlcffndRef == rhs.lcbPlcffndRef &&
           lhs.fcPlcffndTxt == rhs.fcPlcffndTxt &&
           lhs.lcbPlcffndTxt == rhs.lcbPlcffndTxt &&
           lhs.fcPlcfandRef == rhs.fcPlcfandRef &&
           lhs.lcbPlcfandRef == rhs.lcbPlcfandRef &&
           lhs.fcPlcfandTxt == rhs.fcPlcfandTxt &&
           lhs.lcbPlcfandTxt == rhs.lcbPlcfandTxt &&
           lhs.fcPlcfsed == rhs.fcPlcfsed &&
           lhs.lcbPlcfsed == rhs.lcbPlcfsed &&
           lhs.fcPlcfpad == rhs.fcPlcfpad &&
           lhs.lcbPlcfpad == rhs.lcbPlcfpad &&
           lhs.fcPlcfphe == rhs.fcPlcfphe &&
           lhs.lcbPlcfphe == rhs.lcbPlcfphe &&
           lhs.fcSttbfglsy == rhs.fcSttbfglsy &&
           lhs.lcbSttbfglsy == rhs.lcbSttbfglsy &&
           lhs.fcPlcfglsy == rhs.fcPlcfglsy &&
           lhs.lcbPlcfglsy == rhs.lcbPlcfglsy &&
           lhs.fcPlcfhdd == rhs.fcPlcfhdd &&
           lhs.lcbPlcfhdd == rhs.lcbPlcfhdd &&
           lhs.fcPlcfbteChpx == rhs.fcPlcfbteChpx &&
           lhs.lcbPlcfbteChpx == rhs.lcbPlcfbteChpx &&
           lhs.fcPlcfbtePapx == rhs.fcPlcfbtePapx &&
           lhs.lcbPlcfbtePapx == rhs.lcbPlcfbtePapx &&
           lhs.fcPlcfsea == rhs.fcPlcfsea &&
           lhs.lcbPlcfsea == rhs.lcbPlcfsea &&
           lhs.fcSttbfffn == rhs.fcSttbfffn &&
           lhs.lcbSttbfffn == rhs.lcbSttbfffn &&
           lhs.fcPlcffldMom == rhs.fcPlcffldMom &&
           lhs.lcbPlcffldMom == rhs.lcbPlcffldMom &&
           lhs.fcPlcffldHdr == rhs.fcPlcffldHdr &&
           lhs.lcbPlcffldHdr == rhs.lcbPlcffldHdr &&
           lhs.fcPlcffldFtn == rhs.fcPlcffldFtn &&
           lhs.lcbPlcffldFtn == rhs.lcbPlcffldFtn &&
           lhs.fcPlcffldAtn == rhs.fcPlcffldAtn &&
           lhs.lcbPlcffldAtn == rhs.lcbPlcffldAtn &&
           lhs.fcPlcffldMcr == rhs.fcPlcffldMcr &&
           lhs.lcbPlcffldMcr == rhs.lcbPlcffldMcr &&
           lhs.fcSttbfbkmk == rhs.fcSttbfbkmk &&
           lhs.lcbSttbfbkmk == rhs.lcbSttbfbkmk &&
           lhs.fcPlcfbkf == rhs.fcPlcfbkf &&
           lhs.lcbPlcfbkf == rhs.lcbPlcfbkf &&
           lhs.fcPlcfbkl == rhs.fcPlcfbkl &&
           lhs.lcbPlcfbkl == rhs.lcbPlcfbkl &&
           lhs.fcCmds == rhs.fcCmds &&
           lhs.lcbCmds == rhs.lcbCmds &&
           lhs.fcPlcmcr == rhs.fcPlcmcr &&
           lhs.lcbPlcmcr == rhs.lcbPlcmcr &&
           lhs.fcSttbfmcr == rhs.fcSttbfmcr &&
           lhs.lcbSttbfmcr == rhs.lcbSttbfmcr &&
           lhs.fcPrDrvr == rhs.fcPrDrvr &&
           lhs.lcbPrDrvr == rhs.lcbPrDrvr &&
           lhs.fcPrEnvPort == rhs.fcPrEnvPort &&
           lhs.lcbPrEnvPort == rhs.lcbPrEnvPort &&
           lhs.fcPrEnvLand == rhs.fcPrEnvLand &&
           lhs.lcbPrEnvLand == rhs.lcbPrEnvLand &&
           lhs.fcWss == rhs.fcWss &&
           lhs.lcbWss == rhs.lcbWss &&
           lhs.fcDop == rhs.fcDop &&
           lhs.lcbDop == rhs.lcbDop &&
           lhs.fcSttbfAssoc == rhs.fcSttbfAssoc &&
           lhs.lcbSttbfAssoc == rhs.lcbSttbfAssoc &&
           lhs.fcClx == rhs.fcClx &&
           lhs.lcbClx == rhs.lcbClx &&
           lhs.fcPlcfpgdFtn == rhs.fcPlcfpgdFtn &&
           lhs.lcbPlcfpgdFtn == rhs.lcbPlcfpgdFtn &&
           lhs.fcAutosaveSource == rhs.fcAutosaveSource &&
           lhs.lcbAutosaveSource == rhs.lcbAutosaveSource &&
           lhs.fcGrpXstAtnOwners == rhs.fcGrpXstAtnOwners &&
           lhs.lcbGrpXstAtnOwners == rhs.lcbGrpXstAtnOwners &&
           lhs.fcSttbfAtnbkmk == rhs.fcSttbfAtnbkmk &&
           lhs.lcbSttbfAtnbkmk == rhs.lcbSttbfAtnbkmk &&
           lhs.fcPlcdoaMom == rhs.fcPlcdoaMom &&
           lhs.lcbPlcdoaMom == rhs.lcbPlcdoaMom &&
           lhs.fcPlcdoaHdr == rhs.fcPlcdoaHdr &&
           lhs.lcbPlcdoaHdr == rhs.lcbPlcdoaHdr &&
           lhs.fcPlcspaMom == rhs.fcPlcspaMom &&
           lhs.lcbPlcspaMom == rhs.lcbPlcspaMom &&
           lhs.fcPlcspaHdr == rhs.fcPlcspaHdr &&
           lhs.lcbPlcspaHdr == rhs.lcbPlcspaHdr &&
           lhs.fcPlcfAtnbkf == rhs.fcPlcfAtnbkf &&
           lhs.lcbPlcfAtnbkf == rhs.lcbPlcfAtnbkf &&
           lhs.fcPlcfAtnbkl == rhs.fcPlcfAtnbkl &&
           lhs.lcbPlcfAtnbkl == rhs.lcbPlcfAtnbkl &&
           lhs.fcPms == rhs.fcPms &&
           lhs.lcbPms == rhs.lcbPms &&
           lhs.fcFormFldSttbf == rhs.fcFormFldSttbf &&
           lhs.lcbFormFldSttbf == rhs.lcbFormFldSttbf &&
           lhs.fcPlcfendRef == rhs.fcPlcfendRef &&
           lhs.lcbPlcfendRef == rhs.lcbPlcfendRef &&
           lhs.fcPlcfendTxt == rhs.fcPlcfendTxt &&
           lhs.lcbPlcfendTxt == rhs.lcbPlcfendTxt &&
           lhs.fcPlcffldEdn == rhs.fcPlcffldEdn &&
           lhs.lcbPlcffldEdn == rhs.lcbPlcffldEdn &&
           lhs.fcPlcfpgdEdn == rhs.fcPlcfpgdEdn &&
           lhs.lcbPlcfpgdEdn == rhs.lcbPlcfpgdEdn &&
           lhs.fcDggInfo == rhs.fcDggInfo &&
           lhs.lcbDggInfo == rhs.lcbDggInfo &&
           lhs.fcSttbfRMark == rhs.fcSttbfRMark &&
           lhs.lcbSttbfRMark == rhs.lcbSttbfRMark &&
           lhs.fcSttbfCaption == rhs.fcSttbfCaption &&
           lhs.lcbSttbfCaption == rhs.lcbSttbfCaption &&
           lhs.fcSttbfAutoCaption == rhs.fcSttbfAutoCaption &&
           lhs.lcbSttbfAutoCaption == rhs.lcbSttbfAutoCaption &&
           lhs.fcPlcfwkb == rhs.fcPlcfwkb &&
           lhs.lcbPlcfwkb == rhs.lcbPlcfwkb &&
           lhs.fcPlcfspl == rhs.fcPlcfspl &&
           lhs.lcbPlcfspl == rhs.lcbPlcfspl &&
           lhs.fcPlcftxbxTxt == rhs.fcPlcftxbxTxt &&
           lhs.lcbPlcftxbxTxt == rhs.lcbPlcftxbxTxt &&
           lhs.fcPlcffldTxbx == rhs.fcPlcffldTxbx &&
           lhs.lcbPlcffldTxbx == rhs.lcbPlcffldTxbx &&
           lhs.fcPlcfHdrtxbxTxt == rhs.fcPlcfHdrtxbxTxt &&
           lhs.lcbPlcfHdrtxbxTxt == rhs.lcbPlcfHdrtxbxTxt &&
           lhs.fcPlcffldHdrTxbx == rhs.fcPlcffldHdrTxbx &&
           lhs.lcbPlcffldHdrTxbx == rhs.lcbPlcffldHdrTxbx &&
           lhs.fcStwUser == rhs.fcStwUser &&
           lhs.lcbStwUser == rhs.lcbStwUser &&
           lhs.fcSttbttmbd == rhs.fcSttbttmbd &&
           lhs.lcbSttbttmbd == rhs.lcbSttbttmbd &&
           lhs.fcUnused == rhs.fcUnused &&
           lhs.lcbUnused == rhs.lcbUnused &&
           lhs.fcPgdMother == rhs.fcPgdMother &&
           lhs.lcbPgdMother == rhs.lcbPgdMother &&
           lhs.fcBkdMother == rhs.fcBkdMother &&
           lhs.lcbBkdMother == rhs.lcbBkdMother &&
           lhs.fcPgdFtn == rhs.fcPgdFtn &&
           lhs.lcbPgdFtn == rhs.lcbPgdFtn &&
           lhs.fcBkdFtn == rhs.fcBkdFtn &&
           lhs.lcbBkdFtn == rhs.lcbBkdFtn &&
           lhs.fcPgdEdn == rhs.fcPgdEdn &&
           lhs.lcbPgdEdn == rhs.lcbPgdEdn &&
           lhs.fcBkdEdn == rhs.fcBkdEdn &&
           lhs.lcbBkdEdn == rhs.lcbBkdEdn &&
           lhs.fcSttbfIntlFld == rhs.fcSttbfIntlFld &&
           lhs.lcbSttbfIntlFld == rhs.lcbSttbfIntlFld &&
           lhs.fcRouteSlip == rhs.fcRouteSlip &&
           lhs.lcbRouteSlip == rhs.lcbRouteSlip &&
           lhs.fcSttbSavedBy == rhs.fcSttbSavedBy &&
           lhs.lcbSttbSavedBy == rhs.lcbSttbSavedBy &&
           lhs.fcSttbFnm == rhs.fcSttbFnm &&
           lhs.lcbSttbFnm == rhs.lcbSttbFnm &&
           lhs.fcPlcfLst == rhs.fcPlcfLst &&
           lhs.lcbPlcfLst == rhs.lcbPlcfLst &&
           lhs.fcPlfLfo == rhs.fcPlfLfo &&
           lhs.lcbPlfLfo == rhs.lcbPlfLfo &&
           lhs.fcPlcftxbxBkd == rhs.fcPlcftxbxBkd &&
           lhs.lcbPlcftxbxBkd == rhs.lcbPlcftxbxBkd &&
           lhs.fcPlcftxbxHdrBkd == rhs.fcPlcftxbxHdrBkd &&
           lhs.lcbPlcftxbxHdrBkd == rhs.lcbPlcftxbxHdrBkd &&
           lhs.fcDocUndo == rhs.fcDocUndo &&
           lhs.lcbDocUndo == rhs.lcbDocUndo &&
           lhs.fcRgbuse == rhs.fcRgbuse &&
           lhs.lcbRgbuse == rhs.lcbRgbuse &&
           lhs.fcUsp == rhs.fcUsp &&
           lhs.lcbUsp == rhs.lcbUsp &&
           lhs.fcUskf == rhs.fcUskf &&
           lhs.lcbUskf == rhs.lcbUskf &&
           lhs.fcPlcupcRgbuse == rhs.fcPlcupcRgbuse &&
           lhs.lcbPlcupcRgbuse == rhs.lcbPlcupcRgbuse &&
           lhs.fcPlcupcUsp == rhs.fcPlcupcUsp &&
           lhs.lcbPlcupcUsp == rhs.lcbPlcupcUsp &&
           lhs.fcSttbGlsyStyle == rhs.fcSttbGlsyStyle &&
           lhs.lcbSttbGlsyStyle == rhs.lcbSttbGlsyStyle &&
           lhs.fcPlgosl == rhs.fcPlgosl &&
           lhs.lcbPlgosl == rhs.lcbPlgosl &&
           lhs.fcPlcocx == rhs.fcPlcocx &&
           lhs.lcbPlcocx == rhs.lcbPlcocx &&
           lhs.fcPlcfbteLvc == rhs.fcPlcfbteLvc &&
           lhs.lcbPlcfbteLvc == rhs.lcbPlcfbteLvc &&
           lhs.dwLowDateTime == rhs.dwLowDateTime &&
           lhs.dwHighDateTime == rhs.dwHighDateTime &&
           lhs.fcPlcflvc == rhs.fcPlcflvc &&
           lhs.lcbPlcflvc == rhs.lcbPlcflvc &&
           lhs.fcPlcasumy == rhs.fcPlcasumy &&
           lhs.lcbPlcasumy == rhs.lcbPlcasumy &&
           lhs.fcPlcfgram == rhs.fcPlcfgram &&
           lhs.lcbPlcfgram == rhs.lcbPlcfgram &&
           lhs.fcSttbListNames == rhs.fcSttbListNames &&
           lhs.lcbSttbListNames == rhs.lcbSttbListNames &&
           lhs.fcSttbfUssr == rhs.fcSttbfUssr &&
           lhs.lcbSttbfUssr == rhs.lcbSttbfUssr;
}

bool operator!=(const FIB &lhs, const FIB &rhs)
{
    return !(lhs == rhs);
}


// FIBFCLCB implementation

FIBFCLCB::FIBFCLCB()
{
    clear();
}

FIBFCLCB::FIBFCLCB(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

bool FIBFCLCB::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    if (preservePos)
        stream->push();

    fc = stream->readU32();
    lcb = stream->readU32();

    if (preservePos)
        stream->pop();
    return true;
}

void FIBFCLCB::clear()
{
    fc = 0;
    lcb = 0;
}

bool operator==(const FIBFCLCB &lhs, const FIBFCLCB &rhs)
{

    return lhs.fc == rhs.fc &&
           lhs.lcb == rhs.lcb;
}

bool operator!=(const FIBFCLCB &lhs, const FIBFCLCB &rhs)
{
    return !(lhs == rhs);
}


// FRD implementation

const unsigned int FRD::sizeOf = 2;

FRD::FRD()
{
    clear();
}

FRD::FRD(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

bool FRD::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    if (preservePos)
        stream->push();

    nAuto = stream->readS16();

    if (preservePos)
        stream->pop();
    return true;
}

void FRD::clear()
{
    nAuto = 0;
}

bool operator==(const FRD &lhs, const FRD &rhs)
{

    return lhs.nAuto == rhs.nAuto;
}

bool operator!=(const FRD &lhs, const FRD &rhs)
{
    return !(lhs == rhs);
}


// FSPA implementation

FSPA::FSPA()
{
    clear();
}

FSPA::FSPA(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

bool FSPA::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    U16 shifterU16;

    if (preservePos)
        stream->push();

    spid = stream->readS32();
    xaLeft = stream->readS32();
    yaTop = stream->readS32();
    xaRight = stream->readS32();
    yaBottom = stream->readS32();
    shifterU16 = stream->readU16();
    fHdr = shifterU16;
    shifterU16 >>= 1;
    bx = shifterU16;
    shifterU16 >>= 2;
    by = shifterU16;
    shifterU16 >>= 2;
    wr = shifterU16;
    shifterU16 >>= 4;
    wrk = shifterU16;
    shifterU16 >>= 4;
    fRcaSimple = shifterU16;
    shifterU16 >>= 1;
    fBelowText = shifterU16;
    shifterU16 >>= 1;
    fAnchorLock = shifterU16;
    cTxbx = stream->readS32();

    if (preservePos)
        stream->pop();
    return true;
}

void FSPA::clear()
{
    spid = 0;
    xaLeft = 0;
    yaTop = 0;
    xaRight = 0;
    yaBottom = 0;
    fHdr = 0;
    bx = 0;
    by = 0;
    wr = 0;
    wrk = 0;
    fRcaSimple = 0;
    fBelowText = 0;
    fAnchorLock = 0;
    cTxbx = 0;
}

bool operator==(const FSPA &lhs, const FSPA &rhs)
{

    return lhs.spid == rhs.spid &&
           lhs.xaLeft == rhs.xaLeft &&
           lhs.yaTop == rhs.yaTop &&
           lhs.xaRight == rhs.xaRight &&
           lhs.yaBottom == rhs.yaBottom &&
           lhs.fHdr == rhs.fHdr &&
           lhs.bx == rhs.bx &&
           lhs.by == rhs.by &&
           lhs.wr == rhs.wr &&
           lhs.wrk == rhs.wrk &&
           lhs.fRcaSimple == rhs.fRcaSimple &&
           lhs.fBelowText == rhs.fBelowText &&
           lhs.fAnchorLock == rhs.fAnchorLock &&
           lhs.cTxbx == rhs.cTxbx;
}

bool operator!=(const FSPA &lhs, const FSPA &rhs)
{
    return !(lhs == rhs);
}


// FTXBXS implementation

FTXBXS::FTXBXS()
{
    clear();
}

FTXBXS::FTXBXS(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

bool FTXBXS::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    if (preservePos)
        stream->push();

    cTxbx = stream->readS32();
    iNextReuse = stream->readS32();
    cReusable = stream->readS32();
    fReusable = stream->readS16();
    reserved = stream->readU32();
    lid = stream->readS32();
    txidUndo = stream->readS32();

    if (preservePos)
        stream->pop();
    return true;
}

void FTXBXS::clear()
{
    cTxbx = 0;
    iNextReuse = 0;
    cReusable = 0;
    fReusable = 0;
    reserved = 0;
    lid = 0;
    txidUndo = 0;
}

bool operator==(const FTXBXS &lhs, const FTXBXS &rhs)
{

    return lhs.cTxbx == rhs.cTxbx &&
           lhs.iNextReuse == rhs.iNextReuse &&
           lhs.cReusable == rhs.cReusable &&
           lhs.fReusable == rhs.fReusable &&
           lhs.reserved == rhs.reserved &&
           lhs.lid == rhs.lid &&
           lhs.txidUndo == rhs.txidUndo;
}

bool operator!=(const FTXBXS &lhs, const FTXBXS &rhs)
{
    return !(lhs == rhs);
}


// LFO implementation

const unsigned int LFO::sizeOf = 16;

LFO::LFO()
{
    clear();
}

LFO::LFO(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

bool LFO::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    if (preservePos)
        stream->push();

    lsid = stream->readS32();
    unused4 = stream->readS32();
    unused8 = stream->readS32();
    clfolvl = stream->readU8();
    for (int _i = 0; _i < (3); ++_i)
        reserved[_i] = stream->readU8();

    if (preservePos)
        stream->pop();
    return true;
}

void LFO::clear()
{
    lsid = 0;
    unused4 = 0;
    unused8 = 0;
    clfolvl = 0;
    for (int _i = 0; _i < (3); ++_i)
        reserved[_i] = 0;
}

bool operator==(const LFO &lhs, const LFO &rhs)
{

    for (int _i = 0; _i < (3); ++_i) {
        if (lhs.reserved[_i] != rhs.reserved[_i])
            return false;
    }

    return lhs.lsid == rhs.lsid &&
           lhs.unused4 == rhs.unused4 &&
           lhs.unused8 == rhs.unused8 &&
           lhs.clfolvl == rhs.clfolvl;
}

bool operator!=(const LFO &lhs, const LFO &rhs)
{
    return !(lhs == rhs);
}


// LFOLVL implementation

LFOLVL::LFOLVL()
{
    clear();
}

LFOLVL::LFOLVL(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

bool LFOLVL::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    U8 shifterU8;

    if (preservePos)
        stream->push();

    iStartAt = stream->readS32();
    shifterU8 = stream->readU8();
    ilvl = shifterU8;
    shifterU8 >>= 4;
    fStartAt = shifterU8;
    shifterU8 >>= 1;
    fFormatting = shifterU8;
    shifterU8 >>= 1;
    unsigned4_6 = shifterU8;
    for (int _i = 0; _i < (3); ++_i)
        reserved[_i] = stream->readU8();

    if (preservePos)
        stream->pop();
    return true;
}

void LFOLVL::clear()
{
    iStartAt = 0;
    ilvl = 0;
    fStartAt = 0;
    fFormatting = 0;
    unsigned4_6 = 0;
    for (int _i = 0; _i < (3); ++_i)
        reserved[_i] = 0;
}

bool operator==(const LFOLVL &lhs, const LFOLVL &rhs)
{

    for (int _i = 0; _i < (3); ++_i) {
        if (lhs.reserved[_i] != rhs.reserved[_i])
            return false;
    }

    return lhs.iStartAt == rhs.iStartAt &&
           lhs.ilvl == rhs.ilvl &&
           lhs.fStartAt == rhs.fStartAt &&
           lhs.fFormatting == rhs.fFormatting &&
           lhs.unsigned4_6 == rhs.unsigned4_6;
}

bool operator!=(const LFOLVL &lhs, const LFOLVL &rhs)
{
    return !(lhs == rhs);
}


// LSPD implementation

LSPD::LSPD()
{
    clear();
}

LSPD::LSPD(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

bool LSPD::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    if (preservePos)
        stream->push();

    dyaLine = stream->readS16();
    fMultLinespace = stream->readS16();

    if (preservePos)
        stream->pop();
    return true;
}

void LSPD::clear()
{
    dyaLine = 240;
    fMultLinespace = 1;
}

void LSPD::dump() const
{
    wvlog << "Dumping LSPD:" << std::endl;
    wvlog << toString().c_str() << std::endl;
    wvlog << "\nDumping LSPD done." << std::endl;
}

std::string LSPD::toString() const
{
    std::string s( "LSPD:" );
    s += "\ndyaLine=";
    s += int2string( dyaLine );
    s += "\nfMultLinespace=";
    s += int2string( fMultLinespace );
    s += "\nLSPD Done.";
    return s;
}

bool operator==(const LSPD &lhs, const LSPD &rhs)
{

    return lhs.dyaLine == rhs.dyaLine &&
           lhs.fMultLinespace == rhs.fMultLinespace;
}

bool operator!=(const LSPD &lhs, const LSPD &rhs)
{
    return !(lhs == rhs);
}


// LSTF implementation

const unsigned int LSTF::sizeOf = 28;

LSTF::LSTF()
{
    clear();
}

LSTF::LSTF(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

bool LSTF::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    U8 shifterU8;

    if (preservePos)
        stream->push();

    lsid = stream->readS32();
    tplc = stream->readS32();
    for (int _i = 0; _i < (9); ++_i)
        rgistd[_i] = stream->readU16();
    shifterU8 = stream->readU8();
    fSimpleList = shifterU8;
    shifterU8 >>= 1;
    fRestartHdn = shifterU8;
    shifterU8 >>= 1;
    unsigned26_2 = shifterU8;
    reserved = stream->readU8();

    if (preservePos)
        stream->pop();
    return true;
}

void LSTF::clear()
{
    lsid = 0;
    tplc = 0;
    for (int _i = 0; _i < (9); ++_i)
        rgistd[_i] = 0;
    fSimpleList = 0;
    fRestartHdn = 0;
    unsigned26_2 = 0;
    reserved = 0;
}

bool operator==(const LSTF &lhs, const LSTF &rhs)
{

    for (int _i = 0; _i < (9); ++_i) {
        if (lhs.rgistd[_i] != rhs.rgistd[_i])
            return false;
    }

    return lhs.lsid == rhs.lsid &&
           lhs.tplc == rhs.tplc &&
           lhs.fSimpleList == rhs.fSimpleList &&
           lhs.fRestartHdn == rhs.fRestartHdn &&
           lhs.unsigned26_2 == rhs.unsigned26_2 &&
           lhs.reserved == rhs.reserved;
}

bool operator!=(const LSTF &lhs, const LSTF &rhs)
{
    return !(lhs == rhs);
}


// LVLF implementation

LVLF::LVLF()
{
    clear();
}

LVLF::LVLF(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

bool LVLF::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    U8 shifterU8;

    if (preservePos)
        stream->push();

    iStartAt = stream->readU32();
    nfc = stream->readU8();
    shifterU8 = stream->readU8();
    jc = shifterU8;
    shifterU8 >>= 2;
    fLegal = shifterU8;
    shifterU8 >>= 1;
    fNoRestart = shifterU8;
    shifterU8 >>= 1;
    fPrev = shifterU8;
    shifterU8 >>= 1;
    fPrevSpace = shifterU8;
    shifterU8 >>= 1;
    fWord6 = shifterU8;
    shifterU8 >>= 1;
    unused5_7 = shifterU8;
    for (int _i = 0; _i < (9); ++_i)
        rgbxchNums[_i] = stream->readU8();
    ixchFollow = stream->readU8();
    dxaSpace = stream->readS32();
    dxaIndent = stream->readS32();
    cbGrpprlChpx = stream->readU8();
    cbGrpprlPapx = stream->readU8();
    reserved = stream->readU16();

    if (preservePos)
        stream->pop();
    return true;
}

void LVLF::clear()
{
    iStartAt = 0;
    nfc = 0;
    jc = 0;
    fLegal = 0;
    fNoRestart = 0;
    fPrev = 0;
    fPrevSpace = 0;
    fWord6 = 0;
    unused5_7 = 0;
    for (int _i = 0; _i < (9); ++_i)
        rgbxchNums[_i] = 0;
    ixchFollow = 0;
    dxaSpace = 0;
    dxaIndent = 0;
    cbGrpprlChpx = 0;
    cbGrpprlPapx = 0;
    reserved = 0;
}

bool operator==(const LVLF &lhs, const LVLF &rhs)
{

    for (int _i = 0; _i < (9); ++_i) {
        if (lhs.rgbxchNums[_i] != rhs.rgbxchNums[_i])
            return false;
    }

    return lhs.iStartAt == rhs.iStartAt &&
           lhs.nfc == rhs.nfc &&
           lhs.jc == rhs.jc &&
           lhs.fLegal == rhs.fLegal &&
           lhs.fNoRestart == rhs.fNoRestart &&
           lhs.fPrev == rhs.fPrev &&
           lhs.fPrevSpace == rhs.fPrevSpace &&
           lhs.fWord6 == rhs.fWord6 &&
           lhs.unused5_7 == rhs.unused5_7 &&
           lhs.ixchFollow == rhs.ixchFollow &&
           lhs.dxaSpace == rhs.dxaSpace &&
           lhs.dxaIndent == rhs.dxaIndent &&
           lhs.cbGrpprlChpx == rhs.cbGrpprlChpx &&
           lhs.cbGrpprlPapx == rhs.cbGrpprlPapx &&
           lhs.reserved == rhs.reserved;
}

bool operator!=(const LVLF &lhs, const LVLF &rhs)
{
    return !(lhs == rhs);
}


// METAFILEPICT implementation

METAFILEPICT::METAFILEPICT()
{
    clear();
}

METAFILEPICT::METAFILEPICT(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

bool METAFILEPICT::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    if (preservePos)
        stream->push();

    mm = stream->readS16();
    xExt = stream->readS16();
    yExt = stream->readS16();
    hMF = stream->readS16();

    if (preservePos)
        stream->pop();
    return true;
}

void METAFILEPICT::clear()
{
    mm = 0;
    xExt = 0;
    yExt = 0;
    hMF = 0;
}

bool operator==(const METAFILEPICT &lhs, const METAFILEPICT &rhs)
{

    return lhs.mm == rhs.mm &&
           lhs.xExt == rhs.xExt &&
           lhs.yExt == rhs.yExt &&
           lhs.hMF == rhs.hMF;
}

bool operator!=(const METAFILEPICT &lhs, const METAFILEPICT &rhs)
{
    return !(lhs == rhs);
}


// NUMRM implementation

NUMRM::NUMRM()
{
    clear();
}

NUMRM::NUMRM(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

NUMRM::NUMRM(const U8 *ptr)
{
    clear();
    readPtr(ptr);
}

bool NUMRM::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    if (preservePos)
        stream->push();

    fNumRM = stream->readU8();
    unused1 = stream->readU8();
    ibstNumRM = stream->readS16();
    dttmNumRM.read(stream, false);
    for (int _i = 0; _i < (9); ++_i)
        rgbxchNums[_i] = stream->readU8();
    for (int _i = 0; _i < (9); ++_i)
        rgnfc[_i] = stream->readU8();
    unused26 = stream->readS16();
    for (int _i = 0; _i < (9); ++_i)
        PNBR[_i] = stream->readU32();
    for (int _i = 0; _i < (32); ++_i)
        xst[_i] = stream->readU16();

    if (preservePos)
        stream->pop();
    return true;
}

void NUMRM::readPtr(const U8 *ptr)
{

    fNumRM = readU8(ptr);
    ptr += sizeof(U8);
    unused1 = readU8(ptr);
    ptr += sizeof(U8);
    ibstNumRM = readS16(ptr);
    ptr += sizeof(S16);
    dttmNumRM.readPtr(ptr);
    ptr += DTTM::sizeOf;
    for (int _i = 0; _i < (9); ++_i) {
        rgbxchNums[_i] = readU8(ptr);
        ptr += sizeof(U8);
    }
    for (int _i = 0; _i < (9); ++_i) {
        rgnfc[_i] = readU8(ptr);
        ptr += sizeof(U8);
    }
    unused26 = readS16(ptr);
    ptr += sizeof(S16);
    for (int _i = 0; _i < (9); ++_i) {
        PNBR[_i] = readU32(ptr);
        ptr += sizeof(U32);
    }
    for (int _i = 0; _i < (32); ++_i) {
        xst[_i] = readU16(ptr);
        ptr += sizeof(U16);
    }
}

void NUMRM::clear()
{
    fNumRM = 0;
    unused1 = 0;
    ibstNumRM = 0;
    dttmNumRM.clear();
    for (int _i = 0; _i < (9); ++_i)
        rgbxchNums[_i] = 0;
    for (int _i = 0; _i < (9); ++_i)
        rgnfc[_i] = 0;
    unused26 = 0;
    for (int _i = 0; _i < (9); ++_i)
        PNBR[_i] = 0;
    for (int _i = 0; _i < (32); ++_i)
        xst[_i] = 0;
}

void NUMRM::dump() const
{
    wvlog << "Dumping NUMRM:" << std::endl;
    wvlog << toString().c_str() << std::endl;
    wvlog << "\nDumping NUMRM done." << std::endl;
}

std::string NUMRM::toString() const
{
    std::string s( "NUMRM:" );
    s += "\nfNumRM=";
    s += uint2string( fNumRM );
    s += "\nunused1=";
    s += uint2string( unused1 );
    s += "\nibstNumRM=";
    s += int2string( ibstNumRM );
    s += "\ndttmNumRM=";
    s += "\n{" + dttmNumRM.toString() + "}\n";
    for (int _i = 0; _i < (9); ++_i) {
        s += "\nrgbxchNums[" + int2string( _i ) + "]=";
        s += uint2string( rgbxchNums[_i] );
    }
    for (int _i = 0; _i < (9); ++_i) {
        s += "\nrgnfc[" + int2string( _i ) + "]=";
        s += uint2string( rgnfc[_i] );
    }
    s += "\nunused26=";
    s += int2string( unused26 );
    for (int _i = 0; _i < (9); ++_i) {
        s += "\nPNBR[" + int2string( _i ) + "]=";
        s += uint2string( PNBR[_i] );
    }
    for (int _i = 0; _i < (32); ++_i) {
        s += "\nxst[" + int2string( _i ) + "]=";
        s += uint2string( xst[_i] );
    }
    s += "\nNUMRM Done.";
    return s;
}

bool operator==(const NUMRM &lhs, const NUMRM &rhs)
{

    for (int _i = 0; _i < (9); ++_i) {
        if (lhs.rgbxchNums[_i] != rhs.rgbxchNums[_i])
            return false;
    }

    for (int _i = 0; _i < (9); ++_i) {
        if (lhs.rgnfc[_i] != rhs.rgnfc[_i])
            return false;
    }

    for (int _i = 0; _i < (9); ++_i) {
        if (lhs.PNBR[_i] != rhs.PNBR[_i])
            return false;
    }

    for (int _i = 0; _i < (32); ++_i) {
        if (lhs.xst[_i] != rhs.xst[_i])
            return false;
    }

    return lhs.fNumRM == rhs.fNumRM &&
           lhs.unused1 == rhs.unused1 &&
           lhs.ibstNumRM == rhs.ibstNumRM &&
           lhs.dttmNumRM == rhs.dttmNumRM &&
           lhs.unused26 == rhs.unused26;
}

bool operator!=(const NUMRM &lhs, const NUMRM &rhs)
{
    return !(lhs == rhs);
}


// OBJHEADER implementation

OBJHEADER::OBJHEADER()
{
    clear();
}

OBJHEADER::OBJHEADER(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

bool OBJHEADER::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    if (preservePos)
        stream->push();

    lcb = stream->readU32();
    cbHeader = stream->readU16();
    icf = stream->readU16();

    if (preservePos)
        stream->pop();
    return true;
}

void OBJHEADER::clear()
{
    lcb = 0;
    cbHeader = 0;
    icf = 0;
}

bool operator==(const OBJHEADER &lhs, const OBJHEADER &rhs)
{

    return lhs.lcb == rhs.lcb &&
           lhs.cbHeader == rhs.cbHeader &&
           lhs.icf == rhs.icf;
}

bool operator!=(const OBJHEADER &lhs, const OBJHEADER &rhs)
{
    return !(lhs == rhs);
}


// OLST implementation

const unsigned int OLST::sizeOf = 212;

OLST::OLST()
{
    clear();
}

OLST::OLST(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

OLST::OLST(const U8 *ptr)
{
    clear();
    readPtr(ptr);
}

bool OLST::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    if (preservePos)
        stream->push();

    for (int _i = 0; _i < (9); ++_i)
        rganlv[_i].read(stream, false);
    fRestartHdr = stream->readU8();
    fSpareOlst2 = stream->readU8();
    fSpareOlst3 = stream->readU8();
    fSpareOlst4 = stream->readU8();
    for (int _i = 0; _i < (32); ++_i)
        rgxch[_i] = stream->readU16();

    if (preservePos)
        stream->pop();
    return true;
}

void OLST::readPtr(const U8 *ptr)
{

    for (int _i = 0; _i < (9); ++_i) {
        rganlv[_i].readPtr(ptr);
        ptr += ANLV::sizeOf;
    }
    fRestartHdr = readU8(ptr);
    ptr += sizeof(U8);
    fSpareOlst2 = readU8(ptr);
    ptr += sizeof(U8);
    fSpareOlst3 = readU8(ptr);
    ptr += sizeof(U8);
    fSpareOlst4 = readU8(ptr);
    ptr += sizeof(U8);
    for (int _i = 0; _i < (32); ++_i) {
        rgxch[_i] = readU16(ptr);
        ptr += sizeof(U16);
    }
}

void OLST::clear()
{
    for (int _i = 0; _i < (9); ++_i)
        rganlv[_i].clear();
    fRestartHdr = 0;
    fSpareOlst2 = 0;
    fSpareOlst3 = 0;
    fSpareOlst4 = 0;
    for (int _i = 0; _i < (32); ++_i)
        rgxch[_i] = 0;
}

void OLST::dump() const
{
    wvlog << "Dumping OLST:" << std::endl;
    wvlog << toString().c_str() << std::endl;
    wvlog << "\nDumping OLST done." << std::endl;
}

std::string OLST::toString() const
{
    std::string s( "OLST:" );
    for (int _i = 0; _i < (9); ++_i) {
        s += "\nrganlv[" + int2string( _i ) + "]=";
        s += "\n{" + rganlv[_i].toString() + "}\n";
    }
    s += "\nfRestartHdr=";
    s += uint2string( fRestartHdr );
    s += "\nfSpareOlst2=";
    s += uint2string( fSpareOlst2 );
    s += "\nfSpareOlst3=";
    s += uint2string( fSpareOlst3 );
    s += "\nfSpareOlst4=";
    s += uint2string( fSpareOlst4 );
    for (int _i = 0; _i < (32); ++_i) {
        s += "\nrgxch[" + int2string( _i ) + "]=";
        s += uint2string( rgxch[_i] );
    }
    s += "\nOLST Done.";
    return s;
}

bool operator==(const OLST &lhs, const OLST &rhs)
{

    for (int _i = 0; _i < (9); ++_i) {
        if (lhs.rganlv[_i] != rhs.rganlv[_i])
            return false;
    }

    for (int _i = 0; _i < (32); ++_i) {
        if (lhs.rgxch[_i] != rhs.rgxch[_i])
            return false;
    }

    return lhs.fRestartHdr == rhs.fRestartHdr &&
           lhs.fSpareOlst2 == rhs.fSpareOlst2 &&
           lhs.fSpareOlst3 == rhs.fSpareOlst3 &&
           lhs.fSpareOlst4 == rhs.fSpareOlst4;
}

bool operator!=(const OLST &lhs, const OLST &rhs)
{
    return !(lhs == rhs);
}


// PAP implementation

PAP::PAP() : Shared()
{
    clear();
}

PAP::PAP(AbstractOLEStreamReader *stream, bool preservePos) : Shared()
{
    clear();
    read(stream, preservePos);
}

bool PAP::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    U8 shifterU8;
    U16 shifterU16;

    if (preservePos)
        stream->push();

    istd = stream->readU16();
    jc = stream->readU8();
    fKeep = stream->readU8();
    fKeepFollow = stream->readU8();
    fPageBreakBefore = stream->readU8();
    shifterU8 = stream->readU8();
    fBrLnAbove = shifterU8;
    shifterU8 >>= 1;
    fBrLnBelow = shifterU8;
    shifterU8 >>= 1;
    fUnused = shifterU8;
    shifterU8 >>= 2;
    pcVert = shifterU8;
    shifterU8 >>= 2;
    pcHorz = shifterU8;
    brcp = stream->readU8();
    brcl = stream->readU8();
    unused9 = stream->readU8();
    ilvl = stream->readU8();
    fNoLnn = stream->readU8();
    ilfo = stream->readS16();
    nLvlAnm = stream->readU8();
    unused15 = stream->readU8();
    fSideBySide = stream->readU8();
    unused17 = stream->readU8();
    fNoAutoHyph = stream->readU8();
    fWidowControl = stream->readU8();
    dxaRight = stream->readS32();
    dxaLeft = stream->readS32();
    dxaLeft1 = stream->readS32();
    lspd.read(stream, false);
    dyaBefore = stream->readU32();
    dyaAfter = stream->readU32();
    phe.read(stream, false);
    fCrLf = stream->readU8();
    fUsePgsuSettings = stream->readU8();
    fAdjustRight = stream->readU8();
    unused59 = stream->readU8();
    fKinsoku = stream->readU8();
    fWordWrap = stream->readU8();
    fOverflowPunct = stream->readU8();
    fTopLinePunct = stream->readU8();
    fAutoSpaceDE = stream->readU8();
    fAutoSpaceDN = stream->readU8();
    wAlignFont = stream->readU16();
    shifterU16 = stream->readU16();
    fVertical = shifterU16;
    shifterU16 >>= 1;
    fBackward = shifterU16;
    shifterU16 >>= 1;
    fRotateFont = shifterU16;
    shifterU16 >>= 1;
    unused68_3 = shifterU16;
    unused70 = stream->readU16();
    fInTable = stream->readS8();
    fTtp = stream->readS8();
    wr = stream->readU8();
    fLocked = stream->readU8();
    ptap = stream->readU32();
    dxaAbs = stream->readS32();
    dyaAbs = stream->readS32();
    dxaWidth = stream->readS32();
    brcTop.read(stream, false);
    brcLeft.read(stream, false);
    brcBottom.read(stream, false);
    brcRight.read(stream, false);
    brcBetween.read(stream, false);
    brcBar.read(stream, false);
    dxaFromText = stream->readS32();
    dyaFromText = stream->readS32();
    shifterU16 = stream->readU16();
    dyaHeight = shifterU16;
    shifterU16 >>= 15;
    fMinHeight = shifterU16;
    shd.read(stream, false);
    dcs.read(stream, false);
    lvl = stream->readS8();
    fNumRMIns = stream->readS8();
    anld.read(stream, false);
    fPropRMark = stream->readS16();
    ibstPropRMark = stream->readS16();
    dttmPropRMark.read(stream, false);
    numrm.read(stream, false);
    itbdMac = stream->readS16();
    // skipping the std::vector rgdxaTab

    if (preservePos)
        stream->pop();
    return true;
}

void PAP::clear()
{
    istd = 0;
    jc = 0;
    fKeep = 0;
    fKeepFollow = 0;
    fPageBreakBefore = 0;
    fBrLnAbove = 0;
    fBrLnBelow = 0;
    fUnused = 0;
    pcVert = 0;
    pcHorz = 0;
    brcp = 0;
    brcl = 0;
    unused9 = 0;
    ilvl = 0;
    fNoLnn = 0;
    ilfo = 0;
    nLvlAnm = 0;
    unused15 = 0;
    fSideBySide = 0;
    unused17 = 0;
    fNoAutoHyph = 0;
    fWidowControl = 1;
    dxaRight = 0;
    dxaLeft = 0;
    dxaLeft1 = 0;
    lspd.clear();
    dyaBefore = 0;
    dyaAfter = 0;
    phe.clear();
    fCrLf = 0;
    fUsePgsuSettings = 0;
    fAdjustRight = 0;
    unused59 = 0;
    fKinsoku = 0;
    fWordWrap = 0;
    fOverflowPunct = 0;
    fTopLinePunct = 0;
    fAutoSpaceDE = 0;
    fAutoSpaceDN = 0;
    wAlignFont = 0;
    fVertical = 0;
    fBackward = 0;
    fRotateFont = 0;
    unused68_3 = 0;
    unused70 = 0;
    fInTable = 0;
    fTtp = 0;
    wr = 0;
    fLocked = 0;
    ptap = 0;
    dxaAbs = 0;
    dyaAbs = 0;
    dxaWidth = 0;
    brcTop.clear();
    brcLeft.clear();
    brcBottom.clear();
    brcRight.clear();
    brcBetween.clear();
    brcBar.clear();
    dxaFromText = 0;
    dyaFromText = 0;
    dyaHeight = 0;
    fMinHeight = 0;
    shd.clear();
    dcs.clear();
    lvl = 9;
    fNumRMIns = 0;
    anld.clear();
    fPropRMark = 0;
    ibstPropRMark = 0;
    dttmPropRMark.clear();
    numrm.clear();
    itbdMac = 0;
    rgdxaTab.clear();
}

void PAP::dump() const
{
    wvlog << "Dumping PAP:" << std::endl;
    wvlog << toString().c_str() << std::endl;
    wvlog << "\nDumping PAP done." << std::endl;
}

std::string PAP::toString() const
{
    std::string s( "PAP:" );
    s += "\nistd=";
    s += uint2string( istd );
    s += "\njc=";
    s += uint2string( jc );
    s += "\nfKeep=";
    s += uint2string( fKeep );
    s += "\nfKeepFollow=";
    s += uint2string( fKeepFollow );
    s += "\nfPageBreakBefore=";
    s += uint2string( fPageBreakBefore );
    s += "\nfBrLnAbove=";
    s += uint2string( fBrLnAbove );
    s += "\nfBrLnBelow=";
    s += uint2string( fBrLnBelow );
    s += "\nfUnused=";
    s += uint2string( fUnused );
    s += "\npcVert=";
    s += uint2string( pcVert );
    s += "\npcHorz=";
    s += uint2string( pcHorz );
    s += "\nbrcp=";
    s += uint2string( brcp );
    s += "\nbrcl=";
    s += uint2string( brcl );
    s += "\nunused9=";
    s += uint2string( unused9 );
    s += "\nilvl=";
    s += uint2string( ilvl );
    s += "\nfNoLnn=";
    s += uint2string( fNoLnn );
    s += "\nilfo=";
    s += int2string( ilfo );
    s += "\nnLvlAnm=";
    s += uint2string( nLvlAnm );
    s += "\nunused15=";
    s += uint2string( unused15 );
    s += "\nfSideBySide=";
    s += uint2string( fSideBySide );
    s += "\nunused17=";
    s += uint2string( unused17 );
    s += "\nfNoAutoHyph=";
    s += uint2string( fNoAutoHyph );
    s += "\nfWidowControl=";
    s += uint2string( fWidowControl );
    s += "\ndxaRight=";
    s += int2string( dxaRight );
    s += "\ndxaLeft=";
    s += int2string( dxaLeft );
    s += "\ndxaLeft1=";
    s += int2string( dxaLeft1 );
    s += "\nlspd=";
    s += "\n{" + lspd.toString() + "}\n";
    s += "\ndyaBefore=";
    s += uint2string( dyaBefore );
    s += "\ndyaAfter=";
    s += uint2string( dyaAfter );
    s += "\nphe=";
    s += "\n{" + phe.toString() + "}\n";
    s += "\nfCrLf=";
    s += uint2string( fCrLf );
    s += "\nfUsePgsuSettings=";
    s += uint2string( fUsePgsuSettings );
    s += "\nfAdjustRight=";
    s += uint2string( fAdjustRight );
    s += "\nunused59=";
    s += uint2string( unused59 );
    s += "\nfKinsoku=";
    s += uint2string( fKinsoku );
    s += "\nfWordWrap=";
    s += uint2string( fWordWrap );
    s += "\nfOverflowPunct=";
    s += uint2string( fOverflowPunct );
    s += "\nfTopLinePunct=";
    s += uint2string( fTopLinePunct );
    s += "\nfAutoSpaceDE=";
    s += uint2string( fAutoSpaceDE );
    s += "\nfAutoSpaceDN=";
    s += uint2string( fAutoSpaceDN );
    s += "\nwAlignFont=";
    s += uint2string( wAlignFont );
    s += "\nfVertical=";
    s += uint2string( fVertical );
    s += "\nfBackward=";
    s += uint2string( fBackward );
    s += "\nfRotateFont=";
    s += uint2string( fRotateFont );
    s += "\nunused68_3=";
    s += uint2string( unused68_3 );
    s += "\nunused70=";
    s += uint2string( unused70 );
    s += "\nfInTable=";
    s += int2string( fInTable );
    s += "\nfTtp=";
    s += int2string( fTtp );
    s += "\nwr=";
    s += uint2string( wr );
    s += "\nfLocked=";
    s += uint2string( fLocked );
    s += "\nptap=";
    s += uint2string( ptap );
    s += "\ndxaAbs=";
    s += int2string( dxaAbs );
    s += "\ndyaAbs=";
    s += int2string( dyaAbs );
    s += "\ndxaWidth=";
    s += int2string( dxaWidth );
    s += "\nbrcTop=";
    s += "\n{" + brcTop.toString() + "}\n";
    s += "\nbrcLeft=";
    s += "\n{" + brcLeft.toString() + "}\n";
    s += "\nbrcBottom=";
    s += "\n{" + brcBottom.toString() + "}\n";
    s += "\nbrcRight=";
    s += "\n{" + brcRight.toString() + "}\n";
    s += "\nbrcBetween=";
    s += "\n{" + brcBetween.toString() + "}\n";
    s += "\nbrcBar=";
    s += "\n{" + brcBar.toString() + "}\n";
    s += "\ndxaFromText=";
    s += int2string( dxaFromText );
    s += "\ndyaFromText=";
    s += int2string( dyaFromText );
    s += "\ndyaHeight=";
    s += uint2string( dyaHeight );
    s += "\nfMinHeight=";
    s += uint2string( fMinHeight );
    s += "\nshd=";
    s += "\n{" + shd.toString() + "}\n";
    s += "\ndcs=";
    s += "\n{" + dcs.toString() + "}\n";
    s += "\nlvl=";
    s += int2string( lvl );
    s += "\nfNumRMIns=";
    s += int2string( fNumRMIns );
    s += "\nanld=";
    s += "\n{" + anld.toString() + "}\n";
    s += "\nfPropRMark=";
    s += int2string( fPropRMark );
    s += "\nibstPropRMark=";
    s += int2string( ibstPropRMark );
    s += "\ndttmPropRMark=";
    s += "\n{" + dttmPropRMark.toString() + "}\n";
    s += "\nnumrm=";
    s += "\n{" + numrm.toString() + "}\n";
    s += "\nitbdMac=";
    s += int2string( itbdMac );
    s += "\nrgdxaTab=";
    // skipping the std::vector rgdxaTab
    s += "\nPAP Done.";
    return s;
}

bool operator==(const PAP &lhs, const PAP &rhs)
{

    return lhs.istd == rhs.istd &&
           lhs.jc == rhs.jc &&
           lhs.fKeep == rhs.fKeep &&
           lhs.fKeepFollow == rhs.fKeepFollow &&
           lhs.fPageBreakBefore == rhs.fPageBreakBefore &&
           lhs.fBrLnAbove == rhs.fBrLnAbove &&
           lhs.fBrLnBelow == rhs.fBrLnBelow &&
           lhs.fUnused == rhs.fUnused &&
           lhs.pcVert == rhs.pcVert &&
           lhs.pcHorz == rhs.pcHorz &&
           lhs.brcp == rhs.brcp &&
           lhs.brcl == rhs.brcl &&
           lhs.unused9 == rhs.unused9 &&
           lhs.ilvl == rhs.ilvl &&
           lhs.fNoLnn == rhs.fNoLnn &&
           lhs.ilfo == rhs.ilfo &&
           lhs.nLvlAnm == rhs.nLvlAnm &&
           lhs.unused15 == rhs.unused15 &&
           lhs.fSideBySide == rhs.fSideBySide &&
           lhs.unused17 == rhs.unused17 &&
           lhs.fNoAutoHyph == rhs.fNoAutoHyph &&
           lhs.fWidowControl == rhs.fWidowControl &&
           lhs.dxaRight == rhs.dxaRight &&
           lhs.dxaLeft == rhs.dxaLeft &&
           lhs.dxaLeft1 == rhs.dxaLeft1 &&
           lhs.lspd == rhs.lspd &&
           lhs.dyaBefore == rhs.dyaBefore &&
           lhs.dyaAfter == rhs.dyaAfter &&
           lhs.phe == rhs.phe &&
           lhs.fCrLf == rhs.fCrLf &&
           lhs.fUsePgsuSettings == rhs.fUsePgsuSettings &&
           lhs.fAdjustRight == rhs.fAdjustRight &&
           lhs.unused59 == rhs.unused59 &&
           lhs.fKinsoku == rhs.fKinsoku &&
           lhs.fWordWrap == rhs.fWordWrap &&
           lhs.fOverflowPunct == rhs.fOverflowPunct &&
           lhs.fTopLinePunct == rhs.fTopLinePunct &&
           lhs.fAutoSpaceDE == rhs.fAutoSpaceDE &&
           lhs.fAutoSpaceDN == rhs.fAutoSpaceDN &&
           lhs.wAlignFont == rhs.wAlignFont &&
           lhs.fVertical == rhs.fVertical &&
           lhs.fBackward == rhs.fBackward &&
           lhs.fRotateFont == rhs.fRotateFont &&
           lhs.unused68_3 == rhs.unused68_3 &&
           lhs.unused70 == rhs.unused70 &&
           lhs.fInTable == rhs.fInTable &&
           lhs.fTtp == rhs.fTtp &&
           lhs.wr == rhs.wr &&
           lhs.fLocked == rhs.fLocked &&
           lhs.ptap == rhs.ptap &&
           lhs.dxaAbs == rhs.dxaAbs &&
           lhs.dyaAbs == rhs.dyaAbs &&
           lhs.dxaWidth == rhs.dxaWidth &&
           lhs.brcTop == rhs.brcTop &&
           lhs.brcLeft == rhs.brcLeft &&
           lhs.brcBottom == rhs.brcBottom &&
           lhs.brcRight == rhs.brcRight &&
           lhs.brcBetween == rhs.brcBetween &&
           lhs.brcBar == rhs.brcBar &&
           lhs.dxaFromText == rhs.dxaFromText &&
           lhs.dyaFromText == rhs.dyaFromText &&
           lhs.dyaHeight == rhs.dyaHeight &&
           lhs.fMinHeight == rhs.fMinHeight &&
           lhs.shd == rhs.shd &&
           lhs.dcs == rhs.dcs &&
           lhs.lvl == rhs.lvl &&
           lhs.fNumRMIns == rhs.fNumRMIns &&
           lhs.anld == rhs.anld &&
           lhs.fPropRMark == rhs.fPropRMark &&
           lhs.ibstPropRMark == rhs.ibstPropRMark &&
           lhs.dttmPropRMark == rhs.dttmPropRMark &&
           lhs.numrm == rhs.numrm &&
           lhs.itbdMac == rhs.itbdMac &&
           lhs.rgdxaTab == rhs.rgdxaTab;
}

bool operator!=(const PAP &lhs, const PAP &rhs)
{
    return !(lhs == rhs);
}


// PCD implementation

const unsigned int PCD::sizeOf = 8;

PCD::PCD()
{
    clear();
}

PCD::PCD(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

PCD::PCD(const U8 *ptr)
{
    clear();
    readPtr(ptr);
}

bool PCD::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    U16 shifterU16;

    if (preservePos)
        stream->push();

    shifterU16 = stream->readU16();
    fNoParaLast = shifterU16;
    shifterU16 >>= 1;
    fPaphNil = shifterU16;
    shifterU16 >>= 1;
    fCopied = shifterU16;
    shifterU16 >>= 1;
    unused0_3 = shifterU16;
    shifterU16 >>= 5;
    fn = shifterU16;
    fc = stream->readU32();
    prm.read(stream, false);

    if (preservePos)
        stream->pop();
    return true;
}

void PCD::readPtr(const U8 *ptr)
{

    U16 shifterU16;

    shifterU16 = readU16(ptr);
    ptr += sizeof(U16);
    fNoParaLast = shifterU16;
    shifterU16 >>= 1;
    fPaphNil = shifterU16;
    shifterU16 >>= 1;
    fCopied = shifterU16;
    shifterU16 >>= 1;
    unused0_3 = shifterU16;
    shifterU16 >>= 5;
    fn = shifterU16;
    fc = readU32(ptr);
    ptr += sizeof(U32);
    prm.readPtr(ptr);
    ptr += PRM::sizeOf;
}

void PCD::clear()
{
    fNoParaLast = 0;
    fPaphNil = 0;
    fCopied = 0;
    unused0_3 = 0;
    fn = 0;
    fc = 0;
    prm.clear();
}

bool operator==(const PCD &lhs, const PCD &rhs)
{

    return lhs.fNoParaLast == rhs.fNoParaLast &&
           lhs.fPaphNil == rhs.fPaphNil &&
           lhs.fCopied == rhs.fCopied &&
           lhs.unused0_3 == rhs.unused0_3 &&
           lhs.fn == rhs.fn &&
           lhs.fc == rhs.fc &&
           lhs.prm == rhs.prm;
}

bool operator!=(const PCD &lhs, const PCD &rhs)
{
    return !(lhs == rhs);
}


// PGD implementation

PGD::PGD()
{
    clear();
}

PGD::PGD(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

bool PGD::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    U16 shifterU16;

    if (preservePos)
        stream->push();

    shifterU16 = stream->readU16();
    fContinue = shifterU16;
    shifterU16 >>= 1;
    fUnk = shifterU16;
    shifterU16 >>= 1;
    fRight = shifterU16;
    shifterU16 >>= 1;
    fPgnRestart = shifterU16;
    shifterU16 >>= 1;
    fEmptyPage = shifterU16;
    shifterU16 >>= 1;
    fAllFtn = shifterU16;
    shifterU16 >>= 1;
    unused0_6 = shifterU16;
    shifterU16 >>= 1;
    fTableBreaks = shifterU16;
    shifterU16 >>= 1;
    fMarked = shifterU16;
    shifterU16 >>= 1;
    fColumnBreaks = shifterU16;
    shifterU16 >>= 1;
    fTableHeader = shifterU16;
    shifterU16 >>= 1;
    fNewPage = shifterU16;
    shifterU16 >>= 1;
    bkc = shifterU16;
    lnn = stream->readU16();
    pgn = stream->readU16();
    dym = stream->readS32();

    if (preservePos)
        stream->pop();
    return true;
}

void PGD::clear()
{
    fContinue = 0;
    fUnk = 0;
    fRight = 0;
    fPgnRestart = 0;
    fEmptyPage = 0;
    fAllFtn = 0;
    unused0_6 = 0;
    fTableBreaks = 0;
    fMarked = 0;
    fColumnBreaks = 0;
    fTableHeader = 0;
    fNewPage = 0;
    bkc = 0;
    lnn = 0;
    pgn = 0;
    dym = 0;
}

bool operator==(const PGD &lhs, const PGD &rhs)
{

    return lhs.fContinue == rhs.fContinue &&
           lhs.fUnk == rhs.fUnk &&
           lhs.fRight == rhs.fRight &&
           lhs.fPgnRestart == rhs.fPgnRestart &&
           lhs.fEmptyPage == rhs.fEmptyPage &&
           lhs.fAllFtn == rhs.fAllFtn &&
           lhs.unused0_6 == rhs.unused0_6 &&
           lhs.fTableBreaks == rhs.fTableBreaks &&
           lhs.fMarked == rhs.fMarked &&
           lhs.fColumnBreaks == rhs.fColumnBreaks &&
           lhs.fTableHeader == rhs.fTableHeader &&
           lhs.fNewPage == rhs.fNewPage &&
           lhs.bkc == rhs.bkc &&
           lhs.lnn == rhs.lnn &&
           lhs.pgn == rhs.pgn &&
           lhs.dym == rhs.dym;
}

bool operator!=(const PGD &lhs, const PGD &rhs)
{
    return !(lhs == rhs);
}


// PHE2 implementation

PHE2::PHE2()
{
    clear();
}

PHE2::PHE2(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

bool PHE2::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    U32 shifterU32;

    if (preservePos)
        stream->push();

    shifterU32 = stream->readU32();
    fSpare = shifterU32;
    shifterU32 >>= 1;
    fUnk = shifterU32;
    shifterU32 >>= 1;
    dcpTtpNext = shifterU32;
    dxaCol = stream->readS32();
    dymTableHeight = stream->readS32();

    if (preservePos)
        stream->pop();
    return true;
}

void PHE2::clear()
{
    fSpare = 0;
    fUnk = 0;
    dcpTtpNext = 0;
    dxaCol = 0;
    dymTableHeight = 0;
}

bool operator==(const PHE2 &lhs, const PHE2 &rhs)
{

    return lhs.fSpare == rhs.fSpare &&
           lhs.fUnk == rhs.fUnk &&
           lhs.dcpTtpNext == rhs.dcpTtpNext &&
           lhs.dxaCol == rhs.dxaCol &&
           lhs.dymTableHeight == rhs.dymTableHeight;
}

bool operator!=(const PHE2 &lhs, const PHE2 &rhs)
{
    return !(lhs == rhs);
}


// PICF implementation

PICF::PICF() : Shared()
{
    clear();
}

PICF::PICF(AbstractOLEStreamReader *stream, bool preservePos) : Shared()
{
    clear();
    read(stream, preservePos);
}

bool PICF::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    U16 shifterU16;

    if (preservePos)
        stream->push();

    lcb = stream->readU32();
    cbHeader = stream->readU16();
    mfp.read(stream, false);
    for (int _i = 0; _i < (14); ++_i)
        bm_rcWinMF[_i] = stream->readU8();
    dxaGoal = stream->readS16();
    dyaGoal = stream->readS16();
    mx = stream->readU16();
    my = stream->readU16();
    dxaCropLeft = stream->readS16();
    dyaCropTop = stream->readS16();
    dxaCropRight = stream->readS16();
    dyaCropBottom = stream->readS16();
    shifterU16 = stream->readU16();
    brcl = shifterU16;
    shifterU16 >>= 4;
    fFrameEmpty = shifterU16;
    shifterU16 >>= 1;
    fBitmap = shifterU16;
    shifterU16 >>= 1;
    fDrawHatch = shifterU16;
    shifterU16 >>= 1;
    fError = shifterU16;
    shifterU16 >>= 1;
    bpp = shifterU16;
    brcTop.read(stream, false);
    brcLeft.read(stream, false);
    brcBottom.read(stream, false);
    brcRight.read(stream, false);
    dxaOrigin = stream->readS16();
    dyaOrigin = stream->readS16();
    cProps = stream->readS16();

    if (preservePos)
        stream->pop();
    return true;
}

void PICF::clear()
{
    lcb = 0;
    cbHeader = 0;
    mfp.clear();
    for (int _i = 0; _i < (14); ++_i)
        bm_rcWinMF[_i] = 0;
    dxaGoal = 0;
    dyaGoal = 0;
    mx = 0;
    my = 0;
    dxaCropLeft = 0;
    dyaCropTop = 0;
    dxaCropRight = 0;
    dyaCropBottom = 0;
    brcl = 0;
    fFrameEmpty = 0;
    fBitmap = 0;
    fDrawHatch = 0;
    fError = 0;
    bpp = 0;
    brcTop.clear();
    brcLeft.clear();
    brcBottom.clear();
    brcRight.clear();
    dxaOrigin = 0;
    dyaOrigin = 0;
    cProps = 0;
}

bool operator==(const PICF &lhs, const PICF &rhs)
{

    for (int _i = 0; _i < (14); ++_i) {
        if (lhs.bm_rcWinMF[_i] != rhs.bm_rcWinMF[_i])
            return false;
    }

    return lhs.lcb == rhs.lcb &&
           lhs.cbHeader == rhs.cbHeader &&
           lhs.mfp == rhs.mfp &&
           lhs.dxaGoal == rhs.dxaGoal &&
           lhs.dyaGoal == rhs.dyaGoal &&
           lhs.mx == rhs.mx &&
           lhs.my == rhs.my &&
           lhs.dxaCropLeft == rhs.dxaCropLeft &&
           lhs.dyaCropTop == rhs.dyaCropTop &&
           lhs.dxaCropRight == rhs.dxaCropRight &&
           lhs.dyaCropBottom == rhs.dyaCropBottom &&
           lhs.brcl == rhs.brcl &&
           lhs.fFrameEmpty == rhs.fFrameEmpty &&
           lhs.fBitmap == rhs.fBitmap &&
           lhs.fDrawHatch == rhs.fDrawHatch &&
           lhs.fError == rhs.fError &&
           lhs.bpp == rhs.bpp &&
           lhs.brcTop == rhs.brcTop &&
           lhs.brcLeft == rhs.brcLeft &&
           lhs.brcBottom == rhs.brcBottom &&
           lhs.brcRight == rhs.brcRight &&
           lhs.dxaOrigin == rhs.dxaOrigin &&
           lhs.dyaOrigin == rhs.dyaOrigin &&
           lhs.cProps == rhs.cProps;
}

bool operator!=(const PICF &lhs, const PICF &rhs)
{
    return !(lhs == rhs);
}


// RR implementation

RR::RR()
{
    clear();
}

RR::RR(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

bool RR::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    if (preservePos)
        stream->push();

    cb = stream->readU16();
    cbSzRecip = stream->readU16();

    if (preservePos)
        stream->pop();
    return true;
}

void RR::clear()
{
    cb = 0;
    cbSzRecip = 0;
}

bool operator==(const RR &lhs, const RR &rhs)
{

    return lhs.cb == rhs.cb &&
           lhs.cbSzRecip == rhs.cbSzRecip;
}

bool operator!=(const RR &lhs, const RR &rhs)
{
    return !(lhs == rhs);
}


// RS implementation

RS::RS()
{
    clear();
}

RS::RS(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

bool RS::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    if (preservePos)
        stream->push();

    fRouted = stream->readS16();
    fReturnOrig = stream->readS16();
    fTrackStatus = stream->readS16();
    fDirty = stream->readS16();
    nProtect = stream->readS16();
    iStage = stream->readS16();
    delOption = stream->readS16();
    cRecip = stream->readS16();

    if (preservePos)
        stream->pop();
    return true;
}

void RS::clear()
{
    fRouted = 0;
    fReturnOrig = 0;
    fTrackStatus = 0;
    fDirty = 0;
    nProtect = 0;
    iStage = 0;
    delOption = 0;
    cRecip = 0;
}

bool operator==(const RS &lhs, const RS &rhs)
{

    return lhs.fRouted == rhs.fRouted &&
           lhs.fReturnOrig == rhs.fReturnOrig &&
           lhs.fTrackStatus == rhs.fTrackStatus &&
           lhs.fDirty == rhs.fDirty &&
           lhs.nProtect == rhs.nProtect &&
           lhs.iStage == rhs.iStage &&
           lhs.delOption == rhs.delOption &&
           lhs.cRecip == rhs.cRecip;
}

bool operator!=(const RS &lhs, const RS &rhs)
{
    return !(lhs == rhs);
}


// SED implementation

const unsigned int SED::sizeOf = 12;

SED::SED()
{
    clear();
}

SED::SED(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

bool SED::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    if (preservePos)
        stream->push();

    fn = stream->readS16();
    fcSepx = stream->readU32();
    fnMpr = stream->readS16();
    fcMpr = stream->readU32();

    if (preservePos)
        stream->pop();
    return true;
}

void SED::clear()
{
    fn = 0;
    fcSepx = 0;
    fnMpr = 0;
    fcMpr = 0;
}

bool operator==(const SED &lhs, const SED &rhs)
{

    return lhs.fn == rhs.fn &&
           lhs.fcSepx == rhs.fcSepx &&
           lhs.fnMpr == rhs.fnMpr &&
           lhs.fcMpr == rhs.fcMpr;
}

bool operator!=(const SED &lhs, const SED &rhs)
{
    return !(lhs == rhs);
}


// SEP implementation

SEP::SEP() : Shared()
{
    clear();
}

SEP::SEP(AbstractOLEStreamReader *stream, bool preservePos) : Shared()
{
    clear();
    read(stream, preservePos);
}

bool SEP::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    U16 shifterU16;

    if (preservePos)
        stream->push();

    bkc = stream->readU8();
    fTitlePage = stream->readU8();
    fAutoPgn = stream->readS8();
    nfcPgn = stream->readU8();
    fUnlocked = stream->readU8();
    cnsPgn = stream->readU8();
    fPgnRestart = stream->readU8();
    fEndNote = stream->readU8();
    lnc = stream->readS8();
    grpfIhdt = stream->readS8();
    nLnnMod = stream->readU16();
    dxaLnn = stream->readS32();
    dxaPgn = stream->readS16();
    dyaPgn = stream->readS16();
    fLBetween = stream->readS8();
    vjc = stream->readS8();
    dmBinFirst = stream->readU16();
    dmBinOther = stream->readU16();
    dmPaperReq = stream->readU16();
    brcTop.read(stream, false);
    brcLeft.read(stream, false);
    brcBottom.read(stream, false);
    brcRight.read(stream, false);
    fPropRMark = stream->readS16();
    ibstPropRMark = stream->readS16();
    dttmPropRMark.read(stream, false);
    dxtCharSpace = stream->readS32();
    dyaLinePitch = stream->readS32();
    clm = stream->readU16();
    unused62 = stream->readU16();
    dmOrientPage = stream->readU8();
    iHeadingPgn = stream->readU8();
    pgnStart = stream->readU16();
    lnnMin = stream->readS16();
    wTextFlow = stream->readU16();
    unused72 = stream->readU16();
    shifterU16 = stream->readU16();
    pgbApplyTo = shifterU16;
    shifterU16 >>= 3;
    pgbPageDepth = shifterU16;
    shifterU16 >>= 2;
    pgbOffsetFrom = shifterU16;
    shifterU16 >>= 3;
    unused74_8 = shifterU16;
    xaPage = stream->readU32();
    yaPage = stream->readU32();
    xaPageNUp = stream->readU32();
    yaPageNUp = stream->readU32();
    dxaLeft = stream->readU32();
    dxaRight = stream->readU32();
    dyaTop = stream->readS32();
    dyaBottom = stream->readS32();
    dzaGutter = stream->readU32();
    dyaHdrTop = stream->readU32();
    dyaHdrBottom = stream->readU32();
    ccolM1 = stream->readS16();
    fEvenlySpaced = stream->readS8();
    unused123 = stream->readU8();
    dxaColumns = stream->readS32();
    // skipping the std::vector rgdxaColumnWidthSpacing
    dxaColumnWidth = stream->readS32();
    dmOrientFirst = stream->readU8();
    fLayout = stream->readU8();
    unused490 = stream->readU16();
    olstAnm.read(stream, false);

    if (preservePos)
        stream->pop();
    return true;
}

void SEP::clear()
{
    bkc = 2;
    fTitlePage = 0;
    fAutoPgn = 0;
    nfcPgn = 0;
    fUnlocked = 0;
    cnsPgn = 0;
    fPgnRestart = 0;
    fEndNote = 1;
    lnc = 0;
    grpfIhdt = 0;
    nLnnMod = 0;
    dxaLnn = 0;
    dxaPgn = 720;
    dyaPgn = 720;
    fLBetween = 0;
    vjc = 0;
    dmBinFirst = 0;
    dmBinOther = 0;
    dmPaperReq = 0;
    brcTop.clear();
    brcLeft.clear();
    brcBottom.clear();
    brcRight.clear();
    fPropRMark = 0;
    ibstPropRMark = 0;
    dttmPropRMark.clear();
    dxtCharSpace = 0;
    dyaLinePitch = 0;
    clm = 0;
    unused62 = 0;
    dmOrientPage = 1;
    iHeadingPgn = 0;
    pgnStart = 1;
    lnnMin = 0;
    wTextFlow = 0;
    unused72 = 0;
    pgbApplyTo = 0;
    pgbPageDepth = 0;
    pgbOffsetFrom = 0;
    unused74_8 = 0;
    xaPage = 12240;
    yaPage = 15840;
    xaPageNUp = 12240;
    yaPageNUp = 15840;
    dxaLeft = 1800;
    dxaRight = 1800;
    dyaTop = 1440;
    dyaBottom = 1440;
    dzaGutter = 0;
    dyaHdrTop = 720;
    dyaHdrBottom = 720;
    ccolM1 = 0;
    fEvenlySpaced = 1;
    unused123 = 0;
    dxaColumns = 720;
    rgdxaColumnWidthSpacing.clear();
    dxaColumnWidth = 0;
    dmOrientFirst = 0;
    fLayout = 0;
    unused490 = 0;
    olstAnm.clear();
}

void SEP::dump() const
{
    wvlog << "Dumping SEP:" << std::endl;
    wvlog << toString().c_str() << std::endl;
    wvlog << "\nDumping SEP done." << std::endl;
}

std::string SEP::toString() const
{
    std::string s( "SEP:" );
    s += "\nbkc=";
    s += uint2string( bkc );
    s += "\nfTitlePage=";
    s += uint2string( fTitlePage );
    s += "\nfAutoPgn=";
    s += int2string( fAutoPgn );
    s += "\nnfcPgn=";
    s += uint2string( nfcPgn );
    s += "\nfUnlocked=";
    s += uint2string( fUnlocked );
    s += "\ncnsPgn=";
    s += uint2string( cnsPgn );
    s += "\nfPgnRestart=";
    s += uint2string( fPgnRestart );
    s += "\nfEndNote=";
    s += uint2string( fEndNote );
    s += "\nlnc=";
    s += int2string( lnc );
    s += "\ngrpfIhdt=";
    s += int2string( grpfIhdt );
    s += "\nnLnnMod=";
    s += uint2string( nLnnMod );
    s += "\ndxaLnn=";
    s += int2string( dxaLnn );
    s += "\ndxaPgn=";
    s += int2string( dxaPgn );
    s += "\ndyaPgn=";
    s += int2string( dyaPgn );
    s += "\nfLBetween=";
    s += int2string( fLBetween );
    s += "\nvjc=";
    s += int2string( vjc );
    s += "\ndmBinFirst=";
    s += uint2string( dmBinFirst );
    s += "\ndmBinOther=";
    s += uint2string( dmBinOther );
    s += "\ndmPaperReq=";
    s += uint2string( dmPaperReq );
    s += "\nbrcTop=";
    s += "\n{" + brcTop.toString() + "}\n";
    s += "\nbrcLeft=";
    s += "\n{" + brcLeft.toString() + "}\n";
    s += "\nbrcBottom=";
    s += "\n{" + brcBottom.toString() + "}\n";
    s += "\nbrcRight=";
    s += "\n{" + brcRight.toString() + "}\n";
    s += "\nfPropRMark=";
    s += int2string( fPropRMark );
    s += "\nibstPropRMark=";
    s += int2string( ibstPropRMark );
    s += "\ndttmPropRMark=";
    s += "\n{" + dttmPropRMark.toString() + "}\n";
    s += "\ndxtCharSpace=";
    s += int2string( dxtCharSpace );
    s += "\ndyaLinePitch=";
    s += int2string( dyaLinePitch );
    s += "\nclm=";
    s += uint2string( clm );
    s += "\nunused62=";
    s += uint2string( unused62 );
    s += "\ndmOrientPage=";
    s += uint2string( dmOrientPage );
    s += "\niHeadingPgn=";
    s += uint2string( iHeadingPgn );
    s += "\npgnStart=";
    s += uint2string( pgnStart );
    s += "\nlnnMin=";
    s += int2string( lnnMin );
    s += "\nwTextFlow=";
    s += uint2string( wTextFlow );
    s += "\nunused72=";
    s += uint2string( unused72 );
    s += "\npgbApplyTo=";
    s += uint2string( pgbApplyTo );
    s += "\npgbPageDepth=";
    s += uint2string( pgbPageDepth );
    s += "\npgbOffsetFrom=";
    s += uint2string( pgbOffsetFrom );
    s += "\nunused74_8=";
    s += uint2string( unused74_8 );
    s += "\nxaPage=";
    s += uint2string( xaPage );
    s += "\nyaPage=";
    s += uint2string( yaPage );
    s += "\nxaPageNUp=";
    s += uint2string( xaPageNUp );
    s += "\nyaPageNUp=";
    s += uint2string( yaPageNUp );
    s += "\ndxaLeft=";
    s += uint2string( dxaLeft );
    s += "\ndxaRight=";
    s += uint2string( dxaRight );
    s += "\ndyaTop=";
    s += int2string( dyaTop );
    s += "\ndyaBottom=";
    s += int2string( dyaBottom );
    s += "\ndzaGutter=";
    s += uint2string( dzaGutter );
    s += "\ndyaHdrTop=";
    s += uint2string( dyaHdrTop );
    s += "\ndyaHdrBottom=";
    s += uint2string( dyaHdrBottom );
    s += "\nccolM1=";
    s += int2string( ccolM1 );
    s += "\nfEvenlySpaced=";
    s += int2string( fEvenlySpaced );
    s += "\nunused123=";
    s += uint2string( unused123 );
    s += "\ndxaColumns=";
    s += int2string( dxaColumns );
    s += "\nrgdxaColumnWidthSpacing=";
    // skipping the std::vector rgdxaColumnWidthSpacing
    s += "\ndxaColumnWidth=";
    s += int2string( dxaColumnWidth );
    s += "\ndmOrientFirst=";
    s += uint2string( dmOrientFirst );
    s += "\nfLayout=";
    s += uint2string( fLayout );
    s += "\nunused490=";
    s += uint2string( unused490 );
    s += "\nolstAnm=";
    s += "\n{" + olstAnm.toString() + "}\n";
    s += "\nSEP Done.";
    return s;
}

bool operator==(const SEP &lhs, const SEP &rhs)
{

    return lhs.bkc == rhs.bkc &&
           lhs.fTitlePage == rhs.fTitlePage &&
           lhs.fAutoPgn == rhs.fAutoPgn &&
           lhs.nfcPgn == rhs.nfcPgn &&
           lhs.fUnlocked == rhs.fUnlocked &&
           lhs.cnsPgn == rhs.cnsPgn &&
           lhs.fPgnRestart == rhs.fPgnRestart &&
           lhs.fEndNote == rhs.fEndNote &&
           lhs.lnc == rhs.lnc &&
           lhs.grpfIhdt == rhs.grpfIhdt &&
           lhs.nLnnMod == rhs.nLnnMod &&
           lhs.dxaLnn == rhs.dxaLnn &&
           lhs.dxaPgn == rhs.dxaPgn &&
           lhs.dyaPgn == rhs.dyaPgn &&
           lhs.fLBetween == rhs.fLBetween &&
           lhs.vjc == rhs.vjc &&
           lhs.dmBinFirst == rhs.dmBinFirst &&
           lhs.dmBinOther == rhs.dmBinOther &&
           lhs.dmPaperReq == rhs.dmPaperReq &&
           lhs.brcTop == rhs.brcTop &&
           lhs.brcLeft == rhs.brcLeft &&
           lhs.brcBottom == rhs.brcBottom &&
           lhs.brcRight == rhs.brcRight &&
           lhs.fPropRMark == rhs.fPropRMark &&
           lhs.ibstPropRMark == rhs.ibstPropRMark &&
           lhs.dttmPropRMark == rhs.dttmPropRMark &&
           lhs.dxtCharSpace == rhs.dxtCharSpace &&
           lhs.dyaLinePitch == rhs.dyaLinePitch &&
           lhs.clm == rhs.clm &&
           lhs.unused62 == rhs.unused62 &&
           lhs.dmOrientPage == rhs.dmOrientPage &&
           lhs.iHeadingPgn == rhs.iHeadingPgn &&
           lhs.pgnStart == rhs.pgnStart &&
           lhs.lnnMin == rhs.lnnMin &&
           lhs.wTextFlow == rhs.wTextFlow &&
           lhs.unused72 == rhs.unused72 &&
           lhs.pgbApplyTo == rhs.pgbApplyTo &&
           lhs.pgbPageDepth == rhs.pgbPageDepth &&
           lhs.pgbOffsetFrom == rhs.pgbOffsetFrom &&
           lhs.unused74_8 == rhs.unused74_8 &&
           lhs.xaPage == rhs.xaPage &&
           lhs.yaPage == rhs.yaPage &&
           lhs.xaPageNUp == rhs.xaPageNUp &&
           lhs.yaPageNUp == rhs.yaPageNUp &&
           lhs.dxaLeft == rhs.dxaLeft &&
           lhs.dxaRight == rhs.dxaRight &&
           lhs.dyaTop == rhs.dyaTop &&
           lhs.dyaBottom == rhs.dyaBottom &&
           lhs.dzaGutter == rhs.dzaGutter &&
           lhs.dyaHdrTop == rhs.dyaHdrTop &&
           lhs.dyaHdrBottom == rhs.dyaHdrBottom &&
           lhs.ccolM1 == rhs.ccolM1 &&
           lhs.fEvenlySpaced == rhs.fEvenlySpaced &&
           lhs.unused123 == rhs.unused123 &&
           lhs.dxaColumns == rhs.dxaColumns &&
           lhs.rgdxaColumnWidthSpacing == rhs.rgdxaColumnWidthSpacing &&
           lhs.dxaColumnWidth == rhs.dxaColumnWidth &&
           lhs.dmOrientFirst == rhs.dmOrientFirst &&
           lhs.fLayout == rhs.fLayout &&
           lhs.unused490 == rhs.unused490 &&
           lhs.olstAnm == rhs.olstAnm;
}

bool operator!=(const SEP &lhs, const SEP &rhs)
{
    return !(lhs == rhs);
}


// SEPX implementation

SEPX::SEPX()
{
    clearInternal();
}

SEPX::SEPX(AbstractOLEStreamReader *stream, bool preservePos)
{
    clearInternal();
    read(stream, preservePos);
}

SEPX::SEPX(const SEPX &rhs)
{
    cb = rhs.cb;
    grpprl = rhs.grpprl;
}

SEPX::~SEPX()
{
    delete [] grpprl;
}

SEPX &SEPX::operator=(const SEPX &rhs)
{

    // Check for assignment to self
    if (this == &rhs)
        return *this;

    cb = rhs.cb;
    grpprl = rhs.grpprl;

    return *this;
}

bool SEPX::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    if (preservePos)
        stream->push();

    cb = stream->readU16();
    // Attention: I don't know how to read grpprl - U8[]
#ifdef __GNUC__
#warning "Couldn't generate reading code for SEPX::grpprl"
#endif

    if (preservePos)
        stream->pop();
    return true;
}

void SEPX::clear()
{
    delete [] grpprl;
    clearInternal();
}

void SEPX::clearInternal()
{
    cb = 0;
    grpprl = 0;
}

bool operator==(const SEPX &lhs, const SEPX &rhs)
{
    // Attention: I don't know how to compare grpprl - U8[]
#ifdef __GNUC__
#warning "Can't compare SEPX::grpprl items"
#endif

    return lhs.cb == rhs.cb;
}

bool operator!=(const SEPX &lhs, const SEPX &rhs)
{
    return !(lhs == rhs);
}


// STSHI implementation

const unsigned int STSHI::sizeOf = 18;

STSHI::STSHI()
{
    clear();
}

STSHI::STSHI(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

bool STSHI::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    U16 shifterU16;

    if (preservePos)
        stream->push();

    cstd = stream->readU16();
    cbSTDBaseInFile = stream->readU16();
    shifterU16 = stream->readU16();
    fStdStylenamesWritten = shifterU16;
    shifterU16 >>= 1;
    unused4_2 = shifterU16;
    stiMaxWhenSaved = stream->readU16();
    istdMaxFixedWhenSaved = stream->readU16();
    nVerBuiltInNamesWhenSaved = stream->readU16();
    for (int _i = 0; _i < (3); ++_i)
        rgftcStandardChpStsh[_i] = stream->readU16();

    if (preservePos)
        stream->pop();
    return true;
}

void STSHI::clear()
{
    cstd = 0;
    cbSTDBaseInFile = 0;
    fStdStylenamesWritten = 0;
    unused4_2 = 0;
    stiMaxWhenSaved = 0;
    istdMaxFixedWhenSaved = 0;
    nVerBuiltInNamesWhenSaved = 0;
    for (int _i = 0; _i < (3); ++_i)
        rgftcStandardChpStsh[_i] = 0;
}

bool operator==(const STSHI &lhs, const STSHI &rhs)
{

    for (int _i = 0; _i < (3); ++_i) {
        if (lhs.rgftcStandardChpStsh[_i] != rhs.rgftcStandardChpStsh[_i])
            return false;
    }

    return lhs.cstd == rhs.cstd &&
           lhs.cbSTDBaseInFile == rhs.cbSTDBaseInFile &&
           lhs.fStdStylenamesWritten == rhs.fStdStylenamesWritten &&
           lhs.unused4_2 == rhs.unused4_2 &&
           lhs.stiMaxWhenSaved == rhs.stiMaxWhenSaved &&
           lhs.istdMaxFixedWhenSaved == rhs.istdMaxFixedWhenSaved &&
           lhs.nVerBuiltInNamesWhenSaved == rhs.nVerBuiltInNamesWhenSaved;
}

bool operator!=(const STSHI &lhs, const STSHI &rhs)
{
    return !(lhs == rhs);
}


// WKB implementation

WKB::WKB()
{
    clear();
}

WKB::WKB(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

bool WKB::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    U16 shifterU16;

    if (preservePos)
        stream->push();

    fn = stream->readS16();
    grfwkb = stream->readU16();
    lvl = stream->readS16();
    shifterU16 = stream->readU16();
    fnpt = shifterU16;
    shifterU16 >>= 4;
    fnpd = shifterU16;
    doc = stream->readS32();

    if (preservePos)
        stream->pop();
    return true;
}

void WKB::clear()
{
    fn = 0;
    grfwkb = 0;
    lvl = 0;
    fnpt = 0;
    fnpd = 0;
    doc = 0;
}

bool operator==(const WKB &lhs, const WKB &rhs)
{

    return lhs.fn == rhs.fn &&
           lhs.grfwkb == rhs.grfwkb &&
           lhs.lvl == rhs.lvl &&
           lhs.fnpt == rhs.fnpt &&
           lhs.fnpd == rhs.fnpd &&
           lhs.doc == rhs.doc;
}

bool operator!=(const WKB &lhs, const WKB &rhs)
{
    return !(lhs == rhs);
}



} // namespace Word97

} // namespace wvWare
