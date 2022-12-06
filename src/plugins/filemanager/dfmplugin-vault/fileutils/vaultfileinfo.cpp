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
#include "private/vaultfileinfo_p.h"

#include "dfm-base/base/standardpaths.h"
#include "dfm-base/mimetype/mimedatabase.h"
#include "dfm-base/base/schemefactory.h"

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

VaultFileInfoPrivate::VaultFileInfoPrivate(const QUrl &url, AbstractFileInfo *qq)
    : AbstractFileInfoPrivate(url, qq)
{
}

VaultFileInfoPrivate::~VaultFileInfoPrivate()
{
}

QString VaultFileInfoPrivate::iconName()
{
    QString iconName = "dfm_safebox";   // 如果是根目录，用保险柜图标
    if (q->isRoot())
        return iconName;
    else {
        if (!proxy)
            return q->fileMimeType().iconName();
        else
            proxy->nameInfo(AbstractFileInfo::FileNameInfoType::kIconName);
    }
    return QString();
}

QString VaultFileInfoPrivate::fileDisplayPath() const
{
    QUrl currentUrl = url;
    currentUrl.setHost("");
    QString urlStr = currentUrl.toString();
    QByteArray array = urlStr.toLocal8Bit();
    QString filePath = QUrl::fromPercentEncoding(array);
    return filePath;
}

QString VaultFileInfoPrivate::absolutePath() const
{
    if (!proxy)
        return "";

    QString path = proxy->pathInfo(AbstractFileInfo::FilePathInfoType::kPath);
    QUrl virtualUrl = VaultHelper::instance()->pathToVaultVirtualUrl(path);
    return virtualUrl.path();
}

QUrl VaultFileInfoPrivate::vaultUrl() const
{
    if (!proxy)
        return QUrl();

    QUrl url = proxy->urlInfo(AbstractFileInfo::FileUrlInfoType::kUrl);
    url = VaultHelper::instance()->pathToVaultVirtualUrl(url.path());
    return url;
}

QUrl VaultFileInfoPrivate::getUrlByNewFileName(const QString &fileName) const
{
    QUrl theUrl = vaultUrl();

    theUrl.setPath(DFMIO::DFMUtils::buildFilePath(absolutePath().toStdString().c_str(),
                                                  fileName.toStdString().c_str(), nullptr));

    theUrl.setHost("");

    return theUrl;
}

VaultFileInfo::VaultFileInfo(const QUrl &url)
    : AbstractFileInfo(url)
{
    dptr.reset(new VaultFileInfoPrivate(url, this));
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

QString VaultFileInfo::pathInfo(const dfmbase::AbstractFileInfo::FilePathInfoType type) const
{
    switch (type) {
    case FilePathInfoType::kAbsolutePath:
        return dptr.staticCast<VaultFileInfoPrivate>()->absolutePath();
    default:
        return AbstractFileInfo::pathInfo(type);
    }
}

QUrl VaultFileInfo::urlInfo(const AbstractFileInfo::FileUrlInfoType type) const
{
    switch (type) {
    case FileUrlInfoType::kUrl:
        [[fallthrough]];
    case FileUrlInfoType::kRedirectedFileUrl:
        return dptr.staticCast<VaultFileInfoPrivate>()->vaultUrl();
    default:
        return AbstractFileInfo::urlInfo(type);
    }
}
bool VaultFileInfo::exists() const
{
    if (urlInfo(AbstractFileInfo::FileUrlInfoType::kUrl).isEmpty())
        return false;

    return dptr->proxy && dptr->proxy->exists();
}

void VaultFileInfo::refresh()
{
    if (!dptr->proxy) {
        return;
    }

    dptr->proxy->refresh();

    setProxy(InfoFactory::create<AbstractFileInfo>(dptr->proxy->urlInfo(AbstractFileInfo::FileUrlInfoType::kUrl)));
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
    const QString &localFilePath = DFMIO::DFMUtils::buildFilePath(kVaultBasePath.toStdString().c_str(), kVaultDecryptDirName, nullptr);
    QString path = pathInfo(AbstractFileInfo::FilePathInfoType::kFilePath);
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

QVariantHash VaultFileInfo::extraProperties() const
{
    if (!dptr->proxy)
        AbstractFileInfo::extraProperties();
    return dptr->proxy->extraProperties();
}

QUrl VaultFileInfo::getUrlByType(const AbstractFileInfo::FileUrlInfoType type, const QString &fileName) const
{
    switch (type) {
    case FileUrlInfoType::kGetUrlByNewFileName:
        return dptr.staticCast<VaultFileInfoPrivate>()->getUrlByNewFileName(fileName);
    default:
        return AbstractFileInfo::getUrlByType(type, fileName);
    }
}

QIcon VaultFileInfo::fileIcon()
{
    if (isRoot())
        return QIcon::fromTheme(dptr.staticCast<VaultFileInfoPrivate>()->iconName());

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
        QDir dir(pathInfo(AbstractFileInfo::FilePathInfoType::kAbsoluteFilePath));
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

QString VaultFileInfo::nameInfo(const AbstractFileInfo::FileNameInfoType type) const
{

    switch (type) {
    case AbstractFileInfo::FileNameInfoType::kFileCopyName:
        return displayInfo(AbstractFileInfo::DisplayInfoType::kFileDisplayName);
    case AbstractFileInfo::FileNameInfoType::kIconName:
        return dptr.staticCast<VaultFileInfoPrivate>()->iconName();
    default:
        return AbstractFileInfo::nameInfo(type);
    }
}

QString VaultFileInfo::displayInfo(const AbstractFileInfo::DisplayInfoType type) const
{
    if (AbstractFileInfo::DisplayInfoType::kFileDisplayName == type) {
        if (isRoot()) {
            return QObject::tr("My Vault");
        }
        if (dptr->proxy)
            return dptr->proxy->displayInfo(AbstractFileInfo::DisplayInfoType::kFileDisplayName);
    } else if (AbstractFileInfo::DisplayInfoType::kFileDisplayName == type) {
        return dptr.staticCast<VaultFileInfoPrivate>()->fileDisplayPath();
    }

    return AbstractFileInfo::displayInfo(type);
}
}
