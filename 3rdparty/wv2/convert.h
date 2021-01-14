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

#ifndef CONVERT_H
#define CONVERT_H

#include "word95_generated.h"
#include "word97_generated.h"

namespace wvWare {

namespace Word95 {

// This has been added to the template file, as the mapping is
// non-trivial. Shaheed: Please check the implementation
Word97::BRC toWord97(const Word95::BRC &s);
Word97::STSHI toWord97(const Word95::STSHI &s);

Word97::ANLD toWord97(const Word95::ANLD &s);
Word97::ANLV toWord97(const Word95::ANLV &s);
Word97::BKF toWord97(const Word95::BKF &s);
Word97::BKL toWord97(const Word95::BKL &s);
Word97::BRC10 toWord97(const Word95::BRC10 &s);
Word97::BTE toWord97(const Word95::BTE &s);
Word97::CHP toWord97(const Word95::CHP &s);
Word97::DCS toWord97(const Word95::DCS &s);
Word97::DOP toWord97(const Word95::DOP &s);
Word97::DTTM toWord97(const Word95::DTTM &s);
Word97::FIB toWord97(const Word95::FIB &s);
Word97::LSPD toWord97(const Word95::LSPD &s);
Word97::METAFILEPICT toWord97(const Word95::METAFILEPICT &s);
Word97::OBJHEADER toWord97(const Word95::OBJHEADER &s);
Word97::OLST toWord97(const Word95::OLST &s);
Word97::PAP toWord97(const Word95::PAP &s);
Word97::PCD toWord97(const Word95::PCD &s);
Word97::PHE toWord97(const Word95::PHE &s);
Word97::PICF toWord97(const Word95::PICF &s);
Word97::PRM toWord97(const Word95::PRM &s);
Word97::PRM2 toWord97(const Word95::PRM2 &s);
Word97::SED toWord97(const Word95::SED &s);
Word97::SEP toWord97(const Word95::SEP &s);
Word97::SHD toWord97(const Word95::SHD &s);
Word97::TAP toWord97(const Word95::TAP &s);
Word97::TC toWord97(const Word95::TC &s);
Word97::TLP toWord97(const Word95::TLP &s);

} // namespace Word95

} // namespace wvWare

#endif // CONVERT_H
