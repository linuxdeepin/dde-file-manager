/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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
#ifndef CANVASVIEWMENUPROXY_H
#define CANVASVIEWMENUPROXY_H

#include "dfm_desktop_service_global.h"
#include "services/common/menu/menuservice.h"
#include <QObject>

DSB_D_BEGIN_NAMESPACE
class CanvasView;
class CanvasViewMenuProxy : public QObject
{
    Q_OBJECT
public:
    explicit CanvasViewMenuProxy(CanvasView *parent = nullptr);
    ~CanvasViewMenuProxy();

    void showEmptyAreaMenu(const Qt::ItemFlags &indexFlags, const QPoint gridPos);
    void showNormalMenu(const QModelIndex &index, const Qt::ItemFlags &indexFlags, const QPoint gridPos);

private:
    CanvasView *view;
    DSC_NAMESPACE::MenuService *extensionMenuServer = nullptr;
};

DSB_D_END_NAMESPACE

#endif   // CANVASVIEWMENUPROXY_H
