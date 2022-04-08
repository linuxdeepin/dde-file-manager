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
#include "canvasmanager.h"
#include "grid/canvasgrid.h"
#include "view/canvasview.h"
#include "model/canvasmodel.h"
#include "model/canvasselectionmodel.h"
#include "view/operator/fileoperatorproxy.h"
#include "menu/canvasmenu.h"
#include "menu/canvasmenuscene.h"
#include "menu/canvasmenu_defines.h"

#include <services/common/menu/menu_defines.h>

#include <dfm-base/utils/clipboard.h>
#include <interfaces/abstractfileinfo.h>
#include <dfm-base/dfm_global_defines.h>
#include <base/application/application.h>
#include <base/application/settings.h>

#include <QGSettings>

#include <QMenu>
#include <QtDebug>

DSC_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DDP_CANVAS_USE_NAMESPACE

CanvasViewMenuProxy::CanvasViewMenuProxy(CanvasView *parent)
    : QObject(parent), view(parent)
{
    // 获取菜单服务
    extensionMenuServer = MenuService::service();

    // 注册canvas菜单
    extensionMenuServer->registerScene(CanvasMenuCreator::name(), new CanvasMenuCreator);
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

    auto canvasScene = extensionMenuServer->createScene(CanvasMenuCreator::name());

    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = view->model()->rootUrl().toString();
    params[MenuParamKey::kOnDesktop] = true;
    params[MenuParamKey::kwindowId] = view->winId();
    params[MenuParamKey::kIsEmptyArea] = true;
    params[CanvasMenuParams::kDesktopGridPos] = QVariant::fromValue(gridPos);

    if (!canvasScene->initialize(params)) {
        delete canvasScene;
        return;
    }

    QMenu menu(this->view);

    canvasScene->create(&menu);
    canvasScene->updateState(&menu);

    if (QAction *act = menu.exec(QCursor::pos()))
        canvasScene->triggered(act);
    delete canvasScene;
}

void CanvasViewMenuProxy::showNormalMenu(const QModelIndex &index, const Qt::ItemFlags &indexFlags, const QPoint gridPos)
{
    // TODO(lee) 这里的Q_UNUSED参数后续随着业务接入会进行优化
    Q_UNUSED(indexFlags)

    // TODO(Lee)：多文件筛选、多选中包含 计算机 回收站 主目录时不显示扩展菜单

    auto selectUrls = view->selectionModel()->selectedUrls();
    QStringList selectPath;
    for (const auto &temp : selectUrls)
        selectPath << temp.toString();

    auto tgUrl = view->model()->url(index);
    auto canvasScene = extensionMenuServer->createScene(CanvasMenuCreator::name());

    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = view->model()->rootUrl().toString();
    params[MenuParamKey::kFocusFile] = tgUrl.toString();
    params[MenuParamKey::kSelectFiles] = selectPath;
    params[MenuParamKey::kOnDesktop] = true;
    params[MenuParamKey::kIsEmptyArea] = false;
    params[CanvasMenuParams::kDesktopGridPos] = QVariant::fromValue(gridPos);

    if (!canvasScene->initialize(params)) {
        delete canvasScene;
        return;
    }

    QMenu menu;
    canvasScene->create(&menu);
    canvasScene->updateState(&menu);

    if (QAction *act = menu.exec(QCursor::pos()))
        canvasScene->triggered(act);
    delete canvasScene;
}

void CanvasViewMenuProxy::changeIconLevel(bool increase)
{
    CanvasIns->onChangeIconLevel(increase);
}
