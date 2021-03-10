/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
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
#ifndef DSTORAGEINFO_H
#define DSTORAGEINFO_H

#include <dfmglobal.h>

#include <QStorageInfo>

DFM_BEGIN_NAMESPACE

class DStorageInfoPrivate;
class DStorageInfo : public QStorageInfo
{
    Q_DECLARE_PRIVATE(DStorageInfo)

public:
    enum PathHint {
        NoHint = 0x00,
        FollowSymlink = 0x01,
    };

    Q_DECLARE_FLAGS(PathHints, PathHint)

    DStorageInfo();
    explicit DStorageInfo(const QString &path, PathHints hints = PathHint::NoHint);
    explicit DStorageInfo(const QDir &dir, PathHints hints = PathHint::NoHint);
    DStorageInfo(const DStorageInfo &other);
    ~DStorageInfo();

    DStorageInfo &operator=(const DStorageInfo &other);
#ifdef Q_COMPILER_RVALUE_REFS
    DStorageInfo &operator=(DStorageInfo &&other) Q_DECL_NOTHROW { swap(other); return *this; }
#endif

    inline void swap(DStorageInfo &other) Q_DECL_NOTHROW
    { qSwap(d_ptr, other.d_ptr); }

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

    static bool inSameDevice(QString path1, QString path2, PathHints hints = PathHint::NoHint);
    static bool inSameDevice(const DUrl &url1, const DUrl &url2, PathHints hints = PathHint::NoHint);
    static bool isLocalDevice(const QString &path,const bool &isEx = false);
    static bool isLowSpeedDevice(const QString &path);
    static bool isCdRomDevice(const QString &path);
    static bool isSameFile(const QString &filePath1, const QString &filePath2);     //通过文件(文件夹)的inode，判断是否是同一个文件(文件夹)

private:
    friend bool operator==(const DStorageInfo &first, const DStorageInfo &second);

    QExplicitlySharedDataPointer<DStorageInfoPrivate> d_ptr;
};

inline bool operator==(const DStorageInfo &first, const DStorageInfo &second)
{
    if (first.d_ptr == second.d_ptr)
        return true;

    return first.device() == second.device() && first.rootPath() == second.rootPath();
}

inline bool operator!=(const DStorageInfo &first, const DStorageInfo &second)
{
    return !(first == second);
}

DFM_END_NAMESPACE

QT_BEGIN_NAMESPACE
#ifndef QT_NO_DEBUG_STREAM
Q_CORE_EXPORT QDebug operator<<(QDebug debug, const DFM_NAMESPACE::DStorageInfo &info);
#endif
QT_END_NAMESPACE

Q_DECLARE_SHARED(DFM_NAMESPACE::DStorageInfo)
Q_DECLARE_METATYPE(DFM_NAMESPACE::DStorageInfo*)

#endif // DSTORAGEINFO_H
