/*
* Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
*
* Author: Liu Zhangjian<liuzhangjian@uniontech.com>
*
* Maintainer: Liu Zhangjian<liuzhangjian@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#include "dfmvaultremoveprogressview.h"

#include <thread>

#include <QProgressBar>
#include <QFile>
#include <QDir>
#include <QAbstractButton>
#include <QThread>

DFMVaultRemoveProgressView::DFMVaultRemoveProgressView(QWidget *parent)
    : DDialog (parent)
    , m_vaultRmProgressBar(new QProgressBar(this))
{
    this->setTitle(tr("Remove File Vault"));
    this->setMessage(tr("Removing..."));
    this->setIcon(QIcon::fromTheme("dfm_safebox"));
    this->setFixedSize(438, 280);
    this->setCloseButtonVisible(false);

    m_vaultRmProgressBar->setAlignment(Qt::AlignHCenter);
    m_vaultRmProgressBar->setMinimum(0);
    m_vaultRmProgressBar->setMaximum(100);
    this->addContent(m_vaultRmProgressBar);

    this->addButton(tr("Ok"), true, ButtonType::ButtonRecommend);
    this->getButton(0)->setEnabled(false);
    this->getButton(0)->setFixedWidth(this->width()/2);
    this->setContentsMargins(10, 0, 10, 0);

    connect(this, &DFMVaultRemoveProgressView::fileRemoved, this, &DFMVaultRemoveProgressView::onFileRemove);
    connect(this, &DFMVaultRemoveProgressView::removeFinished, this, &DFMVaultRemoveProgressView::onRemoveFinish);
}

DFMVaultRemoveProgressView::~DFMVaultRemoveProgressView()
{
    m_vaultRmProgressBar->setValue(0);
    m_iFiles = 0;
    m_iRmFiles = 0;
    m_iRmDir = 0;
}

void DFMVaultRemoveProgressView::removeVault(const QString &vaultPath)
{
    // 开启线程进行文件删除
    std::thread thread(
                [=]()
    {
        try {
            if (statisticsFiles(vaultPath)){
                removeFileInDir(vaultPath);
                emit removeFinished(true);
            }else {
                emit removeFinished(false);
            }
        } catch (...) {
            emit removeFinished(false);
        }
    });

    thread.detach();
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
    do{
        QFileInfo fileInfo = list.at(i);
        if(fileInfo.fileName() == "." || fileInfo.fileName() == ".."){
            i++;
            continue;
        }

        bool bisDir = fileInfo.isDir();
        if(bisDir){
            m_iFiles++;
            //递归
            statisticsFiles(fileInfo.filePath());
        }else{
            m_iFiles++;
        }

        i++;
    }while(i < list.size());

    return true;
}

void DFMVaultRemoveProgressView::removeFileInDir(const QString &vaultPath)
{
    QDir dir(vaultPath);
    QFileInfoList infoList = dir.entryInfoList(QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::AllDirs);

    if(dir.exists()){
        dir.setFilter(QDir::Files | QDir::NoSymLinks);
        QFileInfoList list = dir.entryInfoList();
    }

    //遍历文件信息列表，进行文件删除
    foreach(QFileInfo fileInfo, infoList){
        if (fileInfo.isDir()){
            //递归
            removeFileInDir(fileInfo.absoluteFilePath());
        }else if (fileInfo.isFile()){
            QFile file(fileInfo.absoluteFilePath());

            //删除文件
            file.remove();
            m_iRmFiles++;
            int value = 100 * (m_iRmFiles + m_iRmDir - 1) / m_iFiles;
            emit fileRemoved(value);
        }
    }

    QDir temp_dir;
    //删除文件夹
    temp_dir.rmdir(vaultPath);
    m_iRmDir++;
    int value = 100 * (m_iRmFiles + m_iRmDir - 1) / m_iFiles;
    emit fileRemoved(value);
}

void DFMVaultRemoveProgressView::onFileRemove(int value)
{
    m_vaultRmProgressBar->setValue(value);
}

void DFMVaultRemoveProgressView::onRemoveFinish(bool result)
{
    if (result){
        this->setMessage(tr("Removed successfully"));
    }else {
        this->setMessage(tr("Failed to remove"));
    }
    this->getButton(0)->setEnabled(true);
}
