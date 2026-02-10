// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "trashpropertydialog.h"
#include "utils/trashcorehelper.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/utils/fileutils.h>

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
    FileInfoPointer info = InfoFactory::create<FileInfo>(trashRootUrl);

    trashIconLabel = new DLabel(this);
    trashIconLabel->setFixedSize(160, 160);
    trashIconLabel->setAlignment(Qt::AlignCenter);

    DHorizontalLine *hLine = new DHorizontalLine(this);
    fileCountAndFileSize = new KeyValueLabel(this);
    updateLeftInfo(info->countChildFile());
    fileCountAndFileSize->setRightValue(FileUtils::formatSize(0), Qt::ElideNone, Qt::AlignRight);
    fileCountAndFileSize->leftWidget()->setFixedWidth(150);

    DFrame *infoFrame = new DFrame;
    infoFrame->setMaximumHeight(48);
    QHBoxLayout *infoLayout = new QHBoxLayout;
    infoLayout->setContentsMargins(10, 10, 10, 10);
    infoLayout->addWidget(fileCountAndFileSize);
    infoFrame->setLayout(infoLayout);

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
    auto data = TrashCoreHelper::calculateTrashRoot();
    updateUI(data.first, data.second);
}

void TrashPropertyDialog::updateUI(qint64 size, int count)
{
    updateLeftInfo(count);
    fileCountAndFileSize->setRightValue(FileUtils::formatSize(size), Qt::ElideNone, Qt::AlignRight);
}

void TrashPropertyDialog::showEvent(QShowEvent *event)
{
    calculateSize();
    DDialog::showEvent(event);
}
