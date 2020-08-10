/* This file is part of the KDE libraries
   Copyright (c) 1999 Waldo Bastian <bastian@kde.org>
   Copyright (c) 2001-2003 Werner Trobin <trobin@kde.org>

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
#ifndef SharedPTR_H
#define SharedPTR_H

#include "dllmagic.h"

namespace wvWare {

/**
 * Reference counting for shared objects.  If you derive your object
 * from this class, then you may use it in conjunction with
 * @ref SharedPtr to control the lifetime of your object.
 *
 * Specifically, all classes that derive from Shared have an internal
 * counter keeping track of how many other objects have a reference to
 * their object.  If used with @ref SharedPtr, then your object will
 * not be deleted until all references to the object have been
 * released.
 *
 * You should probably not ever use any of the methods in this class
 * directly -- let the @ref SharedPtr take care of that.  Just derive
 * your class from Shared and forget about it.
 *
 * @author Waldo Bastian <bastian@kde.org>
 */
class WV2_DLLEXPORT Shared {
public:
   /**
    * Standard constructor.  This will initialize the reference count
    * on this object to 0
    */
   Shared() : count( 0 ) { }

   /**
    * Copy constructor.  This will @em not actually copy the objects
    * but it will initialize the reference count on this object to 0
    */
   Shared( const Shared & ) : count( 0 ) { }

   /**
    * Overloaded assignment operator
    */
   Shared &operator=( const Shared & ) { return *this; }

   /**
    * Increases the reference count by one
    */
   void _Shared_ref() const { count++; }

   /**
    * Releases a reference (decreases the reference count by one).  If
    * the count goes to 0, this object will delete itself
    */
   void _Shared_deref() const { if (!--count) delete this; }

   /**
    * Return the current number of references held
    *
    * @return Number of references
    */
   int _Shared_count() const { return count; }

protected:
   virtual ~Shared() { }
private:
   mutable int count;
};

/**
 * Can be used to control the lifetime of an object that has derived
 * @ref Shared. As long a someone holds a SharedPtr on some Shared
 * object it won't become deleted but is deleted once its reference
 * count is 0.  This struct emulates C++ pointers perfectly. So just
 * use it like a simple C++ pointer.
 *
 * @author Waldo Bastian <bastian@kde.org>
 */
template< class T >
struct SharedPtr
{
public:
  SharedPtr()
    : ptr(0) { }
  SharedPtr( T* t )
    : ptr(t) { if ( ptr ) ptr->_Shared_ref(); }
  SharedPtr( const SharedPtr& p )
    : ptr(p.ptr) { if ( ptr ) ptr->_Shared_ref(); }

  ~SharedPtr() { if ( ptr ) ptr->_Shared_deref(); }

  SharedPtr<T>& operator= ( const SharedPtr<T>& p ) {
    if ( ptr == p.ptr ) return *this;
    if ( ptr ) ptr->_Shared_deref();
    ptr = p.ptr;
    if ( ptr ) ptr->_Shared_ref();
    return *this;
  }
  SharedPtr<T>& operator= ( T* p ) {
    if ( ptr == p ) return *this;
    if ( ptr ) ptr->_Shared_deref();
    ptr = p;
    if ( ptr ) ptr->_Shared_ref();
    return *this;
  }
  bool operator== ( const SharedPtr<T>& p ) const { return ( ptr == p.ptr ); }
  bool operator!= ( const SharedPtr<T>& p ) const { return ( ptr != p.ptr ); }
  bool operator== ( const T* p ) const { return ( ptr == p ); }
  bool operator!= ( const T* p ) const { return ( ptr != p ); }
  bool operator!() const { return ( ptr == 0 ); }
  operator T*() const { return ptr; }

  T* data() { return ptr; }
  const T* data() const { return ptr; }

  const T& operator*() const { return *ptr; }
  T& operator*() { return *ptr; }
  const T* operator->() const { return ptr; }
  T* operator->() { return ptr; }

  int count() const { return ptr->_Shared_count(); } // for debugging purposes
private:
  T* ptr;
};

} // namespace wvWare

#endif
