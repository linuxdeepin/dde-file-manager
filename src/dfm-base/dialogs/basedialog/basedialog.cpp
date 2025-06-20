// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "basedialog.h"

#include "utils/windowutils.h"

#include <dtitlebar.h>

#include <QApplication>
#include <QWindow>
#include <QShowEvent>
#include <QDebug>

DWIDGET_USE_NAMESPACE
using namespace dfmbase;

BaseDialog::BaseDialog(QWidget *parent)
    : DAbstractDialog(parent),
      titlebar(new DTitlebar(this))
{
    titlebar->setBackgroundTransparent(true);

    if (WindowUtils::isWayLand()) {
        //设置对话框窗口最大最小化按钮隐藏
        this->setWindowFlags(this->windowFlags() & ~Qt::WindowMinMaxButtonsHint);
        this->setAttribute(Qt::WA_NativeWindow);
        //this->windowHandle()->setProperty("_d_dwayland_window-type", "wallpaper");
        this->windowHandle()->setProperty("_d_dwayland_minimizable", false);
        this->windowHandle()->setProperty("_d_dwayland_maximizable", false);
        this->windowHandle()->setProperty("_d_dwayland_resizable", false);
    }
}

BaseDialog::~BaseDialog()
{
}

void BaseDialog::setTitle(const QString &title)
{
    titlebar->setTitle(title);
}

void BaseDialog::setTitleFont(const QFont &font)
{
    titlebar->setFont(font);
}

void BaseDialog::resizeEvent(QResizeEvent *event)
{
    titlebar->setFixedWidth(event->size().width());

    DAbstractDialog::resizeEvent(event);
}
