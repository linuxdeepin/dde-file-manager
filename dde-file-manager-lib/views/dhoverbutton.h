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

#ifndef DHOVERBUTTON_H
#define DHOVERBUTTON_H

#include <QPushButton>
#include <QLabel>
#include <QIcon>

class DFileMenu;

class DHoverButton : public QPushButton
{
    Q_OBJECT
public:
    DHoverButton(const QString &normal, const QString &hover, QWidget * parent = 0);
    void setMenu(DFileMenu *menu);
protected:
    void enterEvent(QEvent *e);
    void leaveEvent(QEvent *e);
    void mousePressEvent(QMouseEvent *e);
private:
    void initUI();
    QLabel* m_iconLabel;
    QIcon m_normal;
    QIcon m_hover;
    DFileMenu * m_menu = NULL;
};

#endif // DHOVERBUTTON_H
