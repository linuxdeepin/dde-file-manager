/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
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
#include "dfmcrumbitem.h"
#include "dfmcrumbinterface.h"

#include <QPainter>

DFM_BEGIN_NAMESPACE

class DFMCrumbItemPrivate
{
    Q_DECLARE_PUBLIC(DFMCrumbItem)

public:
    DFMCrumbItemPrivate(DFMCrumbItem *qq);

    DFMCrumbItem *q_ptr = nullptr;

private:

};

DFMCrumbItemPrivate::DFMCrumbItemPrivate(DFMCrumbItem *qq)
    : q_ptr(qq)
{
    qq->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    qq->setObjectName("DCrumbButton");
}


DFMCrumbItem::DFMCrumbItem(DUrl url, QWidget* parent)
    : QPushButton(parent)
    , d_ptr(new DFMCrumbItemPrivate(this))
{
    Q_UNUSED(url);
    //this->setStyleSheet("background: red");
    this->setText("test");
}

DFMCrumbItem::DFMCrumbItem(CrumbData data, QWidget* parent)
    : QPushButton(parent)
    , d_ptr(new DFMCrumbItemPrivate(this))
{
    if (!data.displayText.isEmpty()) {
        this->setText(data.displayText);
    }

    if (!data.iconName.isEmpty()) {
        //set icon
    }
}

DFMCrumbItem::~DFMCrumbItem()
{

}

void DFMCrumbItem::paintEvent(QPaintEvent *event)
{
    QPainter pa(this);

    //pa.fillRect(rect(), Qt::blue);

    QPushButton::paintEvent(event);
}

DFM_END_NAMESPACE
