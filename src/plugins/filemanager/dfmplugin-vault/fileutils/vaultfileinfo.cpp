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
    if (isRoot())
        return iconName;
    else {
        if (!proxy)
            return q->fileMimeType().iconName();
        else
            proxy->nameInfo(NameInfo::kIconName);
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

bool VaultFileInfoPrivate::isRoot() const
{
    bool bRootDir = false;
    const QString &localFilePath = DFMIO::DFMUtils::buildFilePath(kVaultBasePath.toStdString().c_str(), kVaultDecryptDirName, nullptr);
    QString path = q->pathInfo(AbstractFileInfo::FilePathInfoType::kFilePath);
    if (localFilePath == path || localFilePath + "/" == path || localFilePath == path + "/") {
        bRootDir = true;
    }
    return bRootDir;
}

VaultFileInfo::VaultFileInfo(const QUrl &url)
    : AbstractFileInfo(url), d(new VaultFileInfoPrivate(url, this))
{
    dptr.reset(d);
    QUrl tempUrl = VaultHelper::vaultToLocalUrl(url);
    setProxy(InfoFactory::create<AbstractFileInfo>(tempUrl));
}

VaultFileInfo::~VaultFileInfo()
{
}

VaultFileInfo &VaultFileInfo::operator=(const VaultFileInfo &fileinfo)
{
    AbstractFileInfo::operator=(fileinfo);
    if (!d->proxy)
        setProxy(fileinfo.d->proxy);
    else {
        d->url = fileinfo.d->url;
        d->proxy = fileinfo.d->proxy;
    }
    return *this;
}

bool VaultFileInfo::operator==(const VaultFileInfo &fileinfo) const
{
    return d->proxy == fileinfo.d->proxy && d->url == fileinfo.d->url;
}

bool VaultFileInfo::operator!=(const VaultFileInfo &fileinfo) const
{
    return !(operator==(fileinfo));
}

QString VaultFileInfo::pathInfo(const dfmbase::AbstractFileInfo::FilePathInfoType type) const
{
    switch (type) {
    case FilePathInfoType::kAbsolutePath:
        return d->absolutePath();
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
        return d->vaultUrl();
    default:
        return AbstractFileInfo::urlInfo(type);
    }
}
bool VaultFileInfo::exists() const
{
    if (urlInfo(AbstractFileInfo::FileUrlInfoType::kUrl).isEmpty())
        return false;

    return d->proxy && d->proxy->exists();
}

void VaultFileInfo::refresh()
{
    if (!d->proxy) {
        return;
    }

    d->proxy->refresh();

    setProxy(InfoFactory::create<AbstractFileInfo>(d->proxy->urlInfo(AbstractFileInfo::FileUrlInfoType::kUrl)));
}

bool VaultFileInfo::isAttributes(const AbstractFileInfo::FileIsType type) const
{
    switch (type) {
    case FileIsType::kIsFile:
        [[fallthrough]];
    case FileIsType::kIsDir:
        [[fallthrough]];
    case FileIsType::kIsReadable:
        [[fallthrough]];
    case FileIsType::kIsWritable:
        [[fallthrough]];
    case FileIsType::kIsExecutable:
        [[fallthrough]];
    case FileIsType::kIsSymLink:
        [[fallthrough]];
    case FileIsType::kIsHidden:
        return !d->proxy || d->proxy->isAttributes(type);
    default:
        return AbstractFileInfo::isAttributes(type);
    }
}

bool VaultFileInfo::canAttributes(const AbstractFileInfo::FileCanType type) const
{
    switch (type) {
    case FileCanType::kCanDrop:
        if (VaultHelper::instance()->state(PathManager::vaultLockPath()) != VaultState::kUnlocked) {
            return false;
        }

        return !d->proxy || d->proxy->canAttributes(type);
    default:
        return AbstractFileInfo::canAttributes(type);
    }
}

QVariantHash VaultFileInfo::extraProperties() const
{
    if (!d->proxy)
        AbstractFileInfo::extraProperties();
    return d->proxy->extraProperties();
}

QUrl VaultFileInfo::getUrlByType(const AbstractFileInfo::FileUrlInfoType type, const QString &fileName) const
{
    switch (type) {
    case FileUrlInfoType::kGetUrlByNewFileName:
        return d->getUrlByNewFileName(fileName);
    default:
        return AbstractFileInfo::getUrlByType(type, fileName);
    }
}

QIcon VaultFileInfo::fileIcon()
{
    if (d->isRoot())
        return QIcon::fromTheme(d->iconName());

    if (!d->proxy)
        AbstractFileInfo::fileIcon();
    return d->proxy->fileIcon();
}

qint64 VaultFileInfo::size() const
{
    if (!d->proxy)
        AbstractFileInfo::size();
    return d->proxy->size();
}

int VaultFileInfo::countChildFile() const
{
    if (isAttributes(AbstractFileInfo::FileIsType::kIsDir)) {
        QDir dir(pathInfo(AbstractFileInfo::FilePathInfoType::kAbsoluteFilePath));
        QStringList entryList = dir.entryList(QDir::AllEntries | QDir::System
                                              | QDir::NoDotAndDotDot | QDir::Hidden);
        return entryList.size();
    }

    return -1;
}

QVariant VaultFileInfo::extendedAttributes(const AbstractFileInfo::FileExtendedInfoType type) const
{
    switch (type) {
    case FileExtendedInfoType::kSizeFormat:
        if (!d->proxy)
            return AbstractFileInfo::extendedAttributes(type);
        return d->proxy->extendedAttributes(type);
    default:
        return AbstractFileInfo::extendedAttributes(type);
    }
}

QString VaultFileInfo::nameInfo(const NameInfo type) const
{

    switch (type) {
    case NameInfo::kFileCopyName:
        return displayInfo(AbstractFileInfo::DisplayInfoType::kFileDisplayName);
    case NameInfo::kIconName:
        return d->iconName();
    default:
        return AbstractFileInfo::nameInfo(type);
    }
}

QString VaultFileInfo::displayInfo(const AbstractFileInfo::DisplayInfoType type) const
{
    if (AbstractFileInfo::DisplayInfoType::kFileDisplayName == type) {
        if (d->isRoot()) {
            return QObject::tr("My Vault");
        }
        if (d->proxy)
            return d->proxy->displayInfo(AbstractFileInfo::DisplayInfoType::kFileDisplayName);
    } else if (AbstractFileInfo::DisplayInfoType::kFileDisplayName == type) {
        return d->fileDisplayPath();
    }

    return AbstractFileInfo::displayInfo(type);
}
}
