#ifndef DOCTOTEXT_THREAD_SAFE_OLE_STREAM_READER_H
#define DOCTOTEXT_THREAD_SAFE_OLE_STREAM_READER_H

#include <stdint.h>
#include <stdio.h>
#include <string>
#include <vector>
#include "olestream.h"
#include "data_stream.h"
class ThreadSafeOLEStorage;
using namespace wvWare;
using namespace doctotext;
class DataStream;

class ThreadSafeOLEStreamReader : public AbstractOLEStreamReader
{
    friend class ThreadSafeOLEStorage;
private:
    struct Implementation;
    Implementation *impl;
    struct Stream {
        uint64_t m_size;
        std::vector<uint32_t> m_file_positions;
        uint32_t m_sector_size;
        doctotext::DataStream *m_data_stream;
    };
    ThreadSafeOLEStreamReader(ThreadSafeOLEStorage *storage, Stream &stream);
public:
    ~ThreadSafeOLEStreamReader();
    std::string getLastError();
    bool isValid() const;
    int tell() const;
    size_t size() const;
    bool seek(int offset, int whence = SEEK_SET);
    bool readU8(U8 &data);
    U8 readU8();
    bool readS8(S8 &data);
    S8 readS8();
    bool readU16(U16 &data);
    U16 readU16();
    bool readS16(S16 &data);
    S16 readS16();
    bool readU32(U32 &data);
    U32 readU32();
    bool readS32(S32 &data);
    S32 readS32();
    bool read(U8 *buffer, size_t length);
};

#endif // DOCTOTEXT_THREAD_SAFE_OLE_STREAM_READER_H
