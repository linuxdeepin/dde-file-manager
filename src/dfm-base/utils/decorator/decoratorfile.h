// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DECORATORFILE_H
#define DECORATORFILE_H

#include "dfm-base/dfm_base_global.h"

#include <dfm-io/core/dfile.h>

namespace dfmbase {

class DecoratorFilePrivate;
class DecoratorFile
{
public:
    explicit DecoratorFile(const QString &filePath);
    explicit DecoratorFile(const QUrl &url);
    explicit DecoratorFile(QSharedPointer<DFMIO::DFile> dfile);
    ~DecoratorFile() = default;

public:
    QSharedPointer<DFMIO::DFile> filePtr();
    bool exists() const;
    qint64 size() const;

    QByteArray readAll() const;
    qint64 writeAll(const QByteArray &byteArray, DFMIO::DFile::OpenFlag = DFMIO::DFile::OpenFlag::kWriteOnly);

    DFMIO::DFile::Permissions permissions() const;
    bool setPermissions(DFMIO::DFile::Permissions permission);

    DFMIOError lastError() const;

private:
    QSharedPointer<DecoratorFilePrivate> d = nullptr;
};

}

#endif   // DECORATORFILE_H
