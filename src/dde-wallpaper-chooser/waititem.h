/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xinglinkun<xinglinkun@uniontech.com>
 *
 * Maintainer: xinglinkun<xinglinkun@uniontech.com>
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
#ifndef WAITITEM_H
#define WAITITEM_H
#include <QFrame>

#include <dtkwidget_global.h>

#include <QDebug>

DWIDGET_BEGIN_NAMESPACE
class DSpinner;
DWIDGET_END_NAMESPACE

DWIDGET_USE_NAMESPACE

class QWidget;
class QLabel;
class WaitItem : public QFrame
{
public:
    explicit WaitItem(QWidget *parent = nullptr);
    ~WaitItem();

    void initSize(const QSize &);
    void setContantText(const QString &);
    void setAnimation();

private:
    QLabel *m_icon = nullptr;
    QLabel *m_contant = nullptr;
    DSpinner *m_animationSpinner = nullptr;
    double m_proportion = 874 / 1900.0;//根据设计计算比例
    double m_movedistance = 0.0;//移动的相对位置
    QSize m_iconsize = QSize(30, 30);
    QSize m_contantsize = QSize(200, 20);
    QSize m_spinnersize = QSize(24, 24);
};

#endif // WAITITEM_H
