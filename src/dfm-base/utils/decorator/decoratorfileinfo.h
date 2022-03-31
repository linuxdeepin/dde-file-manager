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

#ifndef DECORATORFILEINFO_H
#define DECORATORFILEINFO_H

#include "dfm-base/dfm_base_global.h"

#include <dfm-io/core/dfileinfo.h>

DFMBASE_BEGIN_NAMESPACE

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

    bool exists() const;
    bool isFile() const;
    bool isDir() const;
    bool isSymLink() const;
    QString suffix() const;
    QString completeSuffix() const;
    QString filePath() const;
    QString parentPath() const;

private:
    QSharedPointer<DecoratorFileInfoPrivate> d = nullptr;
};

DFMBASE_END_NAMESPACE

#endif   // DECORATORFILEINFO_H
