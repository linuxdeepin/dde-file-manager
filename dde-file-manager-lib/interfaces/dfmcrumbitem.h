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
#ifndef DFMCRUMBITEM_H
#define DFMCRUMBITEM_H

#include <QPushButton>

#include <dfmglobal.h>

DFM_BEGIN_NAMESPACE

class CrumbData;
class DFMCrumbItemPrivate;
class DFMCrumbItem : public QPushButton
{
    Q_OBJECT
public:
    explicit DFMCrumbItem(DUrl url, QWidget *parent = nullptr);
    DFMCrumbItem(CrumbData data, QWidget *parent = nullptr);
    ~DFMCrumbItem();

protected:
    void paintEvent(QPaintEvent *event);

private:
    QScopedPointer<DFMCrumbItemPrivate> d_ptr;

    Q_DECLARE_PRIVATE(DFMCrumbItem)
};

DFM_END_NAMESPACE

#endif // DFMCRUMBITEM_H
