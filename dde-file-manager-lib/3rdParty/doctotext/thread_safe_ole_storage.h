#ifndef DOCTOTEXT_THREAD_SAFE_OLE_STORAGE_H
#define DOCTOTEXT_THREAD_SAFE_OLE_STORAGE_H

#include <string>
#include <vector>
#include "olestorage.h"

class ThreadSafeOLEStreamReader;
using namespace wvWare;

class ThreadSafeOLEStorage : public AbstractOLEStorage
{
private:
    struct Implementation;
    Implementation *impl;
public:
    ThreadSafeOLEStorage(const std::string &file_name);
    ThreadSafeOLEStorage(const char *buffer, size_t len);
    ~ThreadSafeOLEStorage();
    bool isValid() const;
    bool open(Mode mode);
    void close();
    std::string name() const;
    std::string getLastError();
    bool getStreamsAndStoragesList(std::vector<std::string> &components);
    std::string getCurrentDirectory();
    bool enterDirectory(const std::string &directory_path);
    bool leaveDirectory();
    bool readDirectFromBuffer(unsigned char *buffer, int size, int offset);
    AbstractOLEStreamReader *createStreamReader(const std::string &stream_path);
private:
    void streamDestroyed(OLEStream *stream);
};

#endif // DOCTOTEXT_THREAD_SAFE_OLE_STORAGE_H
