#ifndef BASEMANAGER_H
#define BASEMANAGER_H

#include <QByteArray>

class BaseManager
{
public:
    explicit BaseManager();
    ~BaseManager();

    virtual void load();
    virtual void save();

    static QString getCachePath(const QString& fileBaseName);
    static QString getConfigPath(const QString& fileBaseName);
    static void writeCacheToFile(const QString &path, const QString &content);
    static QString readCacheFromFile(const QString &path);

};

#endif // BASEMANAGER_H
