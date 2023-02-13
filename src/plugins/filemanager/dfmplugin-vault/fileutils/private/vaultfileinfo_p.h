// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTFILEINFO_P_H
#define VAULTFILEINFO_P_H

#include "dfmplugin_vault_global.h"

#include "dfm-base/interfaces/private/abstractfileinfo_p.h"

namespace dfmplugin_vault {

class VaultFileInfo;
class VaultFileInfoPrivate : public DFMBASE_NAMESPACE::AbstractFileInfoPrivate
{
    friend class VaultFileInfo;

public:
    explicit VaultFileInfoPrivate(const QUrl &url, DFMBASE_NAMESPACE::AbstractFileInfo *qq);
    virtual ~VaultFileInfoPrivate();

private:
    QString iconName();
    QString fileDisplayPath() const;
    QString absolutePath() const;
    QUrl vaultUrl() const;
    QUrl getUrlByNewFileName(const QString &fileName) const;
    bool isRoot() const;
};

}

#endif   // VAULTFILEINFO_P_H
