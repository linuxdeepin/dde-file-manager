// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef STORAGEINFO_H
#define STORAGEINFO_H

#include "dfmplugin_fileoperations_global.h"

#include <QStorageInfo>
#include <QSharedPointer>

#undef signals
extern "C" {
#include <gio/gio.h>
}
#define signals public

DPFILEOPERATIONS_BEGIN_NAMESPACE
class StorageDataInfo : public QSharedData
{
public:
    ~StorageDataInfo()
    {
        if (gioInfo) {
            g_object_unref(gioInfo);
        }
    }

    GFileInfo *gioInfo = nullptr;
    QString rootPath;
    QByteArray device;
};

class StorageInfo : public QStorageInfo
{
    friend class DoCopyFilesWorker;
    friend class DoCutFilesWorker;
    friend class FileOperateBaseWorker;
    friend class DoMoveToTrashFilesWorker;
    friend class DoRestoreTrashFilesWorker;

public:
    enum PathHint {
        NoHint = 0x00,
        FollowSymlink = 0x01,
    };

    Q_DECLARE_FLAGS(PathHints, PathHint)

    virtual ~StorageInfo();

private:
    StorageInfo();
    explicit StorageInfo(const QString &path, PathHints hints = PathHint::NoHint);
    explicit StorageInfo(const QDir &dir, PathHints hints = PathHint::NoHint);
    StorageInfo(const StorageInfo &other);

public:
    StorageInfo &operator=(const StorageInfo &other);
#ifdef Q_COMPILER_RVALUE_REFS
    StorageInfo &operator=(StorageInfo &&other) Q_DECL_NOTHROW
    {
        swap(other);
        return *this;
    }
#endif

    inline void swap(StorageInfo &other) Q_DECL_NOTHROW
    {
        qSwap(dataInfo, other.dataInfo);
    }

    void setPath(const QString &path, PathHints hints = PathHint::NoHint);

    QString rootPath() const;
    QByteArray device() const;
    QByteArray fileSystemType() const;

    qint64 bytesTotal() const;
    qint64 bytesFree() const;
    qint64 bytesAvailable() const;

    bool isReadOnly() const;
    bool isLocalDevice() const;

    bool isValid() const;
    void refresh();

private:
    friend bool operator==(const StorageInfo &first, const StorageInfo &second);

    QExplicitlySharedDataPointer<StorageDataInfo> dataInfo;
};

inline bool operator==(const StorageInfo &first, const StorageInfo &second)
{
    if (first.dataInfo == second.dataInfo)
        return true;

    return first.device() == second.device() && first.rootPath() == second.rootPath();
}

inline bool operator!=(const StorageInfo &first, const StorageInfo &second)
{
    return !(first == second);
}
DPFILEOPERATIONS_END_NAMESPACE

QT_BEGIN_NAMESPACE
#ifndef QT_NO_DEBUG_STREAM
Q_CORE_EXPORT QDebug operator<<(QDebug debug, const DPFILEOPERATIONS_NAMESPACE::StorageInfo &info);
#endif
QT_END_NAMESPACE

Q_DECLARE_METATYPE(QSharedPointer<DPFILEOPERATIONS_NAMESPACE::StorageInfo>)

#endif   // STORAGEINFO_H
