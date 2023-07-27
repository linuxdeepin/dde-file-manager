// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultremoveprogressview.h"
#include "utils/vaultdefine.h"
#include "utils/vaultautolock.h"
#include <dfm-base/base/application/settings.h>

#include <DWaterProgress>
#include <DLabel>

#include <QProgressBar>
#include <QFile>
#include <QDir>
#include <QThread>

#include <thread>

DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace dfmplugin_vault;

VaultRemoveProgressView::VaultRemoveProgressView(QWidget *parent)
    : QWidget(parent)
    , vaultRmProgressBar(new DWaterProgress(this))
    , deleteFinishedImageLabel(new DLabel(this))
    , layout(new QHBoxLayout())
{
    vaultRmProgressBar->setFixedSize(80, 80);

    deleteFinishedImageLabel->setPixmap(QIcon::fromTheme("dfm_vault_active_finish").pixmap(90, 90));
    deleteFinishedImageLabel->setAlignment(Qt::AlignHCenter);
    deleteFinishedImageLabel->hide();

    layout->setMargin(0);
    layout->addWidget(vaultRmProgressBar, 1, Qt::AlignCenter);
    this->setLayout(layout);

    connect(this, &VaultRemoveProgressView::fileRemoved, this, &VaultRemoveProgressView::onFileRemove);
}

VaultRemoveProgressView::~VaultRemoveProgressView()
{
    vaultRmProgressBar->setValue(0);
    vaultRmProgressBar->stop();
    filesCount = 0;
    removeFileCount = 0;
    removeDirCount = 0;
}

void VaultRemoveProgressView::removeVault(const QString &vaultLockPath, const QString &vaultUnlockPath)
{
    if (vaultLockPath.isEmpty() || vaultUnlockPath.isEmpty())
        return;
    vaultRmProgressBar->start();
    //! 开启线程进行文件删除
    std::thread thread(
            [=]() {
                try {
                    if (statisticsFiles(vaultLockPath)) {
                        removeFileInDir(vaultLockPath);
                        QDir dir;
                        dir.rmdir(vaultUnlockPath);
                        QFile::remove(kVaultBasePath + QDir::separator() + kRSAPUBKeyFileName);
                        QFile::remove(kVaultBasePath + QDir::separator() + kRSACiphertextFileName);
                        QFile::remove(kVaultBasePath + QDir::separator() + kPasswordHintFileName);
                        QFile::remove(kVaultBasePath + QDir::separator() + kVaultConfigFileName);
                        QFile::remove(kVaultBasePath + QDir::separator() + kPasswordFileName);
                        QFile::remove(kVaultBasePath + QDir::separator() + kRSAPUBKeyFileName + QString(".key"));

                        emit removeFinished(true);
                        //! 清除保险箱所有时间
                        Settings setting(kVaultTimeConfigFile);
                        setting.removeGroup(QString("VaultTime"));
                        VaultAutoLock::instance()->resetConfig();
                    } else {
                        emit removeFinished(false);
                    }
                } catch (...) {
                    emit removeFinished(false);
                }
            });

    thread.detach();
}

void VaultRemoveProgressView::clear()
{
    vaultRmProgressBar->setValue(0);
    vaultRmProgressBar->stop();
    filesCount = 0;
    removeFileCount = 0;
    removeDirCount = 0;
}

bool VaultRemoveProgressView::statisticsFiles(const QString &vaultPath)
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
            filesCount++;
            //! 递归
            statisticsFiles(fileInfo.filePath());
        } else {
            filesCount++;
        }

        i++;
    }

    return true;
}

void VaultRemoveProgressView::removeFileInDir(const QString &vaultPath)
{
    QDir dir(vaultPath);
    QFileInfoList infoList = dir.entryInfoList(QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::AllDirs);

    if (dir.exists()) {
        dir.setFilter(QDir::Files | QDir::NoSymLinks);
        QFileInfoList list = dir.entryInfoList();
    }

    //! 遍历文件信息列表，进行文件删除
    foreach (const QFileInfo &fileInfo, infoList) {
        if (fileInfo.isDir()) {
            //! 递归
            removeFileInDir(fileInfo.absoluteFilePath());
        } else if (fileInfo.isFile()) {
            QFile file(fileInfo.absoluteFilePath());

            //! 删除文件
            file.remove();
            removeFileCount++;
            if (filesCount > 0) {
                int value = 100 * (removeFileCount + removeDirCount - 1) / filesCount;
                emit fileRemoved(value);
            }
        }
    }

    QDir temp_dir;
    //! 删除文件夹
    temp_dir.rmdir(vaultPath);
    removeDirCount++;
    if (filesCount > 0) {
        int value = 100 * (removeFileCount + removeDirCount - 1) / filesCount;
        emit fileRemoved(value);
    }
}

void VaultRemoveProgressView::onFileRemove(int value)
{
    if (vaultRmProgressBar->value() != 100)
        vaultRmProgressBar->setValue(value);
    if (value == 100) { // 100: the vaule of progress bar
        layout->removeWidget(vaultRmProgressBar);
        vaultRmProgressBar->hide();
        layout->addWidget(deleteFinishedImageLabel);
        deleteFinishedImageLabel->show();
    }
}
