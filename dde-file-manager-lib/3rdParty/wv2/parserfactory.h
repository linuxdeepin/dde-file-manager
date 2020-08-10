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

#ifndef PARSERFACTORY_H
#define PARSERFACTORY_H

#include <string>
#include "sharedptr.h"

namespace wvWare
{

class Parser;
class AbstractOLEStorage;

class WV2_DLLEXPORT ParserFactory
{
public:
    /**
     * This method opens a storage on the file, determines the nFib,
     * and creates a proper parser for it.
     * All you have to do with that parser is to call parse() on it
     * and it will start firing callbacks.
     * This method will return 0 if it wasn't successful (e.g unknown
     * version, corrupted file,...).
     */
	static SharedPtr<Parser> createParser(AbstractOLEStorage *storage );
    /**
     * This method opens a storage on a buffer in memory, determines the nFib,
     * and creates a proper parser for it.
     * All you have to do with that parser is to call parse() on it
     * and it will start firing callbacks.
     * This method will return 0 if it wasn't successful (e.g unknown
     * version, corrupted file,...).
     */
    //static SharedPtr<Parser> createParserFromBuffer( AbstractOLEStorage *storage );
};

} // namespace wvWare

#endif // PARSERFACTORY_H
