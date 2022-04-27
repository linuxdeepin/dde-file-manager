/*
 * Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef STORAGEINFO_H
#define STORAGEINFO_H

#include "dfm_common_service_global.h"

#include <QStorageInfo>
#include <QSharedPointer>

#undef signals
extern "C" {
#include <gio/gio.h>
}
#define signals public

DSC_BEGIN_NAMESPACE
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
    bool isLowSpeedDevice() const;

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

DSC_END_NAMESPACE

QT_BEGIN_NAMESPACE
#ifndef QT_NO_DEBUG_STREAM
Q_CORE_EXPORT QDebug operator<<(QDebug debug, const DSC_NAMESPACE::StorageInfo &info);
#endif
QT_END_NAMESPACE

Q_DECLARE_METATYPE(QSharedPointer<DSC_NAMESPACE::StorageInfo>)

#endif   // STORAGEINFO_H
