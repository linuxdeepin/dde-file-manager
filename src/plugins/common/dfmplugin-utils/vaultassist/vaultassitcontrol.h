// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTASSITCONTROL_H
#define VAULTASSITCONTROL_H

#include "dfmplugin_utils_global.h"

#include <QObject>
namespace dfmplugin_utils {
class VaultAssitControl : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(VaultAssitControl)
public:
    static VaultAssitControl *instance();

    inline QString scheme()
    {
        return "dfmvault";
    }

    bool isVaultFile(const QUrl &url);
    QString vaultBaseDirLocalPath();
    QString vaultMountDirLocalPath();
    QString buildVaultLocalPath(const QString &path = "", const QString &base = "");
    QUrl vaultUrlToLocalUrl(const QUrl &url);
    QList<QUrl> transUrlsToLocal(const QList<QUrl> &urls);

private:
    explicit VaultAssitControl(QObject *parent = Q_NULLPTR);

};
}
#endif // VAULTASSITCONTROL_H
