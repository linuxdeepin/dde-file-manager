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
#include "private/tagfileinfo_p.h"
#include "utils/tagmanager.h"

#include "dfm-base/interfaces/private/abstractfileinfo_p.h"
#include "dfm-base/base/schemefactory.h"

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_tag;

TagFileInfo::TagFileInfo(const QUrl &url)
    : AbstractFileInfo(url, new TagFileInfoPrivate(this))
{
    d = static_cast<TagFileInfoPrivate *>(dptr.data());
    if (!localFilePath().isEmpty())
        setProxy(InfoFactory::create<AbstractFileInfo>(QUrl::fromLocalFile(localFilePath())));
}

TagFileInfo::~TagFileInfo()
{
}

bool TagFileInfo::isDir() const
{
    if (d->proxy)
        return d->proxy->isDir();

    return true;
}

bool TagFileInfo::exists() const
{
    if (d->proxy) {
        // TODO(liuyangming): handle path in sql
        if (!localFilePath().startsWith("/home/")
            && !localFilePath().startsWith("/data/home/")
            && !localFilePath().startsWith("/media/"))
            return false;
        return d->proxy->exists();
    }

    QUrl rootUrl;
    rootUrl.setScheme(TagManager::scheme());
    if (url() == rootUrl)
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

bool TagFileInfo::isReadable() const
{
    if (d->proxy)
        return d->proxy->isReadable();

    return true;
}

bool TagFileInfo::isWritable() const
{
    if (d->proxy)
        return d->proxy->isWritable();

    return true;
}

QString TagFileInfo::fileName() const
{
    if (d->proxy)
        return d->proxy->fileName();

    return tagName();
}

QString TagFileInfo::fileDisplayName() const
{
    return fileName();
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
    return url().fragment(QUrl::FullyDecoded);
}

QString TagFileInfo::tagName() const
{
    return url().path().mid(1, url().path().length() - 1);
}
