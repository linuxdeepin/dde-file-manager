/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liyigang<liyigang@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "taskdialog.h"

#include <QListWidgetItem>
#include <QListWidget>
#include <QLayout>

DSC_USE_NAMESPACE

static const int kDefaultWidth = 700;

TaskDialog::TaskDialog(QObject *parent)
    : DAbstractDialog(parent)
{
    initUI();
}
void TaskDialog::addTask(const JobHandlePointer &taskHandler)
{
    Q_UNUSED(taskHandler);
}
/*!
 * \brief TaskDialog::initUI 初始化界面UI
 */
void TaskDialog::initUI()
{
    QFont f = font();
    f.setPixelSize(14);
    setFont(f);

    setWindowFlags((windowFlags() & ~Qt::WindowSystemMenuHint & ~Qt::Dialog) | Qt::Window
                   | Qt::WindowMinMaxButtonsHint);
    setFixedWidth(kDefaultWidth);
    titlebar = new DTitlebar(this);
    titlebar->layout()->setContentsMargins(0, 0, 0, 0);
    titlebar->setMenuVisible(false);
    titlebar->setIcon(QIcon::fromTheme("dde-file-manager"));
    titlebar->setStyleSheet("background-color:rgba(0, 0, 0, 0)");

    taskListWidget = new QListWidget(this);
    taskListWidget->setSelectionMode(QListWidget::NoSelection);
    taskListWidget->viewport()->setAutoFillBackground(false);
    taskListWidget->setFrameShape(QFrame::NoFrame);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(titlebar);
    mainLayout->addWidget(taskListWidget);
    mainLayout->addStretch(1);
    setLayout(mainLayout);

    moveToCenter();
}

TaskDialog::~TaskDialog() {}
