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
#include "dfm-base/utils/sysinfoutils.h"

#include <dfm-io/dfmio_global.h>
#include <dfm-io/dfmio_register.h>
#include <dfm-io/dfmio_utils.h>
#include <dfm-io/core/diofactory.h>

#include <QVariant>

#include <unistd.h>

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

bool DecoratorFileInfo::isValid() const
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
        return d->dfileInfo->attribute(DFMIO::DFileInfo::AttributeID::kStandardIsFile).toBool();
    return false;
}

bool DecoratorFileInfo::isDir() const
{
    if (d->dfileInfo)
        return d->dfileInfo->attribute(DFMIO::DFileInfo::AttributeID::kStandardIsDir).toBool();
    return false;
}

bool DecoratorFileInfo::isSymLink() const
{
    if (d->dfileInfo)
        return d->dfileInfo->attribute(DFMIO::DFileInfo::AttributeID::kStandardIsSymlink).toBool();
    return false;
}

bool DecoratorFileInfo::isHidden() const
{
    if (d->dfileInfo) {
        const bool hidden = d->dfileInfo->attribute(DFMIO::DFileInfo::AttributeID::kStandardIsHidden).toBool();
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

bool DecoratorFileInfo::isWritable() const
{
    if (SysInfoUtils::isRootUser()) {
        return true;
    }
    if (!isValid())
        return false;

    bool success = false;
    const QVariant &value = d->dfileInfo->attribute(DFMIO::DFileInfo::AttributeID::kAccessCanWrite, &success);
    if (success && value.isValid())
        return value.toBool();

    return false;
}

QUrl DecoratorFileInfo::url() const
{
    if (!isValid())
        return QUrl();
    return d->dfileInfo->uri();
}

QUrl DecoratorFileInfo::parentUrl() const
{
    if (d->dfileInfo)
        return DFMIO::DFMUtils::directParentUrl(d->dfileInfo->uri());
    return QUrl();
}

/*!
    Returns the id of the owner of the file.

    On Windows and on systems where files do not have owners this
    function returns ((uint) -2).
*/
uint DecoratorFileInfo::ownerId() const
{
    if (!d->dfileInfo)
        return uint(-2);
    bool success = false;
    const QVariant &value = d->dfileInfo->attribute(DFMIO::DFileInfo::AttributeID::kUnixUID, &success);
    if (success && value.isValid())
        return value.toUInt();
    return uint(-2);
}

QString DecoratorFileInfo::suffix() const
{
    if (d->dfileInfo)
        return d->dfileInfo->attribute(DFMIO::DFileInfo::AttributeID::kStandardSuffix).toString();
    return QString();
}

QString DecoratorFileInfo::completeSuffix() const
{
    if (d->dfileInfo)
        return d->dfileInfo->attribute(DFMIO::DFileInfo::AttributeID::kStandardCompleteSuffix).toString();
    return QString();
}

QString DecoratorFileInfo::filePath() const
{
    if (d->dfileInfo)
        return d->dfileInfo->attribute(DFMIO::DFileInfo::AttributeID::kStandardFilePath).toString();
    return QString();
}

QString DecoratorFileInfo::parentPath() const
{
    if (d->dfileInfo)
        return d->dfileInfo->attribute(DFMIO::DFileInfo::AttributeID::kStandardParentPath).toString();
    return QString();
}

QString DecoratorFileInfo::fileName() const
{
    if (d->dfileInfo)
        return d->dfileInfo->attribute(DFMIO::DFileInfo::AttributeID::kStandardFileName).toString();
    return QString();
}

QString DecoratorFileInfo::symLinkTarget() const
{
    if (d->dfileInfo) {
        QString symLinkTarget = d->dfileInfo->attribute(DFMIO::DFileInfo::AttributeID::kStandardSymlinkTarget).toString();
        if (!symLinkTarget.startsWith("/")) {
            QString currPath = parentPath();
            if (currPath.right(1) != "/")
                currPath += "/";
            symLinkTarget.prepend(currPath);
        }
        return symLinkTarget;
    }
    return QString();
}

quint64 DecoratorFileInfo::size() const
{
    if (d->dfileInfo)
        return d->dfileInfo->attribute(DFMIO::DFileInfo::AttributeID::kStandardSize).toULongLong();
    return 0;
}

QVariant DecoratorFileInfo::customAttribute(const char *key, const dfmio::DFileInfo::DFileAttributeType type)
{
    if (d->dfileInfo)
        return d->dfileInfo->customAttribute(key, type);
    return QVariant();
}

QList<QString> DecoratorFileInfo::standardIconNames() const
{
    if (d->dfileInfo)
        return d->dfileInfo->attribute(DFMIO::DFileInfo::AttributeID::kStandardIcon).toStringList();
    return {};
}

DFMIO::DFile::Permissions DecoratorFileInfo::permissions() const
{
    if (d->dfileInfo)
        return d->dfileInfo->permissions();
    return DFMIO::DFile::Permission::kNoPermission;
}

bool DecoratorFileInfo::notifyAttributeChanged()
{
    if (d->dfileInfo) {
        return d->dfileInfo->setCustomAttribute("xattr::update", DFMIO::DFileInfo::DFileAttributeType::kTypeString, "");
    }

    return false;
}

DFMIOError DecoratorFileInfo::lastError() const
{
    if (d->dfileInfo)
        return d->dfileInfo->lastError();
    return DFMIOError();
}
