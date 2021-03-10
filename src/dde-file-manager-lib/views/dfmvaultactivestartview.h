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

#ifndef DFMVAULTACTIVESTARTVIEW_H
#define DFMVAULTACTIVESTARTVIEW_H

#include <dtkwidget_global.h>

#include <QWidget>

QT_BEGIN_NAMESPACE
class QPushButton;
QT_END_NAMESPACE

class DFMVaultActiveStartView : public QWidget
{
    Q_OBJECT
public:
    explicit DFMVaultActiveStartView(QWidget *parent = nullptr);

signals:
    void sigAccepted();

private slots:
    void slotStartBtnClicked();

public slots:

private:
    QPushButton         *m_pStartBtn;   // 开启包厢按钮
};

#endif // DFMVAULTACTIVESTARTVIEW_H
