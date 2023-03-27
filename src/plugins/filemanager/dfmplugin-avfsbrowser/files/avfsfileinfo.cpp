// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "avfsfileinfo.h"
#include "utils/avfsutils.h"

#include "dfm-base/base/schemefactory.h"
#include "dfm-base/interfaces/private/fileinfo_p.h"

using namespace dfmplugin_avfsbrowser;
DFMBASE_USE_NAMESPACE

AvfsFileInfo::AvfsFileInfo(const QUrl &url)
    : FileInfo(url)
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
        return dptr->proxy ? dptr->proxy->urlOf(UrlInfoType::kUrl) : dptr->url;
    default:
        return FileInfo::urlOf(type);
    }
}

bool AvfsFileInfo::canAttributes(const FileInfo::FileCanType type) const
{
    switch (type) {
    case FileCanType::kCanRedirectionFileUrl:
        return dptr->proxy;
    default:
        return FileInfo::canAttributes(type);
    }
}
