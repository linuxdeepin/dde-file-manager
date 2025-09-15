// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recentfileinfo.h"
#include "utils/recentmanager.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/interfaces/private/fileinfo_p.h>

#include <QDateTime>

DFMBASE_USE_NAMESPACE
namespace dfmplugin_recent {

RecentFileInfo::RecentFileInfo(const QUrl &url)
    : ProxyFileInfo(url)
{
    if (url.path() != "/")
        setProxy(InfoFactory::create<FileInfo>(QUrl::fromLocalFile(url.path())));
}

RecentFileInfo::~RecentFileInfo()
{
}

bool RecentFileInfo::exists() const
{
    return ProxyFileInfo::exists() || url == RecentHelper::rootUrl();
}

QFile::Permissions RecentFileInfo::permissions() const
{
    if (url == RecentHelper::rootUrl()) {
        return QFileDevice::ReadGroup | QFileDevice::ReadOwner | QFileDevice::ReadOther;
    }
    return ProxyFileInfo::permissions();
}

bool RecentFileInfo::isAttributes(const OptInfoType type) const
{
    switch (type) {
    case FileIsType::kIsReadable:
        return permissions().testFlag(QFile::Permission::ReadUser);
    case FileIsType::kIsWritable:
        return permissions().testFlag(QFile::Permission::WriteUser);
    default:
        return ProxyFileInfo::isAttributes(type);
    }
}

bool RecentFileInfo::canAttributes(const CanableInfoType type) const
{
    switch (type) {
    case FileCanType::kCanDelete:
    case FileCanType::kCanTrash:
    case FileCanType::kCanRename:
        return false;
    case FileCanType::kCanRedirectionFileUrl:
        return !proxy.isNull();
    default:
        return ProxyFileInfo::canAttributes(type);
    }
}

QString RecentFileInfo::nameOf(const NameInfoType type) const
{
    switch (type) {
    case NameInfoType::kFileName:
        if (proxy)
            return proxy->nameOf(NameInfoType::kFileName);

        if (UrlRoute::isRootUrl(url))
            return QObject::tr("Recent");

        return QString();
    default:
        return ProxyFileInfo::nameOf(type);
    }
}

QUrl RecentFileInfo::urlOf(const UrlInfoType type) const
{
    switch (type) {
    case FileUrlInfoType::kCustomerStartUrl:
        [[fallthrough]];
    case FileUrlInfoType::kRedirectedFileUrl:
        return proxy ? proxy->urlOf(UrlInfoType::kUrl) : url;
    case FileUrlInfoType::kUrl:
        return url;
    default:
        return ProxyFileInfo::urlOf(type);
    }
}

QVariant RecentFileInfo::customData(int role) const
{
    using namespace dfmbase::Global;
    if (role == kItemFilePathRole) {
        return urlOf(UrlInfoType::kRedirectedFileUrl).path();
    } else if (role == kItemFileLastReadRole) {
        return timeOf(TimeInfoType::kLastRead).value<QDateTime>().toString(FileUtils::dateTimeFormat());
    } else {
        return QVariant();
    }
}

QString RecentFileInfo::displayOf(const FileInfo::DisplayInfoType type) const
{
    if (DisPlayInfoType::kFileDisplayName == type) {
        if (UrlRoute::isRootUrl(url)) {
            return QObject::tr("Recent");
        }
    }
    return ProxyFileInfo::displayOf(type);
}

QVariant RecentFileInfo::timeOf(const FileTimeType type) const
{
    switch (type) {
    case TimeInfoType::kCustomerSupport:
        return timeOf(TimeInfoType::kLastRead);
    default:
        return ProxyFileInfo::timeOf(type);
    }
}

}
