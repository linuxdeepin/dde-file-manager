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
#include "utils/trashhelper.h"

#include "interfaces/private/abstractfileinfo_p.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/standardpaths.h"
#include "dfm-base/utils/systempathutil.h"
#include "dfm-base/utils/fileutils.h"
#include "dfm-base/file/local/desktopfileinfo.h"
#include "dfm-base/utils/decorator/decoratorfile.h"

#include <QSettings>
#include <QCoreApplication>

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

    void updateInfo();
    void inheritParentTrashInfo();

    QString displayName;
    QString baseName;
    QString completeBaseName;
    QString originalFilePath;
    QDateTime deletionDate;
    QString displayDeletionDate;
    QStringList tagNameList;
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
    d->updateInfo();
}

TrashFileInfo::~TrashFileInfo()
{
}

QString TrashFileInfo::fileName() const
{
    const QUrl &url = this->url();
    if (FileUtils::isDesktopFile(url)) {
        DesktopFileInfo dfi(url);
        return dfi.fileDisplayName();
    }

    return d->displayName;
}

QString TrashFileInfo::fileDisplayName() const
{
    return fileName();
}

QString TrashFileInfo::baseName() const
{
    const QString &fileName = d->displayName;
    const QString &suffix = this->suffix();

    if (suffix.isEmpty()) {
        return fileName;
    }

    return fileName.left(fileName.indexOf("."));
}

QString TrashFileInfo::completeBaseName() const
{
    const QString &fileName = d->displayName;
    const QString &suffix = this->suffix();

    if (suffix.isEmpty()) {
        return fileName;
    }

    return fileName.left(fileName.length() - suffix.length() - 1);
}

bool TrashFileInfo::exists() const
{
    return AbstractFileInfo::exists() || url() == TrashHelper::fromTrashFile("/");
}

void TrashFileInfo::refresh()
{
    AbstractFileInfo::refresh();
    d->updateInfo();
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
    if (url() == TrashHelper::fromTrashFile("/")) {
        return true;
    }

    return AbstractFileInfo::isDir();
}

bool TrashFileInfo::canDrop() const
{
    return false;
}

void TrashFileInfoPrivate::updateInfo()
{
    const QString &filePath = proxy->absoluteFilePath();
    const QString &basePath = StandardPaths::location(StandardPaths::kTrashFilesPath);
    const QString &fileBaseName = filePath.mid(basePath.size());

    const QString &location(StandardPaths::location(StandardPaths::kTrashInfosPath) + fileBaseName + ".trashinfo");
    DecoratorFile dfile(location);
    if (dfile.exists()) {
        QSettings setting(location, QSettings::NativeFormat);

        setting.beginGroup("Trash Info");
        setting.setIniCodec("utf-8");

        originalFilePath = QByteArray::fromPercentEncoding(setting.value("Path").toByteArray()) + filePath.mid(basePath.size() + fileBaseName.size());

        displayName = originalFilePath.mid(originalFilePath.lastIndexOf('/') + 1);

        deletionDate = QDateTime::fromString(setting.value("DeletionDate").toString(), Qt::ISODate);
        displayDeletionDate = deletionDate.toString(FileUtils::dateTimeFormat());

        if (displayDeletionDate.isEmpty()) {
            displayDeletionDate = setting.value("DeletionDate").toString();
        }

        const QString &tagNameListTemp = setting.value("TagNameList").toString();

        if (!tagNameListTemp.isEmpty()) {
            tagNameList = tagNameListTemp.split(",");
        }
    } else {
        //inherits from parent trash info
        inheritParentTrashInfo();

        // is trash root path
        if (filePath == basePath || filePath == basePath + "/") {
            displayName = QCoreApplication::translate("PathManager", "Trash");

            return;
        }

        if (SystemPathUtil::instance()->isSystemPath(filePath)) {
            displayName = SystemPathUtil::instance()->systemPathDisplayNameByPath(filePath);
        } else {
            displayName = proxy->fileName();
        }
    }
}

void TrashFileInfoPrivate::inheritParentTrashInfo()
{
    const QString &filePath = proxy->absoluteFilePath();
    QString nameLayer = filePath.right(filePath.length() - StandardPaths::location(StandardPaths::kTrashFilesPath).length() - 1);
    QStringList names = nameLayer.split("/");

    QString name = names.takeFirst();
    QString restPath;
    foreach (QString str, names) {
        restPath += "/" + str;
    }

    const QString &location(StandardPaths::location(StandardPaths::kTrashInfosPath) + QDir::separator() + name + ".trashinfo");
    DecoratorFile dfile(location);
    if (dfile.exists()) {
        QSettings setting(location, QSettings::NativeFormat);

        setting.beginGroup("Trash Info");
        setting.setIniCodec("utf-8");

        originalFilePath = QByteArray::fromPercentEncoding(setting.value("Path").toByteArray()) + restPath;

        deletionDate = QDateTime::fromString(setting.value("DeletionDate").toString(), Qt::ISODate);
        displayDeletionDate = deletionDate.toString(FileUtils::dateTimeFormat());

        if (displayDeletionDate.isEmpty()) {
            displayDeletionDate = setting.value("DeletionDate").toString();
        }
    }
}

DPTRASH_END_NAMESPACE
