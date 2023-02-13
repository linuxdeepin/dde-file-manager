// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
            proxy->nameOf(NameInfoType::kIconName);
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

    QString path = proxy->pathOf(PathInfoType::kPath);
    QUrl virtualUrl = VaultHelper::instance()->pathToVaultVirtualUrl(path);
    return virtualUrl.path();
}

QUrl VaultFileInfoPrivate::vaultUrl() const
{
    if (!proxy)
        return QUrl();

    QUrl url = proxy->urlOf(UrlInfoType::kUrl);
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
    QString path = q->pathOf(PathInfoType::kFilePath);
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

QString VaultFileInfo::pathOf(const PathInfoType type) const
{
    switch (type) {
    case FilePathInfoType::kAbsolutePath:
        return d->absolutePath();
    default:
        return AbstractFileInfo::pathOf(type);
    }
}

QUrl VaultFileInfo::urlOf(const UrlInfoType type) const
{
    switch (type) {
    case FileUrlInfoType::kUrl:
        [[fallthrough]];
    case FileUrlInfoType::kRedirectedFileUrl:
        return d->vaultUrl();
    default:
        return AbstractFileInfo::urlOf(type);
    }
}
bool VaultFileInfo::exists() const
{
    if (urlOf(UrlInfoType::kUrl).isEmpty())
        return false;

    return d->proxy && d->proxy->exists();
}

void VaultFileInfo::refresh()
{
    if (!d->proxy) {
        return;
    }

    d->proxy->refresh();

    setProxy(InfoFactory::create<AbstractFileInfo>(d->proxy->urlOf(UrlInfoType::kUrl)));
}

bool VaultFileInfo::isAttributes(const OptInfoType type) const
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

bool VaultFileInfo::canAttributes(const CanableInfoType type) const
{
    switch (type) {
    case FileCanType::kCanDrop:
        if (VaultHelper::instance()->state(PathManager::vaultLockPath()) != VaultState::kUnlocked) {
            return false;
        }

        return !d->proxy || d->proxy->canAttributes(type);
    case FileCanType::kCanRedirectionFileUrl:
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

QUrl VaultFileInfo::getUrlByType(const UrlInfoType type, const QString &fileName) const
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
    if (isAttributes(OptInfoType::kIsDir)) {
        QDir dir(pathOf(PathInfoType::kAbsoluteFilePath));
        QStringList entryList = dir.entryList(QDir::AllEntries | QDir::System
                                              | QDir::NoDotAndDotDot | QDir::Hidden);
        return entryList.size();
    }

    return -1;
}

QVariant VaultFileInfo::extendAttributes(const ExtInfoType type) const
{
    switch (type) {
    case FileExtendedInfoType::kSizeFormat:
        if (!d->proxy)
            return AbstractFileInfo::extendAttributes(type);
        return d->proxy->extendAttributes(type);
    default:
        return AbstractFileInfo::extendAttributes(type);
    }
}

QString VaultFileInfo::nameOf(const NameInfoType type) const
{

    switch (type) {
    case NameInfoType::kFileCopyName:
        return displayOf(DisPlayInfoType::kFileDisplayName);
    case NameInfoType::kIconName:
        return d->iconName();
    default:
        return AbstractFileInfo::nameOf(type);
    }
}

QString VaultFileInfo::displayOf(const DisPlayInfoType type) const
{
    if (DisPlayInfoType::kFileDisplayName == type) {
        if (d->isRoot()) {
            return QObject::tr("My Vault");
        }
        if (d->proxy)
            return d->proxy->displayOf(DisPlayInfoType::kFileDisplayName);
    } else if (DisPlayInfoType::kFileDisplayPath == type) {
        return d->fileDisplayPath();
    }

    return AbstractFileInfo::displayOf(type);
}
}
