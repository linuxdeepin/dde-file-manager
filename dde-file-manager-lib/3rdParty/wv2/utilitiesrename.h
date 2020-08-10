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

#ifndef UTILITIESREANME_H
#define UTILITIESREANME_H

#include <algorithm>
#include <functional> // std::unary_function is in this header with gcc 2.9x
#include <string>

namespace wvWare {
template<class T> struct Delete : public std::unary_function<T *, void> {
    void operator() (T *t) const
    {
        delete t;
    }
};

std::string int2string( int i );
std::string uint2string( unsigned int i );
}

#endif // UTILITIESREANME_H
