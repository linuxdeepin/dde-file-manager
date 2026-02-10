// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHAREFILEINFO_H
#define SHAREFILEINFO_H

#include "dfmplugin_myshares_global.h"
#include <dfm-base/interfaces/proxyfileinfo.h>

namespace dfmplugin_myshares {
class ShareFileInfoPrivate;
class ShareFileInfo : public DFMBASE_NAMESPACE::ProxyFileInfo
{
    QSharedPointer<ShareFileInfoPrivate> d { nullptr };

public:
    explicit ShareFileInfo(const QUrl &url);
    virtual ~ShareFileInfo() override;

    // AbstractFileInfo interface
public:
    virtual QString displayOf(const DisplayInfoType type) const override;
    virtual QString nameOf(const FileNameInfoType type) const override;
    virtual QUrl urlOf(const FileUrlInfoType type) const override;
    virtual bool isAttributes(const FileIsType type) const override;
    virtual bool canAttributes(const FileCanType type) const override;
    virtual void refresh() override;
    virtual void updateAttributes(const QList<FileInfoAttributeID> &types = {}) override;
};

}

#endif   // SHAREFILEINFO_H
