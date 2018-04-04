/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
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
#include "dfmsidebaritem.h"

DFM_BEGIN_NAMESPACE

class DFMSideBarItemPrivate
{
    Q_DECLARE_PUBLIC(DFMSideBarItem)

public:
    DFMSideBarItemPrivate(DFMSideBarItem *qq);

    bool hasDrag = false;
    bool readOnly = true;
    bool checked = false;

    DUrl url;

    DFMSideBarItem *q_ptr = nullptr;
};

DFMSideBarItemPrivate::DFMSideBarItemPrivate(DFMSideBarItem *qq)
    : q_ptr(qq)
{

}

DFMSideBarItem::DFMSideBarItem(const DUrl &url, QWidget *parent)
    : QWidget(parent)
    , d_ptr(new DFMSideBarItemPrivate(this))
{
    d_func()->url = url;
}

DFMSideBarItem::~DFMSideBarItem()
{

}

bool DFMSideBarItem::hasDrag() const
{
    Q_D(const DFMSideBarItem);

    return d->hasDrag;
}

bool DFMSideBarItem::readOnly() const
{
    Q_D(const DFMSideBarItem);

    return d->readOnly;
}

bool DFMSideBarItem::checked() const
{
    Q_D(const DFMSideBarItem);

    return d->checked;
}

QString DFMSideBarItem::text() const
{

}

void DFMSideBarItem::setIconFromThemeConfig(const QString &group, const QString &key)
{

}

void DFMSideBarItem::setHasDrag(bool hasDrag)
{
    Q_D(DFMSideBarItem);

    d->hasDrag = hasDrag;
}

void DFMSideBarItem::setReadOnly(bool readOnly)
{
    Q_D(DFMSideBarItem);

    d->readOnly = readOnly;
}

void DFMSideBarItem::setChecked(bool checked)
{
    Q_D(DFMSideBarItem);

    d->checked = checked;
}

void DFMSideBarItem::setText(QString text)
{

}

void DFMSideBarItem::playAnimation()
{

}

QMenu *DFMSideBarItem::createStandardContextMenu() const
{

}

bool DFMSideBarItem::canDropMimeData(const QMimeData *data, Qt::DropAction action) const
{

}

bool DFMSideBarItem::dropMimeData(const QMimeData *data, Qt::DropAction action) const
{

}

DFM_END_NAMESPACE
