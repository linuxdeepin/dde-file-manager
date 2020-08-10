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

#include "olestorage.h"
#include "olestream.h"
#include "wvlog.h"

#include <gsf/gsf-input-memory.h>
#include <gsf/gsf-infile.h>
#include <gsf/gsf-infile-msole.h>
#include <gsf/gsf-output-stdio.h>
#include <gsf/gsf-outfile.h>
#include <gsf/gsf-outfile-msole.h>
#include <gsf/gsf-utils.h>

using namespace wvWare;

OLEStorage::OLEStorage() : m_inputFile( 0 ), m_outputFile( 0 ),
    m_fileName( "" ), m_buffer( 0 ), m_buflen( 0 )
{
    // According to Dom it's no problem to call that function multiple times
    gsf_init();
}

OLEStorage::OLEStorage( const std::string& fileName ) : m_inputFile( 0 ),
    m_outputFile( 0 ), m_fileName( fileName ), m_buffer( 0 ), m_buflen( 0 )
{
    // According to Dom it's no problem to call that function multiple times
    gsf_init();
}

OLEStorage::OLEStorage( const unsigned char* buffer, size_t buflen ) :
    m_inputFile( 0 ), m_outputFile( 0 ), m_fileName( "" ), m_buffer( buffer ), m_buflen( buflen )
{
    // According to Dom it's no problem to call that function multiple times
    gsf_init();
}

OLEStorage::~OLEStorage()
{
    close();  // just in case
    // According to Dom it's no problem to call that function multiple times
    gsf_shutdown();
}

void OLEStorage::setName( const std::string& fileName )
{
    // Don't set a new name if we already have an open storage or a buffer
    if ( m_inputFile || m_outputFile || m_buffer )
        return;
    m_fileName = fileName;
}

void OLEStorage::setBuffer( const unsigned char* buffer, size_t buflen )
{
    // Don't set a new buffer if we already have an open storage or a filename
    if ( m_inputFile || m_outputFile || !m_fileName.empty() )
        return;
    m_buffer = buffer;
    m_buflen = buflen;
}

bool OLEStorage::open( Mode mode )
{
    // We already have an open storage, return as appropriate
    if ( m_inputFile && mode == ReadOnly )
        return true;
    else if ( m_outputFile && mode == WriteOnly )
        return true;
    else if ( m_inputFile || m_outputFile )
        return false;

    if ( m_fileName.empty() && ( mode == WriteOnly || !m_buffer ) )
        return false;

    GError* err = 0;

    if ( mode == ReadOnly )  {
        GsfInput* input;
        if ( m_buffer )
            input = GSF_INPUT( gsf_input_memory_new( m_buffer, m_buflen, false ) );
        else
            input = GSF_INPUT( gsf_input_mmap_new( m_fileName.c_str(), &err ) );

        if ( !input )  {
            if ( !err )
                return false;
            wvlog << m_fileName <<  " error: " << err->message << std::endl;
            g_error_free( err );
            return false;
        }

        m_inputFile = GSF_INFILE( gsf_infile_msole_new( input, &err ) );
	g_object_unref( G_OBJECT( input ) );
        if ( !m_inputFile ) {
            if ( !err )
                return false;
            wvlog << m_fileName << " Not an OLE file: " << err->message << std::endl;
            g_error_free( err );
            return false;
        }
    }
    else {
        GsfOutput* output = GSF_OUTPUT( gsf_output_stdio_new( m_fileName.c_str(), &err ) );
        if ( !output )  {
            if ( !err )
                return false;
            wvlog << m_fileName << " error: " << err->message << std::endl;
            g_error_free( err );
            return false;
        }

        m_outputFile = GSF_OUTFILE( gsf_outfile_msole_new( output ) );
	g_object_unref( G_OBJECT(output) );
    }
    return true;
}

void OLEStorage::close()
{
    // check if we still have some open streams and close them properly
    std::list<OLEStream*>::const_iterator it = m_streams.begin();
    std::list<OLEStream*>::const_iterator end = m_streams.end();
    while ( it != end ) {
        OLEStream* stream( *it );
        ++it; // first advance the iterator, as the stream will remove itself from the list
        delete stream;
        wvlog << "Warning: Closing a stream you didn't delete." << std::endl;
    }
    m_streams.clear(); // should be a no-op

    if ( m_inputFile )  {
        g_object_unref( G_OBJECT( m_inputFile ) );
        m_inputFile = 0;
    }

    if ( m_outputFile )  {
        gsf_output_close( reinterpret_cast<GsfOutput*>( m_outputFile ) );
        g_object_unref( G_OBJECT( m_outputFile ) );
        m_outputFile = 0;
    }
}

bool OLEStorage::isValid() const
{
    return m_inputFile || m_outputFile;
}

std::list<std::string> OLEStorage::listDirectory()
{
    std::list<std::string> ret;

    if ( m_outputFile || !m_inputFile ) // outfiles don't seem to support that feature :-(
        return ret;

    GsfInfile* currentDir( m_inputFile );
    if ( !m_path.empty() )
        currentDir = m_path.back().infile;

    int numChildren = gsf_infile_num_children( currentDir );
    for ( int i = 0; i < numChildren; ++i ) {
        GsfInput* entry( gsf_infile_child_by_index( currentDir, i ) );
        const char* name( gsf_input_name( entry ) );
        ret.push_back( name != 0 ? name : "[unnamed]" );
        g_object_unref( G_OBJECT( entry ) );
    }
    return ret;
}

bool OLEStorage::enterDirectory( const std::string& directory )
{
    if ( m_inputFile ) {
        GsfInfile* currentDir( m_inputFile );
        if ( !m_path.empty() )
            currentDir = m_path.back().infile;

        GsfInput* dir( gsf_infile_child_by_name( currentDir, directory.c_str() ) );

        if ( dir && GSF_IS_INFILE( dir ) &&
             gsf_infile_num_children( GSF_INFILE( dir ) ) >= 0 ) {
            m_path.push_back( GSF_INFILE( dir ) );
            return true;
        }
    }
    else if ( m_outputFile ) {
        GsfOutfile* currentDir( m_outputFile );
        if ( !m_path.empty() )
            currentDir = m_path.back().outfile;

        GsfOutput* newDir( gsf_outfile_new_child( currentDir, directory.c_str(), TRUE ) );
        if ( newDir ) {
            m_path.push_back( GSF_OUTFILE( newDir ) );
            return true;
        }
    }
    return false; // no file opened
}

void OLEStorage::leaveDirectory()
{
    if ( !m_path.empty() ) {
        if ( m_inputFile )
            g_object_unref( G_OBJECT( m_path.back().infile ) );
        else if ( m_outputFile ) {
            gsf_output_close( reinterpret_cast<GsfOutput*>( m_path.back().outfile ) );
            g_object_unref( G_OBJECT( m_path.back().outfile ) );
        }
        m_path.pop_back();
    }
}

bool OLEStorage::setPath( const std::string& path )
{
    // Save the old path
    std::deque<Directory> oldPath;
    oldPath.swap( m_path );

    std::string tmp;
    std::string::size_type start = 1, end = 0;
    bool success = true;

    if ( path[ 0 ] != '/' )
        start = 0;

    while ( start < path.length() && success ) {
        end = path.find_first_of( '/', start );
        if ( end != std::string::npos ) {
            tmp = path.substr( start, end - start );
            start = end + 1;
        }
        else {
            tmp = path.substr( start );
            start = std::string::npos;
        }
        if ( !enterDirectory( tmp ) )
            success = false;
    }

    // If we were successful, we save the current m_path in
    // the oldPath, and free m_path using leaveDirectory
    if ( success )
        oldPath.swap( m_path );
    while ( !m_path.empty() )
        leaveDirectory();

    // Now restore the correct path in m_path
    oldPath.swap( m_path );
    return success;
}

std::string OLEStorage::path() const
{
    std::deque<Directory>::const_iterator it( m_path.begin() );
    std::deque<Directory>::const_iterator end( m_path.end() );
    std::string p( "/" );
    for ( ; it != end; ++it ) {
        const char* name = 0;
        if ( m_inputFile )
            name = gsf_input_name( GSF_INPUT( ( *it ).infile ) ); // it->infile doesn't work with gcc 2.95.2
        else if ( m_outputFile )
            name = gsf_output_name( GSF_OUTPUT( ( *it ).outfile ) ); // it->outfile doesn't work with gcc 2.95.2

        if ( name ) {
            p.append( name );
            p.push_back( '/' );
        }
    }
    return p;
}

AbstractOLEStreamReader* OLEStorage::createStreamReader( const std::string& stream )
{
    if ( !m_inputFile )
        return 0;

    GsfInfile* currentDir( m_inputFile );
    if ( !m_path.empty() )
        currentDir = m_path.back().infile;
    GsfInput* input( gsf_infile_child_by_name( currentDir, stream.c_str() ) );

    if ( !input )
        return 0;

	AbstractOLEStreamReader* reader( new OLEStreamReader( input, this ) );
    m_streams.push_back( reader );
    return reader;
}

OLEStreamWriter* OLEStorage::createStreamWriter( const std::string& stream )
{
    if ( !m_outputFile )
        return 0;
    // Don't try to confuse our highly intelligent path system :p
    if ( stream.find('/') != std::string::npos ) {
        wvlog << "Warning: You tried to create a stream with a '/' in its name." << std::endl;
        return 0;
    }

    GsfOutfile* currentDir( m_outputFile );
    if ( !m_path.empty() )
        currentDir = m_path.back().outfile;
    GsfOutput* output( gsf_outfile_new_child( currentDir, stream.c_str(), FALSE ) );

    if ( !output )
        return 0;

    OLEStreamWriter* writer( new OLEStreamWriter( output, this ) );
    m_streams.push_back( writer );
    return writer;
}

void OLEStorage::streamDestroyed( OLEStream* stream )
{
    m_streams.remove( stream );
}
