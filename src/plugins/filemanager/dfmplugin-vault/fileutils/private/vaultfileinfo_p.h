/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
};

}

#endif   // VAULTFILEINFO_P_H
