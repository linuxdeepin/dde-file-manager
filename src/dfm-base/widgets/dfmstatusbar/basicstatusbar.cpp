// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "basicstatusbar.h"
#include "private/basicstatusbar_p.h"
#include "utils/fileutils.h"

#include <dfm-base/base/application/application.h>

#include <DAnchors>

#include <QHBoxLayout>
#include <QLabel>

DWIDGET_USE_NAMESPACE
using namespace dfmbase;

BasicStatusBar::BasicStatusBar(QWidget *parent)
    : QFrame(parent),
      d(new BasicStatusBarPrivate(this))
{
    initUI();
}

QSize BasicStatusBar::sizeHint() const
{
    QSize size = QFrame::sizeHint();

    size.setHeight(qMax(32, size.height()));

    return size;
}

void BasicStatusBar::clearLayoutAndAnchors()
{
    while (d->layout->count() > 0)
        delete d->layout->takeAt(0);

    DAnchorsBase::clearAnchors(this);
}

void BasicStatusBar::itemSelected(const int selectFiles, const int selectFolders, const qint64 filesize, const QList<QUrl> &selectFolderList)
{
    if (!d->tip)
        return;

    d->fileCount = selectFiles;
    d->fileSize = filesize;
    d->folderCount = selectFolders;
    d->folderContains = 0;
    d->showContains = true;

    const bool dirUrlsEmpty = selectFolderList.isEmpty();
    if (!dirUrlsEmpty) {
        d->calcFolderContains(selectFolderList);
    }

    updateStatusMessage();
}

void BasicStatusBar::itemSelected(const QList<FileInfo *> &infoList)
{
    if (!d->tip)
        return;

    d->fileCount = 0;
    d->fileSize = 0;
    d->folderCount = 0;
    d->folderContains = 0;
    d->showContains = true;

    QList<QUrl> selectFolderList;
    for (const FileInfo *info : infoList) {
        if (info->isAttributes(OptInfoType::kIsDir)) {
            d->folderCount += 1;
            selectFolderList << info->urlOf(UrlInfoType::kUrl);
        } else {
            d->fileCount += 1;
            d->fileSize += info->size();
        }
    }

    const bool dirUrlsEmpty = selectFolderList.isEmpty();
    if (!dirUrlsEmpty) {
        d->calcFolderContains(selectFolderList);
    }

    updateStatusMessage();
}

void BasicStatusBar::itemCounted(const int count)
{
    d->discardCurrentJob();

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
    if (!selectedFolders.isEmpty() && !d->showContains)
        selectedFolders = d->selectedNetworkOnlyOneFolder.arg(QString::number(d->folderCount));

    QString selectedFiles;

    if (d->fileCount == 1) {
        selectedFiles = d->selectOnlyOneFile.arg(QString::number(d->fileCount), FileUtils::formatSize(d->fileSize));
    } else if (d->fileCount > 1) {
        selectedFiles = d->selectFiles.arg(QString::number(d->fileCount), FileUtils::formatSize(d->fileSize));
    } else {
        selectedFiles = "";
    }
    if (!selectedFiles.isEmpty() && !d->showContains)
        selectedFiles = d->fileCount > 1 ? (d->selected.arg(d->fileCount)) : (d->onlyOneItemSelected.arg(d->fileCount));

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
