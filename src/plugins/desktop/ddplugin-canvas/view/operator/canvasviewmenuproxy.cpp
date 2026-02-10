// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

#include <dfm-base/utils/clipboard.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/dfm_menu_defines.h>

#include <QtDebug>

DWIDGET_USE_NAMESPACE
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
    fmDebug() << "Checking if menu is disabled";

    QVariantHash params;
    //use qApp->applicationName by defalut;
    //params.insert("ApplicationName", "dde-desktop");
    auto ret = dpfSlotChannel->push("dfmplugin_menu", "slot_Menu_IsDisable", params);
    bool disabled = ret.isValid() ? ret.toBool() : false;
    fmDebug() << "Menu disabled status:" << disabled;
    return disabled;
}

void CanvasViewMenuProxy::showEmptyAreaMenu(const Qt::ItemFlags &indexFlags, const QPoint gridPos)
{
    // extend menu
    if (view->d->hookIfs && view->d->hookIfs->contextMenu(view->screenNum(), view->model()->rootUrl(), QList<QUrl>(), QCursor::pos())) {
        fmInfo() << "Context menu handled by extension module";
        return;
    }

    // TODO(lee) 这里的Q_UNUSED参数后续随着业务接入会进行优化
    Q_UNUSED(indexFlags)
    auto canvasScene = dfmplugin_menu_util::menuSceneCreateScene(CanvasMenuCreator::name());
    if (!canvasScene) {
        fmWarning() << "Failed to create canvas menu scene:" << CanvasMenuCreator::name();
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
        fmWarning() << "Failed to initialize canvas menu scene";
        delete canvasScene;
        return;
    }

    if (menuPtr) {
        fmDebug() << "Deleting existing menu pointer";
        delete menuPtr;
    }

    menuPtr = new DMenu(view);
    canvasScene->create(menuPtr);
    canvasScene->updateState(menuPtr);
    if (QAction *act = menuPtr->exec(QCursor::pos())) {
        QList<QUrl> urls { view->model()->rootUrl() };
        fmInfo() << "Empty area menu action triggered:" << act->text();
        dpfSignalDispatcher->publish("ddplugin_canvas", "signal_CanvasView_ReportMenuData", act->text(), urls);
        canvasScene->triggered(act);
    } else {
        fmDebug() << "Empty area menu cancelled by user";
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
        fmWarning() << "Failed to create canvas menu scene:" << CanvasMenuCreator::name();
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
        fmWarning() << "Failed to initialize canvas menu scene for selected files";
        delete canvasScene;
        return;
    }

    if (menuPtr) {
        fmDebug() << "Deleting existing menu pointer";
        delete menuPtr;
    }

    menuPtr = new DMenu(view);
    canvasScene->create(menuPtr);
    canvasScene->updateState(menuPtr);

    if (QAction *act = menuPtr->exec(QCursor::pos())) {
        dpfSignalDispatcher->publish("ddplugin_canvas", "signal_CanvasView_ReportMenuData", act->text(), selectUrls);
        canvasScene->triggered(act);
    } else {
        fmDebug() << "File context menu cancelled by user";
    }

    delete canvasScene;
}

void CanvasViewMenuProxy::changeIconLevel(bool increase)
{
    CanvasIns->onChangeIconLevel(increase);
}
