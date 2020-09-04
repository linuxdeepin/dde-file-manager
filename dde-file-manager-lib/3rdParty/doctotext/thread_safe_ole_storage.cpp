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

#include "thread_safe_ole_storage.h"

#include "data_stream.h"
#include <math.h>
#include "misc.h"
#include <new>
#include <stdio.h>
#include <string.h>
#include "thread_safe_ole_stream_reader.h"

struct ThreadSafeOLEStorage::Implementation {
    bool m_is_valid_ole;
    std::string m_error;
    std::string m_file_name;
    doctotext::DataStream *m_data_stream;
    uint16_t m_sector_size, m_mini_sector_size;
    uint32_t m_number_of_directories;
    uint16_t m_header_version;
    uint32_t m_number_of_fat_sectors;
    uint32_t m_first_sector_directory_location;
    uint32_t m_first_mini_fat_sector_location;
    uint32_t m_number_of_mini_fat_sectors;
    uint32_t m_first_difat_sector_location;
    uint32_t m_number_of_difat_sectors;
    uint32_t m_mini_stream_cut_off;
    uint16_t m_byte_order;

    std::vector<uint32_t> m_fat_sectors_chain;
    std::vector<uint32_t> m_sectors_chain;
    std::vector<uint32_t> m_mini_sectors_chain;

    struct DirectoryEntry {
        std::string m_name;
        enum ObjectType {
            unknown_unallocated = 0x01,
            storage = 0x01,
            stream = 0x02,
            root_storage = 0x05
        };
        ObjectType m_object_type;
        uint8_t m_color_flag;
        uint32_t m_left_sibling;
        uint32_t m_right_sibling;
        uint32_t m_child;
        uint32_t m_start_sector_location;
        uint64_t m_stream_size;
        bool m_added;
    };
    DirectoryEntry *m_current_directory;
    std::vector<DirectoryEntry *> m_directories;
    std::vector<DirectoryEntry *> m_child_directories;
    std::vector<DirectoryEntry *> m_inside_directories;
    bool m_child_directories_loaded;

    Implementation(const std::string &file_name)
    {
        m_file_name = file_name;
        m_current_directory = NULL;
        m_is_valid_ole = true;
        m_data_stream = NULL;
        m_data_stream = new FileStream(file_name);
        if (!m_data_stream->open()) {
            m_is_valid_ole = false;
            m_error = "File " + file_name + " cannot be open";
        }
        m_child_directories_loaded = false;
        parseHeader();
        getFatArraySectorChain();
        getFatSectorChain();
        getMiniFatSectorChain();
        getStoragesAndStreams();
    }

    Implementation(const char *buffer, size_t size)
    {
        m_file_name = "Memory buffer";
        m_current_directory = NULL;
        m_is_valid_ole = true;
        m_data_stream = NULL;
        m_data_stream = new BufferStream(buffer, size);
        if (!m_data_stream->open()) {
            m_is_valid_ole = false;
            m_error = "Memory buffer cannot be open";
        }
        m_child_directories_loaded = false;
        parseHeader();
        getFatArraySectorChain();
        getFatSectorChain();
        getMiniFatSectorChain();
        getStoragesAndStreams();
    }

    ~Implementation()
    {
        if (m_data_stream)
            delete m_data_stream;
    }

    void getStreamPositions(std::vector<uint32_t> &stream_positions, bool mini_stream, DirectoryEntry *dir_entry)
    {
        stream_positions.clear();
        size_t mini_sectors_count = m_number_of_mini_fat_sectors * m_sector_size / 4;
        size_t sectors_count = m_number_of_fat_sectors * m_sector_size / 4;
        if (mini_stream) {
            uint32_t current_mini_sector_index = 0;
            uint32_t mini_sectors_in_sector = m_sector_size / m_mini_sector_size;
            uint32_t mini_sector_location = m_directories[0]->m_start_sector_location;
            uint32_t mini_sector_position = dir_entry->m_start_sector_location;
            while (mini_sector_position != 0xFFFFFFFE) {
                uint32_t sector_index = mini_sector_position / mini_sectors_in_sector;
                if (sector_index != current_mini_sector_index) {
                    current_mini_sector_index = sector_index;
                    mini_sector_location = m_directories[0]->m_start_sector_location;
                    while (sector_index > 0) {
                        if (mini_sector_location >= sectors_count) {
                            stream_positions.clear();
                            return;
                        }
                        mini_sector_location = m_sectors_chain[mini_sector_location];
                        --sector_index;
                    }
                }
                uint32_t mini_sector_offset = mini_sector_position - current_mini_sector_index * mini_sectors_in_sector;
                uint32_t position = (1 + mini_sector_location) * m_sector_size + mini_sector_offset * m_mini_sector_size;
                stream_positions.push_back(position);
                if (mini_sector_position >= mini_sectors_count) {
                    stream_positions.clear();
                    return;
                }
                mini_sector_position = m_mini_sectors_chain[mini_sector_position];
            }
        } else {
            uint32_t sector_location = dir_entry->m_start_sector_location;
            while (sector_location != 0xFFFFFFFE) {
                uint32_t position = (1 + sector_location) * m_sector_size;
                stream_positions.push_back(position);
                if (sector_location >= sectors_count) {
                    stream_positions.clear();
                    return;
                }
                sector_location = m_sectors_chain[sector_location];
            }
        }
    }

    bool getCurrentDirectoryChilds()
    {
        for (size_t i = 0; i < m_child_directories.size(); ++i)
            m_child_directories[i]->m_added = false;
        if (m_current_directory->m_child == 0xFFFFFFFF)
            return true;
        if (m_current_directory->m_child >= m_directories.size()) {
            m_error = "Index of directory entry is outside the vector";
            return false;
        }
        m_child_directories.push_back(m_directories[m_current_directory->m_child]);
        int index_start = 0;
        int index_end = 0;
        Implementation::DirectoryEntry *current_dir = NULL;
        Implementation::DirectoryEntry *added_dir = NULL;
        while (index_start != index_end + 1) {
            current_dir = m_child_directories[index_start];
            if (current_dir->m_left_sibling != 0xFFFFFFFF) {
                if (current_dir->m_left_sibling >= m_directories.size()) {
                    m_error = "Index of directory entry is outside the vector";
                    return false;
                }
                added_dir = m_directories[current_dir->m_left_sibling];
                if (added_dir->m_added == false) {
                    m_child_directories.push_back(added_dir);
                    ++index_end;
                    added_dir->m_added = true;
                }
            }
            if (current_dir->m_right_sibling != 0xFFFFFFFF) {
                if (current_dir->m_right_sibling >= m_directories.size()) {
                    m_error = "Index of directory entry is outside the vector";
                    return false;
                }
                added_dir = m_directories[current_dir->m_right_sibling];
                if (added_dir->m_added == false) {
                    m_child_directories.push_back(added_dir);
                    ++index_end;
                    added_dir->m_added = true;
                }
            }
            ++index_start;
        }
        return true;
    }

    void getStoragesAndStreams()
    {
        if (!m_is_valid_ole)
            return;
        size_t records_count = m_sector_size / 4;
        size_t directory_count_per_sector = m_sector_size / 128;
        DirectoryEntry *directory = NULL;
        uint32_t directory_location = m_first_sector_directory_location;
        while (directory_location != 0xFFFFFFFE) {
            if (!m_data_stream->seek((1 + directory_location) * m_sector_size, SEEK_SET)) {
                m_error = "Position of sector is outside of the file!";
                m_is_valid_ole = false;
                return;
            }
            for (size_t i = 0; i < directory_count_per_sector; ++i) {
                try {
                    directory = NULL;
                    directory = new DirectoryEntry;
                    directory->m_added = false;
                    uint16_t unichars[32];
                    if (!m_data_stream->read(unichars, sizeof(uint16_t), 32)) {
                        m_error = "Error in reading directory name";
                        m_is_valid_ole = false;
                        return;
                    }
                    for (int j = 0; j < 32; ++j) {
                        unsigned int ch = unichars[j];
                        if (ch == 0)
                            break;
                        if (utf16_unichar_has_4_bytes(ch)) {
                            if (++j < 32)
                                ch = (ch << 16) | unichars[j];
                            else
                                break;
                        }
                        directory->m_name += unichar_to_utf8(ch);
                    }
                    if (!skipBytes(2))
                        return;
                    uint8_t object_type;
                    if (!m_data_stream->read(&object_type, sizeof(uint8_t), 1)) {
                        m_error = "Error in reading type of object";
                        m_is_valid_ole = false;
                        return;
                    }
                    if (object_type != 0x00 && object_type != 0x01 && object_type != 0x02 && object_type != 0x05) {
                        m_error = "Invalid type of object";
                        m_is_valid_ole = false;
                        return;
                    }
                    directory->m_object_type = (DirectoryEntry::ObjectType)object_type;
                    uint8_t color_flag;
                    if (!m_data_stream->read(&color_flag, sizeof(uint8_t), 1)) {
                        m_error = "Error in reading color flag";
                        m_is_valid_ole = false;
                        return;
                    }
                    if (color_flag != 0x00 && color_flag != 0x01) {
                        m_error = "Invalid color flag";
                        m_is_valid_ole = false;
                        return;
                    }
                    directory->m_color_flag = color_flag;
                    if (!m_data_stream->read(&directory->m_left_sibling, sizeof(uint32_t), 1)) {
                        m_error = "Error in reading left sibling";
                        m_is_valid_ole = false;
                        return;
                    }
                    if (!m_data_stream->read(&directory->m_right_sibling, sizeof(uint32_t), 1)) {
                        m_error = "Error in reading directory right sibling";
                        m_is_valid_ole = false;
                        return;
                    }
                    if (!m_data_stream->read(&directory->m_child, sizeof(uint32_t), 1)) {
                        m_error = "Error in reading child";
                        m_is_valid_ole = false;
                        return;
                    }
                    if (!skipBytes(36))
                        return;
                    if (!m_data_stream->read(&directory->m_start_sector_location, sizeof(uint32_t), 1)) {
                        m_error = "Error in reading sector location";
                        m_is_valid_ole = false;
                        return;
                    }
                    if (!m_data_stream->read(&directory->m_stream_size, sizeof(uint64_t), 1)) {
                        m_error = "Error in reading sector size";
                        m_is_valid_ole = false;
                        return;
                    }
                    if (m_header_version == 0x03) {
                        directory->m_stream_size = directory->m_stream_size & 0x00000000FFFFFFFF;
                    }
                    m_directories.push_back(directory);
                } catch (std::bad_alloc &ba) {
                    if (directory != NULL)
                        delete directory;
                    directory = NULL;
                    throw;
                }
            }
            if (directory_location >= m_number_of_fat_sectors * records_count) {
                m_error = "Directory location is outside of the sector chain";
                m_is_valid_ole = false;
                return;
            }
            directory_location = m_sectors_chain[directory_location];
        }
        if (m_directories.size() == 0) {
            m_is_valid_ole = false;
            m_error = "Root directory does not exist";
            return;
        }
        m_current_directory = m_directories[0];
        m_data_stream->close();
    }

    void getMiniFatSectorChain()
    {
        if (!m_is_valid_ole)
            return;
        size_t records_count = m_sector_size / 4;
        m_mini_sectors_chain.reserve(m_number_of_mini_fat_sectors * records_count);
        size_t mini_sector_location = m_first_mini_fat_sector_location;
        for (size_t i = 0; i < m_number_of_mini_fat_sectors; ++i) {
            if (!m_data_stream->seek((1 + mini_sector_location) * m_sector_size, SEEK_SET)) {
                m_error = "Position of sector is outside of the file!";
                m_is_valid_ole = false;
                return;
            }
            if (!m_data_stream->read(&m_mini_sectors_chain[i * records_count], sizeof(uint32_t), records_count)) {
                m_error = "Error in reading sector chain";
                m_is_valid_ole = false;
                return;
            }
            if (mini_sector_location >= m_number_of_fat_sectors * records_count) {
                m_error = "Mini sector location is outside of the sector chain";
                m_is_valid_ole = false;
                return;
            }
            mini_sector_location = m_sectors_chain[mini_sector_location];
            if (mini_sector_location == 0xFFFFFFFE)
                break;
        }
    }

    void getFatSectorChain()
    {
        if (!m_is_valid_ole)
            return;
        size_t records_count = m_sector_size / 4;
        m_sectors_chain.reserve(m_number_of_fat_sectors * records_count);
        for (size_t i = 0; i < m_number_of_fat_sectors; ++i) {
            if (!m_data_stream->seek((1 + m_fat_sectors_chain[i]) * m_sector_size, SEEK_SET)) {
                m_error = "Position of sector is outside of the file!";
                m_is_valid_ole = false;
                return;
            }
            if (!m_data_stream->read(&m_sectors_chain[i * records_count], sizeof(uint32_t), records_count)) {
                m_error = "Error in reading sector chain";
                m_is_valid_ole = false;
                return;
            }
        }
    }

    void getFatArraySectorChain()
    {
        if (!m_is_valid_ole)
            return;
        uint32_t records_count = m_sector_size / 4 - 1;
        m_fat_sectors_chain.reserve(m_number_of_fat_sectors);
        uint32_t remaining_fat_sector_chain_count = m_number_of_fat_sectors;
        for (int i = 0; i < 109; ++i) {
            if (remaining_fat_sector_chain_count == 0)
                break;
            if (!getUint32(m_fat_sectors_chain[i]))
                return;
            --remaining_fat_sector_chain_count;
        }
        uint32_t difat_sector_location = m_first_difat_sector_location;
        for (uint32_t i = 0; i < m_number_of_difat_sectors; ++i) {
            if (!m_data_stream->seek((1 + difat_sector_location) * m_sector_size, SEEK_SET)) {
                m_error = "Position of sector is outside of the file!";
                m_is_valid_ole = false;
                return;
            }
            for (uint32_t j = 0; j < records_count; ++j) {
                if (remaining_fat_sector_chain_count == 0)
                    break;
                if (!getUint32(m_fat_sectors_chain[109 + i * records_count + j]))
                    return;
                --remaining_fat_sector_chain_count;
            }
            if (!getUint32(difat_sector_location))
                return;
            if (difat_sector_location == 0xFFFFFFFE)
                break;
        }
    }

    bool skipBytes(uint32_t bytes_to_skip)
    {
        if (!m_data_stream->seek(bytes_to_skip, SEEK_CUR)) {
            m_is_valid_ole = false;
            m_error = "Cant seek";
            return false;
        }
        return  true;
    }

    bool getUint16(uint16_t &data)
    {
        if (!m_data_stream->read(&data, sizeof(uint16_t), 1)) {
            m_is_valid_ole = false;
            m_error = "Error in reading 16-bit number";
            return false;
        }
        return true;
    }

    bool getUint32(uint32_t &data)
    {
        if (!m_data_stream->read(&data, sizeof(uint32_t), 1)) {
            m_is_valid_ole = false;
            m_error = "Error in reading 32-bit number";
            return false;
        }
        return true;
    }

    void parseHeader()
    {
        uint8_t ole_header[] = {0xD0, 0xCF, 0x11, 0xE0, 0xA1, 0xB1, 0x1A, 0xE1};
        uint8_t readed_ole_header[8];
        if (!m_is_valid_ole)
            return;
        if (!m_data_stream->read(readed_ole_header, sizeof(uint8_t), 8) ||
                memcmp(readed_ole_header, ole_header, 8) != 0) {
            m_is_valid_ole = false;
            m_error = "Header is invalid: no OLE signature";
            return;
        }
        if (!skipBytes(18))
            return;
        if (!getUint16(m_header_version))
            return;
        if (!getUint16(m_byte_order))
            return;
        if (!getUint16(m_sector_size))
            return;
        m_sector_size = (uint16_t)pow(2, m_sector_size);
        if (!getUint16(m_mini_sector_size))
            return;
        m_mini_sector_size = (uint16_t)pow(2, m_mini_sector_size);
        if (!skipBytes(6))
            return;
        if (!getUint32(m_number_of_directories))
            return;
        if (!getUint32(m_number_of_fat_sectors))
            return;
        if (!getUint32(m_first_sector_directory_location))
            return;
        if (!skipBytes(4))
            return;
        if (!getUint32(m_mini_stream_cut_off))
            return;
        if (!getUint32(m_first_mini_fat_sector_location))
            return;
        if (!getUint32(m_number_of_mini_fat_sectors))
            return;
        if (!getUint32(m_first_difat_sector_location))
            return;
        if (!getUint32(m_number_of_difat_sectors))
            return;
    }
};

ThreadSafeOLEStorage::ThreadSafeOLEStorage(const std::string &file_name)
{
    impl = NULL;
    try {
        impl = new Implementation(file_name);
    } catch (std::bad_alloc &ba) {
        if (impl != NULL) {
            delete impl;
        }
        throw;
    }
}

ThreadSafeOLEStorage::ThreadSafeOLEStorage(const char *buffer, size_t len)
{
    impl = NULL;
    try {
        impl = new Implementation(buffer, len);
    } catch (std::bad_alloc &ba) {
        if (impl != NULL) {
            delete impl;
        }
        throw;
    }
}

ThreadSafeOLEStorage::~ThreadSafeOLEStorage()
{
    if (impl != NULL) {
        delete impl;
    }
}

bool ThreadSafeOLEStorage::open(Mode mode)
{
    if (mode == ReadOnly)   //opening in constructor
        return true;
    return false;           //only read mode is supported now
}

bool ThreadSafeOLEStorage::isValid() const
{
    return impl->m_is_valid_ole;
}

void ThreadSafeOLEStorage::close()
{
    if (impl != NULL) {
        delete impl;
        impl = NULL;
    }
}

std::string ThreadSafeOLEStorage::getLastError()
{
    return impl->m_error;
}

std::string ThreadSafeOLEStorage::name() const
{
    return impl->m_file_name;
}

bool ThreadSafeOLEStorage::getStreamsAndStoragesList(std::vector<std::string> &components)
{
    components.clear();
    if (impl == NULL || impl->m_is_valid_ole == false || impl->m_current_directory == NULL)
        return false;
    if (impl->m_child_directories_loaded == false) {
        if (!impl->getCurrentDirectoryChilds())
            return false;
        impl->m_child_directories_loaded = true;
    }
    for (size_t i = 0; i < impl->m_child_directories.size(); ++i) {
        components.push_back(impl->m_child_directories[i]->m_name);
    }
    return true;
}

std::string ThreadSafeOLEStorage::getCurrentDirectory()
{
    if (impl == NULL || impl->m_is_valid_ole == false || impl->m_current_directory == NULL)
        return "";
    return impl->m_current_directory->m_name;
}

bool ThreadSafeOLEStorage::enterDirectory(const std::string &directory_path)
{
    if (impl == NULL || impl->m_is_valid_ole == false || impl->m_current_directory == NULL)
        return false;
    if (impl->m_child_directories_loaded == false) {
        if (!impl->getCurrentDirectoryChilds())
            return false;
        impl->m_child_directories_loaded = true;
    }
    for (size_t i = 0; i < impl->m_child_directories.size(); ++i) {
        if (impl->m_child_directories[i]->m_name == directory_path) {
            if (impl->m_child_directories[i]->m_object_type != Implementation::DirectoryEntry::storage) {
                impl->m_error = "Specified object is not directory";
                return false;
            }
            impl->m_inside_directories.push_back(impl->m_current_directory);
            impl->m_current_directory = impl->m_child_directories[i];
            impl->m_child_directories_loaded = false;
            for (size_t i = 0; i < impl->m_child_directories.size(); ++i)
                impl->m_child_directories[i]->m_added = false;
            impl->m_child_directories.clear();
            return true;
        }
    }
    impl->m_error = "Specified directory does not exist";
    return false;
}

bool ThreadSafeOLEStorage::leaveDirectory()
{
    if (impl == NULL || impl->m_is_valid_ole == false || impl->m_current_directory == NULL)
        return false;
    if (impl->m_inside_directories.size() == 0) {
        impl->m_error = "Already in root directory";
        return false;
    }
    impl->m_current_directory = impl->m_inside_directories.back();
    impl->m_inside_directories.pop_back();
    impl->m_child_directories_loaded = false;
    for (size_t i = 0; i < impl->m_child_directories.size(); ++i)
        impl->m_child_directories[i]->m_added = false;
    impl->m_child_directories.clear();
    return true;
}

AbstractOLEStreamReader *ThreadSafeOLEStorage::createStreamReader(const std::string &stream_path)
{
    if (impl == NULL || impl->m_is_valid_ole == false || impl->m_current_directory == NULL)
        return NULL;
    if (impl->m_child_directories_loaded == false) {
        if (!impl->getCurrentDirectoryChilds())
            return NULL;
        impl->m_child_directories_loaded = true;
    }
    for (size_t i = 0; i < impl->m_child_directories.size(); ++i) {
        if (impl->m_child_directories[i]->m_name == stream_path) {
            if (impl->m_child_directories[i]->m_object_type != Implementation::DirectoryEntry::stream) {
                impl->m_error = "Specified object is not a stream";
                return NULL;
            }
            ThreadSafeOLEStreamReader::Stream stream;
            ThreadSafeOLEStreamReader *ole_stream_reader = NULL;
            stream.m_data_stream = NULL;
            try {
                stream.m_data_stream = impl->m_data_stream->clone();
                stream.m_size = impl->m_child_directories[i]->m_stream_size;
                if (stream.m_size < impl->m_mini_stream_cut_off) {
                    stream.m_sector_size = impl->m_mini_sector_size;
                    impl->getStreamPositions(stream.m_file_positions, true, impl->m_child_directories[i]);
                } else {
                    stream.m_sector_size = impl->m_sector_size;
                    impl->getStreamPositions(stream.m_file_positions, false, impl->m_child_directories[i]);
                }
                ole_stream_reader = new ThreadSafeOLEStreamReader(this, stream);
                if (!ole_stream_reader->isValid()) {
                    impl->m_error = ole_stream_reader->getLastError();
                    delete ole_stream_reader;
                    return NULL;
                }
                return ole_stream_reader;
            } catch (std::bad_alloc &ba) {
                if (stream.m_data_stream != NULL)
                    delete stream.m_data_stream;
                if (ole_stream_reader != NULL)
                    delete ole_stream_reader;
                throw;
            }
        }
    }
    impl->m_error = "Specified stream does not exist";
    return NULL;
}

bool ThreadSafeOLEStorage::readDirectFromBuffer(unsigned char *buffer, int size, int offset)
{
    /*fix buf 42500 文件描述符打开未关闭导致多次创建的时候索引创建失败*/
    if (!impl->m_data_stream->open()) {
        impl->m_error = "Cannotopen file " + impl->m_file_name;
        return false;
    }
    if (!impl->m_data_stream->seek(offset, SEEK_SET)) {
        impl->m_error = "Cant seek to the selected position";
        impl->m_data_stream->close();
        return false;
    }
    if (!impl->m_data_stream->read(buffer, sizeof(unsigned char), size)) {
        impl->m_error = "Cant read from file";
        impl->m_data_stream->close();
        return false;
    }
    impl->m_data_stream->close();
    return true;
}

void ThreadSafeOLEStorage::streamDestroyed(OLEStream *stream)
{
    //nothing to do. Stream is already self-sufficient and storage does not care about it
}
