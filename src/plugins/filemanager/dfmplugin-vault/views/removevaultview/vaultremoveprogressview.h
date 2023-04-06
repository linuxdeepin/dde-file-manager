// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTREMOVEPROGRESSVIEW_H
#define VAULTREMOVEPROGRESSVIEW_H

#include "dfmplugin_vault_global.h"

#include <dtkwidget_global.h>

#include <QWidget>
#include <QHBoxLayout>

DWIDGET_BEGIN_NAMESPACE
class DWaterProgress;
class DLabel;
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
    DTK_WIDGET_NAMESPACE::DLabel *deleteFinishedImageLabel { nullptr };
    QHBoxLayout *layout { nullptr };

    int filesCount { 0 };   //!文件、文件夹数量
    int removeFileCount { 0 };   //! 删除文件数量
    int removeDirCount { 0 };   //! 删除文件夹数量
};
}
#endif   // VAULTREMOVEPROGRESSVIEW_H
