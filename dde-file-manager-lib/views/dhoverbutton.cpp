/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#include "dhoverbutton.h"
#include "dfilemenumanager.h"
#include "dfilemenu.h"
#include <QHBoxLayout>
#include <QDebug>

DHoverButton::DHoverButton(const QString &normal, const QString &hover, QWidget *parent)
    :QPushButton(parent)
{
    QIcon n(normal);
    m_normal = n;
    QIcon h(hover);
    m_hover = h;
    initUI();
}

void DHoverButton::setMenu(DFileMenu *menu)
{
    m_menu = menu;
}

void DHoverButton::enterEvent(QEvent *e)
{
    Q_UNUSED(e)

    m_iconLabel->setPixmap(m_hover.pixmap(16,16));
}

void DHoverButton::leaveEvent(QEvent *e)
{
    Q_UNUSED(e)

    m_iconLabel->setPixmap(m_normal.pixmap(16,16));
}

void DHoverButton::mousePressEvent(QMouseEvent *e)
{
    Q_UNUSED(e)

    if(m_menu)
        m_menu->exec(mapToGlobal(rect().bottomLeft()));
}

void DHoverButton::initUI()
{
    m_iconLabel = new QLabel;
    m_iconLabel->setPixmap(m_normal.pixmap(16,16));
    m_iconLabel->setFixedSize(16, 16);

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_iconLabel);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mainLayout);
}
