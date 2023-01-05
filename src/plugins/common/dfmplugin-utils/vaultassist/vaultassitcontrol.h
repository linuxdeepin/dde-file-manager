/*
* Copyright (C) 2023 Uniontech Software Technology Co., Ltd.
*
* Author:     gongheng <gongheng@uniontech.com>
*
* Maintainer: zhengyouge <zhengyouge@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
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
    explicit VaultAssitControl(QObject *parent = nullptr);

};
}
#endif // VAULTASSITCONTROL_H
