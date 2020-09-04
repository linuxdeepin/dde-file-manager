/****************************************************************************
**
** DocToText - Converts DOC, XLS, XLSB, PPT, RTF, ODF (ODT, ODS, ODP),
**             OOXML (DOCX, XLSX, PPTX), iWork (PAGES, NUMBERS, KEYNOTE),
**             ODFXML (FODP, FODS, FODT), PDF, EML and HTML documents to plain text.
**             Extracts metadata and annotations.
**
** Copyright (c) 2006-2013, SILVERCODERS(R)
** http://silvercoders.com
**
** Project homepage: http://silvercoders.com/en/products/doctotext
**
** This program may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file COPYING.GPL included in the
** packaging of this file.
**
** Please remember that any attempt to workaround the GNU General Public
** License using wrappers, pipes, client/server protocols, and so on
** is considered as license violation. If your program, published on license
** other than GNU General Public License version 2, calls some part of this
** code directly or indirectly, you have to buy commercial license.
** If you do not like our point of view, simply do not use the product.
**
** Licensees holding valid commercial license for this product
** may use this file in accordance with the license published by
** SILVERCODERS and appearing in the file COPYING.COM
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
**
*****************************************************************************/

#include "data_stream.h"

#include <new>
#include <string.h>
#include <sys/stat.h>

using namespace doctotext;
struct FileStream::Implementation {
    FILE *m_file;
    std::string m_file_name;
    bool m_opened;
};

FileStream::FileStream(const std::string &file_name)
{
    impl = NULL;
    try {
        impl = new Implementation;
        impl->m_file = NULL;
        impl->m_opened = false;
        impl->m_file_name = file_name;
    } catch (std::bad_alloc &ba) {
        if (impl) {
            delete impl;
        }
        impl = NULL;
        throw;
    }
}

FileStream::~FileStream()
{
    if (impl) {
        if (impl->m_file)
            fclose(impl->m_file);
        delete impl;
    }
}

bool FileStream::open()
{
    if (impl->m_opened) {
        return true;
    }

    impl->m_file = fopen(impl->m_file_name.c_str(), "rb");

    if (impl->m_file != NULL) {
        impl->m_opened = true;
    }

    return impl->m_opened;
}

bool FileStream::close()
{
    if (!impl->m_opened) {
        return true;
    }

    if (impl->m_file) {
        fclose(impl->m_file);
    }
    impl->m_opened = false;
    impl->m_file = NULL;

    return true;
}

bool FileStream::read(void *data, int element_size, size_t elements_num)
{
    if (!impl->m_opened)
        return false;
    if (fread(data, element_size, elements_num, impl->m_file) != elements_num)
        return false;
    return true;
}

bool FileStream::seek(int offset, int whence)
{
    if (!impl->m_opened)
        return false;
    if (fseek(impl->m_file, offset, whence) != 0)
        return false;
    return true;
}

bool FileStream::eof()
{
    if (!impl->m_opened)
        return true;
    return !(feof(impl->m_file) == 0);
}

int FileStream::getc()
{
    if (!impl->m_opened)
        return 0;
    return fgetc(impl->m_file);
}

bool FileStream::unGetc(int ch)
{
    if (!impl->m_opened)
        return false;
    return ungetc(ch, impl->m_file) == ch;
}

size_t FileStream::size()
{
    if (!impl->m_opened)
        return 0;
//  size_t current = ftell(impl->m_file);
//  if (fseek(impl->m_file, 0, SEEK_END) != 0)
//      return 0;
//  size_t size = ftell(impl->m_file);
//  fseek(impl->m_file, current, SEEK_SET);
//  return size;

    // ftp不支持fseek
    struct stat info;
    if (stat(impl->m_file_name.c_str(), &info) < 0) {
        return 0;
    }
    return info.st_size;
}

size_t FileStream::tell()
{
    return ftell(impl->m_file);
}

std::string FileStream::name()
{
    return impl->m_file_name;
}

DataStream *FileStream::clone()
{
    return new FileStream(impl->m_file_name);
}

struct BufferStream::Implementation {
    const char *m_buffer;
    size_t m_size;
    size_t m_pointer;
};

BufferStream::BufferStream(const char *buffer, size_t size)
{
    impl = NULL;
    try {
        impl = new Implementation;
        impl->m_buffer = buffer;
        impl->m_size = size;
        impl->m_pointer = 0;
    } catch (std::bad_alloc &ba) {
        if (impl)
            delete impl;
        impl = NULL;
        throw;
    }
}

BufferStream::~BufferStream()
{
    if (impl)
        delete impl;
}

bool BufferStream::open()
{
    impl->m_pointer = 0;
    return true;
}

bool BufferStream::close()
{
    return true;
}

bool BufferStream::read(void *data, int element_size, size_t elements_num)
{
    size_t len = element_size * elements_num;
    if (len > impl->m_size - impl->m_pointer)
        return false;
    memcpy(data, impl->m_buffer + impl->m_pointer, len);
    impl->m_pointer += len;
    return true;
}

bool BufferStream::seek(int offset, int whence)
{
    size_t position;
    switch (whence) {
    case SEEK_SET:
        position = offset;
        break;
    case SEEK_CUR:
        position = impl->m_pointer + offset;
        break;
    case SEEK_END:
        position = impl->m_size + offset;
        break;
    default:
        return false;
    }
    if (position > impl->m_size)
        return false;
    impl->m_pointer = position;
    return true;
}

bool BufferStream::eof()
{
    return impl->m_pointer == impl->m_size;
}

int BufferStream::getc()
{
    if (impl->m_size - impl->m_pointer < 1)
        return EOF;
    return impl->m_buffer[impl->m_pointer++];
}

bool BufferStream::unGetc(int ch)
{
    if (impl->m_pointer < 1)
        return false;
    --impl->m_pointer;
}

size_t BufferStream::size()
{
    return impl->m_size;
}

size_t BufferStream::tell()
{
    return impl->m_pointer;
}

std::string BufferStream::name()
{
    return "Memory buffer";
}

DataStream *BufferStream::clone()
{
    return new BufferStream(impl->m_buffer, impl->m_size);
}
