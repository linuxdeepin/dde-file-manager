// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DECORATORFILEINFO_H
#define DECORATORFILEINFO_H

#include "dfm-base/dfm_base_global.h"

#include <dfm-io/dfileinfo.h>

namespace dfmbase {

class DecoratorFileInfoPrivate;
class DecoratorFileInfo
{
public:
    explicit DecoratorFileInfo(const QString &filePath);
    explicit DecoratorFileInfo(const QUrl &url);
    explicit DecoratorFileInfo(QSharedPointer<DFMIO::DFileInfo> dfileInfo);
    ~DecoratorFileInfo() = default;

public:
    QSharedPointer<DFMIO::DFileInfo> fileInfoPtr();

    bool isValid() const;
    bool exists() const;
    bool isFile() const;
    bool isDir() const;
    bool isSymLink() const;
    bool isWritable() const;

    QUrl url() const;
    QUrl parentUrl() const;
    uint ownerId() const;

    QString suffix() const;
    QString completeSuffix() const;
    QString filePath() const;
    QString parentPath() const;
    QString fileName() const;
    QString symLinkTarget() const;
    quint64 size() const;
    QVariant customAttribute(const char *key, const DFMIO::DFileInfo::DFileAttributeType type);

    QList<QString> standardIconNames() const;

    DFMIO::DFile::Permissions permissions() const;

    bool notifyAttributeChanged();

    DFMIOError lastError() const;

private:
    QSharedPointer<DecoratorFileInfoPrivate> d = nullptr;
};

}

#endif   // DECORATORFILEINFO_H
