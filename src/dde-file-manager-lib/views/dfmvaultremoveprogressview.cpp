/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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

#include "dfmvaultremoveprogressview.h"
#include "dfmsettings.h"
#include "accessibility/ac-lib-file-manager.h"
#include "vault/vaultglobaldefine.h"

#include <DWaterProgress>

#include <QProgressBar>
#include <QFile>
#include <QDir>
#include <QThread>
#include <QHBoxLayout>

#include <thread>

DFMVaultRemoveProgressView::DFMVaultRemoveProgressView(QWidget *parent)
    : QWidget(parent)
    , m_vaultRmProgressBar(new DWaterProgress(this))
{
    m_vaultRmProgressBar->setFixedSize(80, 80);
    AC_SET_ACCESSIBLE_NAME(m_vaultRmProgressBar, AC_VAULT_DELETE_PROGRESS);
    QHBoxLayout *layout = new QHBoxLayout();
    layout->setMargin(0);
    layout->addWidget(m_vaultRmProgressBar, 1, Qt::AlignCenter);

    this->setLayout(layout);
    connect(this, &DFMVaultRemoveProgressView::fileRemoved, this, &DFMVaultRemoveProgressView::onFileRemove);
}

DFMVaultRemoveProgressView::~DFMVaultRemoveProgressView()
{
    m_vaultRmProgressBar->setValue(0);
    m_vaultRmProgressBar->stop();
    m_iFiles = 0;
    m_iRmFiles = 0;
    m_iRmDir = 0;
}

void DFMVaultRemoveProgressView::removeVault(const QString &vaultLockPath, const QString &vaultUnlockPath)
{
    if (vaultLockPath.isEmpty() || vaultUnlockPath.isEmpty())
        return;
    m_vaultRmProgressBar->start();
    // 开启线程进行文件删除
    std::thread thread(
    [ = ]() {
        try {
            if (statisticsFiles(vaultLockPath)) {
                removeFileInDir(vaultLockPath);
                QDir dir;
                dir.rmdir(vaultUnlockPath);
                QFile::remove(VAULT_BASE_PATH + QDir::separator() + RSA_PUB_KEY_FILE_NAME);
                QFile::remove(VAULT_BASE_PATH + QDir::separator() + RSA_CIPHERTEXT_FILE_NAME);
                QFile::remove(VAULT_BASE_PATH + QDir::separator() + PASSWORD_HINT_FILE_NAME);
                QFile::remove(VAULT_BASE_PATH + QDir::separator() + VAULT_CONFIG_FILE_NAME);
                QFile::remove(VAULT_BASE_PATH + QDir::separator() + PASSWORD_FILE_NAME);
                QFile::remove(VAULT_BASE_PATH + QDir::separator() + RSA_PUB_KEY_FILE_NAME + QString(".key"));

                emit removeFinished(true);
                //! 清除保险箱所有时间
                DFM_NAMESPACE::DFMSettings setting(VAULT_TIME_CONFIG_FILE);
                setting.removeGroup(QString("VaultTime"));
            } else {
                emit removeFinished(false);
            }
        } catch (...) {
            emit removeFinished(false);
        }
    });

    thread.detach();
}

void DFMVaultRemoveProgressView::clear()
{
    m_vaultRmProgressBar->setValue(0);
    m_vaultRmProgressBar->stop();
    m_iFiles = 0;
    m_iRmFiles = 0;
    m_iRmDir = 0;
}

bool DFMVaultRemoveProgressView::statisticsFiles(const QString &vaultPath)
{
    QDir dir(vaultPath);
    if (!dir.exists())
        return false;

    dir.setFilter(QDir::Dirs | QDir::Files);
    dir.setSorting(QDir::DirsFirst);
    QFileInfoList list = dir.entryInfoList(QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::AllDirs);
    int i = 0;
    while (i < list.size()) {
        QFileInfo fileInfo = list.at(i);

        bool bisDir = fileInfo.isDir();
        if (bisDir) {
            m_iFiles++;
            //递归
            statisticsFiles(fileInfo.filePath());
        } else {
            m_iFiles++;
        }

        i++;
    }

    return true;
}

void DFMVaultRemoveProgressView::removeFileInDir(const QString &vaultPath)
{
    QDir dir(vaultPath);
    QFileInfoList infoList = dir.entryInfoList(QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::AllDirs);

    if (dir.exists()) {
        dir.setFilter(QDir::Files | QDir::NoSymLinks);
        QFileInfoList list = dir.entryInfoList();
    }

    //遍历文件信息列表，进行文件删除
    foreach (const QFileInfo &fileInfo, infoList) {
        if (fileInfo.isDir()) {
            //递归
            removeFileInDir(fileInfo.absoluteFilePath());
        } else if (fileInfo.isFile()) {
            QFile file(fileInfo.absoluteFilePath());

            //删除文件
            file.remove();
            m_iRmFiles++;
            if (m_iFiles > 0) {
                int value = 100 * (m_iRmFiles + m_iRmDir - 1) / m_iFiles;
                emit fileRemoved(value);
            }
        }
    }

    QDir temp_dir;
    //删除文件夹
    temp_dir.rmdir(vaultPath);
    m_iRmDir++;
    if (m_iFiles > 0) {
        int value = 100 * (m_iRmFiles + m_iRmDir - 1) / m_iFiles;
        emit fileRemoved(value);
    }
}

void DFMVaultRemoveProgressView::onFileRemove(int value)
{
    if (m_vaultRmProgressBar->value() != 100)
        m_vaultRmProgressBar->setValue(value);
}
