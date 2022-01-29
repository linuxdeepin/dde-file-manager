/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: liuyangming<liuyangming@uniontech.com>
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
#include "trashfileinfo.h"
#include "utils/trashmanager.h"

#include "interfaces/private/abstractfileinfo_p.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/standardpaths.h"

DFMBASE_USE_NAMESPACE
DPTRASH_BEGIN_NAMESPACE
class TrashFileInfoPrivate : public AbstractFileInfoPrivate
{
public:
    explicit TrashFileInfoPrivate(AbstractFileInfo *qq)
        : AbstractFileInfoPrivate(qq)
    {
    }

    virtual ~TrashFileInfoPrivate();
};

TrashFileInfoPrivate::~TrashFileInfoPrivate()
{
}

TrashFileInfo::TrashFileInfo(const QUrl &url)
    : AbstractFileInfo(url, new TrashFileInfoPrivate(this))
{
    d = static_cast<TrashFileInfoPrivate *>(dptr.data());

    const QString &trashFilesPath = StandardPaths::location(StandardPaths::kTrashFilesPath);

    if (!QDir().mkpath(trashFilesPath)) {
        qWarning() << "mkpath trash files path failed, path =" << trashFilesPath;
    }
    setProxy(InfoFactory::create<AbstractFileInfo>(QUrl::fromLocalFile(trashFilesPath + url.path())));
    //Todo(yanghao): updateInfo
}

TrashFileInfo::~TrashFileInfo()
{
}

QString TrashFileInfo::fileName() const
{
    //Todo(yanghao): isDesktopFile
    return AbstractFileInfo::fileName();
}

bool TrashFileInfo::exists() const
{
    return AbstractFileInfo::exists() || url() == TrashManager::fromTrashFile("/");
}

bool TrashFileInfo::canRename() const
{
    return false;
}

QFile::Permissions TrashFileInfo::permissions() const
{
    QFileDevice::Permissions p = AbstractFileInfo::permissions();

    p &= ~QFileDevice::WriteOwner;
    p &= ~QFileDevice::WriteUser;
    p &= ~QFileDevice::WriteGroup;
    p &= ~QFileDevice::WriteOther;

    return p;
}

bool TrashFileInfo::isReadable() const
{
    return true;
}

bool TrashFileInfo::isWritable() const
{
    return true;
}

bool TrashFileInfo::isDir() const
{
    if (url() == TrashManager::fromTrashFile("/")) {
        return true;
    }

    return AbstractFileInfo::isDir();
}

DPTRASH_END_NAMESPACE
