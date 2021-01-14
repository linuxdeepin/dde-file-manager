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

#ifndef OLESTREAM_H
#define OLESTREAM_H

#include "olestorage.h"
#include "global.h"  // U8,... typedefs
#include <stack>
#include <stdio.h> //seek type

namespace wvWare {

class OLEStream
{
public:
    /**
     * Create an OLE stream
     */
	OLEStream(AbstractOLEStorage *storage );
    virtual ~OLEStream();

    /**
     * Is this still a valid stream?
     */
    virtual bool isValid() const = 0;

    /**
     * works like plain fseek
     */
    virtual bool seek( int offset, int whence = SEEK_SET ) = 0;
    /**
     * works like plain ftell
     */
    virtual int tell() const = 0;
    /**
     * The size of the stream
     */
    virtual size_t size() const = 0;

    /**
     * Push the current offset on the stack
     */
    void push();
    /**
     * Pop the topmost position (false if the stack was empty)
     */
    bool pop();

private:
    /**
     * we don't want to allow copying and assigning streams
     */
    OLEStream( const OLEStream& rhs );
    /**
     * we don't want to allow copying and assigning streams
     */
    OLEStream& operator=( const OLEStream& rhs );

    std::stack<int> m_positions;
    /**
     *  for bookkeeping :}
     */
	AbstractOLEStorage *m_storage;
};

class AbstractOLEStreamReader : public OLEStream
{
	public:
		AbstractOLEStreamReader(AbstractOLEStorage* storage) : OLEStream(storage){}
		virtual ~AbstractOLEStreamReader(){}
		virtual bool isValid() const = 0;
        virtual bool seek( int offset, int whence = SEEK_SET ) = 0;
		virtual int tell() const = 0;
		virtual size_t size() const = 0;
		virtual U8 readU8() = 0;
		virtual S8 readS8() = 0;
		virtual U16 readU16() = 0;
		virtual S16 readS16() = 0;
		virtual U32 readU32() = 0;
		virtual S32 readS32() = 0;
		virtual bool read( U8 *buffer, size_t length ) = 0;
};

} // namespace wvWare

#endif // OLESTREAM_H
