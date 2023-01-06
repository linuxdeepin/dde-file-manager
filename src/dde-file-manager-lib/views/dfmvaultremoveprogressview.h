// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMVAULTREMOVEPROGRESSVIEW_H
#define DFMVAULTREMOVEPROGRESSVIEW_H

#include <dtkwidget_global.h>
#include <QWidget>

DWIDGET_BEGIN_NAMESPACE
class DWaterProgress;
DWIDGET_END_NAMESPACE

DWIDGET_USE_NAMESPACE
class DFMVaultRemoveProgressView : public QWidget
{
    Q_OBJECT
public:
    explicit DFMVaultRemoveProgressView(QWidget *parent = nullptr);
    ~DFMVaultRemoveProgressView();

    /**
     * @brief removeVault 删除保险箱
     * @param vaultPath 保险箱全路径
     */
    void removeVault(const QString &vaultLockPath, const QString &vaultUnlockPath);

    void clear();

private:
    /**
     * @brief statisticsFiles 统计指定目录下的文件数量
     * @param vaultPath 保险箱全路径
     * @return
     */
    bool statisticsFiles(const QString &vaultPath);

    /**
     * @brief removeFileInDir 递归删除指定文件夹下的文件
     * @param vaultPath 保险箱全路径
     */
    void removeFileInDir(const QString &vaultPath);

signals:
    void fileRemoved(int value);

    void removeFinished(bool result);

private slots:
    void onFileRemove(int value);

private:
    DWaterProgress *m_vaultRmProgressBar {nullptr};

    int m_iFiles = 0;   //文件、文件夹数量
    int m_iRmFiles = 0; //删除文件数量
    int m_iRmDir = 0;   //删除文件夹数量
};

#endif // DFMVAULTREMOVEPROGRESSVIEW_H
