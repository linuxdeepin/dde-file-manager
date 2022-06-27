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
#include "filepreviewdialogstatusbar.h"
#include <QHBoxLayout>

using namespace dfmplugin_filepreview;

FilePreviewDialogStatusBar::FilePreviewDialogStatusBar(QWidget *parent)
    : QFrame(parent)
{
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
