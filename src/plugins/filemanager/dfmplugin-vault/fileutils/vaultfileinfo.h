// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTFILEINFO_H
#define VAULTFILEINFO_H

#include "dfmplugin_vault_global.h"
#include <dfm-base/interfaces/proxyfileinfo.h>

namespace dfmplugin_vault {
class VaultFileInfoPrivate;
class VaultFileInfo : public DFMBASE_NAMESPACE::ProxyFileInfo
{
    friend class VaultFileInfoPrivate;
    VaultFileInfoPrivate *d;

public:
    explicit VaultFileInfo(const QUrl &url);
    explicit VaultFileInfo(const QUrl &url, const FileInfoPointer &proxy);
    virtual ~VaultFileInfo() override;

    virtual VaultFileInfo &operator=(const VaultFileInfo &fileinfo);
    virtual bool operator==(const VaultFileInfo &fileinfo) const;
    virtual bool operator!=(const VaultFileInfo &fileinfo) const;

    virtual QString pathOf(const FilePathInfoType type) const override;
    virtual QUrl urlOf(const FileUrlInfoType type) const override;
    virtual bool exists() const override;
    virtual void refresh() override;
    virtual bool isAttributes(const FileIsType type) const override;
    virtual bool canAttributes(const FileCanType type) const override;

    virtual qint64 size() const override;
    virtual int countChildFile() const override;
    virtual QVariant extendAttributes(const FileExtendedInfoType type) const override;
    virtual QVariantHash extraProperties() const override;
    virtual QUrl getUrlByType(const FileUrlInfoType type, const QString &fileName) const override;

    virtual QIcon fileIcon() override;

    virtual QString nameOf(const FileNameInfoType type) const override;

    virtual QString displayOf(const DisplayInfoType type) const override;
    virtual QString viewOfTip(const ViewType type) const override;
};
}
#endif   //! VAULTFILEINFO_H
