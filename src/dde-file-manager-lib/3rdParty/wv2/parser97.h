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

#ifndef PARSER97_H
#define PARSER97_H

#include "parser9x.h"

namespace wvWare
{

/**
 * This class is the main parser class for Word97 documents.
 */
class Parser97 : public Parser9x
{
public:
	Parser97( AbstractOLEStorage* storage, AbstractOLEStreamReader* wordDocument );
    virtual ~Parser97();

private:
    // Don't copy or assign us
    Parser97( const Parser97& rhs );
    Parser97& operator=( const Parser97& rhs );

    void init();
};

} // namespace wvWare

#endif // PARSER97_H
