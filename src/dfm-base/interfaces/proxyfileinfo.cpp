// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-base/interfaces/proxyfileinfo.h>
#include <dfm-base/file/local/asyncfileinfo.h>

DFMBASE_USE_NAMESPACE

#define CALL_PROXY(Fun) \
    if (proxy) return proxy->Fun;

ProxyFileInfo::ProxyFileInfo(const QUrl &url)
    : FileInfo(url)
{
}

ProxyFileInfo::~ProxyFileInfo()
{
    removeNotifyUrl(url, QString::number(quintptr(this), 16));
}

QUrl ProxyFileInfo::fileUrl() const
{
    return this->url;
}

bool ProxyFileInfo::exists() const
{
    CALL_PROXY(exists());

    return FileInfo::exists();
}

void ProxyFileInfo::refresh()
{
    CALL_PROXY(refresh());

    return FileInfo::refresh();
}

void ProxyFileInfo::setProxy(const FileInfoPointer &proxy)
{
    if (proxy.isNull())
        return;
    this->proxy = proxy;
    setNotifyUrl(url, QString::number(quintptr(this), 16));
}

void ProxyFileInfo::setNotifyUrl(const QUrl &url, const QString &token)
{
    if (proxy.isNull())
        return;
    auto tproxy = proxy.dynamicCast<ProxyFileInfo>();
    if (!tproxy.isNull()) {
        tproxy->setNotifyUrl(url, token);
        return;
    }
    auto async = proxy.dynamicCast<AsyncFileInfo>();
    if (async.isNull())
        return;
    async->setNotifyUrl(url, token);
}

void ProxyFileInfo::removeNotifyUrl(const QUrl &url, const QString &token)
{
    if (proxy.isNull())
        return;
    auto tproxy = proxy.dynamicCast<ProxyFileInfo>();
    if (!tproxy.isNull()) {
        tproxy->removeNotifyUrl(url, token);
        return;
    }
    auto async = proxy.dynamicCast<AsyncFileInfo>();
    if (async.isNull())
        return;
    async->removeNotifyUrl(url, token);
}

QString dfmbase::ProxyFileInfo::filePath() const
{
    CALL_PROXY(filePath());

    return FileInfo::filePath();
}

QString dfmbase::ProxyFileInfo::absoluteFilePath() const
{
    CALL_PROXY(absoluteFilePath());

    return FileInfo::absoluteFilePath();
}

QString dfmbase::ProxyFileInfo::fileName() const
{
    CALL_PROXY(fileName());

    return FileInfo::fileName();
}

QString dfmbase::ProxyFileInfo::baseName() const
{
    CALL_PROXY(baseName());

    return FileInfo::baseName();
}

QString dfmbase::ProxyFileInfo::completeBaseName() const
{
    CALL_PROXY(completeBaseName());

    return FileInfo::completeBaseName();
}

QString dfmbase::ProxyFileInfo::suffix() const
{
    CALL_PROXY(suffix());

    return FileInfo::suffix();
}

QString dfmbase::ProxyFileInfo::completeSuffix() const
{
    CALL_PROXY(completeSuffix());

    return FileInfo::completeSuffix();
}

QString dfmbase::ProxyFileInfo::path() const
{
    CALL_PROXY(path());

    return FileInfo::path();
}

QString dfmbase::ProxyFileInfo::absolutePath() const
{
    CALL_PROXY(absolutePath());

    return FileInfo::absolutePath();
}

bool dfmbase::ProxyFileInfo::isReadable() const
{
    CALL_PROXY(isReadable());

    return FileInfo::isReadable();
}

bool dfmbase::ProxyFileInfo::isWritable() const
{
    CALL_PROXY(isWritable());

    return FileInfo::isWritable();
}

bool dfmbase::ProxyFileInfo::isExecutable() const
{
    CALL_PROXY(isExecutable());

    return FileInfo::isExecutable();
}

bool dfmbase::ProxyFileInfo::isHidden() const
{
    CALL_PROXY(isHidden());

    return FileInfo::isHidden();
}

bool dfmbase::ProxyFileInfo::isNativePath() const
{
    CALL_PROXY(isNativePath());

    return FileInfo::isNativePath();
}

bool dfmbase::ProxyFileInfo::isFile() const
{
    CALL_PROXY(isFile());

    return FileInfo::isFile();
}

bool dfmbase::ProxyFileInfo::isDir() const
{
    CALL_PROXY(isDir());

    return FileInfo::isDir();
}

bool dfmbase::ProxyFileInfo::isSymLink() const
{
    CALL_PROXY(isSymLink());

    return FileInfo::isSymLink();
}

bool dfmbase::ProxyFileInfo::isRoot() const
{
    CALL_PROXY(isRoot());

    return FileInfo::isRoot();
}

bool dfmbase::ProxyFileInfo::isBundle() const
{
    CALL_PROXY(isBundle());

    return FileInfo::isBundle();
}

QString dfmbase::ProxyFileInfo::symLinkTarget() const
{
    CALL_PROXY(symLinkTarget());

    return FileInfo::symLinkTarget();
}

QString dfmbase::ProxyFileInfo::owner() const
{
    CALL_PROXY(owner());

    return FileInfo::owner();
}

uint dfmbase::ProxyFileInfo::ownerId() const
{
    CALL_PROXY(ownerId());

    return FileInfo::ownerId();
}

QString dfmbase::ProxyFileInfo::group() const
{
    CALL_PROXY(group());

    return FileInfo::group();
}

uint dfmbase::ProxyFileInfo::groupId() const
{
    CALL_PROXY(groupId());

    return FileInfo::groupId();
}

bool dfmbase::ProxyFileInfo::permission(QFileDevice::Permissions permissions) const
{
    CALL_PROXY(permission(permissions));

    return FileInfo::permission(permissions);
}

QFileDevice::Permissions dfmbase::ProxyFileInfo::permissions() const
{
    CALL_PROXY(permissions());

    return FileInfo::permissions();
}

int dfmbase::ProxyFileInfo::countChildFile() const
{
    CALL_PROXY(countChildFile());

    return FileInfo::countChildFile();
}

qint64 dfmbase::ProxyFileInfo::size() const
{
    CALL_PROXY(size());

    return FileInfo::size();
}

quint32 dfmbase::ProxyFileInfo::birthTime() const
{
    CALL_PROXY(birthTime());

    return FileInfo::birthTime();
}

quint32 dfmbase::ProxyFileInfo::metadataChangeTime() const
{
    CALL_PROXY(metadataChangeTime());

    return FileInfo::metadataChangeTime();
}

quint32 dfmbase::ProxyFileInfo::lastModified() const
{
    CALL_PROXY(lastModified());

    return FileInfo::lastModified();
}

quint32 dfmbase::ProxyFileInfo::lastRead() const
{
    CALL_PROXY(lastRead());

    return FileInfo::lastRead();
}

bool dfmbase::ProxyFileInfo::initQuerier()
{
    CALL_PROXY(initQuerier());

    return FileInfo::initQuerier();
}

void dfmbase::ProxyFileInfo::initQuerierAsync(int ioPriority, initQuerierAsyncCallback func, void *userData)
{
    CALL_PROXY(initQuerierAsync(ioPriority, func, userData));

    return FileInfo::initQuerierAsync(ioPriority, func, userData);
}

void dfmbase::ProxyFileInfo::cacheAttribute(dfmio::DFileInfo::AttributeID id, const QVariant &value)
{
    CALL_PROXY(cacheAttribute(id, value));

    return FileInfo::cacheAttribute(id, value);
}

QString dfmbase::ProxyFileInfo::nameOf(const FileNameInfoType type) const
{
    CALL_PROXY(nameOf(type));

    return FileInfo::nameOf(type);
}

QString dfmbase::ProxyFileInfo::pathOf(const FilePathInfoType type) const
{
    CALL_PROXY(pathOf(type));

    return FileInfo::pathOf(type);
}

QString dfmbase::ProxyFileInfo::displayOf(const DisplayInfoType type) const
{
    CALL_PROXY(displayOf(type));

    return FileInfo::displayOf(type);
}

QUrl dfmbase::ProxyFileInfo::urlOf(const FileUrlInfoType type) const
{
    CALL_PROXY(urlOf(type));

    return FileInfo::urlOf(type);
}

QUrl dfmbase::ProxyFileInfo::getUrlByType(const FileUrlInfoType type, const QString &fileName) const
{
    CALL_PROXY(getUrlByType(type, fileName));

    return FileInfo::getUrlByType(type, fileName);
}

bool dfmbase::ProxyFileInfo::isAttributes(const FileIsType type) const
{
    CALL_PROXY(isAttributes(type));

    return FileInfo::isAttributes(type);
}

bool dfmbase::ProxyFileInfo::canAttributes(const FileCanType type) const
{
    CALL_PROXY(canAttributes(type));

    return FileInfo::canAttributes(type);
}

QVariant dfmbase::ProxyFileInfo::extendAttributes(const FileExtendedInfoType type) const
{
    CALL_PROXY(extendAttributes(type));

    return FileInfo::extendAttributes(type);
}

int dfmbase::ProxyFileInfo::countChildFileAsync() const
{
    CALL_PROXY(countChildFileAsync());

    return FileInfo::countChildFileAsync();
}

QVariant dfmbase::ProxyFileInfo::timeOf(const FileTimeType type) const
{
    CALL_PROXY(timeOf(type));

    return FileInfo::timeOf(type);
}

QIcon dfmbase::ProxyFileInfo::fileIcon()
{
    CALL_PROXY(fileIcon());

    return FileInfo::fileIcon();
}

QMimeType dfmbase::ProxyFileInfo::fileMimeType(QMimeDatabase::MatchMode mode)
{
    CALL_PROXY(fileMimeType(mode));

    return FileInfo::fileMimeType(mode);
}

QMimeType dfmbase::ProxyFileInfo::fileMimeTypeAsync(QMimeDatabase::MatchMode mode)
{
    CALL_PROXY(fileMimeTypeAsync(mode));

    return FileInfo::fileMimeTypeAsync(mode);
}

QVariantHash dfmbase::ProxyFileInfo::extraProperties() const
{
    CALL_PROXY(extraProperties());

    return FileInfo::extraProperties();
}

QVariant dfmbase::ProxyFileInfo::customData(int role) const
{
    CALL_PROXY(customData(role));

    return FileInfo::customData(role);
}

FileInfo::FileType dfmbase::ProxyFileInfo::fileType() const
{
    CALL_PROXY(fileType());

    return FileInfo::fileType();
}

Qt::DropActions dfmbase::ProxyFileInfo::supportedOfAttributes(const SupportType type) const
{
    CALL_PROXY(supportedOfAttributes(type));

    return FileInfo::supportedOfAttributes(type);
}

QString dfmbase::ProxyFileInfo::viewOfTip(const ViewType type) const
{
    CALL_PROXY(viewOfTip(type));

    return FileInfo::viewOfTip(type);
}

QVariant dfmbase::ProxyFileInfo::customAttribute(const char *key, const dfmio::DFileInfo::DFileAttributeType type)
{
    CALL_PROXY(customAttribute(key, type));

    return FileInfo::customAttribute(key, type);
}

QMap<dfmio::DFileInfo::AttributeExtendID, QVariant> dfmbase::ProxyFileInfo::mediaInfoAttributes(dfmio::DFileInfo::MediaType type,
                                                                                                QList<dfmio::DFileInfo::AttributeExtendID> ids) const
{
    CALL_PROXY(mediaInfoAttributes(type, ids));

    return FileInfo::mediaInfoAttributes(type, ids);
}

void dfmbase::ProxyFileInfo::setExtendedAttributes(const FileExtendedInfoType &key, const QVariant &value)
{
    CALL_PROXY(setExtendedAttributes(key, value));

    return FileInfo::setExtendedAttributes(key, value);
}

void ProxyFileInfo::updateAttributes(const QList<FileInfo::FileInfoAttributeID> &types)
{
    CALL_PROXY(updateAttributes(types));

    return FileInfo::updateAttributes(types);
}
