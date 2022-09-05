// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "basedialog.h"
#include "dfmglobal.h"

#include <dwindowclosebutton.h>
#include <dtitlebar.h>

#include <QApplication>
#include <QDesktopWidget>
#include <QWindow>
#include <QShowEvent>
#include <QDebug>

BaseDialog::BaseDialog(QWidget *parent) : DAbstractDialog(parent)
{
    m_titlebar = new DTitlebar(this);
    m_titlebar->setBackgroundTransparent(true);
//    m_titlebar->setWindowFlags(Qt::WindowCloseButtonHint | Qt::WindowTitleHint);
    if(DFMGlobal::isWayLand())
    {
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
    m_titlebar->setTitle(title);
}

void BaseDialog::resizeEvent(QResizeEvent *event)
{
    m_titlebar->setFixedWidth(event->size().width());

    DAbstractDialog::resizeEvent(event);
}
