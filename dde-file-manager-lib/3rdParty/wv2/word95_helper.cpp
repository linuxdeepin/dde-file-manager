/* This file is part of the wvWare 2 project
   Copyright (C) 2001 S.R.Haque <srhaque@iee.org>
   Copyright (C) 2003 Werner Trobin <trobin@kde.org>

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

#include "word95_helper.h"
#include "word95_generated.h"
#include "olestream.h"
#include "styles.h"

#include "wvlog.h"

namespace wvWare
{

namespace Word95
{

namespace SPRM
{

typedef enum {
    sprmNoop = 0,
    sprmPIstd = 2,
    sprmPIstdPermute = 3,
    sprmPIncLvl = 4,
    sprmPJc = 5,
    sprmPFSideBySide = 6,
    sprmPFKeep = 7,
    sprmPFKeepFollow = 8,
    sprmPFPageBreakBefore = 9,
    sprmPBrcl = 10,
    sprmPBrcp = 11,
    sprmPAnld = 12,
    sprmPNLvlAnm = 13,
    sprmPFNoLineNumb = 14,
    sprmPChgTabsPapx = 15,
    sprmPDxaRight = 16,
    sprmPDxaLeft = 17,
    sprmPNest = 18,
    sprmPDxaLeft1 = 19,
    sprmPDyaLine = 20,
    sprmPDyaBefore = 21,
    sprmPDyaAfter = 22,
    sprmPChgTabs = 23,
    sprmPFInTable = 24,
    sprmPFTtp = 25,
    sprmPDxaAbs = 26,
    sprmPDyaAbs = 27,
    sprmPDxaWidth = 28,
    sprmPPc = 29,
    sprmPBrcTop10 = 30,
    sprmPBrcLeft10 = 31,
    sprmPBrcBottom10 = 32,
    sprmPBrcRight10 = 33,
    sprmPBrcBetween10 = 34,
    sprmPBrcBar10 = 35,
    sprmPFromText10 = 36,
    sprmPWr = 37,
    sprmPBrcTop = 38,
    sprmPBrcLeft = 39,
    sprmPBrcBottom = 40,
    sprmPBrcRight = 41,
    sprmPBrcBetween = 42,
    sprmPBrcBar = 43,
    sprmPFNoAutoHyph = 44,
    sprmPWHeightAbs = 45,
    sprmPDcs = 46,
    sprmPShd = 47,
    sprmPDyaFromText = 48,
    sprmPDxaFromText = 49,
    sprmPFLocked = 50,
    sprmPFWidowControl = 51,
    sprmPRuler = 52,
    sprmCFStrikeRM = 65,
    sprmCFRMark = 66,
    sprmCFFldVanish = 67,
    sprmCPicLocation = 68,
    sprmCIbstRMark = 69,
    sprmCDttmRMark = 70,
    sprmCFData = 71,
    sprmCRMReason = 72,
    sprmCChse = 73,
    sprmCSymbol = 74,
    sprmCFOle2 = 75,
    sprmCIstd = 80,
    sprmCIstdPermute = 81,
    sprmCDefault = 82,
    sprmCPlain = 83,
    sprmCFBold = 85,
    sprmCFItalic = 86,
    sprmCFStrike = 87,
    sprmCFOutline = 88,
    sprmCFShadow = 89,
    sprmCFSmallCaps = 90,
    sprmCFCaps = 91,
    sprmCFVanish = 92,
    sprmCFtc = 93,
    sprmCKul = 94,
    sprmCSizePos = 95,
    sprmCDxaSpace = 96,
    sprmCLid = 97,
    sprmCIco = 98,
    sprmCHps = 99,
    sprmCHpsInc = 100,
    sprmCHpsPos = 101,
    sprmCHpsPosAdj = 102,
    sprmCMajority = 103,
    sprmCIss = 104,
    sprmCHpsNew50 = 105,
    sprmCHpsInc1 = 106,
    sprmCHpsKern = 107,
    sprmCMajority50 = 108,
    sprmCHpsMul = 109,
    sprmCCondHyhen = 110,
    sprmCFSpec = 117,
    sprmCFObj = 118,
    sprmPicBrcl = 119,
    sprmPicScale = 120,
    sprmPicBrcTop = 121,
    sprmPicBrcLeft = 122,
    sprmPicBrcBottom = 123,
    sprmPicBrcRight = 124,
    sprmSScnsPgn = 131,
    sprmSiHeadingPgn = 132,
    sprmSOlstAnm = 133,
    sprmSDxaColWidth = 136,
    sprmSDxaColSpacing = 137,
    sprmSFEvenlySpaced = 138,
    sprmSFProtected = 139,
    sprmSDmBinFirst = 140,
    sprmSDmBinOther = 141,
    sprmSBkc = 142,
    sprmSFTitlePage = 143,
    sprmSCcolumns = 144,
    sprmSDxaColumns = 145,
    sprmSFAutoPgn = 146,
    sprmSNfcPgn = 147,
    sprmSDyaPgn = 148,
    sprmSDxaPgn = 149,
    sprmSFPgnRestart = 150,
    sprmSFEndnote = 151,
    sprmSLnc = 152,
    sprmSGprfIhdt = 153,
    sprmSNLnnMod = 154,
    sprmSDxaLnn = 155,
    sprmSDyaHdrTop = 156,
    sprmSDyaHdrBottom = 157,
    sprmSLBetween = 158,
    sprmSVjc = 159,
    sprmSLnnMin = 160,
    sprmSPgnStart = 161,
    sprmSBOrientation = 162,
    sprmSBCustomize = 163,
    sprmSXaPage = 164,
    sprmSYaPage = 165,
    sprmSDxaLeft = 166,
    sprmSDxaRight = 167,
    sprmSDyaTop = 168,
    sprmSDyaBottom = 169,
    sprmSDzaGutter = 170,
    sprmSDMPaperReq = 171,
    sprmTJc = 182,
    sprmTDxaLeft = 183,
    sprmTDxaGapHalf = 184,
    sprmTFCantSplit = 185,
    sprmTTableHeader = 186,
    sprmTTableBorders = 187,
    sprmTDefTable10 = 188,
    sprmTDyaRowHeight = 189,
    sprmTDefTable = 190,
    sprmTDefTableShd = 191,
    sprmTTlp = 192,
    sprmTSetBrc = 193,
    sprmTInsert = 194,
    sprmTDelete = 195,
    sprmTDxaCol = 196,
    sprmTMerge = 197,
    sprmTSplit = 198,
    sprmTSetBrc10 = 199,
    sprmTSetShd = 200
} opcodes;

// The length of the SPRM parameter
U16 determineParameterLength( U8 sprm, const U8* in )
{
    switch ( sprm ) {
        case SPRM::sprmNoop:
            return 0;
        case SPRM::sprmPIstd:
            return 2;
        case SPRM::sprmPIstdPermute:
            break; // Variable.
        case SPRM::sprmPIncLvl:
        case SPRM::sprmPJc:
        case SPRM::sprmPFSideBySide:
        case SPRM::sprmPFKeep:
        case SPRM::sprmPFKeepFollow:
        case SPRM::sprmPFPageBreakBefore:
        case SPRM::sprmPBrcl:
        case SPRM::sprmPBrcp:
            return 1;
        case SPRM::sprmPAnld:
            break; // Variable.
        case SPRM::sprmPNLvlAnm:
        case SPRM::sprmPFNoLineNumb:
            return 1;
        case SPRM::sprmPChgTabsPapx:
            break; // Variable.
        case SPRM::sprmPDxaRight:
            return 2;
        case SPRM::sprmPDxaLeft:
        case SPRM::sprmPNest:
        case SPRM::sprmPDxaLeft1:
            return 2;
        case SPRM::sprmPDyaLine:
            return 4;
        case SPRM::sprmPDyaBefore:
        case SPRM::sprmPDyaAfter:
            return 2;
        case SPRM::sprmPChgTabs:
            if ( *in == 255 ) {
                U8 itbdDelMax = in[1];
                U8 itbdAddMax = in[1 + itbdDelMax * 4];
                return 1 + itbdDelMax * 4 + itbdAddMax * 3;
            }
            else
                return *in + 1;
        case SPRM::sprmPFInTable:
        case SPRM::sprmPFTtp:
            return 1;
        case SPRM::sprmPDxaAbs:
        case SPRM::sprmPDyaAbs:
        case SPRM::sprmPDxaWidth:
            return 2;
        case SPRM::sprmPPc:
            return 1;
        case SPRM::sprmPBrcTop10:
        case SPRM::sprmPBrcLeft10:
        case SPRM::sprmPBrcBottom10:
        case SPRM::sprmPBrcRight10:
        case SPRM::sprmPBrcBetween10:
        case SPRM::sprmPBrcBar10:
        case SPRM::sprmPFromText10:
            return 2;
        case SPRM::sprmPWr:
            return 1;
        case SPRM::sprmPBrcTop:
        case SPRM::sprmPBrcLeft:
        case SPRM::sprmPBrcBottom:
        case SPRM::sprmPBrcRight:
        case SPRM::sprmPBrcBetween:
        case SPRM::sprmPBrcBar:
            return 2;
        case SPRM::sprmPFNoAutoHyph:
            return 1;
        case SPRM::sprmPWHeightAbs:
        case SPRM::sprmPDcs:
        case SPRM::sprmPShd:
        case SPRM::sprmPDyaFromText:
        case SPRM::sprmPDxaFromText:
            return 2;
        case SPRM::sprmPFLocked:
        case SPRM::sprmPFWidowControl:
            return 1;
        case SPRM::sprmPRuler:
            break; // Variable.
        case SPRM::sprmCFStrikeRM:
        case SPRM::sprmCFRMark:
        case SPRM::sprmCFFldVanish:
            return 1;
        case SPRM::sprmCPicLocation:
            break; // Variable.
        case SPRM::sprmCIbstRMark:
            return 2;
        case SPRM::sprmCDttmRMark:
            return 4;
        case SPRM::sprmCFData:
            return 1;
        case SPRM::sprmCRMReason:
            return 2;
        case SPRM::sprmCChse:
            return 3;
        case SPRM::sprmCSymbol:
            break; // Variable.
        case SPRM::sprmCFOle2:
            return 1;
        case SPRM::sprmCIstd:
            return 2;
        case SPRM::sprmCIstdPermute:
        case SPRM::sprmCDefault:
            break; // Variable.
        case SPRM::sprmCPlain:
            return 0;
        case SPRM::sprmCFBold:
        case SPRM::sprmCFItalic:
        case SPRM::sprmCFStrike:
        case SPRM::sprmCFOutline:
        case SPRM::sprmCFShadow:
        case SPRM::sprmCFSmallCaps:
        case SPRM::sprmCFCaps:
        case SPRM::sprmCFVanish:
            return 1;
        case SPRM::sprmCFtc:
            return 2;
        case SPRM::sprmCKul:
            return 1;
        case SPRM::sprmCSizePos:
            return 3;
        case SPRM::sprmCDxaSpace:
        case SPRM::sprmCLid:
            return 2;
        case SPRM::sprmCIco:
        case SPRM::sprmCHps:
        case SPRM::sprmCHpsInc:
        case SPRM::sprmCHpsPos:
        case SPRM::sprmCHpsPosAdj:
            return 1;
        case SPRM::sprmCMajority:
            break; // Variable.
        case SPRM::sprmCIss:
            return 1;
        case SPRM::sprmCHpsNew50:
        case SPRM::sprmCHpsInc1:
            break; // Variable.
        case SPRM::sprmCHpsKern:
            return 2;
        case SPRM::sprmCMajority50:
            break; // Variable.
        case SPRM::sprmCHpsMul:
        case SPRM::sprmCCondHyhen:
            return 2;
        case SPRM::sprmCFSpec:
        case SPRM::sprmCFObj:
        case SPRM::sprmPicBrcl:
            return 1;
        case SPRM::sprmPicScale:
            break; // Variable.
        case SPRM::sprmPicBrcTop:
        case SPRM::sprmPicBrcLeft:
        case SPRM::sprmPicBrcBottom:
        case SPRM::sprmPicBrcRight:
            return 2;
        case SPRM::sprmSScnsPgn:
        case SPRM::sprmSiHeadingPgn:
            return 1;
        case SPRM::sprmSOlstAnm:
            break; // Variable.
        case SPRM::sprmSDxaColWidth:
        case SPRM::sprmSDxaColSpacing:
            return 3;
        case SPRM::sprmSFEvenlySpaced:
        case SPRM::sprmSFProtected:
            return 1;
        case SPRM::sprmSDmBinFirst:
        case SPRM::sprmSDmBinOther:
            return 2;
        case SPRM::sprmSBkc:
        case SPRM::sprmSFTitlePage:
            return 1;
        case SPRM::sprmSCcolumns:
        case SPRM::sprmSDxaColumns:
            return 2;
        case SPRM::sprmSFAutoPgn:
        case SPRM::sprmSNfcPgn:
            return 1;
        case SPRM::sprmSDyaPgn:
        case SPRM::sprmSDxaPgn:
            return 2;
        case SPRM::sprmSFPgnRestart:
        case SPRM::sprmSFEndnote:
        case SPRM::sprmSLnc:
        case SPRM::sprmSGprfIhdt:
            return 1;
        case SPRM::sprmSNLnnMod:
        case SPRM::sprmSDxaLnn:
        case SPRM::sprmSDyaHdrTop:
        case SPRM::sprmSDyaHdrBottom:
            return 2;
        case SPRM::sprmSLBetween:
        case SPRM::sprmSVjc:
            return 1;
        case SPRM::sprmSLnnMin:
        case SPRM::sprmSPgnStart:
            return 2;
        case SPRM::sprmSBOrientation:
        case SPRM::sprmSBCustomize:
            return 1;
        case SPRM::sprmSXaPage:
        case SPRM::sprmSYaPage:
        case SPRM::sprmSDxaLeft:
        case SPRM::sprmSDxaRight:
        case SPRM::sprmSDyaTop:
        case SPRM::sprmSDyaBottom:
        case SPRM::sprmSDzaGutter:
        case SPRM::sprmSDMPaperReq:
        case SPRM::sprmTJc:
        case SPRM::sprmTDxaLeft:
        case SPRM::sprmTDxaGapHalf:
            return 2;
        case SPRM::sprmTFCantSplit:
        case SPRM::sprmTTableHeader:
            return 1;
        case SPRM::sprmTTableBorders:
            return 12;
        case SPRM::sprmTDefTable10:
            return readU16(in) + 1;
        case SPRM::sprmTDyaRowHeight:
            return 2;
        case SPRM::sprmTDefTable:
            return readU16(in) + 1;
        case SPRM::sprmTDefTableShd:
            break; // Variable.
        case SPRM::sprmTTlp:
            return 4;
        case SPRM::sprmTSetBrc:
            return 5;
        case SPRM::sprmTInsert:
            return 4;
        case SPRM::sprmTDelete:
            return 2;
        case SPRM::sprmTDxaCol:
            return 4;
        case SPRM::sprmTMerge:
        case SPRM::sprmTSplit:
            return 2;
        case SPRM::sprmTSetBrc10:
            return 5;
        case SPRM::sprmTSetShd:
            return 4;
    }

    // Get length of variable size operand.
    return *in + 1;
}

} // namespace SPRM
} // namespace Word95
} // namespace wvWare
