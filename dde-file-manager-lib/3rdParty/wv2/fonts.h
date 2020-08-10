/* This file is part of the wvWare 2 project
   Copyright (C) 2002-2003 Werner Trobin <trobin@kde.org>

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

#ifndef FONTS_H
#define FONTS_H

#include "global.h"
#include <vector>

namespace wvWare
{
	class AbstractOLEStreamReader;
    namespace Word97
    {
        struct FIB;
        struct FFN;
    }

    /**
     * The FontCollection can be used to map ftc (font code) values to font names.
     */
    class FontCollection
    {
    public:
        /**
         * You shouldn't construct such objects yourself, the Parser interface
         * provides accesst to the internal font collection.
         * @param reader is either a table or a document stream (97+ / older versions)
         */
		FontCollection( AbstractOLEStreamReader* reader, const Word97::FIB& fib );
        ~FontCollection();

        /**
         * Returns the matching FFN structure for the given font code. If no matching
         * FFN is found, a fallback defaulting to "Helvetica" is returned.
         */
        const Word97::FFN& font( S16 ftc ) const;

        /**
         * Dump all the font information, used for debugging.
         */
        void dump() const;

    private:
        FontCollection( const FontCollection& rhs );
        FontCollection& operator=( const FontCollection& rhs );

        std::vector<Word97::FFN*> m_fonts;
        Word97::FFN* m_fallbackFont;
    };

} // namespace wvWare

#endif // FONTS_H
