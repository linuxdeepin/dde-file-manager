// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "avfsfileinfo.h"
#include "utils/avfsutils.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/interfaces/private/fileinfo_p.h>

using namespace dfmplugin_avfsbrowser;
DFMBASE_USE_NAMESPACE

AvfsFileInfo::AvfsFileInfo(const QUrl &url)
    : ProxyFileInfo(url)
{
    setProxy(InfoFactory::create<FileInfo>(AvfsUtils::avfsUrlToLocal(url)));
}

AvfsFileInfo::~AvfsFileInfo()
{
}

QUrl AvfsFileInfo::urlOf(const FileInfo::FileUrlInfoType type) const
{
    switch (type) {
    case FileUrlInfoType::kRedirectedFileUrl:
        return proxy ? proxy->urlOf(UrlInfoType::kUrl) : url;
    case FileUrlInfoType::kUrl:
        return url;
    default:
        return ProxyFileInfo::urlOf(type);
    }
}

bool AvfsFileInfo::canAttributes(const FileInfo::FileCanType type) const
{
    switch (type) {
    case FileCanType::kCanRedirectionFileUrl:
        return !proxy.isNull();
    default:
        return ProxyFileInfo::canAttributes(type);
    }
}
