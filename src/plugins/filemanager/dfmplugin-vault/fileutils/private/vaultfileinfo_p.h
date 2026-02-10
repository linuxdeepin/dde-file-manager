// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTFILEINFO_P_H
#define VAULTFILEINFO_P_H

#include "dfmplugin_vault_global.h"

#include <dfm-base/interfaces/private/fileinfo_p.h>

namespace dfmplugin_vault {

class VaultFileInfo;
class VaultFileInfoPrivate
{
    friend class VaultFileInfo;

public:
    explicit VaultFileInfoPrivate(const QUrl &url, VaultFileInfo *qq);
    virtual ~VaultFileInfoPrivate();

private:
    QString fileDisplayPath() const;
    QString absolutePath(const QString &path) const;
    QUrl vaultUrl(const QUrl &url) const;
    QUrl getUrlByNewFileName(const QString &fileName) const;

    VaultFileInfo *const q;
    QUrl localUrl;
    bool isRoot { false };
};

}

#endif   // VAULTFILEINFO_P_H
