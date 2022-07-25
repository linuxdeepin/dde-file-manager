/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
#ifndef VAULTFILEWATCHER_H
#define VAULTFILEWATCHER_H

#include "dfmplugin_vault_global.h"
#include "dfm-base/interfaces/abstractfilewatcher.h"

namespace dfmplugin_vault {
class VaultFileWatcherPrivate;
class VaultFileWatcher : public DFMBASE_NAMESPACE::AbstractFileWatcher
{
    Q_OBJECT
public:
    explicit VaultFileWatcher() = delete;
    explicit VaultFileWatcher(const QUrl &url, QObject *parent = nullptr);
    virtual ~VaultFileWatcher() override;

private slots:
    void onFileDeleted(const QUrl &url);
    void onFileAttributeChanged(const QUrl &url);
    void onFileRename(const QUrl &fromUrl, const QUrl &toUrl);
    void onSubfileCreated(const QUrl &url);

private:
    VaultFileWatcherPrivate *dptr { nullptr };
};
}
#endif   // VAULTFILEWATCHER_H
