// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BOOKMARK_H
#define BOOKMARK_H

#include <QObject>
#include "debugobejct.h"
#include <QString>
#include <QDateTime>
#include "dfileinfo.h"

class BookMark;
typedef QExplicitlySharedDataPointer<BookMark> BookMarkPointer;

class BookMark : public DAbstractFileInfo
{
public:
    explicit BookMark(const DUrl &url);
    BookMark(const QString &name, const DUrl &sourceUrl);
    ~BookMark() override;

    DUrl sourceUrl() const;
    QString getName() const;
    QString getMountPoint() const;

    bool exists() const override;

    QString fileDisplayName() const override;

    bool canRedirectionFileUrl() const override;
    DUrl redirectedFileUrl() const override;

    DUrl parentUrl() const override;

    DUrl getUrlByNewFileName(const QString &name) const override;

    QDateTime created() const override;
    QDateTime lastModified() const override;

    bool canDrop() const override;

public:
    QDateTime m_created;
    QDateTime m_lastModified;
    QString mountPoint;
    QString locateUrl;
    // AbstractFileInfo interface

private:
    // always care about these two value when using it.
    // related functions: exists(), canRedirectionFileUrl(), redirectionFileUrl()
    mutable QString udisksDBusPath;
    mutable QString udisksMountPoint;
};

#endif // BOOKMARK_H
