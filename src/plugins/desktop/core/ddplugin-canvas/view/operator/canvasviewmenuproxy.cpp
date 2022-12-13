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
#include "view/canvasview_p.h"
#include "model/canvasproxymodel.h"
#include "model/canvasselectionmodel.h"
#include "view/operator/fileoperatorproxy.h"
#include "menu/canvasmenuscene.h"
#include "menu/canvasmenu_defines.h"

#include "plugins/common/core/dfmplugin-menu/menu_eventinterface_helper.h"

#include "dfm-base/utils/clipboard.h"
#include "dfm-base/interfaces/abstractfileinfo.h"
#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/base/application/application.h"
#include "dfm-base/base/application/settings.h"
#include "dfm-base/dfm_menu_defines.h"

#include <QGSettings>
#include <QMenu>
#include <QtDebug>

DFMGLOBAL_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace ddplugin_canvas;

CanvasViewMenuProxy::CanvasViewMenuProxy(CanvasView *parent)
    : QObject(parent), view(parent)
{
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
    // extend menu
    if (view->d->hookIfs && view->d->hookIfs->contextMenu(view->screenNum(), view->model()->rootUrl(), QList<QUrl>(), QCursor::pos()))
        return;

    // TODO(lee) 这里的Q_UNUSED参数后续随着业务接入会进行优化
    Q_UNUSED(indexFlags)
    auto canvasScene = dfmplugin_menu_util::menuSceneCreateScene(CanvasMenuCreator::name());
    if (!canvasScene) {
        qWarning() << "Create scene failed, scene name: " << CanvasMenuCreator::name();
        return;
    }

    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = view->model()->rootUrl();
    params[MenuParamKey::kOnDesktop] = true;
    params[MenuParamKey::kWindowId] = view->winId();
    params[MenuParamKey::kIsEmptyArea] = true;
    params[CanvasMenuParams::kDesktopGridPos] = QVariant::fromValue(gridPos);
    params[CanvasMenuParams::kDesktopCanvasView] = QVariant::fromValue((qlonglong)view);

    if (!canvasScene->initialize(params)) {
        delete canvasScene;
        return;
    }

    QMenu menu(this->view);

    canvasScene->create(&menu);
    canvasScene->updateState(&menu);

    if (QAction *act = menu.exec(QCursor::pos())) {
        QList<QUrl> urls { view->model()->rootUrl() };
        dpfSignalDispatcher->publish("ddplugin_canvas", "signal_CanvasView_ReportMenuData", act->text(), urls);
        canvasScene->triggered(act);
    }
    delete canvasScene;
}

void CanvasViewMenuProxy::showNormalMenu(const QModelIndex &index, const Qt::ItemFlags &indexFlags, const QPoint gridPos)
{
    auto selectUrls = view->selectionModel()->selectedUrls();
    auto tgUrl = view->model()->fileUrl(index);

    // extend menu
    {
        // first is focus
        selectUrls.removeAll(tgUrl);
        selectUrls.prepend(tgUrl);
        if (view->d->hookIfs && view->d->hookIfs->contextMenu(view->screenNum(), view->model()->rootUrl(), selectUrls, QCursor::pos()))
            return;
    }

    // TODO(lee) 这里的Q_UNUSED参数后续随着业务接入会进行优化
    Q_UNUSED(indexFlags)

    auto canvasScene = dfmplugin_menu_util::menuSceneCreateScene(CanvasMenuCreator::name());
    if (!canvasScene) {
        qWarning() << "Create scene failed, scene name: " << CanvasMenuCreator::name();
        return;
    }

    // TODO(Lee)：多文件筛选、多选中包含 计算机 回收站 主目录时不显示扩展菜单

    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = view->model()->rootUrl();
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(selectUrls);
    params[MenuParamKey::kOnDesktop] = true;
    params[MenuParamKey::kWindowId] = view->winId();
    params[MenuParamKey::kIsEmptyArea] = false;
    params[MenuParamKey::kIndexFlags] = QVariant::fromValue(indexFlags);
    params[CanvasMenuParams::kDesktopGridPos] = QVariant::fromValue(gridPos);
    params[CanvasMenuParams::kDesktopCanvasView] = QVariant::fromValue((qlonglong)view);
    params = dfmplugin_menu_util::menuPerfectParams(params);

    if (!canvasScene->initialize(params)) {
        delete canvasScene;
        return;
    }

    QMenu menu(view);
    canvasScene->create(&menu);
    canvasScene->updateState(&menu);

    if (QAction *act = menu.exec(QCursor::pos())) {
        dpfSignalDispatcher->publish("ddplugin_canvas", "signal_CanvasView_ReportMenuData", act->text(), selectUrls);
        canvasScene->triggered(act);
    }
    delete canvasScene;
}

void CanvasViewMenuProxy::changeIconLevel(bool increase)
{
    CanvasIns->onChangeIconLevel(increase);
}
