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

#include "convert.h"

namespace wvWare {

namespace Word95 {

// This has been added to the template file, as the mapping is
// non-trivial. Shaheed: Please check the implementation
Word97::BRC toWord97(const Word95::BRC &s)
{

    Word97::BRC ret;

    // Check the BRC documentation
    if ( s.dxpLineWidth < 6 ) {
        ret.dptLineWidth = s.dxpLineWidth * 6;
        ret.brcType = s.brcType;
    } else if ( s.dxpLineWidth == 6 ) {
        ret.dptLineWidth = 6; // what's the default?
        ret.brcType = 6; // dotted
    } else { // s.dxpLineWidth == 7
        ret.dptLineWidth = 6; // what's the default?
        ret.brcType = 7;
    }
    ret.fShadow = s.fShadow;
    ret.ico = s.ico;
    ret.dptSpace = s.dxpSpace;
    return ret;
}

Word97::STSHI toWord97(const Word95::STSHI &s)
{
    Word97::STSHI ret;

    ret.cstd = s.cstd;
    ret.cbSTDBaseInFile = s.cbSTDBaseInFile;
    ret.fStdStylenamesWritten = s.fStdStylenamesWritten;
    ret.unused4_2 = s.unused4_2;
    ret.stiMaxWhenSaved = s.stiMaxWhenSaved;
    ret.istdMaxFixedWhenSaved = s.istdMaxFixedWhenSaved;
    ret.nVerBuiltInNamesWhenSaved = s.nVerBuiltInNamesWhenSaved;
    ret.rgftcStandardChpStsh[0] = s.ftcStandardChpStsh;
    ret.rgftcStandardChpStsh[1] = s.ftcStandardChpStsh; // fake them
    ret.rgftcStandardChpStsh[2] = s.ftcStandardChpStsh; // fake them

    return ret;
}

Word97::ANLD toWord97(const Word95::ANLD &s)
{

    Word97::ANLD ret;

    ret.nfc = s.nfc;
    ret.cxchTextBefore = s.cxchTextBefore;
    ret.cxchTextAfter = s.cxchTextAfter;
    ret.jc = s.jc;
    ret.fPrev = s.fPrev;
    ret.fHang = s.fHang;
    ret.fSetBold = s.fSetBold;
    ret.fSetItalic = s.fSetItalic;
    ret.fSetSmallCaps = s.fSetSmallCaps;
    ret.fSetCaps = s.fSetCaps;
    ret.fSetStrike = s.fSetStrike;
    ret.fSetKul = s.fSetKul;
    ret.fPrevSpace = s.fPrevSpace;
    ret.fBold = s.fBold;
    ret.fItalic = s.fItalic;
    ret.fSmallCaps = s.fSmallCaps;
    ret.fCaps = s.fCaps;
    ret.fStrike = s.fStrike;
    ret.kul = s.kul;
    ret.ico = s.ico;
    ret.ftc = s.ftc;
    ret.hps = s.hps;
    ret.iStartAt = s.iStartAt;
    ret.dxaIndent = s.dxaIndent;
    ret.dxaSpace = s.dxaSpace;
    ret.fNumber1 = s.fNumber1;
    ret.fNumberAcross = s.fNumberAcross;
    ret.fRestartHdn = s.fRestartHdn;
    ret.fSpareX = s.fSpareX;
    for (int i = 0; i < (32); ++i)
        ret.rgxch[i] = s.rgchAnld[i];

    return ret;
}

Word97::ANLV toWord97(const Word95::ANLV &s)
{

    Word97::ANLV ret;

    ret.nfc = s.nfc;
    ret.cxchTextBefore = s.cxchTextBefore;
    ret.cxchTextAfter = s.cxchTextAfter;
    ret.jc = s.jc;
    ret.fPrev = s.fPrev;
    ret.fHang = s.fHang;
    ret.fSetBold = s.fSetBold;
    ret.fSetItalic = s.fSetItalic;
    ret.fSetSmallCaps = s.fSetSmallCaps;
    ret.fSetCaps = s.fSetCaps;
    ret.fSetStrike = s.fSetStrike;
    ret.fSetKul = s.fSetKul;
    ret.fPrevSpace = s.fPrevSpace;
    ret.fBold = s.fBold;
    ret.fItalic = s.fItalic;
    ret.fSmallCaps = s.fSmallCaps;
    ret.fCaps = s.fCaps;
    ret.fStrike = s.fStrike;
    ret.kul = s.kul;
    ret.ico = s.ico;
    ret.ftc = s.ftc;
    ret.hps = s.hps;
    ret.iStartAt = s.iStartAt;
    ret.dxaIndent = s.dxaIndent;
    ret.dxaSpace = s.dxaSpace;

    return ret;
}

Word97::BKF toWord97(const Word95::BKF &s)
{

    Word97::BKF ret;

    ret.ibkl = s.ibkl;
    ret.itcFirst = s.itcFirst;
    ret.fPub = s.fPub;
    ret.itcLim = s.itcLim;
    ret.fCol = s.fCol;

    return ret;
}

Word97::BKL toWord97(const Word95::BKL &s)
{

    Word97::BKL ret;

    ret.ibkf = s.ibkf;

    return ret;
}

/* Please check...
Word97::BRC toWord97(const Word95::BRC &s) {

    Word97::BRC ret;

    ret.brcType=s.brcType;
    ret.fShadow=s.fShadow;
    ret.ico=s.ico;

    return ret;
} */

Word97::BRC10 toWord97(const Word95::BRC10 &s)
{

    Word97::BRC10 ret;

    ret.dxpLine2Width = s.dxpLine2Width;
    ret.dxpSpaceBetween = s.dxpSpaceBetween;
    ret.dxpLine1Width = s.dxpLine1Width;
    ret.dxpSpace = s.dxpSpace;
    ret.fShadow = s.fShadow;
    ret.fSpare = s.fSpare;

    return ret;
}

Word97::BTE toWord97(const Word95::BTE &s)
{

    Word97::BTE ret;

    ret.pn = s.pn;

    return ret;
}

Word97::CHP toWord97(const Word95::CHP &s)
{

    Word97::CHP ret;

    ret.fBold = s.fBold;
    ret.fItalic = s.fItalic;
    ret.fRMarkDel = s.fRMarkDel;
    ret.fOutline = s.fOutline;
    ret.fFldVanish = s.fFldVanish;
    ret.fSmallCaps = s.fSmallCaps;
    ret.fCaps = s.fCaps;
    ret.fVanish = s.fVanish;
    ret.fRMark = s.fRMark;
    ret.fSpec = s.fSpec;
    ret.fStrike = s.fStrike;
    ret.fObj = s.fObj;
    ret.fShadow = s.fShadow;
    ret.fLowerCase = s.fLowerCase;
    ret.fData = s.fData;
    ret.fOle2 = s.fOle2;
    ret.ftcAscii = s.ftc;
    ret.hps = s.hps;
    ret.dxaSpace = s.dxaSpace;
    ret.iss = s.iss;
    ret.fSysVanish = s.fSysVanish;
    ret.ico = s.ico;
    ret.kul = s.kul;
    ret.hpsPos = s.hpsPos;
    ret.lid = s.lid;
    ret.fcPic_fcObj_lTagObj = s.fcPic_fcObj_lTagObj;
    ret.ibstRMark = s.ibstRMark;
    ret.dttmRMark = toWord97(s.dttmRMark);
    ret.istd = s.istd;
    ret.ftcSym = s.ftcSym;
    ret.xchSym = s.chSym;
    ret.fChsDiff = s.fChsDiff;
    ret.idslRMReason = s.idslRMReason;
    ret.ysr = s.ysr;
    ret.chYsr = s.chYsr;
    ret.chse = s.chse;
    ret.hpsKern = s.hpsKern;

    return ret;
}

/* Please check...
Word97::CHPX toWord97(const Word95::CHPX &s) {

    Word97::CHPX ret;

    ret.cb=s.cb;
    ret.grpprl=s.grpprl;

    return ret;
} */

/* Please check...
Word97::CHPXFKP toWord97(const Word95::CHPXFKP &s) {

    Word97::CHPXFKP ret;

    ret.rgb=s.rgb;
    ret.unusedSpace=s.unusedSpace;
    ret.grpchpx=s.grpchpx;
    ret.crun=s.crun;

    return ret;
} */

Word97::DCS toWord97(const Word95::DCS &s)
{

    Word97::DCS ret;

    ret.fdct = s.fdct;
    ret.lines = s.lines;
    ret.unused1 = s.unused1;

    return ret;
}

Word97::DOP toWord97(const Word95::DOP &s)
{

    Word97::DOP ret;

    ret.fFacingPages = s.fFacingPages;
    ret.fWidowControl = s.fWidowControl;
    ret.fPMHMainDoc = s.fPMHMainDoc;
    ret.grfSuppression = s.grfSuppression;
    ret.fpc = s.fpc;
    ret.unused0_7 = s.unused0_7;
    ret.grpfIhdt = s.grpfIhdt;
    ret.rncFtn = s.rncFtn;
    ret.nFtn = s.nFtn;
    ret.fOutlineDirtySave = s.fOutlineDirtySave;
    ret.unused4_1 = s.unused4_1;
    ret.fOnlyMacPics = s.fOnlyMacPics;
    ret.fOnlyWinPics = s.fOnlyWinPics;
    ret.fLabelDoc = s.fLabelDoc;
    ret.fHyphCapitals = s.fHyphCapitals;
    ret.fAutoHyphen = s.fAutoHyphen;
    ret.fFormNoFields = s.fFormNoFields;
    ret.fLinkStyles = s.fLinkStyles;
    ret.fRevMarking = s.fRevMarking;
    ret.fBackup = s.fBackup;
    ret.fExactCWords = s.fExactCWords;
    ret.fPagHidden = s.fPagHidden;
    ret.fPagResults = s.fPagResults;
    ret.fLockAtn = s.fLockAtn;
    ret.fMirrorMargins = s.fMirrorMargins;
    ret.fDfltTrueType = s.fDfltTrueType;
    ret.fPagSuppressTopSpacing = s.fPagSuppressTopSpacing;
    ret.fProtEnabled = s.fProtEnabled;
    ret.fDispFormFldSel = s.fDispFormFldSel;
    ret.fRMView = s.fRMView;
    ret.fRMPrint = s.fRMPrint;
    ret.fLockRev = s.fLockRev;
    ret.fEmbedFonts = s.fEmbedFonts;
    ret.copts_fNoTabForInd = s.copts_fNoTabForInd;
    ret.copts_fNoSpaceRaiseLower = s.copts_fNoSpaceRaiseLower;
    ret.copts_fSuppressSpbfAfterPageBreak = s.copts_fSuppressSpbfAfterPageBreak;
    ret.copts_fWrapTrailSpaces = s.copts_fWrapTrailSpaces;
    ret.copts_fMapPrintTextColor = s.copts_fMapPrintTextColor;
    ret.copts_fNoColumnBalance = s.copts_fNoColumnBalance;
    ret.copts_fConvMailMergeEsc = s.copts_fConvMailMergeEsc;
    ret.copts_fSupressTopSpacing = s.copts_fSupressTopSpacing;
    ret.copts_fOrigWordTableRules = s.copts_fOrigWordTableRules;
    ret.copts_fTransparentMetafiles = s.copts_fTransparentMetafiles;
    ret.copts_fShowBreaksInFrames = s.copts_fShowBreaksInFrames;
    ret.copts_fSwapBordersFacingPgs = s.copts_fSwapBordersFacingPgs;
    ret.unused8_12 = s.unused8_12;
    ret.dxaTab = s.dxaTab;
    ret.wSpare = s.wSpare;
    ret.dxaHotZ = s.dxaHotZ;
    ret.cConsecHypLim = s.cConsecHypLim;
    ret.wSpare2 = s.wSpare2;
    ret.dttmCreated = toWord97(s.dttmCreated);
    ret.dttmRevised = toWord97(s.dttmRevised);
    ret.dttmLastPrint = toWord97(s.dttmLastPrint);
    ret.nRevision = s.nRevision;
    ret.tmEdited = s.tmEdited;
    ret.cWords = s.cWords;
    ret.cCh = s.cCh;
    ret.cPg = s.cPg;
    ret.cParas = s.cParas;
    ret.rncEdn = s.rncEdn;
    ret.nEdn = s.nEdn;
    ret.epc = s.epc;
    ret.nfcFtnRef2 = s.nfcFtnRef;
    ret.nfcEdnRef2 = s.nfcEdnRef;
    ret.fPrintFormData = s.fPrintFormData;
    ret.fSaveFormData = s.fSaveFormData;
    ret.fShadeFormData = s.fShadeFormData;
    ret.unused54_13 = s.unused54_13;
    ret.fWCFtnEdn = s.fWCFtnEdn;
    ret.cLines = s.cLines;
    ret.cWordsFtnEnd = s.cWordsFtnEnd;
    ret.cChFtnEdn = s.cChFtnEdn;
    ret.cPgFtnEdn = s.cPgFtnEdn;
    ret.cParasFtnEdn = s.cParasFtnEdn;
    ret.cLinesFtnEdn = s.cLinesFtnEdn;
    ret.lKeyProtDoc = s.lKeyProtDoc;
    ret.wvkSaved = s.wvkSaved;
    ret.wScaleSaved = s.wScaleSaved;
    ret.zkSaved = s.zkSaved;

    return ret;
}

Word97::DTTM toWord97(const Word95::DTTM &s)
{

    Word97::DTTM ret;

    ret.mint = s.mint;
    ret.hr = s.hr;
    ret.dom = s.dom;
    ret.mon = s.mon;
    ret.yr = s.yr;
    ret.wdy = s.wdy;

    return ret;
}

/* Please check...
Word97::FFN toWord97(const Word95::FFN &s) {

    Word97::FFN ret;

    ret.cbFfnM1=s.cbFfnM1;
    ret.prq=s.prq;
    ret.fTrueType=s.fTrueType;
    ret.unused1_3=s.unused1_3;
    ret.ff=s.ff;
    ret.unused1_7=s.unused1_7;
    ret.wWeight=s.wWeight;
    ret.chs=s.chs;
    ret.ixchSzAlt=s.ibszAlt;
    ret.xszFfn=s.szFfn;

    return ret;
} */

Word97::FIB toWord97(const Word95::FIB &s)
{

    Word97::FIB ret;

    ret.wIdent = s.wIdent;
    ret.nFib = s.nFib;
    ret.nProduct = s.nProduct;
    ret.lid = s.lid;
    ret.pnNext = s.pnNext;
    ret.fDot = s.fDot;
    ret.fGlsy = s.fGlsy;
    ret.fComplex = s.fComplex;
    ret.fHasPic = s.fHasPic;
    ret.cQuickSaves = s.cQuickSaves;
    ret.fEncrypted = s.fEncrypted;
    ret.fReadOnlyRecommended = s.fReadOnlyRecommended;
    ret.fWriteReservation = s.fWriteReservation;
    ret.fExtChar = s.fExtChar;
    ret.nFibBack = s.nFibBack;
    ret.lKey = s.lKey;
    ret.envr = s.envr;
    ret.chs = s.chse;
    ret.chsTables = s.chseTables;
    ret.fcMin = s.fcMin;
    ret.fcMac = s.fcMac;
    ret.cbMac = s.cbMac;
    ret.ccpText = s.ccpText;
    ret.ccpFtn = s.ccpFtn;
    ret.ccpHdd = s.ccpHdd;
    ret.ccpMcr = s.ccpMcr;
    ret.ccpAtn = s.ccpAtn;
    ret.ccpEdn = s.ccpEdn;
    ret.ccpTxbx = s.ccpTxbx;
    ret.ccpHdrTxbx = s.ccpHdrTxbx;
    ret.fcStshfOrig = s.fcStshfOrig;
    ret.lcbStshfOrig = s.lcbStshfOrig;
    ret.fcStshf = s.fcStshf;
    ret.lcbStshf = s.lcbStshf;
    ret.fcPlcffndRef = s.fcPlcffndRef;
    ret.lcbPlcffndRef = s.lcbPlcffndRef;
    ret.fcPlcffndTxt = s.fcPlcffndTxt;
    ret.lcbPlcffndTxt = s.lcbPlcffndTxt;
    ret.fcPlcfandRef = s.fcPlcfandRef;
    ret.lcbPlcfandRef = s.lcbPlcfandRef;
    ret.fcPlcfandTxt = s.fcPlcfandTxt;
    ret.lcbPlcfandTxt = s.lcbPlcfandTxt;
    ret.fcPlcfsed = s.fcPlcfsed;
    ret.lcbPlcfsed = s.lcbPlcfsed;
    ret.fcPlcfpad = s.fcPlcfpad;
    ret.lcbPlcfpad = s.lcbPlcfpad;
    ret.fcPlcfphe = s.fcPlcfphe;
    ret.lcbPlcfphe = s.lcbPlcfphe;
    ret.fcSttbfglsy = s.fcSttbfglsy;
    ret.lcbSttbfglsy = s.lcbSttbfglsy;
    ret.fcPlcfglsy = s.fcPlcfglsy;
    ret.lcbPlcfglsy = s.lcbPlcfglsy;
    ret.fcPlcfhdd = s.fcPlcfhdd;
    ret.lcbPlcfhdd = s.lcbPlcfhdd;
    ret.fcPlcfbteChpx = s.fcPlcfbteChpx;
    ret.lcbPlcfbteChpx = s.lcbPlcfbteChpx;
    ret.fcPlcfbtePapx = s.fcPlcfbtePapx;
    ret.lcbPlcfbtePapx = s.lcbPlcfbtePapx;
    ret.fcPlcfsea = s.fcPlcfsea;
    ret.lcbPlcfsea = s.lcbPlcfsea;
    ret.fcSttbfffn = s.fcSttbfffn;
    ret.lcbSttbfffn = s.lcbSttbfffn;
    ret.fcPlcffldMom = s.fcPlcffldMom;
    ret.lcbPlcffldMom = s.lcbPlcffldMom;
    ret.fcPlcffldHdr = s.fcPlcffldHdr;
    ret.lcbPlcffldHdr = s.lcbPlcffldHdr;
    ret.fcPlcffldFtn = s.fcPlcffldFtn;
    ret.lcbPlcffldFtn = s.lcbPlcffldFtn;
    ret.fcPlcffldAtn = s.fcPlcffldAtn;
    ret.lcbPlcffldAtn = s.lcbPlcffldAtn;
    ret.fcPlcffldMcr = s.fcPlcffldMcr;
    ret.lcbPlcffldMcr = s.lcbPlcffldMcr;
    ret.fcSttbfbkmk = s.fcSttbfbkmk;
    ret.lcbSttbfbkmk = s.lcbSttbfbkmk;
    ret.fcPlcfbkf = s.fcPlcfbkf;
    ret.lcbPlcfbkf = s.lcbPlcfbkf;
    ret.fcPlcfbkl = s.fcPlcfbkl;
    ret.lcbPlcfbkl = s.lcbPlcfbkl;
    ret.fcCmds = s.fcCmds;
    ret.lcbCmds = s.lcbCmds;
    ret.fcPlcmcr = s.fcPlcmcr;
    ret.lcbPlcmcr = s.lcbPlcmcr;
    ret.fcSttbfmcr = s.fcSttbfmcr;
    ret.lcbSttbfmcr = s.lcbSttbfmcr;
    ret.fcPrDrvr = s.fcPrDrvr;
    ret.lcbPrDrvr = s.lcbPrDrvr;
    ret.fcPrEnvPort = s.fcPrEnvPort;
    ret.lcbPrEnvPort = s.lcbPrEnvPort;
    ret.fcPrEnvLand = s.fcPrEnvLand;
    ret.lcbPrEnvLand = s.lcbPrEnvLand;
    ret.fcWss = s.fcWss;
    ret.lcbWss = s.lcbWss;
    ret.fcDop = s.fcDop;
    ret.lcbDop = s.lcbDop;
    ret.fcSttbfAssoc = s.fcSttbfAssoc;
    ret.lcbSttbfAssoc = s.lcbSttbfAssoc;
    ret.fcClx = s.fcClx;
    ret.lcbClx = s.lcbClx;
    ret.fcPlcfpgdFtn = s.fcPlcfpgdFtn;
    ret.lcbPlcfpgdFtn = s.lcbPlcfpgdFtn;
    ret.fcAutosaveSource = s.fcAutosaveSource;
    ret.lcbAutosaveSource = s.lcbAutosaveSource;
    ret.fcGrpXstAtnOwners = s.fcGrpStAtnOwners;
    ret.lcbGrpXstAtnOwners = s.lcbGrpStAtnOwners;
    ret.fcSttbfAtnbkmk = s.fcSttbfAtnbkmk;
    ret.lcbSttbfAtnbkmk = s.lcbSttbfAtnbkmk;
    ret.pnChpFirst = s.pnChpFirst;
    ret.pnPapFirst = s.pnPapFirst;
    ret.cpnBteChp = s.cpnBteChp;
    ret.cpnBtePap = s.cpnBtePap;
    ret.fcPlcdoaMom = s.fcPlcfdoaMom;
    ret.lcbPlcdoaMom = s.lcbPlcfdoaMom;
    ret.fcPlcdoaHdr = s.fcPlcfdoaHdr;
    ret.lcbPlcdoaHdr = s.lcbPlcfdoaHdr;
    ret.fcPlcfAtnbkf = s.fcPlcfAtnbkf;
    ret.lcbPlcfAtnbkf = s.lcbPlcfAtnbkf;
    ret.fcPlcfAtnbkl = s.fcPlcfAtnbkl;
    ret.lcbPlcfAtnbkl = s.lcbPlcfAtnbkl;
    ret.fcPms = s.fcPms;
    ret.lcbPms = s.lcbPms;
    ret.fcFormFldSttbf = s.fcFormFldSttbf;
    ret.lcbFormFldSttbf = s.lcbFormFldSttbf;
    ret.fcPlcfendRef = s.fcPlcfendRef;
    ret.lcbPlcfendRef = s.lcbPlcfendRef;
    ret.fcPlcfendTxt = s.fcPlcfendTxt;
    ret.lcbPlcfendTxt = s.lcbPlcfendTxt;
    ret.fcPlcffldEdn = s.fcPlcffldEdn;
    ret.lcbPlcffldEdn = s.lcbPlcffldEdn;
    ret.fcPlcfpgdEdn = s.fcPlcfpgdEdn;
    ret.lcbPlcfpgdEdn = s.lcbPlcfpgdEdn;
    ret.fcSttbfRMark = s.fcSttbfRMark;
    ret.lcbSttbfRMark = s.lcbSttbfRMark;
    ret.fcSttbfCaption = s.fcSttbfCaption;
    ret.lcbSttbfCaption = s.lcbSttbfCaption;
    ret.fcSttbfAutoCaption = s.fcSttbfAutoCaption;
    ret.lcbSttbfAutoCaption = s.lcbSttbfAutoCaption;
    ret.fcPlcfwkb = s.fcPlcfwkb;
    ret.lcbPlcfwkb = s.lcbPlcfwkb;
    ret.fcPlcftxbxTxt = s.fcPlcftxbxTxt;
    ret.lcbPlcftxbxTxt = s.lcbPlcftxbxTxt;
    ret.fcPlcffldTxbx = s.fcPlcffldTxbx;
    ret.lcbPlcffldTxbx = s.lcbPlcffldTxbx;
    ret.fcPlcfHdrtxbxTxt = s.fcPlcfHdrtxbxTxt;
    ret.lcbPlcfHdrtxbxTxt = s.lcbPlcfHdrtxbxTxt;
    ret.fcPlcffldHdrTxbx = s.fcPlcffldHdrTxbx;
    ret.lcbPlcffldHdrTxbx = s.lcbPlcffldHdrTxbx;
    ret.fcStwUser = s.fcStwUser;
    ret.lcbStwUser = s.lcbStwUser;
    ret.fcSttbttmbd = s.fcSttbttmbd;
    ret.lcbSttbttmbd = s.lcbSttbttmbd;
    ret.fcUnused = s.fcUnused;
    ret.lcbUnused = s.lcbUnused;
    ret.fcPgdMother = s.fcPgdMother;
    ret.lcbPgdMother = s.lcbPgdMother;
    ret.fcBkdMother = s.fcBkdMother;
    ret.lcbBkdMother = s.lcbBkdMother;
    ret.fcPgdFtn = s.fcPgdFtn;
    ret.lcbPgdFtn = s.lcbPgdFtn;
    ret.fcBkdFtn = s.fcBkdFtn;
    ret.lcbBkdFtn = s.lcbBkdFtn;
    ret.fcPgdEdn = s.fcPgdEdn;
    ret.lcbPgdEdn = s.lcbPgdEdn;
    ret.fcBkdEdn = s.fcBkdEdn;
    ret.lcbBkdEdn = s.lcbBkdEdn;
    ret.fcSttbfIntlFld = s.fcSttbfIntlFld;
    ret.lcbSttbfIntlFld = s.lcbSttbfIntlFld;
    ret.fcRouteSlip = s.fcRouteSlip;
    ret.lcbRouteSlip = s.lcbRouteSlip;
    ret.fcSttbSavedBy = s.fcSttbSavedBy;
    ret.lcbSttbSavedBy = s.lcbSttbSavedBy;
    ret.fcSttbFnm = s.fcSttbFnm;
    ret.lcbSttbFnm = s.lcbSttbFnm;

    return ret;
}

/* Please check...
Word97::FLD toWord97(const Word95::FLD &s) {

    Word97::FLD ret;

    ret.ch=s.ch;

    return ret;
} */

Word97::LSPD toWord97(const Word95::LSPD &s)
{

    Word97::LSPD ret;

    ret.dyaLine = s.dyaLine;
    ret.fMultLinespace = s.fMultLinespace;

    return ret;
}

Word97::METAFILEPICT toWord97(const Word95::METAFILEPICT &s)
{

    Word97::METAFILEPICT ret;

    ret.mm = s.mm;
    ret.xExt = s.xExt;
    ret.yExt = s.yExt;
    ret.hMF = s.hMF;

    return ret;
}

Word97::OBJHEADER toWord97(const Word95::OBJHEADER &s)
{

    Word97::OBJHEADER ret;

    ret.lcb = s.lcb;
    ret.cbHeader = s.cbHeader;
    ret.icf = s.icf;

    return ret;
}

Word97::OLST toWord97(const Word95::OLST &s)
{

    Word97::OLST ret;

    for (int i = 0; i < (9); ++i)
        ret.rganlv[i] = toWord97(s.rganlv[i]);
    ret.fRestartHdr = s.fRestartHdr;
    ret.fSpareOlst2 = s.fSpareOlst2;
    ret.fSpareOlst3 = s.fSpareOlst3;
    ret.fSpareOlst4 = s.fSpareOlst4;
    for (int i = 0; i < (32); ++i)
        ret.rgxch[i] = s.rgch[i];

    return ret;
}

Word97::PAP toWord97(const Word95::PAP &s)
{

    Word97::PAP ret;

    ret.istd = s.istd;
    ret.jc = s.jc;
    ret.fKeep = s.fKeep;
    ret.fKeepFollow = s.fKeepFollow;
    ret.fPageBreakBefore = s.fPageBreakBefore;
    ret.fBrLnAbove = s.fBrLnAbove;
    ret.fBrLnBelow = s.fBrLnBelow;
    ret.fUnused = s.fUnused;
    ret.pcVert = s.pcVert;
    ret.pcHorz = s.pcHorz;
    ret.brcp = s.brcp;
    ret.brcl = s.brcl;
    ret.unused9 = s.unused9;
    ret.nLvlAnm = s.nLvlAnm;
    ret.fNoLnn = s.fNoLnn;
    ret.fSideBySide = s.fSideBySide;
    ret.dxaRight = s.dxaRight;
    ret.dxaLeft = s.dxaLeft;
    ret.dxaLeft1 = s.dxaLeft1;
    ret.lspd = toWord97(s.lspd);
    ret.dyaBefore = s.dyaBefore;
    ret.dyaAfter = s.dyaAfter;
    ret.phe = toWord97(s.phe);
    ret.fWidowControl = s.fWidowControl;
    ret.fInTable = s.fInTable;
    ret.fTtp = s.fTtp;
    ret.ptap = s.ptap;
    ret.dxaAbs = s.dxaAbs;
    ret.dyaAbs = s.dyaAbs;
    ret.dxaWidth = s.dxaWidth;
    ret.brcTop = toWord97(s.brcTop);
    ret.brcLeft = toWord97(s.brcLeft);
    ret.brcBottom = toWord97(s.brcBottom);
    ret.brcRight = toWord97(s.brcRight);
    ret.brcBetween = toWord97(s.brcBetween);
    ret.brcBar = toWord97(s.brcBar);
    ret.dxaFromText = s.dxaFromText;
    ret.dyaFromText = s.dyaFromText;
    ret.wr = s.wr;
    ret.fLocked = s.fLocked;
    ret.dyaHeight = s.dyaHeight;
    ret.fMinHeight = s.fMinHeight;
    ret.shd = toWord97(s.shd);
    ret.dcs = toWord97(s.dcs);
    ret.anld = toWord97(s.anld);
    ret.itbdMac = s.itbdMac;

    return ret;
}

/* Please check...
Word97::PAPX toWord97(const Word95::PAPX &s) {

    Word97::PAPX ret;

    ret.cw=s.cw;
    ret.cb=s.cb;

    return ret;
} */

/* Please check...
Word97::PAPXFKP toWord97(const Word95::PAPXFKP &s) {

    Word97::PAPXFKP ret;

    ret.rgfc=s.rgfc;
    ret.rgbx=s.rgbx;
    ret.grppapx=s.grppapx;
    ret.crun=s.crun;

    return ret;
} */

Word97::PCD toWord97(const Word95::PCD &s)
{

    Word97::PCD ret;

    ret.fNoParaLast = s.fNoParaLast;
    ret.fPaphNil = s.fPaphNil;
    ret.fCopied = s.fCopied;
    ret.unused0_3 = s.unused0_3;
    ret.fn = s.fn;
    ret.fc = s.fc;
    ret.prm = toWord97(s.prm);

    return ret;
}

/* Please check...
Word97::PGD toWord97(const Word95::PGD &s) {

    Word97::PGD ret;

    ret.fContinue=s.fContinue;
    ret.fUnk=s.fUnk;
    ret.fRight=s.fRight;
    ret.fPgnRestart=s.fPgnRestart;
    ret.fEmptyPage=s.fEmptyPage;
    ret.fAllFtn=s.fAllFtn;
    ret.fTableBreaks=s.fTableBreaks;
    ret.fMarked=s.fMarked;
    ret.fColumnBreaks=s.fColumnBreaks;
    ret.fTableHeader=s.fTableHeader;
    ret.fNewPage=s.fNewPage;
    ret.bkc=s.bkc;
    ret.lnn=s.lnn;
    ret.pgn=s.pgn;

    return ret;
} */

Word97::PHE toWord97(const Word95::PHE &s)
{

    Word97::PHE ret;

    ret.fSpare = s.fSpare;
    ret.fUnk = s.fUnk;
    ret.fDiffLines = s.fDiffLines;
    ret.unused0_3 = s.unused0_3;
    ret.clMac = s.clMac;
    ret.dxaCol = s.dxaCol;
    ret.dym = s.dylLine_dylHeight;

    return ret;
}

Word97::PICF toWord97(const Word95::PICF &s)
{

    Word97::PICF ret;

    ret.lcb = s.lcb;
    ret.cbHeader = s.cbHeader;
    ret.mfp = toWord97(s.mfp);
    for (int i = 0; i < (14); ++i)
        ret.bm_rcWinMF[i] = s.bm_rcWinMF[i];
    ret.dxaGoal = s.dxaGoal;
    ret.dyaGoal = s.dyaGoal;
    ret.mx = s.mx;
    ret.my = s.my;
    ret.dxaCropLeft = s.dxaCropLeft;
    ret.dyaCropTop = s.dyaCropTop;
    ret.dxaCropRight = s.dxaCropRight;
    ret.dyaCropBottom = s.dyaCropBottom;
    ret.brcl = s.brcl;
    ret.fFrameEmpty = s.fFrameEmpty;
    ret.fBitmap = s.fBitmap;
    ret.fDrawHatch = s.fDrawHatch;
    ret.fError = s.fError;
    ret.bpp = s.bpp;
    ret.brcTop = toWord97(s.brcTop);
    ret.brcLeft = toWord97(s.brcLeft);
    ret.brcBottom = toWord97(s.brcBottom);
    ret.brcRight = toWord97(s.brcRight);
    ret.dxaOrigin = s.dxaOrigin;
    ret.dyaOrigin = s.dyaOrigin;

    return ret;
}

/* Please check...
Word97::PLCF toWord97(const Word95::PLCF &s) {

    Word97::PLCF ret;

    ret.rgfc=s.rgfc;

    return ret;
} */

Word97::PRM toWord97(const Word95::PRM &s)
{

    Word97::PRM ret;

    ret.fComplex = s.fComplex;
    ret.isprm = s.sprm;
    ret.val = s.val;

    return ret;
}

Word97::PRM2 toWord97(const Word95::PRM2 &s)
{

    Word97::PRM2 ret;

    ret.fComplex = s.fComplex;
    ret.igrpprl = s.igrpprl;

    return ret;
}

Word97::SED toWord97(const Word95::SED &s)
{

    Word97::SED ret;

    ret.fcSepx = s.fcSepx;
    ret.fnMpr = s.fnMpr;
    ret.fcMpr = s.fcMpr;

    return ret;
}

Word97::SEP toWord97(const Word95::SEP &s)
{

    Word97::SEP ret;

    ret.bkc = s.bkc;
    ret.fTitlePage = s.fTitlePage;
    ret.ccolM1 = s.ccolM1;
    ret.dxaColumns = s.dxaColumns;
    ret.fAutoPgn = s.fAutoPgn;
    ret.nfcPgn = s.nfcPgn;
    ret.pgnStart = s.pgnStart;
    ret.fUnlocked = s.fUnlocked;
    ret.cnsPgn = s.cnsPgn;
    ret.fPgnRestart = s.fPgnRestart;
    ret.fEndNote = s.fEndNote;
    ret.lnc = s.lnc;
    ret.grpfIhdt = s.grpfIhdt;
    ret.nLnnMod = s.nLnnMod;
    ret.dxaLnn = s.dxaLnn;
    ret.dyaHdrTop = s.dyaHdrTop;
    ret.dyaHdrBottom = s.dyaHdrBottom;
    ret.dxaPgn = s.dxaPgn;
    ret.dyaPgn = s.dyaPgn;
    ret.fLBetween = s.fLBetween;
    ret.vjc = s.vjc;
    ret.lnnMin = s.lnnMin;
    ret.dmOrientPage = s.dmOrientPage;
    ret.iHeadingPgn = s.iHeadingPgn;
    ret.xaPage = s.xaPage;
    ret.yaPage = s.yaPage;
    ret.dxaLeft = s.dxaLeft;
    ret.dxaRight = s.dxaRight;
    ret.dyaTop = s.dyaTop;
    ret.dyaBottom = s.dyaBottom;
    ret.dzaGutter = s.dzaGutter;
    ret.dmBinFirst = s.dmBinFirst;
    ret.dmBinOther = s.dmBinOther;
    ret.dmPaperReq = s.dmPaperReq;
    ret.fEvenlySpaced = s.fEvenlySpaced;
    ret.dxaColumnWidth = s.dxaColumnWidth;
    for (int i = 0; i < (89); ++i)
        ret.rgdxaColumnWidthSpacing[i] = s.rgdxaColumnWidthSpacing[i];
    ret.olstAnm = toWord97(s.olstAnm);

    return ret;
}

/* Please check...
Word97::SEPX toWord97(const Word95::SEPX &s) {

    Word97::SEPX ret;

    ret.grpprl=s.grpprl;

    return ret;
} */

Word97::SHD toWord97(const Word95::SHD &s)
{

    Word97::SHD ret;

    ret.icoFore = s.icoFore;
    ret.icoBack = s.icoBack;
    ret.ipat = s.ipat;

    return ret;
}

/* Please check...
Word97::STSHI toWord97(const Word95::STSHI &s) {

    Word97::STSHI ret;

    ret.cstd=s.cstd;
    ret.cbSTDBaseInFile=s.cbSTDBaseInFile;
    ret.fStdStylenamesWritten=s.fStdStylenamesWritten;
    ret.unused4_2=s.unused4_2;
    ret.stiMaxWhenSaved=s.stiMaxWhenSaved;
    ret.istdMaxFixedWhenSaved=s.istdMaxFixedWhenSaved;
    ret.nVerBuiltInNamesWhenSaved=s.nVerBuiltInNamesWhenSaved;

    return ret;
} */

Word97::TAP toWord97(const Word95::TAP &s)
{

    Word97::TAP ret;

    ret.jc = s.jc;
    ret.dxaGapHalf = s.dxaGapHalf;
    ret.dyaRowHeight = s.dyaRowHeight;
    ret.fCantSplit = s.fCantSplit;
    ret.fTableHeader = s.fTableHeader;
    ret.tlp = toWord97(s.tlp);
    ret.fCaFull = s.fCaFull;
    ret.fFirstRow = s.fFirstRow;
    ret.fLastRow = s.fLastRow;
    ret.fOutline = s.fOutline;
    ret.itcMac = s.itcMac;
    ret.dxaAdjust = s.dxaAdjust;
    for (int i = 0; i < (6); ++i)
        ret.rgbrcTable[i] = toWord97(s.rgbrcTable[i]);

    return ret;
}

/* Please check...
Word97::TBD toWord97(const Word95::TBD &s) {

    Word97::TBD ret;

    ret.jc=s.jc;
    ret.tlc=s.tlc;
    ret.unused0_6=s.unused0_6;

    return ret;
} */

Word97::TC toWord97(const Word95::TC &s)
{

    Word97::TC ret;

    ret.fFirstMerged = s.fFirstMerged;
    ret.fMerged = s.fMerged;
    ret.fUnused = s.fUnused;
    ret.brcTop = toWord97(s.brcTop);
    ret.brcLeft = toWord97(s.brcLeft);
    ret.brcBottom = toWord97(s.brcBottom);
    ret.brcRight = toWord97(s.brcRight);

    return ret;
}

Word97::TLP toWord97(const Word95::TLP &s)
{

    Word97::TLP ret;

    ret.itl = s.itl;
    ret.fBorders = s.fBorders;
    ret.fShading = s.fShading;
    ret.fFont = s.fFont;
    ret.fColor = s.fColor;
    ret.fBestFit = s.fBestFit;
    ret.fHdrRows = s.fHdrRows;
    ret.fLastRow = s.fLastRow;
    ret.fHdrCols = s.fHdrCols;
    ret.fLastCol = s.fLastCol;
    ret.unused2_9 = s.unused2_9;

    return ret;
}

} // namespace Word95

} // namespace wvWare
