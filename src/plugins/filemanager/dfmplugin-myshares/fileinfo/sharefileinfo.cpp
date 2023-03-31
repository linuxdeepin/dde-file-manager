// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sharefileinfo.h"
#include "private/sharefileinfo_p.h"

#include <dfm-base/base/schemefactory.h>

#include <dfm-framework/event/event.h>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_myshares;

ShareFileInfo::ShareFileInfo(const QUrl &url)
    : ProxyFileInfo(url), d(new ShareFileInfoPrivate(this))
{
    QString path = url.path();
    setProxy(InfoFactory::create<FileInfo>(QUrl::fromLocalFile(path)));
}

ShareFileInfo::~ShareFileInfo()
{
}

QString ShareFileInfo::displayOf(const DisPlayInfoType type) const
{
    if (DisPlayInfoType::kFileDisplayName == type)
        return d->fileName();
    return ProxyFileInfo::displayOf(type);
}

QString ShareFileInfo::nameOf(const NameInfoType type) const
{
    switch (type) {
    case NameInfoType::kFileName:
        [[fallthrough]];
    case NameInfoType::kFileCopyName:
        return d->fileName();
    default:
        return ProxyFileInfo::nameOf(type);
    }
}

QUrl ShareFileInfo::urlOf(const UrlInfoType type) const
{
    switch (type) {
    case FileUrlInfoType::kRedirectedFileUrl:
        return QUrl::fromLocalFile(url.path());
    case FileUrlInfoType::kUrl:
        return url;
    default:
        return ProxyFileInfo::urlOf(type);
    }
}

bool ShareFileInfo::isAttributes(const OptInfoType type) const
{
    switch (type) {
    case FileIsType::kIsDir:
        return true;
    default:
        return ProxyFileInfo::isAttributes(type);
    }
}

bool ShareFileInfo::canAttributes(const CanableInfoType type) const
{
    switch (type) {
    case FileCanType::kCanRename:
        [[fallthrough]];
    case FileCanType::kCanDrag:
        return false;
    case FileCanType::kCanRedirectionFileUrl:
        return proxy;
    default:
        return ProxyFileInfo::canAttributes(type);
    }
}

ShareFileInfoPrivate::ShareFileInfoPrivate(ShareFileInfo *qq)
    : q(qq)
{
    refresh();
}

ShareFileInfoPrivate::~ShareFileInfoPrivate()
{
}

void ShareFileInfoPrivate::refresh()
{
    if (q->fileUrl().path() != "/")
        info = dpfSlotChannel->push("dfmplugin_dirshare", "slot_Share_ShareInfoOfFilePath", q->fileUrl().path()).value<QVariantMap>();
}

QString ShareFileInfoPrivate::fileName() const
{
    return info.value(ShareInfoKeys::kName).toString();
}

void dfmplugin_myshares::ShareFileInfo::refresh()
{
    d->refresh();
}
