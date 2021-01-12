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

#ifndef FIELDS_H
#define FIELDS_H

#include "parser.h"

namespace wvWare
{
    namespace Word97
    {
        struct FIB;
    }
    template<class T> class PLCF;
	class AbstractOLEStreamReader;

    struct FLD
    {
        FLD();
		FLD( AbstractOLEStreamReader* stream, bool preservePos = false );
        FLD( const U8* ptr );

		bool read( AbstractOLEStreamReader* stream, bool preservePos = false );
        bool readPtr( const U8* ptr );

        void clear();

        // Data
        U8 ch;
        union
        {
            U8 flt;
            struct
            {
                U8 fDiffer:1;
                U8 fZomieEmbed:1;
                U8 fResultDirty:1;
                U8 fResultEdited:1;
                U8 fLocked:1;
                U8 fPrivateResult:1;
                U8 fNested:1;
                U8 fHasSep:1;
            } flags;
        };

        // Size of the structure (needed for the PLCF template)
        static const unsigned int sizeOf;
    };

    bool operator==( const FLD &lhs, const FLD &rhs );
    bool operator!=( const FLD &lhs, const FLD &rhs );


    class Fields
    {
    public:
		Fields( AbstractOLEStreamReader* tableStream, const Word97::FIB& fib );
        ~Fields();

        const FLD* fldForCP( Parser::SubDocument subDocument, U32 cp ) const;

    private:
        Fields( const Fields& rhs );
        Fields& operator=( const Fields& rhs );

		void read( U32 fc, U32 lcb, AbstractOLEStreamReader* tableStream, PLCF<FLD>** plcf );
		void sanityCheck( const AbstractOLEStreamReader* tableStream, U32 nextFC, U32 lcb ) const;
        const FLD* fldForCP( const PLCF<FLD>* plcf, U32 cp ) const;

        PLCF<FLD>* m_main;
        PLCF<FLD>* m_header;
        PLCF<FLD>* m_footnote;
        PLCF<FLD>* m_annotation;
        PLCF<FLD>* m_endnote;
        PLCF<FLD>* m_textbox;
        PLCF<FLD>* m_headerTextbox;
    };

} // namespace wvWare

#endif // FIELDS_H
