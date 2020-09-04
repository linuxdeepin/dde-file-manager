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

#ifndef PARAGRAPHPROPERTIES_H
#define PARAGRAPHPROPERTIES_H

#include "word97_generated.h"

namespace wvWare
{

    class ListInfo;
    class ListInfoProvider;

    /**
     * A tiny helper class to wrap the PAP and any additional information
     * we want to pass to the consumer. Right now we have a ListInfo object
     * if the paragraph belongs to some list.
     */
    class ParagraphProperties : public Shared
    {
    public:
        ParagraphProperties();
        explicit ParagraphProperties( const Word97::PAP& pap );
        ParagraphProperties( const ParagraphProperties& rhs );
        ~ParagraphProperties();

        Word97::PAP& pap();
        const Word97::PAP& pap() const;
        /**
         * If this paragraph belongs to a list, the ListInfo object will be
         * valid and contain useful information about the formatting of the
         * list counter. If the paragraph is not inside a list, this method
         * will return 0.
         */
        const ListInfo* listInfo() const;

        /**
         * @internal
         */
        void createListInfo( ListInfoProvider& listInfoProvider );

    private:
        ParagraphProperties& operator=( const ParagraphProperties& rhs );

        Word97::PAP m_pap;
        ListInfo* m_listInfo;
    };

} // namespace wvWare

#endif // PARAGRAPHPROPERTIES_H
