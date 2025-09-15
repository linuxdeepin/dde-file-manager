// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RECENTFILEINFO_H
#define RECENTFILEINFO_H

#include "dfmplugin_recent_global.h"

#include <dfm-base/interfaces/proxyfileinfo.h>

namespace dfmplugin_recent {
class RecentFileInfo : public DFMBASE_NAMESPACE::ProxyFileInfo
{
public:
    explicit RecentFileInfo(const QUrl &url);
    ~RecentFileInfo() override;

    virtual bool exists() const override;
    virtual QFile::Permissions permissions() const override;
    virtual bool isAttributes(const FileIsType type) const override;
    virtual bool canAttributes(const FileCanType type) const override;

    virtual QString nameOf(const FileNameInfoType type) const override;
    virtual QUrl urlOf(const FileUrlInfoType type) const override;

    virtual QVariant customData(int role) const override;

    virtual QString displayOf(const DisplayInfoType type) const override;
    virtual QVariant timeOf(const FileTimeType type) const override;
};

using RecentFileInfoPointer = QSharedPointer<RecentFileInfo>;
}

#endif   // RECENTFILEINFO_H
