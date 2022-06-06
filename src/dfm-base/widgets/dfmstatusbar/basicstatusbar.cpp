/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#include "basicstatusbar.h"
#include "private/basicstatusbar_p.h"
#include "utils/fileutils.h"

#include <DAnchors>

#include <QHBoxLayout>
#include <QLabel>

DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

BasicStatusBar::BasicStatusBar(QWidget *parent)
    : QFrame(parent),
      d(new BasicStatusBarPrivate(this))
{
    initUI();
}

QSize BasicStatusBar::sizeHint() const
{
    QSize size = QFrame::sizeHint();

    size.setHeight(qMax(25, size.height()));

    return size;
}

void BasicStatusBar::clearLayoutAndAnchors()
{
    while (d->layout->count() > 0)
        delete d->layout->takeAt(0);

    DAnchorsBase::clearAnchors(this);
}

void BasicStatusBar::itemSelected(const QList<AbstractFileInfo *> &infoList)
{
    if (!d->tip)
        return;

    d->fileCount = 0;
    d->fileSize = 0;
    d->folderCount = 0;
    d->folderContains = 0;

    QList<QUrl> dirUrlList;
    for (const AbstractFileInfo *info : infoList) {
        if (info->isDir()) {
            d->folderCount += 1;
            dirUrlList << info->url();
        } else {
            d->fileCount += 1;
            d->fileSize += info->size();
        }
    }

    if (!dirUrlList.isEmpty())
        d->calcFolderContains(dirUrlList);

    updateStatusMessage();
}

void BasicStatusBar::itemCounted(const int count)
{
    if (d->fileStatisticsJog && d->fileStatisticsJog->isRunning()) {
        d->fileStatisticsJog->stop();
        d->fileStatisticsJog->wait();
        if (d->fileStatisticsJog->disconnect())
            d->isJobDisconnect = true;
    }

    d->tip->setText(d->counted.arg(QString::number(count)));
}

void BasicStatusBar::updateStatusMessage()
{
    QString selectedFolders;

    if (d->folderCount == 1 && d->folderContains == 1) {
        selectedFolders = d->selectOnlyOneFolder.arg(QString::number(d->folderCount), d->onlyOneItemCounted.arg(d->folderContains));
    } else if (d->folderCount == 1 && d->folderContains != 1) {
        selectedFolders = d->selectOnlyOneFolder.arg(QString::number(d->folderCount), d->counted.arg(d->folderContains));
    } else if (d->folderCount > 1 && d->folderContains == 1) {
        selectedFolders = d->selectFolders.arg(QString::number(d->folderCount), d->onlyOneItemCounted.arg(d->folderContains));
    } else if (d->folderCount > 1 && d->folderContains != 1) {
        selectedFolders = d->selectFolders.arg(QString::number(d->folderCount), d->counted.arg(d->folderContains));
    } else {
        selectedFolders = "";
    }

    QString selectedFiles;

    if (d->fileCount == 1) {
        selectedFiles = d->selectOnlyOneFile.arg(QString::number(d->fileCount), FileUtils::formatSize(d->fileSize));
    } else if (d->fileCount > 1) {
        selectedFiles = d->selectFiles.arg(QString::number(d->fileCount), FileUtils::formatSize(d->fileSize));
    } else {
        selectedFiles = "";
    }

    if (selectedFolders.isEmpty()) {
        d->tip->setText(QString("%1").arg(selectedFiles));
    } else if (selectedFiles.isEmpty()) {
        d->tip->setText(QString("%1").arg(selectedFolders));
    } else {
        d->tip->setText(QString("%1,%2").arg(selectedFolders, selectedFiles));
    }
}

void BasicStatusBar::insertWidget(const int index, QWidget *widget, int stretch, Qt::Alignment alignment)
{
    if (d->layout)
        d->layout->insertWidget(index, widget, stretch, alignment);
}

void BasicStatusBar::addWidget(QWidget *widget, int stretch, Qt::Alignment alignment)
{
    if (d->layout)
        d->layout->addWidget(widget, stretch, alignment);
}

void BasicStatusBar::setTipText(const QString &tip)
{
    if (d->tip)
        d->tip->setText(tip);
}

void BasicStatusBar::initUI()
{
    setBackgroundRole(QPalette::Window);
    setFocusPolicy(Qt::NoFocus);

    d->initTipLabel();
    d->initLayout();
}
