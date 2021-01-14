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

#ifndef OLESTORAGE_H
#define OLESTORAGE_H

#include <string>
#include <list>
#include <deque>

namespace wvWare
{

class OLEStream;
class AbstractOLEStreamReader;

class AbstractOLEStorage
{
	friend class OLEStream;
	friend class AbstractOLEStreamReader;
	public:
		/**
		 * The mode of the storage. libgsf doesn't support storages opened
		 * for reading and writing like libole2 did.
		 */
		enum Mode { ReadOnly, WriteOnly };
		AbstractOLEStorage(){}
		virtual ~AbstractOLEStorage(){}
		virtual bool isValid() const = 0;
		virtual bool open( Mode mode ) = 0;
		virtual void close() = 0;
		virtual std::string name() const = 0;
        virtual bool readDirectFromBuffer(unsigned char* buf, int size, int offset) = 0;
		virtual AbstractOLEStreamReader* createStreamReader( const std::string& stream ) = 0;
	private:
		virtual void streamDestroyed(OLEStream* stream) = 0;
};

} // namespace wvWare

#endif // OLESTORAGE_H
