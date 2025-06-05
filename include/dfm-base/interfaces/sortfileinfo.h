// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SORTFILEINFO_H
#define SORTFILEINFO_H

#include <dfm-base/dfm_base_global.h>

#include <QScopedPointer>
#include <QUrl>
#include <QSharedPointer>

namespace dfmbase {
class SortFileInfoPrivate;
class SortFileInfo
{
public:
    SortFileInfo();
    ~SortFileInfo();

    void setUrl(const QUrl &url);
    void setSize(const qint64 size);
    void setFile(const bool isfile);
    void setDir(const bool isdir);
    void setSymlink(const bool isSymlink);
    void setHide(const bool ishide);
    void setReadable(const bool readable);
    void setWriteable(const bool writeable);
    void setExecutable(const bool executable);
    void setLastReadTime(const qint64 time);
    void setLastModifiedTime(const qint64 time);
    void setCreateTime(const qint64 time);
    void setDisplayType(const QString &displayType);

    QUrl fileUrl() const;
    qint64 fileSize() const;
    bool isFile() const;
    bool isDir() const;
    bool isSymLink() const;
    bool isHide() const;
    bool isReadable() const;
    bool isWriteable() const;
    bool isExecutable() const;
    qint64 lastReadTime() const;
    qint64 lastModifiedTime() const;
    qint64 createTime() const;
    QString displayType() const;
    
    void setHighlightContent(const QString &content);
    QString highlightContent() const;

private:
    QScopedPointer<SortFileInfoPrivate> d;
};
}
typedef QSharedPointer<DFMBASE_NAMESPACE::SortFileInfo> SortInfoPointer;
Q_DECLARE_METATYPE(SortInfoPointer)
Q_DECLARE_METATYPE(QList<SortInfoPointer>)

#endif // SORTFILEINFO_H
