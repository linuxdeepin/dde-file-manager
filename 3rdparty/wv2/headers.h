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

#ifndef HEADERS_H
#define HEADERS_H

#include "global.h"
#include <utility>
#include <vector>

namespace wvWare
{

	class AbstractOLEStreamReader;

    /**
     * @internal
     * A tiny helper class to move some header/footer code out of the parser.
     * Might not be ultra-elegant, but I don't like it if the parser code
     * grows too much.
     * Abstract base class for the Word 6/7 and Word 8 variants.
     */
    class Headers
    {
    public:
		Headers( U32 fcPlcfhdd, U32 lcbPlcfhdd, AbstractOLEStreamReader* tableStream, WordVersion version );
        virtual ~Headers();

        /**
         * Returns the header if there is any for the given mask. If we didn't find
         * any header the pair's values are 0, 0.
         */
        virtual std::pair<U32, U32> findHeader( int sectionNumber, unsigned char mask ) const = 0;

        /**
         * A helper method to implement Word 6 support.
         */
        virtual void headerMask( U8 sep_grpfIhdt );

    protected:
        std::vector<U32> m_headers;
        static const int headerTypes;
    };

} // namespace wvWare

#endif // HEADERS_H
