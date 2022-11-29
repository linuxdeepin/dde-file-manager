/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
#include "vaultfileinfo.h"
#include "utils/vaultdefine.h"
#include "utils/vaulthelper.h"
#include "utils/pathmanager.h"

#include "dfm-base/base/standardpaths.h"
#include "dfm-base/mimetype/mimedatabase.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/interfaces/private/abstractfileinfo_p.h"

#include <dfm-io/dfmio_utils.h>

#include <DFileIconProvider>

#include <QDateTime>
#include <QDir>
#include <QDebug>
#include <qplatformdefs.h>

#include <sys/stat.h>

DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
namespace dfmplugin_vault {
VaultFileInfo::VaultFileInfo(const QUrl &url)
    : AbstractFileInfo(url)
{
    QUrl tempUrl = VaultHelper::vaultToLocalUrl(url);
    setProxy(InfoFactory::create<AbstractFileInfo>(tempUrl));
}

VaultFileInfo::~VaultFileInfo()
{
}

VaultFileInfo &VaultFileInfo::operator=(const VaultFileInfo &fileinfo)
{
    AbstractFileInfo::operator=(fileinfo);
    if (!dptr->proxy)
        setProxy(fileinfo.dptr->proxy);
    else {
        dptr->url = fileinfo.dptr->url;
        dptr->proxy = fileinfo.dptr->proxy;
    }
    return *this;
}

bool VaultFileInfo::operator==(const VaultFileInfo &fileinfo) const
{
    return dptr->proxy == fileinfo.dptr->proxy && dptr->url == fileinfo.dptr->url;
}

bool VaultFileInfo::operator!=(const VaultFileInfo &fileinfo) const
{
    return !(operator==(fileinfo));
}

QString VaultFileInfo::absolutePath() const
{
    if (!dptr->proxy)
        return "";

    QString path = dptr->proxy->path();
    QUrl virtualUrl = VaultHelper::instance()->pathToVaultVirtualUrl(path);
    return virtualUrl.path();
}

bool VaultFileInfo::exists() const
{
    if (url().isEmpty())
        return false;

    return dptr->proxy && dptr->proxy->exists();
}

void VaultFileInfo::refresh()
{
    if (!dptr->proxy) {
        return;
    }

    dptr->proxy->refresh();

    setProxy(InfoFactory::create<AbstractFileInfo>(dptr->proxy->url()));
}

bool VaultFileInfo::isReadable() const
{
    return !dptr->proxy || dptr->proxy->isReadable();
}

bool VaultFileInfo::isWritable() const
{
    return !dptr->proxy || dptr->proxy->isWritable();
}

bool VaultFileInfo::isExecutable() const
{
    return !dptr->proxy || dptr->proxy->isExecutable();
}

bool VaultFileInfo::isHidden() const
{
    return !dptr->proxy || dptr->proxy->isHidden();
}

bool VaultFileInfo::isFile() const
{
    return !dptr->proxy || dptr->proxy->isFile();
}

bool VaultFileInfo::isDir() const
{
    return !dptr->proxy || dptr->proxy->isDir();
}

bool VaultFileInfo::isSymLink() const
{
    return !dptr->proxy || dptr->proxy->isSymLink();
}

bool VaultFileInfo::isRoot() const
{
    bool bRootDir = false;
    QString localFilePath = DFMIO::DFMUtils::buildFilePath(kVaultBasePath.toStdString().c_str(), kVaultDecryptDirName, nullptr);
    QString path = filePath();
    if (localFilePath == path || localFilePath + "/" == path || localFilePath == path + "/") {
        bRootDir = true;
    }
    return bRootDir;
}

bool VaultFileInfo::canDrop()
{
    if (VaultHelper::instance()->state(PathManager::vaultLockPath()) != VaultState::kUnlocked) {
        return false;
    }

    return !dptr->proxy || dptr->proxy->canDrop();
}

QUrl VaultFileInfo::url() const
{
    if (!dptr->proxy)
        return QUrl();

    QUrl url = dptr->proxy->url();
    url = VaultHelper::instance()->pathToVaultVirtualUrl(url.path());
    return url;
}

QString VaultFileInfo::iconName()
{
    QString iconName = "dfm_safebox";   // 如果是根目录，用保险柜图标
    if (isRoot())
        return iconName;
    else {
        if (!dptr->proxy)
            return fileMimeType().iconName();
        else
            dptr->proxy->iconName();
    }
    return QString();
}

QVariantHash VaultFileInfo::extraProperties() const
{
    if (!dptr->proxy)
        AbstractFileInfo::extraProperties();
    return dptr->proxy->extraProperties();
}

QUrl VaultFileInfo::getUrlByNewFileName(const QString &fileName) const
{
    QUrl theUrl = url();

    theUrl.setPath(DFMIO::DFMUtils::buildFilePath(absolutePath().toStdString().c_str(),
                                                  fileName.toStdString().c_str(), nullptr));

    theUrl.setHost("");

    return theUrl;
}

QUrl VaultFileInfo::redirectedFileUrl() const
{
    return url();
}

QIcon VaultFileInfo::fileIcon()
{
    if (isRoot())
        return QIcon::fromTheme(iconName());

    if (!dptr->proxy)
        AbstractFileInfo::fileIcon();
    return dptr->proxy->fileIcon();
}

qint64 VaultFileInfo::size() const
{
    if (!dptr->proxy)
        AbstractFileInfo::size();
    return dptr->proxy->size();
}

int VaultFileInfo::countChildFile() const
{
    if (isDir()) {
        QDir dir(absoluteFilePath());
        QStringList entryList = dir.entryList(QDir::AllEntries | QDir::System
                                              | QDir::NoDotAndDotDot | QDir::Hidden);
        return entryList.size();
    }

    return -1;
}

QString VaultFileInfo::sizeFormat() const
{
    if (!dptr->proxy)
        return AbstractFileInfo::sizeFormat();
    return dptr->proxy->sizeFormat();
}

QString VaultFileInfo::fileDisplayName() const
{
    if (isRoot()) {
        return QObject::tr("My Vault");
    }
    if (!dptr->proxy)
        return AbstractFileInfo::fileDisplayName();
    return dptr->proxy->fileDisplayName();
}

QString VaultFileInfo::fileCopyName() const
{
    return VaultFileInfo::fileDisplayName();
}

QString VaultFileInfo::fileDisplayPath() const
{
    QUrl currentUrl = url();
    currentUrl.setHost("");
    QString urlStr = currentUrl.toString();
    QByteArray array = urlStr.toLocal8Bit();
    QString filePath = QUrl::fromPercentEncoding(array);
    return filePath;
}
}
