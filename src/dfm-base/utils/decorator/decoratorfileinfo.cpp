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

#include "decoratorfileinfo.h"

#include "dfm-base/utils/fileutils.h"

#include <dfm-io/dfmio_global.h>
#include <dfm-io/dfmio_register.h>
#include <dfm-io/core/diofactory.h>

#include <QVariant>

DFMBASE_BEGIN_NAMESPACE

class DecoratorFileInfoPrivate
{
public:
    explicit DecoratorFileInfoPrivate(DecoratorFileInfo *q)
        : q(q)
    {
    }
    ~DecoratorFileInfoPrivate() = default;

public:
    DecoratorFileInfo *q = nullptr;
    QSharedPointer<DFMIO::DFileInfo> dfileInfo = nullptr;
};

DFMBASE_END_NAMESPACE

DFMBASE_USE_NAMESPACE

DecoratorFileInfo::DecoratorFileInfo(const QString &filePath)
    : d(new DecoratorFileInfoPrivate(this))
{
    const QUrl &url = QUrl::fromLocalFile(filePath);
    QSharedPointer<DFMIO::DIOFactory> factory = produceQSharedIOFactory(url.scheme(), static_cast<QUrl>(url));
    if (factory)
        d->dfileInfo = factory->createFileInfo();
}

DecoratorFileInfo::DecoratorFileInfo(const QUrl &url)
    : d(new DecoratorFileInfoPrivate(this))
{
    QSharedPointer<DFMIO::DIOFactory> factory = produceQSharedIOFactory(url.scheme(), static_cast<QUrl>(url));
    if (factory)
        d->dfileInfo = factory->createFileInfo();
}

DecoratorFileInfo::DecoratorFileInfo(QSharedPointer<dfmio::DFileInfo> dfileInfo)
    : d(new DecoratorFileInfoPrivate(this))
{
    d->dfileInfo = dfileInfo;
}

QSharedPointer<dfmio::DFileInfo> DecoratorFileInfo::fileInfoPtr()
{
    return d->dfileInfo;
}

bool DecoratorFileInfo::exists() const
{
    if (d->dfileInfo)
        return d->dfileInfo->exists();
    return false;
}

bool DecoratorFileInfo::isFile() const
{
    if (d->dfileInfo)
        return d->dfileInfo->attribute(DFMIO::DFileInfo::AttributeID::StandardIsFile).toBool();
    return false;
}

bool DecoratorFileInfo::isDir() const
{
    if (d->dfileInfo)
        return d->dfileInfo->attribute(DFMIO::DFileInfo::AttributeID::StandardIsDir).toBool();
    return false;
}

bool DecoratorFileInfo::isSymLink() const
{
    if (d->dfileInfo)
        return d->dfileInfo->attribute(DFMIO::DFileInfo::AttributeID::StandardIsSymlink).toBool();
    return false;
}

bool DecoratorFileInfo::isHidden() const
{
    if (d->dfileInfo) {
        const bool hidden = d->dfileInfo->attribute(DFMIO::DFileInfo::AttributeID::StandardIsHidden).toBool();
        if (hidden)
            return true;
    }

    static DFMBASE_NAMESPACE::Match match("PrivateFiles");

    const QString &fileName = this->fileName();
    const bool hidden = match.match(this->filePath(), fileName);
    if (hidden)
        return hidden;

    return false;
}

QString DecoratorFileInfo::suffix() const
{
    if (d->dfileInfo)
        return d->dfileInfo->attribute(DFMIO::DFileInfo::AttributeID::StandardSuffix).toString();
    return QString();
}

QString DecoratorFileInfo::completeSuffix() const
{
    if (d->dfileInfo)
        return d->dfileInfo->attribute(DFMIO::DFileInfo::AttributeID::StandardCompleteSuffix).toString();
    return QString();
}

QString DecoratorFileInfo::filePath() const
{
    if (d->dfileInfo)
        return d->dfileInfo->attribute(DFMIO::DFileInfo::AttributeID::StandardFilePath).toString();
    return QString();
}

QString DecoratorFileInfo::parentPath() const
{
    if (d->dfileInfo)
        return d->dfileInfo->attribute(DFMIO::DFileInfo::AttributeID::StandardParentPath).toString();
    return QString();
}

QString DecoratorFileInfo::fileName() const
{
    if (d->dfileInfo)
        return d->dfileInfo->attribute(DFMIO::DFileInfo::AttributeID::StandardFileName).toString();
    return QString();
}

QList<QString> DecoratorFileInfo::standardIconNames() const
{
    if (d->dfileInfo)
        return d->dfileInfo->attribute(DFMIO::DFileInfo::AttributeID::StandardIcon).toStringList();
    return {};
}

DFMIO::DFile::Permissions DecoratorFileInfo::permissions() const
{
    if (d->dfileInfo)
        return d->dfileInfo->permissions();
    return DFMIO::DFile::Permission::NoPermission;
}

bool DecoratorFileInfo::notifyAttributeChanged()
{
    if (d->dfileInfo) {
        return d->dfileInfo->setCustomAttribute("xattr::update", DFMIO::DFileInfo::DFileAttributeType::TypeString, "");
    }

    return false;
}

DFMIOError DecoratorFileInfo::lastError() const
{
    if (d->dfileInfo)
        return d->dfileInfo->lastError();
    return DFMIOError();
}
