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

#include "word95_generated.h"
#include "olestream.h"
#include <string.h>  // memset(), memcpy()
#include "wvlog.h"

namespace wvWare {

namespace Word95 {


// DTTM implementation

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

    U8 shifterU8;

    if (preservePos)
        stream->push();

    shifterU8 = stream->readU8();
    fComplex = shifterU8;
    shifterU8 >>= 1;
    sprm = shifterU8;
    val = stream->readU8();

    if (preservePos)
        stream->pop();
    return true;
}

void PRM::readPtr(const U8 *ptr)
{

    U8 shifterU8;

    shifterU8 = readU8(ptr);
    ptr += sizeof(U8);
    fComplex = shifterU8;
    shifterU8 >>= 1;
    sprm = shifterU8;
    val = readU8(ptr);
    ptr += sizeof(U8);
}

void PRM::clear()
{
    fComplex = 0;
    sprm = 0;
    val = 0;
}

bool operator==(const PRM &lhs, const PRM &rhs)
{

    return lhs.fComplex == rhs.fComplex &&
           lhs.sprm == rhs.sprm &&
           lhs.val == rhs.val;
}

bool operator!=(const PRM &lhs, const PRM &rhs)
{
    return !(lhs == rhs);
}


// SHD implementation

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

const unsigned int PHE::sizeOf = 6;

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
    dxaCol = stream->readU16();
    dylLine_dylHeight = stream->readU16();

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
    dxaCol = readU16(ptr);
    ptr += sizeof(U16);
    dylLine_dylHeight = readU16(ptr);
    ptr += sizeof(U16);
}

void PHE::clear()
{
    fSpare = 0;
    fUnk = 0;
    fDiffLines = 0;
    unused0_3 = 0;
    clMac = 0;
    dxaCol = 0;
    dylLine_dylHeight = 0;
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
    s += "\ndxaCol=";
    s += uint2string( dxaCol );
    s += "\ndylLine_dylHeight=";
    s += uint2string( dylLine_dylHeight );
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
           lhs.dxaCol == rhs.dxaCol &&
           lhs.dylLine_dylHeight == rhs.dylLine_dylHeight;
}

bool operator!=(const PHE &lhs, const PHE &rhs)
{
    return !(lhs == rhs);
}


// BRC implementation

const unsigned int BRC::sizeOf = 2;

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
    dxpLineWidth = shifterU16;
    shifterU16 >>= 3;
    brcType = shifterU16;
    shifterU16 >>= 2;
    fShadow = shifterU16;
    shifterU16 >>= 1;
    ico = shifterU16;
    shifterU16 >>= 5;
    dxpSpace = shifterU16;

    if (preservePos)
        stream->pop();
    return true;
}

void BRC::readPtr(const U8 *ptr)
{

    U16 shifterU16;

    shifterU16 = readU16(ptr);
    ptr += sizeof(U16);
    dxpLineWidth = shifterU16;
    shifterU16 >>= 3;
    brcType = shifterU16;
    shifterU16 >>= 2;
    fShadow = shifterU16;
    shifterU16 >>= 1;
    ico = shifterU16;
    shifterU16 >>= 5;
    dxpSpace = shifterU16;
}

void BRC::clear()
{
    dxpLineWidth = 0;
    brcType = 0;
    fShadow = 0;
    ico = 0;
    dxpSpace = 0;
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
    s += "\ndxpLineWidth=";
    s += uint2string( dxpLineWidth );
    s += "\nbrcType=";
    s += uint2string( brcType );
    s += "\nfShadow=";
    s += uint2string( fShadow );
    s += "\nico=";
    s += uint2string( ico );
    s += "\ndxpSpace=";
    s += uint2string( dxpSpace );
    s += "\nBRC Done.";
    return s;
}

bool operator==(const BRC &lhs, const BRC &rhs)
{

    return lhs.dxpLineWidth == rhs.dxpLineWidth &&
           lhs.brcType == rhs.brcType &&
           lhs.fShadow == rhs.fShadow &&
           lhs.ico == rhs.ico &&
           lhs.dxpSpace == rhs.dxpSpace;
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

    itl = stream->readU16();
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

    itl = readU16(ptr);
    ptr += sizeof(U16);
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
    s += uint2string( itl );
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

const unsigned int TC::sizeOf = 10;

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
    fUnused = shifterU16;
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
    fUnused = shifterU16;
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
    fUnused = 0;
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
    s += "\nfUnused=";
    s += uint2string( fUnused );
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
           lhs.fUnused == rhs.fUnused &&
           lhs.brcTop == rhs.brcTop &&
           lhs.brcLeft == rhs.brcLeft &&
           lhs.brcBottom == rhs.brcBottom &&
           lhs.brcRight == rhs.brcRight;
}

bool operator!=(const TC &lhs, const TC &rhs)
{
    return !(lhs == rhs);
}


// DPHEAD implementation

DPHEAD::DPHEAD()
{
    clear();
}

DPHEAD::DPHEAD(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

bool DPHEAD::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    if (preservePos)
        stream->push();

    dpk = stream->readU16();
    cb = stream->readU16();
    xa = stream->readU16();
    ya = stream->readU16();
    dxa = stream->readU16();
    dya = stream->readU16();

    if (preservePos)
        stream->pop();
    return true;
}

void DPHEAD::clear()
{
    dpk = 0;
    cb = 0;
    xa = 0;
    ya = 0;
    dxa = 0;
    dya = 0;
}

bool operator==(const DPHEAD &lhs, const DPHEAD &rhs)
{

    return lhs.dpk == rhs.dpk &&
           lhs.cb == rhs.cb &&
           lhs.xa == rhs.xa &&
           lhs.ya == rhs.ya &&
           lhs.dxa == rhs.dxa &&
           lhs.dya == rhs.dya;
}

bool operator!=(const DPHEAD &lhs, const DPHEAD &rhs)
{
    return !(lhs == rhs);
}


// DPTXBX implementation

DPTXBX::DPTXBX()
{
    clear();
}

DPTXBX::DPTXBX(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

bool DPTXBX::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    U16 shifterU16;

    if (preservePos)
        stream->push();

    dphead.read(stream, false);
    lnpc = stream->readU32();
    lnpw = stream->readU16();
    lnps = stream->readU16();
    dlpcFg = stream->readU32();
    dlpcBg = stream->readU32();
    flpp = stream->readU16();
    shdwpi = stream->readU16();
    xaOffset = stream->readU16();
    yaOffset = stream->readU16();
    shifterU16 = stream->readU16();
    fRoundCorners = shifterU16;
    shifterU16 >>= 1;
    zaShape = shifterU16;
    dzaInternalMargin = stream->readU16();

    if (preservePos)
        stream->pop();
    return true;
}

void DPTXBX::clear()
{
    dphead.clear();
    lnpc = 0;
    lnpw = 0;
    lnps = 0;
    dlpcFg = 0;
    dlpcBg = 0;
    flpp = 0;
    shdwpi = 0;
    xaOffset = 0;
    yaOffset = 0;
    fRoundCorners = 0;
    zaShape = 0;
    dzaInternalMargin = 0;
}

bool operator==(const DPTXBX &lhs, const DPTXBX &rhs)
{

    return lhs.dphead == rhs.dphead &&
           lhs.lnpc == rhs.lnpc &&
           lhs.lnpw == rhs.lnpw &&
           lhs.lnps == rhs.lnps &&
           lhs.dlpcFg == rhs.dlpcFg &&
           lhs.dlpcBg == rhs.dlpcBg &&
           lhs.flpp == rhs.flpp &&
           lhs.shdwpi == rhs.shdwpi &&
           lhs.xaOffset == rhs.xaOffset &&
           lhs.yaOffset == rhs.yaOffset &&
           lhs.fRoundCorners == rhs.fRoundCorners &&
           lhs.zaShape == rhs.zaShape &&
           lhs.dzaInternalMargin == rhs.dzaInternalMargin;
}

bool operator!=(const DPTXBX &lhs, const DPTXBX &rhs)
{
    return !(lhs == rhs);
}


// DPPOLYLINE implementation

DPPOLYLINE::DPPOLYLINE()
{
    clearInternal();
}

DPPOLYLINE::DPPOLYLINE(AbstractOLEStreamReader *stream, bool preservePos)
{
    clearInternal();
    read(stream, preservePos);
}

DPPOLYLINE::DPPOLYLINE(const DPPOLYLINE &rhs)
{
    dphead = rhs.dphead;
    lnpc = rhs.lnpc;
    lnpw = rhs.lnpw;
    lnps = rhs.lnps;
    dlpcFg = rhs.dlpcFg;
    dlpcBg = rhs.dlpcBg;
    flpp = rhs.flpp;
    eppsStart = rhs.eppsStart;
    eppwStart = rhs.eppwStart;
    epplStart = rhs.epplStart;
    unused30_6 = rhs.unused30_6;
    eppsEnd = rhs.eppsEnd;
    eppwEnd = rhs.eppwEnd;
    epplEnd = rhs.epplEnd;
    unused32_6 = rhs.unused32_6;
    shdwpi = rhs.shdwpi;
    xaOffset = rhs.xaOffset;
    yaOffset = rhs.yaOffset;
    fPolygon = rhs.fPolygon;
    cpt = rhs.cpt;
    xaFirst = rhs.xaFirst;
    yaFirst = rhs.yaFirst;
    xaEnd = rhs.xaEnd;
    yaEnd = rhs.yaEnd;
    rgpta = rhs.rgpta;
}

DPPOLYLINE::~DPPOLYLINE()
{
    delete [] rgpta;
}

DPPOLYLINE &DPPOLYLINE::operator=(const DPPOLYLINE &rhs)
{

    // Check for assignment to self
    if (this == &rhs)
        return *this;

    dphead = rhs.dphead;
    lnpc = rhs.lnpc;
    lnpw = rhs.lnpw;
    lnps = rhs.lnps;
    dlpcFg = rhs.dlpcFg;
    dlpcBg = rhs.dlpcBg;
    flpp = rhs.flpp;
    eppsStart = rhs.eppsStart;
    eppwStart = rhs.eppwStart;
    epplStart = rhs.epplStart;
    unused30_6 = rhs.unused30_6;
    eppsEnd = rhs.eppsEnd;
    eppwEnd = rhs.eppwEnd;
    epplEnd = rhs.epplEnd;
    unused32_6 = rhs.unused32_6;
    shdwpi = rhs.shdwpi;
    xaOffset = rhs.xaOffset;
    yaOffset = rhs.yaOffset;
    fPolygon = rhs.fPolygon;
    cpt = rhs.cpt;
    xaFirst = rhs.xaFirst;
    yaFirst = rhs.yaFirst;
    xaEnd = rhs.xaEnd;
    yaEnd = rhs.yaEnd;
    rgpta = rhs.rgpta;

    return *this;
}

bool DPPOLYLINE::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    U16 shifterU16;

    if (preservePos)
        stream->push();

    dphead.read(stream, false);
    lnpc = stream->readU32();
    lnpw = stream->readU16();
    lnps = stream->readU16();
    dlpcFg = stream->readU32();
    dlpcBg = stream->readU32();
    flpp = stream->readU16();
    shifterU16 = stream->readU16();
    eppsStart = shifterU16;
    shifterU16 >>= 2;
    eppwStart = shifterU16;
    shifterU16 >>= 2;
    epplStart = shifterU16;
    shifterU16 >>= 2;
    unused30_6 = shifterU16;
    shifterU16 = stream->readU16();
    eppsEnd = shifterU16;
    shifterU16 >>= 2;
    eppwEnd = shifterU16;
    shifterU16 >>= 2;
    epplEnd = shifterU16;
    shifterU16 >>= 2;
    unused32_6 = shifterU16;
    shdwpi = stream->readU16();
    xaOffset = stream->readU16();
    yaOffset = stream->readU16();
    shifterU16 = stream->readU16();
    fPolygon = shifterU16;
    shifterU16 >>= 1;
    cpt = shifterU16;
    xaFirst = stream->readU16();
    yaFirst = stream->readU16();
    xaEnd = stream->readU16();
    yaEnd = stream->readU16();
    // Attention: I don't know how to read rgpta - U16[]
#ifdef __GNUC__
#warning "Couldn't generate reading code for DPPOLYLINE::rgpta"
#endif

    if (preservePos)
        stream->pop();
    return true;
}

void DPPOLYLINE::clear()
{
    delete [] rgpta;
    clearInternal();
}

void DPPOLYLINE::clearInternal()
{
    dphead.clear();
    lnpc = 0;
    lnpw = 0;
    lnps = 0;
    dlpcFg = 0;
    dlpcBg = 0;
    flpp = 0;
    eppsStart = 0;
    eppwStart = 0;
    epplStart = 0;
    unused30_6 = 0;
    eppsEnd = 0;
    eppwEnd = 0;
    epplEnd = 0;
    unused32_6 = 0;
    shdwpi = 0;
    xaOffset = 0;
    yaOffset = 0;
    fPolygon = 0;
    cpt = 0;
    xaFirst = 0;
    yaFirst = 0;
    xaEnd = 0;
    yaEnd = 0;
    rgpta = 0;
}

bool operator==(const DPPOLYLINE &lhs, const DPPOLYLINE &rhs)
{
    // Attention: I don't know how to compare rgpta - U16[]
#ifdef __GNUC__
#warning "Can't compare DPPOLYLINE::rgpta items"
#endif

    return lhs.dphead == rhs.dphead &&
           lhs.lnpc == rhs.lnpc &&
           lhs.lnpw == rhs.lnpw &&
           lhs.lnps == rhs.lnps &&
           lhs.dlpcFg == rhs.dlpcFg &&
           lhs.dlpcBg == rhs.dlpcBg &&
           lhs.flpp == rhs.flpp &&
           lhs.eppsStart == rhs.eppsStart &&
           lhs.eppwStart == rhs.eppwStart &&
           lhs.epplStart == rhs.epplStart &&
           lhs.unused30_6 == rhs.unused30_6 &&
           lhs.eppsEnd == rhs.eppsEnd &&
           lhs.eppwEnd == rhs.eppwEnd &&
           lhs.epplEnd == rhs.epplEnd &&
           lhs.unused32_6 == rhs.unused32_6 &&
           lhs.shdwpi == rhs.shdwpi &&
           lhs.xaOffset == rhs.xaOffset &&
           lhs.yaOffset == rhs.yaOffset &&
           lhs.fPolygon == rhs.fPolygon &&
           lhs.cpt == rhs.cpt &&
           lhs.xaFirst == rhs.xaFirst &&
           lhs.yaFirst == rhs.yaFirst &&
           lhs.xaEnd == rhs.xaEnd &&
           lhs.yaEnd == rhs.yaEnd;
}

bool operator!=(const DPPOLYLINE &lhs, const DPPOLYLINE &rhs)
{
    return !(lhs == rhs);
}


// TAP implementation

TAP::TAP() : Shared()
{
    clearInternal();
}

TAP::TAP(AbstractOLEStreamReader *stream, bool preservePos) : Shared()
{
    clearInternal();
    read(stream, preservePos);
}

TAP::TAP(const TAP &rhs) : Shared()
{
    jc = rhs.jc;
    dxaGapHalf = rhs.dxaGapHalf;
    dyaRowHeight = rhs.dyaRowHeight;
    fCantSplit = rhs.fCantSplit;
    fTableHeader = rhs.fTableHeader;
    tlp = rhs.tlp;
    fCaFull = rhs.fCaFull;
    fFirstRow = rhs.fFirstRow;
    fLastRow = rhs.fLastRow;
    fOutline = rhs.fOutline;
    unused12_4 = rhs.unused12_4;
    itcMac = rhs.itcMac;
    dxaAdjust = rhs.dxaAdjust;
    rgdxaCenter = new U16[itcMac + 1];
    memcpy(rgdxaCenter, rhs.rgdxaCenter, sizeof(U16) * (itcMac + 1));
    rgtc = new TC[itcMac];
    memcpy(rgtc, rhs.rgtc, sizeof(TC) * (itcMac));
    rgshd = new SHD[itcMac];
    memcpy(rgshd, rhs.rgshd, sizeof(SHD) * (itcMac));
    memcpy(&rgbrcTable, &rhs.rgbrcTable, sizeof(rgbrcTable));
}

TAP::~TAP()
{
    delete [] rgdxaCenter;
    delete [] rgtc;
    delete [] rgshd;
}

TAP &TAP::operator=(const TAP &rhs)
{

    // Check for assignment to self
    if (this == &rhs)
        return *this;

    jc = rhs.jc;
    dxaGapHalf = rhs.dxaGapHalf;
    dyaRowHeight = rhs.dyaRowHeight;
    fCantSplit = rhs.fCantSplit;
    fTableHeader = rhs.fTableHeader;
    tlp = rhs.tlp;
    fCaFull = rhs.fCaFull;
    fFirstRow = rhs.fFirstRow;
    fLastRow = rhs.fLastRow;
    fOutline = rhs.fOutline;
    unused12_4 = rhs.unused12_4;
    itcMac = rhs.itcMac;
    dxaAdjust = rhs.dxaAdjust;
    delete [] rgdxaCenter;
    rgdxaCenter = new U16[itcMac + 1];
    memcpy(rgdxaCenter, rhs.rgdxaCenter, sizeof(U16) * (itcMac + 1));
    delete [] rgtc;
    rgtc = new TC[itcMac];
    memcpy(rgtc, rhs.rgtc, sizeof(TC) * (itcMac));
    delete [] rgshd;
    rgshd = new SHD[itcMac];
    memcpy(rgshd, rhs.rgshd, sizeof(SHD) * (itcMac));
    memcpy(&rgbrcTable, &rhs.rgbrcTable, sizeof(rgbrcTable));

    return *this;
}

bool TAP::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    U16 shifterU16;

    if (preservePos)
        stream->push();

    jc = stream->readU16();
    dxaGapHalf = stream->readU16();
    dyaRowHeight = stream->readU16();
    fCantSplit = stream->readU8();
    fTableHeader = stream->readU8();
    tlp.read(stream, false);
    shifterU16 = stream->readU16();
    fCaFull = shifterU16;
    shifterU16 >>= 1;
    fFirstRow = shifterU16;
    shifterU16 >>= 1;
    fLastRow = shifterU16;
    shifterU16 >>= 1;
    fOutline = shifterU16;
    shifterU16 >>= 1;
    unused12_4 = shifterU16;
    itcMac = stream->readU16();
    dxaAdjust = stream->readU16();
    rgdxaCenter = new U16[itcMac + 1];
    for (int _i = 0; _i < (itcMac + 1); ++_i)
        rgdxaCenter[_i] = stream->readU16();
    rgtc = new TC[itcMac];
    for (int _i = 0; _i < (itcMac); ++_i)
        rgtc[_i].read(stream, false);
    rgshd = new SHD[itcMac];
    for (int _i = 0; _i < (itcMac); ++_i)
        rgshd[_i].read(stream, false);
    for (int _i = 0; _i < (6); ++_i)
        rgbrcTable[_i].read(stream, false);

    if (preservePos)
        stream->pop();
    return true;
}

void TAP::clear()
{
    delete [] rgdxaCenter;
    delete [] rgtc;
    delete [] rgshd;
    clearInternal();
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
    s += uint2string( jc );
    s += "\ndxaGapHalf=";
    s += uint2string( dxaGapHalf );
    s += "\ndyaRowHeight=";
    s += uint2string( dyaRowHeight );
    s += "\nfCantSplit=";
    s += uint2string( fCantSplit );
    s += "\nfTableHeader=";
    s += uint2string( fTableHeader );
    s += "\ntlp=";
    s += "\n{" + tlp.toString() + "}\n";
    s += "\nfCaFull=";
    s += uint2string( fCaFull );
    s += "\nfFirstRow=";
    s += uint2string( fFirstRow );
    s += "\nfLastRow=";
    s += uint2string( fLastRow );
    s += "\nfOutline=";
    s += uint2string( fOutline );
    s += "\nunused12_4=";
    s += uint2string( unused12_4 );
    s += "\nitcMac=";
    s += uint2string( itcMac );
    s += "\ndxaAdjust=";
    s += uint2string( dxaAdjust );
    for (int _i = 0; _i < (itcMac + 1); ++_i) {
        s += "\nrgdxaCenter[" + int2string( _i ) + "]=";
        s += uint2string( rgdxaCenter[_i] );
    }
    for (int _i = 0; _i < (itcMac); ++_i) {
        s += "\nrgtc[" + int2string( _i ) + "]=";
        s += "\n{" + rgtc[_i].toString() + "}\n";
    }
    for (int _i = 0; _i < (itcMac); ++_i) {
        s += "\nrgshd[" + int2string( _i ) + "]=";
        s += "\n{" + rgshd[_i].toString() + "}\n";
    }
    for (int _i = 0; _i < (6); ++_i) {
        s += "\nrgbrcTable[" + int2string( _i ) + "]=";
        s += "\n{" + rgbrcTable[_i].toString() + "}\n";
    }
    s += "\nTAP Done.";
    return s;
}

void TAP::clearInternal()
{
    jc = 0;
    dxaGapHalf = 0;
    dyaRowHeight = 0;
    fCantSplit = 0;
    fTableHeader = 0;
    tlp.clear();
    fCaFull = 0;
    fFirstRow = 0;
    fLastRow = 0;
    fOutline = 0;
    unused12_4 = 0;
    itcMac = 0;
    dxaAdjust = 0;
    rgdxaCenter = 0;
    rgtc = 0;
    rgshd = 0;
    for (int _i = 0; _i < (6); ++_i)
        rgbrcTable[_i].clear();
}

bool operator==(const TAP &lhs, const TAP &rhs)
{

    if ((lhs.itcMac) != (rhs.itcMac))
        return false;
    for (int _i = 0; _i < (lhs.itcMac); ++_i) {
        if (lhs.rgdxaCenter[_i] != rhs.rgdxaCenter[_i])
            return false;
    }

    if ((lhs.itcMac) != (rhs.itcMac))
        return false;
    for (int _i = 0; _i < (lhs.itcMac); ++_i) {
        if (lhs.rgtc[_i] != rhs.rgtc[_i])
            return false;
    }

    if ((lhs.itcMac) != (rhs.itcMac))
        return false;
    for (int _i = 0; _i < (lhs.itcMac); ++_i) {
        if (lhs.rgshd[_i] != rhs.rgshd[_i])
            return false;
    }

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
           lhs.fCaFull == rhs.fCaFull &&
           lhs.fFirstRow == rhs.fFirstRow &&
           lhs.fLastRow == rhs.fLastRow &&
           lhs.fOutline == rhs.fOutline &&
           lhs.unused12_4 == rhs.unused12_4 &&
           lhs.itcMac == rhs.itcMac &&
           lhs.dxaAdjust == rhs.dxaAdjust;
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
        rgchAnld[_i] = stream->readU8();

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
        rgchAnld[_i] = readU8(ptr);
        ptr += sizeof(U8);
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
        rgchAnld[_i] = 0;
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
        s += "\nrgchAnld[" + int2string( _i ) + "]=";
        s += uint2string( rgchAnld[_i] );
    }
    s += "\nANLD Done.";
    return s;
}

bool operator==(const ANLD &lhs, const ANLD &rhs)
{

    for (int _i = 0; _i < (32); ++_i) {
        if (lhs.rgchAnld[_i] != rhs.rgchAnld[_i])
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

const unsigned int BTE::sizeOf = 2;

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

    pn = stream->readU16();

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
    unused2 = stream->readU16();
    ftc = stream->readU16();
    hps = stream->readU16();
    dxaSpace = stream->readU16();
    shifterU8 = stream->readU8();
    iss = shifterU8;
    shifterU8 >>= 3;
    unused10_3 = shifterU8;
    shifterU8 >>= 3;
    fSysVanish = shifterU8;
    shifterU8 >>= 1;
    unused10_7 = shifterU8;
    shifterU8 = stream->readU8();
    ico = shifterU8;
    shifterU8 >>= 5;
    kul = shifterU8;
    hpsPos = stream->readS16();
    lid = stream->readU16();
    fcPic_fcObj_lTagObj = stream->readU32();
    ibstRMark = stream->readU16();
    dttmRMark.read(stream, false);
    unused26 = stream->readU16();
    istd = stream->readU16();
    ftcSym = stream->readU16();
    chSym = stream->readU8();
    fChsDiff = stream->readU8();
    idslRMReason = stream->readU16();
    ysr = stream->readU8();
    chYsr = stream->readU8();
    chse = stream->readU16();
    hpsKern = stream->readU16();

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
    unused2 = 0;
    ftc = 0;
    hps = 0;
    dxaSpace = 0;
    iss = 0;
    unused10_3 = 0;
    fSysVanish = 0;
    unused10_7 = 0;
    ico = 0;
    kul = 0;
    hpsPos = 0;
    lid = 0;
    fcPic_fcObj_lTagObj = 0;
    ibstRMark = 0;
    dttmRMark.clear();
    unused26 = 0;
    istd = 0;
    ftcSym = 0;
    chSym = 0;
    fChsDiff = 0;
    idslRMReason = 0;
    ysr = 0;
    chYsr = 0;
    chse = 0;
    hpsKern = 0;
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
    s += "\nunused2=";
    s += uint2string( unused2 );
    s += "\nftc=";
    s += uint2string( ftc );
    s += "\nhps=";
    s += uint2string( hps );
    s += "\ndxaSpace=";
    s += uint2string( dxaSpace );
    s += "\niss=";
    s += uint2string( iss );
    s += "\nunused10_3=";
    s += uint2string( unused10_3 );
    s += "\nfSysVanish=";
    s += uint2string( fSysVanish );
    s += "\nunused10_7=";
    s += uint2string( unused10_7 );
    s += "\nico=";
    s += uint2string( ico );
    s += "\nkul=";
    s += uint2string( kul );
    s += "\nhpsPos=";
    s += int2string( hpsPos );
    s += "\nlid=";
    s += uint2string( lid );
    s += "\nfcPic_fcObj_lTagObj=";
    s += uint2string( fcPic_fcObj_lTagObj );
    s += "\nibstRMark=";
    s += uint2string( ibstRMark );
    s += "\ndttmRMark=";
    s += "\n{" + dttmRMark.toString() + "}\n";
    s += "\nunused26=";
    s += uint2string( unused26 );
    s += "\nistd=";
    s += uint2string( istd );
    s += "\nftcSym=";
    s += uint2string( ftcSym );
    s += "\nchSym=";
    s += uint2string( chSym );
    s += "\nfChsDiff=";
    s += uint2string( fChsDiff );
    s += "\nidslRMReason=";
    s += uint2string( idslRMReason );
    s += "\nysr=";
    s += uint2string( ysr );
    s += "\nchYsr=";
    s += uint2string( chYsr );
    s += "\nchse=";
    s += uint2string( chse );
    s += "\nhpsKern=";
    s += uint2string( hpsKern );
    s += "\nCHP Done.";
    return s;
}

bool operator==(const CHP &lhs, const CHP &rhs)
{

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
           lhs.unused2 == rhs.unused2 &&
           lhs.ftc == rhs.ftc &&
           lhs.hps == rhs.hps &&
           lhs.dxaSpace == rhs.dxaSpace &&
           lhs.iss == rhs.iss &&
           lhs.unused10_3 == rhs.unused10_3 &&
           lhs.fSysVanish == rhs.fSysVanish &&
           lhs.unused10_7 == rhs.unused10_7 &&
           lhs.ico == rhs.ico &&
           lhs.kul == rhs.kul &&
           lhs.hpsPos == rhs.hpsPos &&
           lhs.lid == rhs.lid &&
           lhs.fcPic_fcObj_lTagObj == rhs.fcPic_fcObj_lTagObj &&
           lhs.ibstRMark == rhs.ibstRMark &&
           lhs.dttmRMark == rhs.dttmRMark &&
           lhs.unused26 == rhs.unused26 &&
           lhs.istd == rhs.istd &&
           lhs.ftcSym == rhs.ftcSym &&
           lhs.chSym == rhs.chSym &&
           lhs.fChsDiff == rhs.fChsDiff &&
           lhs.idslRMReason == rhs.idslRMReason &&
           lhs.ysr == rhs.ysr &&
           lhs.chYsr == rhs.chYsr &&
           lhs.chse == rhs.chse &&
           lhs.hpsKern == rhs.hpsKern;
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


// DO implementation

DO::DO()
{
    clear();
}

DO::DO(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

bool DO::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    U16 shifterU16;

    if (preservePos)
        stream->push();

    fc = stream->readU32();
    dok = stream->readU16();
    cb = stream->readU16();
    bx = stream->readU8();
    by = stream->readU8();
    dhgt = stream->readU16();
    shifterU16 = stream->readU16();
    fAnchorLock = shifterU16;
    shifterU16 >>= 1;
    unused8 = shifterU16;
    rgdp = stream->readU8();

    if (preservePos)
        stream->pop();
    return true;
}

void DO::clear()
{
    fc = 0;
    dok = 0;
    cb = 0;
    bx = 0;
    by = 0;
    dhgt = 0;
    fAnchorLock = 0;
    unused8 = 0;
    rgdp = 0;
}

bool operator==(const DO &lhs, const DO &rhs)
{

    return lhs.fc == rhs.fc &&
           lhs.dok == rhs.dok &&
           lhs.cb == rhs.cb &&
           lhs.bx == rhs.bx &&
           lhs.by == rhs.by &&
           lhs.dhgt == rhs.dhgt &&
           lhs.fAnchorLock == rhs.fAnchorLock &&
           lhs.unused8 == rhs.unused8 &&
           lhs.rgdp == rhs.rgdp;
}

bool operator!=(const DO &lhs, const DO &rhs)
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
    fReadOnlyRecommended = shifterU8;
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
    fWriteReservation = shifterU8;
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
    nRevision = stream->readU16();
    tmEdited = stream->readU32();
    cWords = stream->readU32();
    cCh = stream->readU32();
    cPg = stream->readU16();
    cParas = stream->readU32();
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
    cLines = stream->readU32();
    cWordsFtnEnd = stream->readU32();
    cChFtnEdn = stream->readU32();
    cPgFtnEdn = stream->readU16();
    cParasFtnEdn = stream->readU32();
    cLinesFtnEdn = stream->readU32();
    lKeyProtDoc = stream->readU32();
    shifterU16 = stream->readU16();
    wvkSaved = shifterU16;
    shifterU16 >>= 3;
    wScaleSaved = shifterU16;
    shifterU16 >>= 9;
    zkSaved = shifterU16;
    shifterU16 >>= 2;
    unused82_14 = shifterU16;

    if (preservePos)
        stream->pop();
    return true;
}

void DOP::clear()
{
    fFacingPages = 0;
    fWidowControl = 0;
    fPMHMainDoc = 0;
    grfSuppression = 0;
    fpc = 0;
    unused0_7 = 0;
    grpfIhdt = 0;
    rncFtn = 0;
    nFtn = 0;
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
    fReadOnlyRecommended = 0;
    fDfltTrueType = 0;
    fPagSuppressTopSpacing = 0;
    fProtEnabled = 0;
    fDispFormFldSel = 0;
    fRMView = 0;
    fRMPrint = 0;
    fWriteReservation = 0;
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
    dxaTab = 0;
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
    unused82_14 = 0;
}

bool operator==(const DOP &lhs, const DOP &rhs)
{

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
           lhs.fReadOnlyRecommended == rhs.fReadOnlyRecommended &&
           lhs.fDfltTrueType == rhs.fDfltTrueType &&
           lhs.fPagSuppressTopSpacing == rhs.fPagSuppressTopSpacing &&
           lhs.fProtEnabled == rhs.fProtEnabled &&
           lhs.fDispFormFldSel == rhs.fDispFormFldSel &&
           lhs.fRMView == rhs.fRMView &&
           lhs.fRMPrint == rhs.fRMPrint &&
           lhs.fWriteReservation == rhs.fWriteReservation &&
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
           lhs.unused82_14 == rhs.unused82_14;
}

bool operator!=(const DOP &lhs, const DOP &rhs)
{
    return !(lhs == rhs);
}


// DPARC implementation

DPARC::DPARC()
{
    clear();
}

DPARC::DPARC(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

bool DPARC::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    U16 shifterU16;

    if (preservePos)
        stream->push();

    dphead.read(stream, false);
    lnpc = stream->readU32();
    lnpw = stream->readU16();
    lnps = stream->readU16();
    dlpcFg = stream->readU32();
    dlpcBg = stream->readU32();
    flpp = stream->readU16();
    shdwpi = stream->readU16();
    xaOffset = stream->readU16();
    yaOffset = stream->readU16();
    shifterU16 = stream->readU16();
    fLeft = shifterU16;
    shifterU16 >>= 8;
    fUp = shifterU16;

    if (preservePos)
        stream->pop();
    return true;
}

void DPARC::clear()
{
    dphead.clear();
    lnpc = 0;
    lnpw = 0;
    lnps = 0;
    dlpcFg = 0;
    dlpcBg = 0;
    flpp = 0;
    shdwpi = 0;
    xaOffset = 0;
    yaOffset = 0;
    fLeft = 0;
    fUp = 0;
}

bool operator==(const DPARC &lhs, const DPARC &rhs)
{

    return lhs.dphead == rhs.dphead &&
           lhs.lnpc == rhs.lnpc &&
           lhs.lnpw == rhs.lnpw &&
           lhs.lnps == rhs.lnps &&
           lhs.dlpcFg == rhs.dlpcFg &&
           lhs.dlpcBg == rhs.dlpcBg &&
           lhs.flpp == rhs.flpp &&
           lhs.shdwpi == rhs.shdwpi &&
           lhs.xaOffset == rhs.xaOffset &&
           lhs.yaOffset == rhs.yaOffset &&
           lhs.fLeft == rhs.fLeft &&
           lhs.fUp == rhs.fUp;
}

bool operator!=(const DPARC &lhs, const DPARC &rhs)
{
    return !(lhs == rhs);
}


// DPCALLOUT implementation

DPCALLOUT::DPCALLOUT()
{
    clear();
}

DPCALLOUT::DPCALLOUT(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

bool DPCALLOUT::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    if (preservePos)
        stream->push();

    dphead.read(stream, false);
    unused12 = stream->readU16();
    dzaOffset = stream->readU16();
    dzaDescent = stream->readU16();
    dzaLength = stream->readU16();
    dptxbx.read(stream, false);
    dpPolyLine.read(stream, false);

    if (preservePos)
        stream->pop();
    return true;
}

void DPCALLOUT::clear()
{
    dphead.clear();
    unused12 = 0;
    dzaOffset = 0;
    dzaDescent = 0;
    dzaLength = 0;
    dptxbx.clear();
    dpPolyLine.clear();
}

bool operator==(const DPCALLOUT &lhs, const DPCALLOUT &rhs)
{

    return lhs.dphead == rhs.dphead &&
           lhs.unused12 == rhs.unused12 &&
           lhs.dzaOffset == rhs.dzaOffset &&
           lhs.dzaDescent == rhs.dzaDescent &&
           lhs.dzaLength == rhs.dzaLength &&
           lhs.dptxbx == rhs.dptxbx &&
           lhs.dpPolyLine == rhs.dpPolyLine;
}

bool operator!=(const DPCALLOUT &lhs, const DPCALLOUT &rhs)
{
    return !(lhs == rhs);
}


// DPELLIPSE implementation

DPELLIPSE::DPELLIPSE()
{
    clear();
}

DPELLIPSE::DPELLIPSE(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

bool DPELLIPSE::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    if (preservePos)
        stream->push();

    dphead.read(stream, false);
    lnpc = stream->readU32();
    lnpw = stream->readU16();
    lnps = stream->readU16();
    dlpcFg = stream->readU32();
    dlpcBg = stream->readU32();
    flpp = stream->readU16();
    shdwpi = stream->readU16();
    xaOffset = stream->readU16();
    yaOffset = stream->readU16();

    if (preservePos)
        stream->pop();
    return true;
}

void DPELLIPSE::clear()
{
    dphead.clear();
    lnpc = 0;
    lnpw = 0;
    lnps = 0;
    dlpcFg = 0;
    dlpcBg = 0;
    flpp = 0;
    shdwpi = 0;
    xaOffset = 0;
    yaOffset = 0;
}

bool operator==(const DPELLIPSE &lhs, const DPELLIPSE &rhs)
{

    return lhs.dphead == rhs.dphead &&
           lhs.lnpc == rhs.lnpc &&
           lhs.lnpw == rhs.lnpw &&
           lhs.lnps == rhs.lnps &&
           lhs.dlpcFg == rhs.dlpcFg &&
           lhs.dlpcBg == rhs.dlpcBg &&
           lhs.flpp == rhs.flpp &&
           lhs.shdwpi == rhs.shdwpi &&
           lhs.xaOffset == rhs.xaOffset &&
           lhs.yaOffset == rhs.yaOffset;
}

bool operator!=(const DPELLIPSE &lhs, const DPELLIPSE &rhs)
{
    return !(lhs == rhs);
}


// DPLINE implementation

DPLINE::DPLINE()
{
    clear();
}

DPLINE::DPLINE(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

bool DPLINE::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    U16 shifterU16;

    if (preservePos)
        stream->push();

    dphead.read(stream, false);
    xaStart = stream->readU16();
    yaStart = stream->readU16();
    xaEnd = stream->readU16();
    yaEnd = stream->readU16();
    lnpc = stream->readU32();
    lnpw = stream->readU16();
    lnps = stream->readU16();
    shifterU16 = stream->readU16();
    eppsStart = shifterU16;
    shifterU16 >>= 2;
    eppwStart = shifterU16;
    shifterU16 >>= 2;
    epplStart = shifterU16;
    shifterU16 >>= 2;
    unused24_6 = shifterU16;
    shifterU16 = stream->readU16();
    eppsEnd = shifterU16;
    shifterU16 >>= 2;
    eppwEnd = shifterU16;
    shifterU16 >>= 2;
    epplEnd = shifterU16;
    shifterU16 >>= 2;
    unused26_6 = shifterU16;
    shdwpi = stream->readU16();
    xaOffset = stream->readU16();
    yaOffset = stream->readU16();

    if (preservePos)
        stream->pop();
    return true;
}

void DPLINE::clear()
{
    dphead.clear();
    xaStart = 0;
    yaStart = 0;
    xaEnd = 0;
    yaEnd = 0;
    lnpc = 0;
    lnpw = 0;
    lnps = 0;
    eppsStart = 0;
    eppwStart = 0;
    epplStart = 0;
    unused24_6 = 0;
    eppsEnd = 0;
    eppwEnd = 0;
    epplEnd = 0;
    unused26_6 = 0;
    shdwpi = 0;
    xaOffset = 0;
    yaOffset = 0;
}

bool operator==(const DPLINE &lhs, const DPLINE &rhs)
{

    return lhs.dphead == rhs.dphead &&
           lhs.xaStart == rhs.xaStart &&
           lhs.yaStart == rhs.yaStart &&
           lhs.xaEnd == rhs.xaEnd &&
           lhs.yaEnd == rhs.yaEnd &&
           lhs.lnpc == rhs.lnpc &&
           lhs.lnpw == rhs.lnpw &&
           lhs.lnps == rhs.lnps &&
           lhs.eppsStart == rhs.eppsStart &&
           lhs.eppwStart == rhs.eppwStart &&
           lhs.epplStart == rhs.epplStart &&
           lhs.unused24_6 == rhs.unused24_6 &&
           lhs.eppsEnd == rhs.eppsEnd &&
           lhs.eppwEnd == rhs.eppwEnd &&
           lhs.epplEnd == rhs.epplEnd &&
           lhs.unused26_6 == rhs.unused26_6 &&
           lhs.shdwpi == rhs.shdwpi &&
           lhs.xaOffset == rhs.xaOffset &&
           lhs.yaOffset == rhs.yaOffset;
}

bool operator!=(const DPLINE &lhs, const DPLINE &rhs)
{
    return !(lhs == rhs);
}


// DPRECT implementation

DPRECT::DPRECT()
{
    clear();
}

DPRECT::DPRECT(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

bool DPRECT::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    U16 shifterU16;

    if (preservePos)
        stream->push();

    dphead.read(stream, false);
    lnpc = stream->readU32();
    lnpw = stream->readU16();
    lnps = stream->readU16();
    dlpcFg = stream->readU32();
    dlpcBg = stream->readU32();
    flpp = stream->readU16();
    shdwpi = stream->readU16();
    xaOffset = stream->readU16();
    yaOffset = stream->readU16();
    shifterU16 = stream->readU16();
    fRoundCorners = shifterU16;
    shifterU16 >>= 1;
    zaShape = shifterU16;

    if (preservePos)
        stream->pop();
    return true;
}

void DPRECT::clear()
{
    dphead.clear();
    lnpc = 0;
    lnpw = 0;
    lnps = 0;
    dlpcFg = 0;
    dlpcBg = 0;
    flpp = 0;
    shdwpi = 0;
    xaOffset = 0;
    yaOffset = 0;
    fRoundCorners = 0;
    zaShape = 0;
}

bool operator==(const DPRECT &lhs, const DPRECT &rhs)
{

    return lhs.dphead == rhs.dphead &&
           lhs.lnpc == rhs.lnpc &&
           lhs.lnpw == rhs.lnpw &&
           lhs.lnps == rhs.lnps &&
           lhs.dlpcFg == rhs.dlpcFg &&
           lhs.dlpcBg == rhs.dlpcBg &&
           lhs.flpp == rhs.flpp &&
           lhs.shdwpi == rhs.shdwpi &&
           lhs.xaOffset == rhs.xaOffset &&
           lhs.yaOffset == rhs.yaOffset &&
           lhs.fRoundCorners == rhs.fRoundCorners &&
           lhs.zaShape == rhs.zaShape;
}

bool operator!=(const DPRECT &lhs, const DPRECT &rhs)
{
    return !(lhs == rhs);
}


// DPSAMPLE implementation

DPSAMPLE::DPSAMPLE()
{
    clear();
}

DPSAMPLE::DPSAMPLE(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

bool DPSAMPLE::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    U16 shifterU16;

    if (preservePos)
        stream->push();

    dphead.read(stream, false);
    lnpc = stream->readU32();
    lnpw = stream->readU16();
    lnps = stream->readU16();
    dlpcFg = stream->readU32();
    dlpcBg = stream->readU32();
    flpp = stream->readU16();
    shifterU16 = stream->readU16();
    eppsStart = shifterU16;
    shifterU16 >>= 2;
    eppwStart = shifterU16;
    shifterU16 >>= 2;
    epplStart = shifterU16;
    shifterU16 >>= 2;
    unused30_6 = shifterU16;
    shifterU16 = stream->readU16();
    eppsEnd = shifterU16;
    shifterU16 >>= 2;
    eppwEnd = shifterU16;
    shifterU16 >>= 2;
    epplEnd = shifterU16;
    shifterU16 >>= 2;
    unused32_6 = shifterU16;
    shdwpi = stream->readU16();
    xaOffset = stream->readU16();
    yaOffset = stream->readU16();
    unused40 = stream->readU16();
    dzaOffset = stream->readU16();
    dzaDescent = stream->readU16();
    dzaLength = stream->readU16();
    shifterU16 = stream->readU16();
    fRoundCorners = shifterU16;
    shifterU16 >>= 1;
    zaShape = shifterU16;
    dzaInternalMargin = stream->readU16();

    if (preservePos)
        stream->pop();
    return true;
}

void DPSAMPLE::clear()
{
    dphead.clear();
    lnpc = 0;
    lnpw = 0;
    lnps = 0;
    dlpcFg = 0;
    dlpcBg = 0;
    flpp = 0;
    eppsStart = 0;
    eppwStart = 0;
    epplStart = 0;
    unused30_6 = 0;
    eppsEnd = 0;
    eppwEnd = 0;
    epplEnd = 0;
    unused32_6 = 0;
    shdwpi = 0;
    xaOffset = 0;
    yaOffset = 0;
    unused40 = 0;
    dzaOffset = 0;
    dzaDescent = 0;
    dzaLength = 0;
    fRoundCorners = 0;
    zaShape = 0;
    dzaInternalMargin = 0;
}

bool operator==(const DPSAMPLE &lhs, const DPSAMPLE &rhs)
{

    return lhs.dphead == rhs.dphead &&
           lhs.lnpc == rhs.lnpc &&
           lhs.lnpw == rhs.lnpw &&
           lhs.lnps == rhs.lnps &&
           lhs.dlpcFg == rhs.dlpcFg &&
           lhs.dlpcBg == rhs.dlpcBg &&
           lhs.flpp == rhs.flpp &&
           lhs.eppsStart == rhs.eppsStart &&
           lhs.eppwStart == rhs.eppwStart &&
           lhs.epplStart == rhs.epplStart &&
           lhs.unused30_6 == rhs.unused30_6 &&
           lhs.eppsEnd == rhs.eppsEnd &&
           lhs.eppwEnd == rhs.eppwEnd &&
           lhs.epplEnd == rhs.epplEnd &&
           lhs.unused32_6 == rhs.unused32_6 &&
           lhs.shdwpi == rhs.shdwpi &&
           lhs.xaOffset == rhs.xaOffset &&
           lhs.yaOffset == rhs.yaOffset &&
           lhs.unused40 == rhs.unused40 &&
           lhs.dzaOffset == rhs.dzaOffset &&
           lhs.dzaDescent == rhs.dzaDescent &&
           lhs.dzaLength == rhs.dzaLength &&
           lhs.fRoundCorners == rhs.fRoundCorners &&
           lhs.zaShape == rhs.zaShape &&
           lhs.dzaInternalMargin == rhs.dzaInternalMargin;
}

bool operator!=(const DPSAMPLE &lhs, const DPSAMPLE &rhs)
{
    return !(lhs == rhs);
}


// FDOA implementation

FDOA::FDOA()
{
    clear();
}

FDOA::FDOA(AbstractOLEStreamReader *stream, bool preservePos)
{
    clear();
    read(stream, preservePos);
}

bool FDOA::read(AbstractOLEStreamReader *stream, bool preservePos)
{

    if (preservePos)
        stream->push();

    fc = stream->readU32();
    ctxbx = stream->readU16();

    if (preservePos)
        stream->pop();
    return true;
}

void FDOA::clear()
{
    fc = 0;
    ctxbx = 0;
}

bool operator==(const FDOA &lhs, const FDOA &rhs)
{

    return lhs.fc == rhs.fc &&
           lhs.ctxbx == rhs.ctxbx;
}

bool operator!=(const FDOA &lhs, const FDOA &rhs)
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

    U16 shifterU16;

    if (preservePos)
        stream->push();

    wIdent = stream->readU16();
    nFib = stream->readU16();
    nProduct = stream->readU16();
    lid = stream->readU16();
    pnNext = stream->readU16();
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
    unused10_9 = shifterU16;
    shifterU16 >>= 1;
    fReadOnlyRecommended = shifterU16;
    shifterU16 >>= 1;
    fWriteReservation = shifterU16;
    shifterU16 >>= 1;
    fExtChar = shifterU16;
    shifterU16 >>= 1;
    unused10_13 = shifterU16;
    nFibBack = stream->readU16();
    lKey = stream->readU32();
    envr = stream->readU8();
    unused19 = stream->readU8();
    chse = stream->readU16();
    chseTables = stream->readU16();
    fcMin = stream->readU32();
    fcMac = stream->readU32();
    cbMac = stream->readU32();
    fcSpare0 = stream->readU32();
    fcSpare1 = stream->readU32();
    fcSpare2 = stream->readU32();
    fcSpare3 = stream->readU32();
    ccpText = stream->readU32();
    ccpFtn = stream->readU32();
    ccpHdd = stream->readU32();
    ccpMcr = stream->readU32();
    ccpAtn = stream->readU32();
    ccpEdn = stream->readU32();
    ccpTxbx = stream->readU32();
    ccpHdrTxbx = stream->readU32();
    ccpSpare2 = stream->readU32();
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
    fcGrpStAtnOwners = stream->readU32();
    lcbGrpStAtnOwners = stream->readU32();
    fcSttbfAtnbkmk = stream->readU32();
    lcbSttbfAtnbkmk = stream->readU32();
    wSpare4Fib = stream->readU16();
    pnChpFirst = stream->readU16();
    pnPapFirst = stream->readU16();
    cpnBteChp = stream->readU16();
    cpnBtePap = stream->readU16();
    fcPlcfdoaMom = stream->readU32();
    lcbPlcfdoaMom = stream->readU32();
    fcPlcfdoaHdr = stream->readU32();
    lcbPlcfdoaHdr = stream->readU32();
    fcUnused1 = stream->readU32();
    lcbUnused1 = stream->readU32();
    fcUnused2 = stream->readU32();
    lcbUnused2 = stream->readU32();
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
    fcUnused3 = stream->readU32();
    lcbUnused3 = stream->readU32();
    fcSttbfRMark = stream->readU32();
    lcbSttbfRMark = stream->readU32();
    fcSttbfCaption = stream->readU32();
    lcbSttbfCaption = stream->readU32();
    fcSttbfAutoCaption = stream->readU32();
    lcbSttbfAutoCaption = stream->readU32();
    fcPlcfwkb = stream->readU32();
    lcbPlcfwkb = stream->readU32();
    fcUnused4 = stream->readU32();
    lcbUnused4 = stream->readU32();
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
    unused10_9 = 0;
    fReadOnlyRecommended = 0;
    fWriteReservation = 0;
    fExtChar = 0;
    unused10_13 = 0;
    nFibBack = 0;
    lKey = 0;
    envr = 0;
    unused19 = 0;
    chse = 0;
    chseTables = 0;
    fcMin = 0;
    fcMac = 0;
    cbMac = 0;
    fcSpare0 = 0;
    fcSpare1 = 0;
    fcSpare2 = 0;
    fcSpare3 = 0;
    ccpText = 0;
    ccpFtn = 0;
    ccpHdd = 0;
    ccpMcr = 0;
    ccpAtn = 0;
    ccpEdn = 0;
    ccpTxbx = 0;
    ccpHdrTxbx = 0;
    ccpSpare2 = 0;
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
    fcGrpStAtnOwners = 0;
    lcbGrpStAtnOwners = 0;
    fcSttbfAtnbkmk = 0;
    lcbSttbfAtnbkmk = 0;
    wSpare4Fib = 0;
    pnChpFirst = 0;
    pnPapFirst = 0;
    cpnBteChp = 0;
    cpnBtePap = 0;
    fcPlcfdoaMom = 0;
    lcbPlcfdoaMom = 0;
    fcPlcfdoaHdr = 0;
    lcbPlcfdoaHdr = 0;
    fcUnused1 = 0;
    lcbUnused1 = 0;
    fcUnused2 = 0;
    lcbUnused2 = 0;
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
    fcUnused3 = 0;
    lcbUnused3 = 0;
    fcSttbfRMark = 0;
    lcbSttbfRMark = 0;
    fcSttbfCaption = 0;
    lcbSttbfCaption = 0;
    fcSttbfAutoCaption = 0;
    lcbSttbfAutoCaption = 0;
    fcPlcfwkb = 0;
    lcbPlcfwkb = 0;
    fcUnused4 = 0;
    lcbUnused4 = 0;
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
           lhs.unused10_9 == rhs.unused10_9 &&
           lhs.fReadOnlyRecommended == rhs.fReadOnlyRecommended &&
           lhs.fWriteReservation == rhs.fWriteReservation &&
           lhs.fExtChar == rhs.fExtChar &&
           lhs.unused10_13 == rhs.unused10_13 &&
           lhs.nFibBack == rhs.nFibBack &&
           lhs.lKey == rhs.lKey &&
           lhs.envr == rhs.envr &&
           lhs.unused19 == rhs.unused19 &&
           lhs.chse == rhs.chse &&
           lhs.chseTables == rhs.chseTables &&
           lhs.fcMin == rhs.fcMin &&
           lhs.fcMac == rhs.fcMac &&
           lhs.cbMac == rhs.cbMac &&
           lhs.fcSpare0 == rhs.fcSpare0 &&
           lhs.fcSpare1 == rhs.fcSpare1 &&
           lhs.fcSpare2 == rhs.fcSpare2 &&
           lhs.fcSpare3 == rhs.fcSpare3 &&
           lhs.ccpText == rhs.ccpText &&
           lhs.ccpFtn == rhs.ccpFtn &&
           lhs.ccpHdd == rhs.ccpHdd &&
           lhs.ccpMcr == rhs.ccpMcr &&
           lhs.ccpAtn == rhs.ccpAtn &&
           lhs.ccpEdn == rhs.ccpEdn &&
           lhs.ccpTxbx == rhs.ccpTxbx &&
           lhs.ccpHdrTxbx == rhs.ccpHdrTxbx &&
           lhs.ccpSpare2 == rhs.ccpSpare2 &&
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
           lhs.fcGrpStAtnOwners == rhs.fcGrpStAtnOwners &&
           lhs.lcbGrpStAtnOwners == rhs.lcbGrpStAtnOwners &&
           lhs.fcSttbfAtnbkmk == rhs.fcSttbfAtnbkmk &&
           lhs.lcbSttbfAtnbkmk == rhs.lcbSttbfAtnbkmk &&
           lhs.wSpare4Fib == rhs.wSpare4Fib &&
           lhs.pnChpFirst == rhs.pnChpFirst &&
           lhs.pnPapFirst == rhs.pnPapFirst &&
           lhs.cpnBteChp == rhs.cpnBteChp &&
           lhs.cpnBtePap == rhs.cpnBtePap &&
           lhs.fcPlcfdoaMom == rhs.fcPlcfdoaMom &&
           lhs.lcbPlcfdoaMom == rhs.lcbPlcfdoaMom &&
           lhs.fcPlcfdoaHdr == rhs.fcPlcfdoaHdr &&
           lhs.lcbPlcfdoaHdr == rhs.lcbPlcfdoaHdr &&
           lhs.fcUnused1 == rhs.fcUnused1 &&
           lhs.lcbUnused1 == rhs.lcbUnused1 &&
           lhs.fcUnused2 == rhs.fcUnused2 &&
           lhs.lcbUnused2 == rhs.lcbUnused2 &&
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
           lhs.fcUnused3 == rhs.fcUnused3 &&
           lhs.lcbUnused3 == rhs.lcbUnused3 &&
           lhs.fcSttbfRMark == rhs.fcSttbfRMark &&
           lhs.lcbSttbfRMark == rhs.lcbSttbfRMark &&
           lhs.fcSttbfCaption == rhs.fcSttbfCaption &&
           lhs.lcbSttbfCaption == rhs.lcbSttbfCaption &&
           lhs.fcSttbfAutoCaption == rhs.fcSttbfAutoCaption &&
           lhs.lcbSttbfAutoCaption == rhs.lcbSttbfAutoCaption &&
           lhs.fcPlcfwkb == rhs.fcPlcfwkb &&
           lhs.lcbPlcfwkb == rhs.lcbPlcfwkb &&
           lhs.fcUnused4 == rhs.fcUnused4 &&
           lhs.lcbUnused4 == rhs.lcbUnused4 &&
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
           lhs.lcbSttbFnm == rhs.lcbSttbFnm;
}

bool operator!=(const FIB &lhs, const FIB &rhs)
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

    dyaLine = stream->readU16();
    fMultLinespace = stream->readU16();

    if (preservePos)
        stream->pop();
    return true;
}

void LSPD::clear()
{
    dyaLine = 0;
    fMultLinespace = 0;
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
    s += uint2string( dyaLine );
    s += "\nfMultLinespace=";
    s += uint2string( fMultLinespace );
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

    mm = stream->readU16();
    xExt = stream->readU16();
    yExt = stream->readU16();
    hMF = stream->readU16();

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
    for (int _i = 0; _i < (64); ++_i)
        rgch[_i] = stream->readU8();

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
    for (int _i = 0; _i < (64); ++_i) {
        rgch[_i] = readU8(ptr);
        ptr += sizeof(U8);
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
    for (int _i = 0; _i < (64); ++_i)
        rgch[_i] = 0;
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
    for (int _i = 0; _i < (64); ++_i) {
        s += "\nrgch[" + int2string( _i ) + "]=";
        s += uint2string( rgch[_i] );
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

    for (int _i = 0; _i < (64); ++_i) {
        if (lhs.rgch[_i] != rhs.rgch[_i])
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
    clearInternal();
}

PAP::PAP(AbstractOLEStreamReader *stream, bool preservePos) : Shared()
{
    clearInternal();
    read(stream, preservePos);
}

PAP::PAP(const PAP &rhs) : Shared()
{
    istd = rhs.istd;
    jc = rhs.jc;
    fKeep = rhs.fKeep;
    fKeepFollow = rhs.fKeepFollow;
    fPageBreakBefore = rhs.fPageBreakBefore;
    fBrLnAbove = rhs.fBrLnAbove;
    fBrLnBelow = rhs.fBrLnBelow;
    fUnused = rhs.fUnused;
    pcVert = rhs.pcVert;
    pcHorz = rhs.pcHorz;
    brcp = rhs.brcp;
    brcl = rhs.brcl;
    unused9 = rhs.unused9;
    nLvlAnm = rhs.nLvlAnm;
    fNoLnn = rhs.fNoLnn;
    fSideBySide = rhs.fSideBySide;
    dxaRight = rhs.dxaRight;
    dxaLeft = rhs.dxaLeft;
    dxaLeft1 = rhs.dxaLeft1;
    lspd = rhs.lspd;
    dyaBefore = rhs.dyaBefore;
    dyaAfter = rhs.dyaAfter;
    phe = rhs.phe;
    fAutoHyph = rhs.fAutoHyph;
    fWidowControl = rhs.fWidowControl;
    fInTable = rhs.fInTable;
    fTtp = rhs.fTtp;
    ptap = rhs.ptap;
    dxaAbs = rhs.dxaAbs;
    dyaAbs = rhs.dyaAbs;
    dxaWidth = rhs.dxaWidth;
    brcTop = rhs.brcTop;
    brcLeft = rhs.brcLeft;
    brcBottom = rhs.brcBottom;
    brcRight = rhs.brcRight;
    brcBetween = rhs.brcBetween;
    brcBar = rhs.brcBar;
    dxaFromText = rhs.dxaFromText;
    dyaFromText = rhs.dyaFromText;
    wr = rhs.wr;
    fLocked = rhs.fLocked;
    dyaHeight = rhs.dyaHeight;
    fMinHeight = rhs.fMinHeight;
    shd = rhs.shd;
    dcs = rhs.dcs;
    anld = rhs.anld;
    itbdMac = rhs.itbdMac;
    rgdxaTab = new U16[itbdMac];
    memcpy(rgdxaTab, rhs.rgdxaTab, sizeof(U16) * (itbdMac));
    rgtbd = new U8[itbdMac];
    memcpy(rgtbd, rhs.rgtbd, sizeof(U8) * (itbdMac));
}

PAP::~PAP()
{
    delete [] rgdxaTab;
    delete [] rgtbd;
}

PAP &PAP::operator=(const PAP &rhs)
{

    // Check for assignment to self
    if (this == &rhs)
        return *this;

    istd = rhs.istd;
    jc = rhs.jc;
    fKeep = rhs.fKeep;
    fKeepFollow = rhs.fKeepFollow;
    fPageBreakBefore = rhs.fPageBreakBefore;
    fBrLnAbove = rhs.fBrLnAbove;
    fBrLnBelow = rhs.fBrLnBelow;
    fUnused = rhs.fUnused;
    pcVert = rhs.pcVert;
    pcHorz = rhs.pcHorz;
    brcp = rhs.brcp;
    brcl = rhs.brcl;
    unused9 = rhs.unused9;
    nLvlAnm = rhs.nLvlAnm;
    fNoLnn = rhs.fNoLnn;
    fSideBySide = rhs.fSideBySide;
    dxaRight = rhs.dxaRight;
    dxaLeft = rhs.dxaLeft;
    dxaLeft1 = rhs.dxaLeft1;
    lspd = rhs.lspd;
    dyaBefore = rhs.dyaBefore;
    dyaAfter = rhs.dyaAfter;
    phe = rhs.phe;
    fAutoHyph = rhs.fAutoHyph;
    fWidowControl = rhs.fWidowControl;
    fInTable = rhs.fInTable;
    fTtp = rhs.fTtp;
    ptap = rhs.ptap;
    dxaAbs = rhs.dxaAbs;
    dyaAbs = rhs.dyaAbs;
    dxaWidth = rhs.dxaWidth;
    brcTop = rhs.brcTop;
    brcLeft = rhs.brcLeft;
    brcBottom = rhs.brcBottom;
    brcRight = rhs.brcRight;
    brcBetween = rhs.brcBetween;
    brcBar = rhs.brcBar;
    dxaFromText = rhs.dxaFromText;
    dyaFromText = rhs.dyaFromText;
    wr = rhs.wr;
    fLocked = rhs.fLocked;
    dyaHeight = rhs.dyaHeight;
    fMinHeight = rhs.fMinHeight;
    shd = rhs.shd;
    dcs = rhs.dcs;
    anld = rhs.anld;
    itbdMac = rhs.itbdMac;
    delete [] rgdxaTab;
    rgdxaTab = new U16[itbdMac];
    memcpy(rgdxaTab, rhs.rgdxaTab, sizeof(U16) * (itbdMac));
    delete [] rgtbd;
    rgtbd = new U8[itbdMac];
    memcpy(rgtbd, rhs.rgtbd, sizeof(U8) * (itbdMac));

    return *this;
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
    nLvlAnm = stream->readU8();
    fNoLnn = stream->readU8();
    fSideBySide = stream->readU8();
    dxaRight = stream->readS16();
    dxaLeft = stream->readS16();
    dxaLeft1 = stream->readS16();
    lspd.read(stream, false);
    dyaBefore = stream->readU16();
    dyaAfter = stream->readU16();
    phe.read(stream, false);
    fAutoHyph = stream->readU8();
    fWidowControl = stream->readU8();
    fInTable = stream->readU8();
    fTtp = stream->readU8();
    ptap = stream->readU16();
    dxaAbs = stream->readS16();
    dyaAbs = stream->readS16();
    dxaWidth = stream->readU16();
    brcTop.read(stream, false);
    brcLeft.read(stream, false);
    brcBottom.read(stream, false);
    brcRight.read(stream, false);
    brcBetween.read(stream, false);
    brcBar.read(stream, false);
    dxaFromText = stream->readU16();
    dyaFromText = stream->readU16();
    wr = stream->readU8();
    fLocked = stream->readU8();
    shifterU16 = stream->readU16();
    dyaHeight = shifterU16;
    shifterU16 >>= 15;
    fMinHeight = shifterU16;
    shd.read(stream, false);
    dcs.read(stream, false);
    anld.read(stream, false);
    itbdMac = stream->readU16();
    rgdxaTab = new U16[itbdMac];
    for (int _i = 0; _i < (itbdMac); ++_i)
        rgdxaTab[_i] = stream->readU16();
    rgtbd = new U8[itbdMac];
    for (int _i = 0; _i < (itbdMac); ++_i)
        rgtbd[_i] = stream->readU8();

    if (preservePos)
        stream->pop();
    return true;
}

void PAP::clear()
{
    delete [] rgdxaTab;
    delete [] rgtbd;
    clearInternal();
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
    s += "\nnLvlAnm=";
    s += uint2string( nLvlAnm );
    s += "\nfNoLnn=";
    s += uint2string( fNoLnn );
    s += "\nfSideBySide=";
    s += uint2string( fSideBySide );
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
    s += "\nfAutoHyph=";
    s += uint2string( fAutoHyph );
    s += "\nfWidowControl=";
    s += uint2string( fWidowControl );
    s += "\nfInTable=";
    s += uint2string( fInTable );
    s += "\nfTtp=";
    s += uint2string( fTtp );
    s += "\nptap=";
    s += uint2string( ptap );
    s += "\ndxaAbs=";
    s += int2string( dxaAbs );
    s += "\ndyaAbs=";
    s += int2string( dyaAbs );
    s += "\ndxaWidth=";
    s += uint2string( dxaWidth );
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
    s += uint2string( dxaFromText );
    s += "\ndyaFromText=";
    s += uint2string( dyaFromText );
    s += "\nwr=";
    s += uint2string( wr );
    s += "\nfLocked=";
    s += uint2string( fLocked );
    s += "\ndyaHeight=";
    s += uint2string( dyaHeight );
    s += "\nfMinHeight=";
    s += uint2string( fMinHeight );
    s += "\nshd=";
    s += "\n{" + shd.toString() + "}\n";
    s += "\ndcs=";
    s += "\n{" + dcs.toString() + "}\n";
    s += "\nanld=";
    s += "\n{" + anld.toString() + "}\n";
    s += "\nitbdMac=";
    s += uint2string( itbdMac );
    for (int _i = 0; _i < (itbdMac); ++_i) {
        s += "\nrgdxaTab[" + int2string( _i ) + "]=";
        s += uint2string( rgdxaTab[_i] );
    }
    for (int _i = 0; _i < (itbdMac); ++_i) {
        s += "\nrgtbd[" + int2string( _i ) + "]=";
        s += uint2string( rgtbd[_i] );
    }
    s += "\nPAP Done.";
    return s;
}

void PAP::clearInternal()
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
    nLvlAnm = 0;
    fNoLnn = 0;
    fSideBySide = 0;
    dxaRight = 0;
    dxaLeft = 0;
    dxaLeft1 = 0;
    lspd.clear();
    dyaBefore = 0;
    dyaAfter = 0;
    phe.clear();
    fAutoHyph = 0;
    fWidowControl = 0;
    fInTable = 0;
    fTtp = 0;
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
    wr = 0;
    fLocked = 0;
    dyaHeight = 0;
    fMinHeight = 0;
    shd.clear();
    dcs.clear();
    anld.clear();
    itbdMac = 0;
    rgdxaTab = 0;
    rgtbd = 0;
}

bool operator==(const PAP &lhs, const PAP &rhs)
{

    if ((lhs.itbdMac) != (rhs.itbdMac))
        return false;
    for (int _i = 0; _i < (lhs.itbdMac); ++_i) {
        if (lhs.rgdxaTab[_i] != rhs.rgdxaTab[_i])
            return false;
    }

    if ((lhs.itbdMac) != (rhs.itbdMac))
        return false;
    for (int _i = 0; _i < (lhs.itbdMac); ++_i) {
        if (lhs.rgtbd[_i] != rhs.rgtbd[_i])
            return false;
    }

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
           lhs.nLvlAnm == rhs.nLvlAnm &&
           lhs.fNoLnn == rhs.fNoLnn &&
           lhs.fSideBySide == rhs.fSideBySide &&
           lhs.dxaRight == rhs.dxaRight &&
           lhs.dxaLeft == rhs.dxaLeft &&
           lhs.dxaLeft1 == rhs.dxaLeft1 &&
           lhs.lspd == rhs.lspd &&
           lhs.dyaBefore == rhs.dyaBefore &&
           lhs.dyaAfter == rhs.dyaAfter &&
           lhs.phe == rhs.phe &&
           lhs.fAutoHyph == rhs.fAutoHyph &&
           lhs.fWidowControl == rhs.fWidowControl &&
           lhs.fInTable == rhs.fInTable &&
           lhs.fTtp == rhs.fTtp &&
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
           lhs.wr == rhs.wr &&
           lhs.fLocked == rhs.fLocked &&
           lhs.dyaHeight == rhs.dyaHeight &&
           lhs.fMinHeight == rhs.fMinHeight &&
           lhs.shd == rhs.shd &&
           lhs.dcs == rhs.dcs &&
           lhs.anld == rhs.anld &&
           lhs.itbdMac == rhs.itbdMac;
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
    unused0_0 = shifterU16;
    shifterU16 >>= 5;
    fGhost = shifterU16;
    shifterU16 >>= 2;
    unused0_7 = shifterU16;
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
    fColOnly = shifterU16;
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

    if (preservePos)
        stream->pop();
    return true;
}

void PGD::clear()
{
    unused0_0 = 0;
    fGhost = 0;
    unused0_7 = 0;
    fContinue = 0;
    fUnk = 0;
    fRight = 0;
    fPgnRestart = 0;
    fEmptyPage = 0;
    fAllFtn = 0;
    fColOnly = 0;
    fTableBreaks = 0;
    fMarked = 0;
    fColumnBreaks = 0;
    fTableHeader = 0;
    fNewPage = 0;
    bkc = 0;
    lnn = 0;
    pgn = 0;
}

bool operator==(const PGD &lhs, const PGD &rhs)
{

    return lhs.unused0_0 == rhs.unused0_0 &&
           lhs.fGhost == rhs.fGhost &&
           lhs.unused0_7 == rhs.unused0_7 &&
           lhs.fContinue == rhs.fContinue &&
           lhs.fUnk == rhs.fUnk &&
           lhs.fRight == rhs.fRight &&
           lhs.fPgnRestart == rhs.fPgnRestart &&
           lhs.fEmptyPage == rhs.fEmptyPage &&
           lhs.fAllFtn == rhs.fAllFtn &&
           lhs.fColOnly == rhs.fColOnly &&
           lhs.fTableBreaks == rhs.fTableBreaks &&
           lhs.fMarked == rhs.fMarked &&
           lhs.fColumnBreaks == rhs.fColumnBreaks &&
           lhs.fTableHeader == rhs.fTableHeader &&
           lhs.fNewPage == rhs.fNewPage &&
           lhs.bkc == rhs.bkc &&
           lhs.lnn == rhs.lnn &&
           lhs.pgn == rhs.pgn;
}

bool operator!=(const PGD &lhs, const PGD &rhs)
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
    dxaGoal = stream->readU16();
    dyaGoal = stream->readU16();
    mx = stream->readU16();
    my = stream->readU16();
    dxaCropLeft = stream->readU16();
    dyaCropTop = stream->readU16();
    dxaCropRight = stream->readU16();
    dyaCropBottom = stream->readU16();
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
    dxaOrigin = stream->readU16();
    dyaOrigin = stream->readU16();

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
           lhs.dyaOrigin == rhs.dyaOrigin;
}

bool operator!=(const PICF &lhs, const PICF &rhs)
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

    U16 shifterU16;

    if (preservePos)
        stream->push();

    shifterU16 = stream->readU16();
    fSwap = shifterU16;
    shifterU16 >>= 1;
    fUnk = shifterU16;
    shifterU16 >>= 1;
    fn = shifterU16;
    fcSepx = stream->readU32();
    fnMpr = stream->readU16();
    fcMpr = stream->readU32();

    if (preservePos)
        stream->pop();
    return true;
}

void SED::clear()
{
    fSwap = 0;
    fUnk = 0;
    fn = 0;
    fcSepx = 0;
    fnMpr = 0;
    fcMpr = 0;
}

bool operator==(const SED &lhs, const SED &rhs)
{

    return lhs.fSwap == rhs.fSwap &&
           lhs.fUnk == rhs.fUnk &&
           lhs.fn == rhs.fn &&
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

    if (preservePos)
        stream->push();

    bkc = stream->readU8();
    fTitlePage = stream->readU8();
    ccolM1 = stream->readU16();
    dxaColumns = stream->readU16();
    fAutoPgn = stream->readU8();
    nfcPgn = stream->readU8();
    pgnStart = stream->readU16();
    fUnlocked = stream->readU8();
    cnsPgn = stream->readU8();
    fPgnRestart = stream->readU8();
    fEndNote = stream->readU8();
    lnc = stream->readU8();
    grpfIhdt = stream->readU8();
    nLnnMod = stream->readU16();
    dxaLnn = stream->readU16();
    dyaHdrTop = stream->readU16();
    dyaHdrBottom = stream->readU16();
    dxaPgn = stream->readU16();
    dyaPgn = stream->readU16();
    fLBetween = stream->readU8();
    vjc = stream->readU8();
    lnnMin = stream->readU16();
    dmOrientPage = stream->readU8();
    iHeadingPgn = stream->readU8();
    xaPage = stream->readU16();
    yaPage = stream->readU16();
    dxaLeft = stream->readU16();
    dxaRight = stream->readU16();
    dyaTop = stream->readU16();
    dyaBottom = stream->readU16();
    dzaGutter = stream->readU16();
    dmBinFirst = stream->readU16();
    dmBinOther = stream->readU16();
    dmPaperReq = stream->readU16();
    fEvenlySpaced = stream->readU8();
    unused55 = stream->readU8();
    dxaColumnWidth = stream->readU16();
    for (int _i = 0; _i < (89); ++_i)
        rgdxaColumnWidthSpacing[_i] = stream->readU16();
    olstAnm.read(stream, false);

    if (preservePos)
        stream->pop();
    return true;
}

void SEP::clear()
{
    bkc = 2;
    fTitlePage = 0;
    ccolM1 = 0;
    dxaColumns = 0;
    fAutoPgn = 0;
    nfcPgn = 0;
    pgnStart = 0;
    fUnlocked = 0;
    cnsPgn = 0;
    fPgnRestart = 0;
    fEndNote = true;
    lnc = 0;
    grpfIhdt = 0;
    nLnnMod = 0;
    dxaLnn = 0;
    dyaHdrTop = 720;
    dyaHdrBottom = 720;
    dxaPgn = 720;
    dyaPgn = 720;
    fLBetween = 0;
    vjc = 0;
    lnnMin = 0;
    dmOrientPage = 1;
    iHeadingPgn = 0;
    xaPage = 12240;
    yaPage = 15840;
    dxaLeft = 0;
    dxaRight = 0;
    dyaTop = 0;
    dyaBottom = 0;
    dzaGutter = 0;
    dmBinFirst = 0;
    dmBinOther = 0;
    dmPaperReq = 0;
    fEvenlySpaced = true;
    unused55 = 0;
    dxaColumnWidth = 0;
    for (int _i = 0; _i < (89); ++_i)
        rgdxaColumnWidthSpacing[_i] = 0;
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
    s += "\nccolM1=";
    s += uint2string( ccolM1 );
    s += "\ndxaColumns=";
    s += uint2string( dxaColumns );
    s += "\nfAutoPgn=";
    s += uint2string( fAutoPgn );
    s += "\nnfcPgn=";
    s += uint2string( nfcPgn );
    s += "\npgnStart=";
    s += uint2string( pgnStart );
    s += "\nfUnlocked=";
    s += uint2string( fUnlocked );
    s += "\ncnsPgn=";
    s += uint2string( cnsPgn );
    s += "\nfPgnRestart=";
    s += uint2string( fPgnRestart );
    s += "\nfEndNote=";
    s += uint2string( fEndNote );
    s += "\nlnc=";
    s += uint2string( lnc );
    s += "\ngrpfIhdt=";
    s += uint2string( grpfIhdt );
    s += "\nnLnnMod=";
    s += uint2string( nLnnMod );
    s += "\ndxaLnn=";
    s += uint2string( dxaLnn );
    s += "\ndyaHdrTop=";
    s += uint2string( dyaHdrTop );
    s += "\ndyaHdrBottom=";
    s += uint2string( dyaHdrBottom );
    s += "\ndxaPgn=";
    s += uint2string( dxaPgn );
    s += "\ndyaPgn=";
    s += uint2string( dyaPgn );
    s += "\nfLBetween=";
    s += uint2string( fLBetween );
    s += "\nvjc=";
    s += uint2string( vjc );
    s += "\nlnnMin=";
    s += uint2string( lnnMin );
    s += "\ndmOrientPage=";
    s += uint2string( dmOrientPage );
    s += "\niHeadingPgn=";
    s += uint2string( iHeadingPgn );
    s += "\nxaPage=";
    s += uint2string( xaPage );
    s += "\nyaPage=";
    s += uint2string( yaPage );
    s += "\ndxaLeft=";
    s += uint2string( dxaLeft );
    s += "\ndxaRight=";
    s += uint2string( dxaRight );
    s += "\ndyaTop=";
    s += uint2string( dyaTop );
    s += "\ndyaBottom=";
    s += uint2string( dyaBottom );
    s += "\ndzaGutter=";
    s += uint2string( dzaGutter );
    s += "\ndmBinFirst=";
    s += uint2string( dmBinFirst );
    s += "\ndmBinOther=";
    s += uint2string( dmBinOther );
    s += "\ndmPaperReq=";
    s += uint2string( dmPaperReq );
    s += "\nfEvenlySpaced=";
    s += uint2string( fEvenlySpaced );
    s += "\nunused55=";
    s += uint2string( unused55 );
    s += "\ndxaColumnWidth=";
    s += uint2string( dxaColumnWidth );
    for (int _i = 0; _i < (89); ++_i) {
        s += "\nrgdxaColumnWidthSpacing[" + int2string( _i ) + "]=";
        s += uint2string( rgdxaColumnWidthSpacing[_i] );
    }
    s += "\nolstAnm=";
    s += "\n{" + olstAnm.toString() + "}\n";
    s += "\nSEP Done.";
    return s;
}

bool operator==(const SEP &lhs, const SEP &rhs)
{

    for (int _i = 0; _i < (89); ++_i) {
        if (lhs.rgdxaColumnWidthSpacing[_i] != rhs.rgdxaColumnWidthSpacing[_i])
            return false;
    }

    return lhs.bkc == rhs.bkc &&
           lhs.fTitlePage == rhs.fTitlePage &&
           lhs.ccolM1 == rhs.ccolM1 &&
           lhs.dxaColumns == rhs.dxaColumns &&
           lhs.fAutoPgn == rhs.fAutoPgn &&
           lhs.nfcPgn == rhs.nfcPgn &&
           lhs.pgnStart == rhs.pgnStart &&
           lhs.fUnlocked == rhs.fUnlocked &&
           lhs.cnsPgn == rhs.cnsPgn &&
           lhs.fPgnRestart == rhs.fPgnRestart &&
           lhs.fEndNote == rhs.fEndNote &&
           lhs.lnc == rhs.lnc &&
           lhs.grpfIhdt == rhs.grpfIhdt &&
           lhs.nLnnMod == rhs.nLnnMod &&
           lhs.dxaLnn == rhs.dxaLnn &&
           lhs.dyaHdrTop == rhs.dyaHdrTop &&
           lhs.dyaHdrBottom == rhs.dyaHdrBottom &&
           lhs.dxaPgn == rhs.dxaPgn &&
           lhs.dyaPgn == rhs.dyaPgn &&
           lhs.fLBetween == rhs.fLBetween &&
           lhs.vjc == rhs.vjc &&
           lhs.lnnMin == rhs.lnnMin &&
           lhs.dmOrientPage == rhs.dmOrientPage &&
           lhs.iHeadingPgn == rhs.iHeadingPgn &&
           lhs.xaPage == rhs.xaPage &&
           lhs.yaPage == rhs.yaPage &&
           lhs.dxaLeft == rhs.dxaLeft &&
           lhs.dxaRight == rhs.dxaRight &&
           lhs.dyaTop == rhs.dyaTop &&
           lhs.dyaBottom == rhs.dyaBottom &&
           lhs.dzaGutter == rhs.dzaGutter &&
           lhs.dmBinFirst == rhs.dmBinFirst &&
           lhs.dmBinOther == rhs.dmBinOther &&
           lhs.dmPaperReq == rhs.dmPaperReq &&
           lhs.fEvenlySpaced == rhs.fEvenlySpaced &&
           lhs.unused55 == rhs.unused55 &&
           lhs.dxaColumnWidth == rhs.dxaColumnWidth &&
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

    cb = stream->readU8();
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

const unsigned int STSHI::sizeOf = 14;

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
    ftcStandardChpStsh = stream->readU16();

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
    ftcStandardChpStsh = 0;
}

bool operator==(const STSHI &lhs, const STSHI &rhs)
{

    return lhs.cstd == rhs.cstd &&
           lhs.cbSTDBaseInFile == rhs.cbSTDBaseInFile &&
           lhs.fStdStylenamesWritten == rhs.fStdStylenamesWritten &&
           lhs.unused4_2 == rhs.unused4_2 &&
           lhs.stiMaxWhenSaved == rhs.stiMaxWhenSaved &&
           lhs.istdMaxFixedWhenSaved == rhs.istdMaxFixedWhenSaved &&
           lhs.nVerBuiltInNamesWhenSaved == rhs.nVerBuiltInNamesWhenSaved &&
           lhs.ftcStandardChpStsh == rhs.ftcStandardChpStsh;
}

bool operator!=(const STSHI &lhs, const STSHI &rhs)
{
    return !(lhs == rhs);
}



} // namespace Word95

} // namespace wvWare
