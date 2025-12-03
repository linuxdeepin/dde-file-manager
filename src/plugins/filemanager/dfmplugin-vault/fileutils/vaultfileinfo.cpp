// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultfileinfo.h"
#include "utils/vaultdefine.h"
#include "utils/vaulthelper.h"
#include "utils/pathmanager.h"
#include "private/vaultfileinfo_p.h"

#include <dfm-base/base/standardpaths.h>
#include <dfm-base/base/schemefactory.h>

#include <dfm-io/dfmio_utils.h>

#include <DFileIconProvider>

#include <QDateTime>
#include <QDir>
#include <QDebug>
#include <qplatformdefs.h>

#include <sys/stat.h>

DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace dfmplugin_vault;

VaultFileInfoPrivate::VaultFileInfoPrivate(const QUrl &url, VaultFileInfo *qq)
    : q(qq)
{
    localUrl = VaultHelper::vaultToLocalUrl(url);
    isRoot = (url == VaultHelper::instance()->rootUrl() ? true : false);
}

VaultFileInfoPrivate::~VaultFileInfoPrivate()
{
}

QString VaultFileInfoPrivate::fileDisplayPath() const
{
    QUrl currentUrl = q->fileUrl();
    currentUrl.setHost("");
    QString urlStr = currentUrl.toString();
    QByteArray array = urlStr.toLocal8Bit();
    QString filePath = QUrl::fromPercentEncoding(array);
    return filePath;
}

QString VaultFileInfoPrivate::absolutePath(const QString &path) const
{

    QUrl virtualUrl = VaultHelper::instance()->pathToVaultVirtualUrl(path);
    return virtualUrl.path();
}

QUrl VaultFileInfoPrivate::vaultUrl(const QUrl &url) const
{
    auto tmpurl = VaultHelper::instance()->pathToVaultVirtualUrl(url.path());
    return tmpurl;
}

QUrl VaultFileInfoPrivate::getUrlByNewFileName(const QString &fileName) const
{
    QUrl theUrl = q->urlOf(FileInfo::FileUrlInfoType::kUrl);
    QString newPath = DFMIO::DFMUtils::buildFilePath(q->pathOf(PathInfoType::kAbsolutePath).toStdString().c_str(),
                                                     fileName.toStdString().c_str(), nullptr);
    if (!newPath.startsWith(QDir::separator()))
        newPath = QDir::separator() + newPath;
    theUrl.setPath(newPath);

    theUrl.setHost("");

    return theUrl;
}

VaultFileInfo::VaultFileInfo(const QUrl &url)
    : ProxyFileInfo(url), d(new VaultFileInfoPrivate(url, this))
{
    d->localUrl = VaultHelper::vaultToLocalUrl(url);
    setProxy(InfoFactory::create<FileInfo>(d->localUrl, Global::CreateFileInfoType::kCreateFileInfoAsyncAndCache));
}

VaultFileInfo::VaultFileInfo(const QUrl &url, const FileInfoPointer &proxy)
    : ProxyFileInfo(url), d(new VaultFileInfoPrivate(url, this))
{
    d->localUrl = VaultHelper::vaultToLocalUrl(url);
    setProxy(proxy);
}

VaultFileInfo::~VaultFileInfo()
{
    delete d;
}

VaultFileInfo &VaultFileInfo::operator=(const VaultFileInfo &fileinfo)
{
    ProxyFileInfo::operator=(fileinfo);
    if (!proxy)
        setProxy(fileinfo.proxy);
    else {
        url = fileinfo.url;
        proxy = fileinfo.proxy;
    }
    return *this;
}

bool VaultFileInfo::operator==(const VaultFileInfo &fileinfo) const
{
    return proxy == fileinfo.proxy && url == fileinfo.url;
}

bool VaultFileInfo::operator!=(const VaultFileInfo &fileinfo) const
{
    return !(operator==(fileinfo));
}

QString VaultFileInfo::pathOf(const PathInfoType type) const
{
    switch (type) {
    case FilePathInfoType::kAbsolutePath:
        if (!proxy) {
            fmWarning() << "Vault: No proxy available for absolute path";
            return "";
        }
        return d->absolutePath(proxy->pathOf(type));
    default:
        return ProxyFileInfo::pathOf(type);
    }
}

QUrl VaultFileInfo::urlOf(const UrlInfoType type) const
{
    switch (type) {
    case FileUrlInfoType::kUrl:
        return url;
    case FileUrlInfoType::kRedirectedFileUrl:
        return d->localUrl;
    default:
        return ProxyFileInfo::urlOf(type);
    }
}

bool VaultFileInfo::exists() const
{
    if (urlOf(UrlInfoType::kUrl).isEmpty()) {
        fmDebug() << "Vault: URL is empty, file does not exist";
        return false;
    }

    return proxy && proxy->exists();
}

void VaultFileInfo::refresh()
{
    if (!proxy) {
        fmWarning() << "Vault: No proxy available for refresh";
        return;
    }

    proxy->refresh();
}

bool VaultFileInfo::isAttributes(const OptInfoType type) const
{
    switch (type) {
    case FileIsType::kIsFile:
        return !proxy || proxy->isAttributes(type);
    case FileIsType::kIsDir: {
        if (d->isRoot)
            return true;
        return !proxy || proxy->isAttributes(type);
    }
    case FileIsType::kIsReadable:
        [[fallthrough]];
    case FileIsType::kIsWritable:
        [[fallthrough]];
    case FileIsType::kIsExecutable:
        [[fallthrough]];
    case FileIsType::kIsSymLink:
        [[fallthrough]];
    case FileIsType::kIsHidden:
        return !proxy || proxy->isAttributes(type);
    default:
        return ProxyFileInfo::isAttributes(type);
    }
}

bool VaultFileInfo::canAttributes(const CanableInfoType type) const
{
    switch (type) {
    case FileCanType::kCanDrop:
        if (VaultHelper::instance()->state(PathManager::vaultLockPath()) != VaultState::kUnlocked) {
            return false;
        }
        return !proxy || proxy->canAttributes(type);
    case FileCanType::kCanRedirectionFileUrl:
        return !proxy.isNull();
    default:
        return ProxyFileInfo::canAttributes(type);
    }
}

QVariantHash VaultFileInfo::extraProperties() const
{
    if (!proxy) {
        fmDebug() << "Vault: No proxy available for extraProperties, URL:" << url.toString();
        return ProxyFileInfo::extraProperties();
    }
    return proxy->extraProperties();
}

QUrl VaultFileInfo::getUrlByType(const UrlInfoType type, const QString &fileName) const
{
    switch (type) {
    case FileUrlInfoType::kGetUrlByNewFileName:
        return d->getUrlByNewFileName(fileName);
    default:
        return ProxyFileInfo::getUrlByType(type, fileName);
    }
}

QIcon VaultFileInfo::fileIcon()
{
    if (d->isRoot) {
        QString iconName = "safebox";   // 如果是根目录，用保险柜图标
        return QIcon::fromTheme(iconName);
    }

    if (!proxy) {
        fmDebug() << "Vault: No proxy available for fileIcon, URL:" << url.toString();
        return ProxyFileInfo::fileIcon();
    }
    return proxy->fileIcon();
}

QString VaultFileInfo::viewOfTip(const FileInfo::ViewType type) const
{
    if (type == ViewType::kEmptyDir) {
        if (url == VaultHelper::instance()->rootUrl()) {
            return FileInfo::viewOfTip(type);
        }
    }

    return ProxyFileInfo::viewOfTip(type);
}

qint64 VaultFileInfo::size() const
{
    if (!proxy) {
        fmDebug() << "Vault: No proxy available for size, URL:" << url.toString();
        return ProxyFileInfo::size();
    }
    return proxy->size();
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
        if (!proxy) {
            fmDebug() << "Vault: No proxy available, using base extended attributes";
            return ProxyFileInfo::extendAttributes(type);
        }
        return proxy->extendAttributes(type);
    default:
        return ProxyFileInfo::extendAttributes(type);
    }
}

QString VaultFileInfo::nameOf(const NameInfoType type) const
{
    switch (type) {
    case NameInfoType::kFileCopyName:
        return displayOf(DisPlayInfoType::kFileDisplayName);
    case NameInfoType::kIconName: {
        QString iconName = "safebox";   // 如果是根目录，用保险柜图标
        if (d->isRoot) {
            return iconName;
        } else {
            if (!proxy) {
                fmDebug() << "Vault: No proxy available for icon name, URL:" << url.toString();
                return const_cast<VaultFileInfo *>(this)->fileMimeType(QMimeDatabase::MatchDefault).iconName();
            } else {
                return proxy->nameOf(NameInfoType::kIconName);
            }
        }
    }
    default:
        return ProxyFileInfo::nameOf(type);
    }
}

QString VaultFileInfo::displayOf(const DisPlayInfoType type) const
{
    switch (type) {
    case DisPlayInfoType::kFileDisplayName:
        if (d->isRoot)
            return QObject::tr("File Vault");
        if (!proxy) {
            fmDebug() << "Vault: No proxy available for display name, URL:" << url.toString();
            return ProxyFileInfo::displayOf(type);
        }
        return proxy->displayOf(DisPlayInfoType::kFileDisplayName);
    case DisPlayInfoType::kFileDisplayPath:
        return d->fileDisplayPath();
    default:
        return ProxyFileInfo::displayOf(type);
    }
}
