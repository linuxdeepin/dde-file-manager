/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lanxuesong<lanxuesong@uniontech.com>
 *
 * Maintainer: lanxuesong<lanxuesong@uniontech.com>
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

#ifndef DECORATORFILE_H
#define DECORATORFILE_H

#include "dfm-base/dfm_base_global.h"

#include <dfm-io/core/dfile.h>

DFMBASE_BEGIN_NAMESPACE

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
    qint64 writeAll(const QByteArray &byteArray);

    DFMIO::DFile::Permissions permissions() const;
    bool setPermissions(DFMIO::DFile::Permissions permission);

    DFMIOError lastError() const;

private:
    QSharedPointer<DecoratorFilePrivate> d = nullptr;
};

DFMBASE_END_NAMESPACE

#endif   // DECORATORFILE_H
