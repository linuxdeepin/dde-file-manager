/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
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
#include "statusbar.h"
#include "private/statusbar_p.h"
#include "fileviewitem.h"
#include "dfm-base/utils/fileutils.h"

#include <DAnchors>

#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>

DFMBASE_USE_NAMESPACE
DPWORKSPACE_USE_NAMESPACE

StatusBar::StatusBar(QWidget *parent)
    : QFrame(parent),
      d(new StatusBarPrivate(this))
{
    initUI();
}

QSize StatusBar::sizeHint() const
{
    QSize size = QFrame::sizeHint();

    size.setHeight(qMax(25, size.height()));

    return size;
}

void StatusBar::resetScalingSlider(const int stepCount)
{
    if (scaleSlider)
        scaleSlider->setMaximum(stepCount);
}

void StatusBar::setScalingVisible(const bool visible)
{
    if (!scaleSlider)
        return;

    if (visible)
        scaleSlider->show();
    else
        scaleSlider->hide();
}

int StatusBar::scalingValue()
{
    if (!scaleSlider)
        return 1;

    return scaleSlider->value();
}

QSlider *StatusBar::scalingSlider() const
{
    return scaleSlider;
}

void StatusBar::itemSelected(const QList<const FileViewItem *> &itemList)
{
    if (!label)
        return;

    d->fileCount = 0;
    d->fileSize = 0;
    d->folderCount = 0;
    d->folderContains = 0;

    for (const FileViewItem *item : itemList) {
        if (item->fileinfo()->isDir()) {
            d->folderCount += 1;
            // TODO(liuyangming): caculate folder contains items count
        } else {
            d->fileCount += 1;
            d->fileSize += item->fileinfo()->size();
        }
    }

    updateStatusMessage();
}

void StatusBar::itemCounted(const int count)
{
    label->setText(d->counted.arg(QString::number(count)));
}

void StatusBar::updateStatusMessage()
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
        label->setText(QString("%1").arg(selectedFiles));
    } else if (selectedFiles.isEmpty()) {
        label->setText(QString("%1").arg(selectedFolders));
    } else {
        label->setText(QString("%1,%2").arg(selectedFolders, selectedFiles));
    }
}

void StatusBar::initUI()
{
    setBackgroundRole(QPalette::Window);
    setFocusPolicy(Qt::NoFocus);

    label = new QLabel(d->counted.arg("0"), this);
    label->setAlignment(Qt::AlignCenter);
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    initLoadingIndicator();
    initScalingSlider();
    initLayout();
}

void StatusBar::initScalingSlider()
{
    scaleSlider = new QSlider(this);
    scaleSlider->setOrientation(Qt::Horizontal);
    scaleSlider->adjustSize();
    scaleSlider->setFixedWidth(120);
    scaleSlider->setMaximum(1);
    scaleSlider->setMinimum(0);
    scaleSlider->setTickInterval(1);
    scaleSlider->setPageStep(1);
    scaleSlider->hide();
}

void StatusBar::initLoadingIndicator()
{
    QStringList seq;

    for (int i = 1; i != 91; ++i)
        seq.append(QString(":/images/images/Spinner/Spinner%1.png").arg(i, 2, 10, QChar('0')));

    loadingIndicator = new DPictureSequenceView(this);
    loadingIndicator->setFixedSize(18, 18);
    loadingIndicator->setPictureSequence(seq, true);
    loadingIndicator->setSpeed(20);
    loadingIndicator->hide();
}

void StatusBar::initLayout()
{
    layout = new QHBoxLayout(this);
    setLayout(layout);

    clearLayoutAndAnchors();
    layout->addWidget(loadingIndicator);
    layout->addWidget(label);
    layout->addWidget(scaleSlider, 0, Qt::AlignRight);
    layout->setSpacing(14);
    layout->setContentsMargins(0, 0, 4, 0);
}

void StatusBar::clearLayoutAndAnchors()
{
    while (layout->count() > 0)
        delete layout->takeAt(0);

    DAnchorsBase::clearAnchors(this);
    DAnchorsBase::clearAnchors(scaleSlider);
}
