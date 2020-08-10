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

#ifndef FUNCTOR_H
#define FUNCTOR_H

namespace wvWare
{
    /**
     * FunctorBase is provided to allow polymorphic handling of different
     * types of functors. Use it like the real functor.
     */
    class FunctorBase
    {
    public:
        virtual ~FunctorBase() = 0;
        virtual void operator()() const;
    };

    /**
     * The Functor class is used to pass on callback information to the
     * consumer filter. You may copy and assign it, to invoke it just
     * call operator().
     */
    template<class ParserT, typename Data>
    class Functor : public FunctorBase
    {
    public:
        typedef void (ParserT::*F)( const Data& );

        Functor( ParserT& parser, F f, const Data& data )
            : m_parser( &parser ), f_( f ), m_data( data ) {}
        virtual void operator()() const { (m_parser->*f_)( m_data ); }

    private:
        ParserT* m_parser;
        F  f_;
        Data m_data;
    };

    /**
     * A small helper function to avoid unnecessary uglyness in the template
     * creation code. The function figures out the types and we don't have to
     * specify them explicitly every time we create a functor.
     */
    template<class ParserT, typename Data>
    Functor<ParserT, Data> make_functor( ParserT& parser, void (ParserT::*f) ( const Data& ), const Data& data )
    {
        return Functor<ParserT, Data>( parser, f, data );
    }

} // namespace wvWare

#endif // FUNCTOR_H
