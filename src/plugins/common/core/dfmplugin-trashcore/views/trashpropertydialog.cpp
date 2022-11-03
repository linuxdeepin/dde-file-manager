/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/standardpaths.h"
#include "dfm-base/utils/fileutils.h"
#include "dfm-base/utils/decorator/decoratorfileenumerator.h"

#include <DHorizontalLine>

DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace dfmplugin_trashcore;
TrashPropertyDialog::TrashPropertyDialog(QWidget *parent)
    : DDialog(parent)
{
    initUI();
}

TrashPropertyDialog::~TrashPropertyDialog()
{
}

void TrashPropertyDialog::initUI()
{
    setFixedWidth(320);
    setTitle(tr("Trash"));

    const QUrl &trashRootUrl = FileUtils::trashRootUrl();
    AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(trashRootUrl);

    trashIconLabel = new DLabel(this);
    trashIconLabel->setFixedSize(160, 160);
    trashIconLabel->setAlignment(Qt::AlignCenter);

    DHorizontalLine *hLine = new DHorizontalLine(this);
    fileCountAndFileSize = new KeyValueLabel(this);
    updateLeftInfo(info->countChildFile());
    fileCountAndFileSize->setRightValue(FileUtils::formatSize(0), Qt::ElideNone, Qt::AlignHCenter);

    QFrame *infoFrame = new QFrame;
    infoFrame->setMaximumHeight(48);
    QHBoxLayout *infoLayout = new QHBoxLayout;
    infoLayout->setContentsMargins(10, 10, 10, 10);
    infoLayout->addWidget(fileCountAndFileSize);
    infoFrame->setLayout(infoLayout);

    QString backColor = palette().color(QPalette::Base).name();
    infoFrame->setStyleSheet(QString("background-color: %1; border-radius: 8px;").arg(backColor));

    QFrame *contenFrame = new QFrame;

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(trashIconLabel, 0, Qt::AlignHCenter);
    mainLayout->addWidget(hLine);
    mainLayout->addWidget(infoFrame);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    contenFrame->setLayout(mainLayout);

    addContent(contenFrame);
}

void TrashPropertyDialog::updateLeftInfo(const int &count)
{
    QIcon trashIcon;
    if (count > 0) {
        trashIcon = QIcon::fromTheme("user-trash-full");
    } else {
        trashIcon = QIcon::fromTheme("user-trash");
    }
    if (trashIconLabel)
        trashIconLabel->setPixmap(trashIcon.pixmap(trashIconLabel->size()));

    QString itemStr = tr("item");
    if (count > 1)
        itemStr = tr("items");
    if (fileCountAndFileSize)
        fileCountAndFileSize->setLeftValue(QString(tr("Contains %1 %2")).arg(QString::number(count), itemStr), Qt::ElideMiddle, Qt::AlignLeft, true);
}

void TrashPropertyDialog::calculateSize()
{
    qint64 size = 0;
    int count = 0;
    DecoratorFileEnumerator enumerator(FileUtils::trashRootUrl());
    if (!enumerator.isValid())
        return;
    while (enumerator.hasNext()) {
        const QUrl &urlNext = enumerator.next();
        ++count;
        AbstractFileInfoPointer fileInfo = InfoFactory::create<AbstractFileInfo>(urlNext);
        if (!fileInfo)
            continue;
        size += fileInfo->size();
    }

    updateUI(size, count);
}

void TrashPropertyDialog::updateUI(qint64 size, int count)
{
    updateLeftInfo(count);
    fileCountAndFileSize->setRightValue(FileUtils::formatSize(size), Qt::ElideNone, Qt::AlignHCenter);
}

void TrashPropertyDialog::showEvent(QShowEvent *event)
{
    calculateSize();
    DDialog::showEvent(event);
}
