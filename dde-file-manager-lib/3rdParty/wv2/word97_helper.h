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

#ifndef WORD97_HELPER_H
#define WORD97_HELPER_H

#include "global.h"

namespace wvWare
{
	class AbstractOLEStreamReader;
    class ParagraphProperties;
    class StyleSheet;

    namespace Word97
    {
        struct TAP;
    }

    namespace Word97
    {
		ParagraphProperties* initPAPFromStyle( const U8* exceptions, const StyleSheet* stylesheet, AbstractOLEStreamReader* dataStream, WordVersion version );
		Word97::TAP* initTAP( const U8* exceptions, AbstractOLEStreamReader* dataStream, WordVersion version );

        namespace SPRM
        {
            U16 unzippedOpCode( U8 isprm );
            U16 determineParameterLength( U16 sprm, const U8* in, WordVersion version );
            U16 word6toWord8( U8 sprm );
        }
    } // namespace Word97
} // namespace wvWare

#endif // WORD97_HELPER_H
