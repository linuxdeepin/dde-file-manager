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

#include "parser.h"
#include "olestream.h"
#include "handlers.h"

using namespace wvWare;

Parser::Parser( AbstractOLEStorage* storage, AbstractOLEStreamReader* wordDocument ) :
    m_inlineHandler( new InlineReplacementHandler ), m_subDocumentHandler( new SubDocumentHandler ),
    m_tableHandler( new TableHandler ), m_textHandler( new TextHandler ),
    m_ourInlineHandler( true ), m_ourSubDocumentHandler( true ), m_ourTableHandler( true ),
    m_ourTextHandler( true ), m_storage( storage ), m_wordDocument( wordDocument ), m_okay( true )
{
    if ( !storage || !wordDocument ) {
        m_okay = false;
        return;
    }

    // Let the "real" constructor find out which additional
    // streams it needs, as only it knows the FIB!
}

Parser::~Parser()
{
    // In case we're still using the default handlers we have
    // to clean them up. Messy, but we shouldn't use some smart
    // pointer here, as it's very restrictive for the user of
    // this library (std::auto_ptr would make it impossible to
    // use MI, SharedPtr would potentially lead to circular references).
    if ( m_ourInlineHandler )
        delete m_inlineHandler;
    if ( m_ourSubDocumentHandler )
        delete m_subDocumentHandler;
    if ( m_ourTableHandler )
        delete m_tableHandler;
    if ( m_ourTextHandler )
        delete m_textHandler;

    // Don't forget to close everything properly here
    delete m_wordDocument;
    m_storage->close();
    delete m_storage;
}

void Parser::setInlineReplacementHandler( InlineReplacementHandler* handler )
{
    if ( m_ourInlineHandler ) {
        m_ourInlineHandler = false;
        delete m_inlineHandler;
    }
    m_inlineHandler = handler;
}

void Parser::setSubDocumentHandler( SubDocumentHandler* handler )
{
    if ( m_ourSubDocumentHandler ) {
        m_ourSubDocumentHandler = false;
        delete m_subDocumentHandler;
    }
    m_subDocumentHandler = handler;
}

void Parser::setTableHandler( TableHandler* handler )
{
    if ( m_ourTableHandler ) {
        m_ourTableHandler = false;
        delete m_tableHandler;
    }
    m_tableHandler = handler;
}

void Parser::setTextHandler( TextHandler* handler )
{
    if ( m_ourTextHandler ) {
        m_ourTextHandler = false;
        delete m_textHandler;
    }
    m_textHandler = handler;
}
