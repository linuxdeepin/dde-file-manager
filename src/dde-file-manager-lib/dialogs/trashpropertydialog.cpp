/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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
#include "shutil/fileutils.h"
#include "dfileservices.h"
#include "dfilestatisticsjob.h"

#include <DHorizontalLine>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QWindow>

DWIDGET_USE_NAMESPACE
DFM_USE_NAMESPACE

TrashPropertyDialog::TrashPropertyDialog(const DUrl &url, QWidget *parent)
    : DDialog(parent)
    , m_url(url)
{
    initUI();
}

TrashPropertyDialog::~TrashPropertyDialog()
{

}

void TrashPropertyDialog::initUI()
{
    if (DFMGlobal::isWayLand()) {
        //设置对话框窗口最大最小化按钮隐藏
        this->setWindowFlags(this->windowFlags() & ~Qt::WindowMinMaxButtonsHint);
        this->setAttribute(Qt::WA_NativeWindow);
        //this->windowHandle()->setProperty("_d_dwayland_window-type", "wallpaper");
        this->windowHandle()->setProperty("_d_dwayland_minimizable", false);
        this->windowHandle()->setProperty("_d_dwayland_maximizable", false);
        this->windowHandle()->setProperty("_d_dwayland_resizable", false);
    }

    setFixedWidth(320);
    setTitle(tr("Trash"));

    const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(this, m_url);
    QIcon trashIcon;
    if (fileInfo->filesCount() > 0) {
        trashIcon = QIcon::fromTheme("user-trash-full");
    } else {
        trashIcon = QIcon::fromTheme("user-trash");
    }

    m_iconLabel = new QLabel(this);
    m_iconLabel->setFixedSize(160, 160);
    m_iconLabel->setPixmap(trashIcon.pixmap(m_iconLabel->size()));
    m_iconLabel->setAlignment(Qt::AlignCenter);

    const int fCount = fileInfo->filesCount();
    QString itemStr = tr("item");
    if (fCount != 1)
        itemStr = tr("items");

    DHorizontalLine *hLine = new DHorizontalLine(this);
    m_countLabel = new QLabel(tr("Contains %1 %2").arg(QString::number(fCount), itemStr), this);
    m_sizeLabel = new QLabel(this);

    QFrame *infoFrame = new QFrame;
    infoFrame->setFixedHeight(48);
    QHBoxLayout *infoLayout = new QHBoxLayout;
    infoLayout->addWidget(m_countLabel);
    infoLayout->addStretch();
    infoLayout->addWidget(m_sizeLabel);
    infoLayout->setContentsMargins(10, 0, 10, 0);
    infoFrame->setLayout(infoLayout);

    QString backColor = palette().color(QPalette::Base).name();
    infoFrame->setStyleSheet(QString("background-color: %1; border-radius: 8px;").arg(backColor));

    QFrame *contenFrame = new QFrame;

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_iconLabel, 0, Qt::AlignHCenter);
    mainLayout->addWidget(hLine);
    //mainLayout->addLayout(infoLayout);
    mainLayout->addWidget(infoFrame);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    contenFrame->setLayout(mainLayout);

    addContent(contenFrame);

    startComputerFolderSize(m_url);


}

void TrashPropertyDialog::startComputerFolderSize(const DUrl &url)
{
    DFileStatisticsJob *worker = new DFileStatisticsJob(this);

    connect(worker, &DFileStatisticsJob::finished, worker, &DFileStatisticsJob::deleteLater);
    connect(worker, &DFileStatisticsJob::dataNotify, this, &TrashPropertyDialog::updateFolderSize);
    worker->start({url});
}

void TrashPropertyDialog::updateFolderSize(qint64 size)
{
    m_sizeLabel->setText(FileUtils::formatSize(size));
}


