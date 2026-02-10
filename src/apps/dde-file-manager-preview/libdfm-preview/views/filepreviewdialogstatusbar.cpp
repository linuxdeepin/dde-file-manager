// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filepreviewdialogstatusbar.h"
#include <QHBoxLayout>

using namespace dfmplugin_filepreview;

Q_DECLARE_LOGGING_CATEGORY(logLibFilePreview)

FilePreviewDialogStatusBar::FilePreviewDialogStatusBar(QWidget *parent)
    : QFrame(parent)
{
    qCDebug(logLibFilePreview) << "FilePreviewDialogStatusBar: initializing status bar";
    
    QSize iconSize(16, 16);
    preBtn = new QPushButton(this);
    preBtn->setObjectName("PreButton");
    preBtn->setIcon(QIcon::fromTheme("go-previous").pixmap(iconSize));
    preBtn->setIconSize(iconSize);
    preBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    preBtn->setShortcut(QKeySequence::Back);
    preBtn->setFocusPolicy(Qt::NoFocus);

    nextBtn = new QPushButton(this);
    nextBtn->setObjectName("NextButton");
    nextBtn->setIcon(QIcon::fromTheme("go-next").pixmap(iconSize));
    nextBtn->setIconSize(iconSize);
    nextBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    nextBtn->setShortcut(QKeySequence::Forward);
    nextBtn->setFocusPolicy(Qt::NoFocus);

    previewTitle = new QLabel(this);
    previewTitle->setObjectName("TitleLabel");
    previewTitle->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    previewTitle->hide();

    openBtn = new QPushButton(QObject::tr("Open", "button"), this);
    openBtn->setObjectName("OpenButton");
    QFont font = openBtn->font();
    font.setPixelSize(12);
    openBtn->setFont(font);
    openBtn->setMinimumWidth(60);
    openBtn->setShortcut(QKeySequence::Open);

    QHBoxLayout *layout = new QHBoxLayout(this);

    layout->setContentsMargins(10, 10, 10, 10);
    layout->addWidget(preBtn);
    layout->addWidget(nextBtn);
    layout->addWidget(previewTitle);
    layout->addWidget(openBtn, 0, Qt::AlignRight);

    setLayout(layout);
    
    qCDebug(logLibFilePreview) << "FilePreviewDialogStatusBar: status bar initialization completed";
}

QLabel *FilePreviewDialogStatusBar::title() const
{
    return previewTitle;
}

QPushButton *FilePreviewDialogStatusBar::preButton() const
{
    return preBtn;
}

QPushButton *FilePreviewDialogStatusBar::nextButton() const
{
    return nextBtn;
}

QPushButton *FilePreviewDialogStatusBar::openButton() const
{
    return openBtn;
}
