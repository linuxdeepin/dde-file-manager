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

#include <DHorizontalLine>

DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPTRASHCORE_USE_NAMESPACE
TrashPropertyDialog::TrashPropertyDialog(QWidget *parent)
    : DDialog(parent),
      fileCalculationUtils(new FileStatisticsJob())
{
    initUI();
}

TrashPropertyDialog::~TrashPropertyDialog()
{
    fileCalculationUtils->stop();
    fileCalculationUtils->deleteLater();
}

void TrashPropertyDialog::initUI()
{
    setFixedWidth(320);
    setTitle(tr("Trash"));

    QString path = StandardPaths::location(StandardPaths::kTrashFilesPath);
    QUrl url(QUrl::fromLocalFile(path));

    AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(url);

    QIcon trashIcon;
    if (info->countChildFile() > 0) {
        trashIcon = QIcon::fromTheme("user-trash-full");
    } else {
        trashIcon = QIcon::fromTheme("user-trash");
    }

    trashIconLabel = new DLabel(this);
    trashIconLabel->setFixedSize(160, 160);
    trashIconLabel->setPixmap(trashIcon.pixmap(trashIconLabel->size()));
    trashIconLabel->setAlignment(Qt::AlignCenter);

    const int fCount = info->countChildFile();
    QString itemStr = tr("item");
    if (fCount != 1)
        itemStr = tr("items");

    DHorizontalLine *hLine = new DHorizontalLine(this);
    fileCountAndFileSize = new KeyValueLabel(this);
    fileCountAndFileSize->setLeftValue(QString(tr("Contains %1 %2")).arg(QString::number(fCount), itemStr));
    fileCountAndFileSize->setRightValue(FileUtils::formatSize(0), Qt::ElideNone, Qt::AlignRight);

    QFrame *infoFrame = new QFrame;
    infoFrame->setFixedHeight(48);
    QHBoxLayout *infoLayout = new QHBoxLayout;
    infoLayout->setContentsMargins(10, 10, 10, 0);
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

    connect(fileCalculationUtils, &FileStatisticsJob::sizeChanged, this, &TrashPropertyDialog::slotTrashDirSizeChange);
    fileCalculationUtils->start(QList<QUrl>() << url);
}

void TrashPropertyDialog::slotTrashDirSizeChange(qint64 size)
{
    fileCountAndFileSize->setRightValue(FileUtils::formatSize(size), Qt::ElideNone, Qt::AlignRight);
}

void TrashPropertyDialog::showEvent(QShowEvent *event)
{
    QString path = StandardPaths::location(StandardPaths::kTrashFilesPath);
    QUrl url(QUrl::fromLocalFile(path));
    fileCalculationUtils->start(QList<QUrl>() << url);
    DDialog::showEvent(event);
}
