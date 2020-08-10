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

#ifndef WORD95_HELPER_H
#define WORD95_HELPER_H

#include "global.h"


namespace wvWare
{
    namespace Word95
    {
        namespace SPRM
        {

            U16 determineParameterLength( U8 sprm, const U8* in );

        } // namespace SPRM
    } // namespace Word95
} // namespace wvWare

#endif // WORD95_HELPER_H
