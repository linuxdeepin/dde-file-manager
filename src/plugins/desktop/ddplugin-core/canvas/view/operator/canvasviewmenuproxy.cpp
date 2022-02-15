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
#include "canvasviewmenuproxy.h"
#include "displayconfig.h"
#include "canvas/canvasmanager.h"
#include "canvas/grid/canvasgrid.h"
#include "canvas/view/canvasview.h"
#include "canvas/view/canvasmodel.h"
#include "canvas/view/canvasselectionmodel.h"
#include "canvas/view/operator/fileoperaterproxy.h"
#include "canvas/menu/canvasmenu.h"

#include "dfm-base/utils/clipboard.h"
#include "dfm-base/interfaces/abstractfileinfo.h"
#include "dfm-base/dfm_global_defines.h"
#include "base/application/application.h"
#include "base/application/settings.h"

#include <QGSettings>

#include <QMenu>
#include <QtDebug>

DSC_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DSB_D_USE_NAMESPACE

CanvasViewMenuProxy::CanvasViewMenuProxy(CanvasView *parent)
    : QObject(parent), view(parent)
{
    // 获取扩展菜单服务
    auto &ctx = dpfInstance.serviceContext();
    extensionMenuServer = ctx.service<MenuService>(MenuService::name());
}

CanvasViewMenuProxy::~CanvasViewMenuProxy()
{
}

bool CanvasViewMenuProxy::disableMenu()
{
    // the gsetting control is higher than json profile. it doesn't check json profile if there is gsetting value.
    if (QGSettings::isSchemaInstalled("com.deepin.dde.filemanager.desktop")) {
        QGSettings set("com.deepin.dde.filemanager.desktop", "/com/deepin/dde/filemanager/desktop/");
        QVariant var = set.get("contextMenu");
        if (var.isValid())
            return !var.toBool();
    }

    return Application::appObtuselySetting()->value("ApplicationAttribute", "DisableDesktopContextMenu", false).toBool();
}

void CanvasViewMenuProxy::showEmptyAreaMenu(const Qt::ItemFlags &indexFlags, const QPoint gridPos)
{
    // TODO(lee) 这里的Q_UNUSED参数后续随着业务接入会进行优化
    Q_UNUSED(indexFlags)

    QMenu *menu = new QMenu;
    menu = extensionMenuServer->createMenu(view,
                                           MenuScene::kDesktopMenu,
                                           AbstractMenu::MenuMode::kEmpty,
                                           view->model()->rootUrl(),
                                           QUrl(),
                                           {},
                                           kAllExtensionAction,
                                           QVariant::fromValue(gridPos));

    if (menu) {
        menu->exec(QCursor::pos());
    }
}

void CanvasViewMenuProxy::showNormalMenu(const QModelIndex &index, const Qt::ItemFlags &indexFlags, const QPoint gridPos)
{
    // TODO(lee) 这里的Q_UNUSED参数后续随着业务接入会进行优化
    Q_UNUSED(indexFlags)

    auto selectUrls = view->selectionModel()->selectedUrls();
    auto tgUrl = view->model()->url(index);
    QMenu *menu = extensionMenuServer->createMenu(view,
                                                  MenuScene::kDesktopMenu,
                                                  AbstractMenu::MenuMode::kNormal,
                                                  QUrl(),
                                                  tgUrl,
                                                  selectUrls,
                                                  kAllExtensionAction,
                                                  QVariant::fromValue(gridPos));

    if (menu) {
        menu->exec(QCursor::pos());
    }
}

void CanvasViewMenuProxy::changeIconLevel(bool increase)
{
    CanvasIns->onChangeIconLevel(increase);
}

