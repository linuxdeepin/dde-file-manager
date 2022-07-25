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
#ifndef VAULTREMOVEPROGRESSVIEW_H
#define VAULTREMOVEPROGRESSVIEW_H

#include "dfmplugin_vault_global.h"

#include <dtkwidget_global.h>
#include <QWidget>

DWIDGET_BEGIN_NAMESPACE
class DWaterProgress;
DWIDGET_END_NAMESPACE

namespace dfmplugin_vault {
class VaultRemoveProgressView : public QWidget
{
    Q_OBJECT
public:
    explicit VaultRemoveProgressView(QWidget *parent = nullptr);
    ~VaultRemoveProgressView();

    /*!
     * /brief removeVault 删除保险箱
     * /param vaultPath 保险箱全路径
     */
    void removeVault(const QString &vaultLockPath, const QString &vaultUnlockPath);

    void clear();

private:
    /*!
     * /brief statisticsFiles 统计指定目录下的文件数量
     * /param vaultPath 保险箱全路径
     * /return
     */
    bool statisticsFiles(const QString &vaultPath);

    /*!
     * /brief removeFileInDir 递归删除指定文件夹下的文件
     * /param vaultPath 保险箱全路径
     */
    void removeFileInDir(const QString &vaultPath);

signals:
    void fileRemoved(int value);

    void removeFinished(bool result);

private slots:
    void onFileRemove(int value);

private:
    DTK_WIDGET_NAMESPACE::DWaterProgress *vaultRmProgressBar { nullptr };

    int filesCount { 0 };   //!文件、文件夹数量
    int removeFileCount { 0 };   //! 删除文件数量
    int removeDirCount { 0 };   //! 删除文件夹数量
};
}
#endif   // VAULTREMOVEPROGRESSVIEW_H
