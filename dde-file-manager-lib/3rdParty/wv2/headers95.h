/* This file is part of the wvWare 2 project
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

#ifndef HEADERS95_H
#define HEADERS95_H

#include "headers.h"

namespace wvWare
{
    /**
     * @internal
     * A tiny helper class to move some header/footer code out of the parser.
     * Might not be ultra-elegant, but I don't like it if the parser code
     * grows too much.
     */
    class Headers95 : public Headers
    {
    public:
		Headers95( U32 fcPlcfhdd, U32 lcbPlcfhdd, AbstractOLEStreamReader* tableStream, U8 dop_grpfIhdt );

        /**
         * Returns the header if there is any for the given mask. If we didn't find
         * any header the pair's values are 0, 0.
         */
        virtual std::pair<U32, U32> findHeader( int sectionNumber, unsigned char mask ) const;

        virtual void headerMask( U8 sep_grpfIhdt );

    private:
        // Counts the '1' bits in <mask> from the lsb up to <limit> (exclusively)
        int countOnes( U8 mask, U8 limit ) const;

        const int ihddOffset;
        std::vector<U32> m_headerCount;
        std::vector<U8> m_grpfIhdt;
    };

} // namespace wvWare

#endif // HEADERS95_H
