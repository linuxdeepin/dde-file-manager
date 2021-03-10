/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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
