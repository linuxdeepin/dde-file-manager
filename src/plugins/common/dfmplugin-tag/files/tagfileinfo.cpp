/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#include "tagfileinfo.h"
#include "utils/tagmanager.h"
#include "tagfileinfo_p.h"

#include "dfm-base/interfaces/private/abstractfileinfo_p.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/utils/fileutils.h"

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_tag;

TagFileInfo::TagFileInfo(const QUrl &url)
    : AbstractFileInfo(url), d(new TagFileInfoPrivate(url, this))
{
    dptr.reset(d);
    if (!localFilePath().isEmpty())
        setProxy(InfoFactory::create<AbstractFileInfo>(QUrl::fromLocalFile(localFilePath())));
}

TagFileInfo::~TagFileInfo()
{
}

bool TagFileInfo::exists() const
{
    if (d->proxy) {
        // TODO(liuyangming): handle path in sql
        if (!localFilePath().startsWith("/home/")
            && !localFilePath().startsWith(FileUtils::bindPathTransform("/home/", true))
            && !localFilePath().startsWith("/media/"))
            return false;
        return d->proxy->exists();
    }

    QUrl rootUrl;
    rootUrl.setScheme(TagManager::scheme());
    if (urlInfo(AbstractFileInfo::FileUrlInfoType::kUrl) == rootUrl)
        return true;

    const QMap<QString, QColor> &tagMap = TagManager::instance()->getAllTags();

    return tagMap.contains(tagName());
}

QFileDevice::Permissions TagFileInfo::permissions() const
{
    if (d->proxy)
        return d->proxy->permissions();

    return QFile::ReadGroup | QFile::ReadOwner | QFile::ReadUser | QFile::ReadOther
            | QFile::WriteGroup | QFile::WriteOwner | QFile::WriteUser | QFile::WriteOther;
}

bool TagFileInfo::isAttributes(const AbstractFileInfo::FileIsType type) const
{
    switch (type) {
    case FileIsType::kIsDir:
        if (d->proxy)
            return d->proxy->isAttributes(type);

        return true;
    case FileIsType::kIsReadable:
        if (d->proxy)
            return d->proxy->isAttributes(type);

        return true;
    case FileIsType::kIsWritable:
        if (d->proxy)
            return d->proxy->isAttributes(type);

        return true;
    default:
        return AbstractFileInfo::isAttributes(type);
    }
}

QString TagFileInfo::nameInfo(const NameInfo type) const
{
    switch (type) {
    case NameInfo::kFileName:
    case NameInfo::kFileCopyName:
        return d->fileName();
    default:
        return AbstractFileInfo::nameInfo(type);
    }
}

QString TagFileInfo::displayInfo(const AbstractFileInfo::DisplayInfoType type) const
{
    if (AbstractFileInfo::DisplayInfoType::kFileDisplayName == type)
        return d->fileName();
    return AbstractFileInfo::displayInfo(type);
}

AbstractFileInfo::FileType TagFileInfo::fileType() const
{
    if (d->proxy)
        return d->proxy->fileType();

    return FileType::kDirectory;
}

QIcon TagFileInfo::fileIcon()
{
    if (d->proxy)
        return d->proxy->fileIcon();

    return QIcon::fromTheme("folder");
}

QString TagFileInfo::localFilePath() const
{
    return urlInfo(AbstractFileInfo::FileUrlInfoType::kUrl).fragment(QUrl::FullyDecoded);
}

QString TagFileInfo::tagName() const
{
    return d->fileName();
}

TagFileInfoPrivate::TagFileInfoPrivate(const QUrl &url, AbstractFileInfo *qq)
    : AbstractFileInfoPrivate(url, qq)
{
}

TagFileInfoPrivate::~TagFileInfoPrivate()
{
}

QString TagFileInfoPrivate::fileName() const
{
    if (proxy)
        return proxy->nameInfo(NameInfo::kFileName);

    return url.path().mid(1, url.path().length() - 1);
}
