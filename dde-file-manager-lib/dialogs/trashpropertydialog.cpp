/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#include "trashpropertydialog.h"
#include "dseparatorhorizontal.h"
#include "shutil/fileutils.h"
#include "dfileservices.h"
#include "dfilestatisticsjob.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>

DWIDGET_USE_NAMESPACE
DFM_USE_NAMESPACE

TrashPropertyDialog::TrashPropertyDialog(const DUrl& url, QWidget *parent) : DDialog(parent)
{
    m_url = url;
    initUI();
}

TrashPropertyDialog::~TrashPropertyDialog()
{

}

void TrashPropertyDialog::initUI()
{
    setFixedSize(320, 300);
    setTitle("");

    const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(this, m_url);
    QIcon trashIcon;
    if(fileInfo->filesCount()>0){
        trashIcon.addFile(":/images/images/trash_full.png");
        trashIcon.addFile(":/images/images/trash_full@2x.png");
    }else{
        trashIcon.addFile(":/images/images/trash.png");
        trashIcon.addFile(":/images/images/trash@2x.png");
    }

    m_iconLabel = new QLabel(this);
    m_iconLabel->setFixedSize(160, 160);
    m_iconLabel->setPixmap(trashIcon.pixmap(m_iconLabel->size()));
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_nameLable = new QLabel(tr("Trash"), this);
    m_nameLable->setAlignment(Qt::AlignCenter);

    const int fCount = fileInfo->filesCount();
    QString itemStr = tr("item");
    if(fCount != 1)
        itemStr = tr("items");

    DSeparatorHorizontal* hLine = new DSeparatorHorizontal(this);
    m_countLabel = new QLabel(tr("Contains %1 %2").arg(QString::number(fCount),itemStr), this);
    m_sizeLabel = new QLabel(this);

    QHBoxLayout* infoLayout = new QHBoxLayout;
    infoLayout->addWidget(m_countLabel);
    infoLayout->addStretch();
    infoLayout->addWidget(m_sizeLabel);
    infoLayout->setContentsMargins(10, 0, 0, 0);

    QFrame* contenFrame = new QFrame;

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_iconLabel, 0, Qt::AlignHCenter);
    mainLayout->addWidget(m_nameLable, 0, Qt::AlignHCenter);
    mainLayout->addWidget(hLine);
    mainLayout->addLayout(infoLayout);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    contenFrame->setLayout(mainLayout);

    addContent(contenFrame);

    startComputerFolderSize(m_url);
}

void TrashPropertyDialog::startComputerFolderSize(const DUrl &url)
{
    DFileStatisticsJob* worker = new DFileStatisticsJob(this);

    connect(worker, &DFileStatisticsJob::finished, worker, &DFileStatisticsJob::deleteLater);
    connect(worker, &DFileStatisticsJob::dataNotify, this, &TrashPropertyDialog::updateFolderSize);

    worker->start({url});
}

void TrashPropertyDialog::updateFolderSize(qint64 size)
{
    m_sizeLabel->setText(FileUtils::formatSize(size));
}


