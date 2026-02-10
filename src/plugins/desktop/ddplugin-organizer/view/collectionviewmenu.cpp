// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "collectionviewmenu.h"
#include "collectionview.h"
#include "models/collectionmodel.h"
#include "menus/organizermenu_defines.h"
#include "private/surface.h"

#include "desktoputils/ddplugin_eventinterface_helper.h"
#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"
#include "plugins/desktop/ddplugin-canvas/menu/canvasmenu_defines.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/dfm_desktop_defines.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>

#include <QItemSelectionModel>

DFMGLOBAL_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

using namespace ddplugin_organizer;

CollectionViewMenu::CollectionViewMenu(CollectionView *parent)
    : QObject(parent), view(parent)
{
}

bool CollectionViewMenu::disableMenu()
{
    QVariantHash params;
    // use qApp->applicationName by defalut;
    //params.insert("ApplicationName", "dde-desktop");
    auto ret = dpfSlotChannel->push("dfmplugin_menu", "slot_Menu_IsDisable", params);

    if (ret.isValid())
        return ret.toBool();
    return false;
}

void CollectionViewMenu::emptyAreaMenu()
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = view->model()->fileUrl(view->model()->rootIndex());
    params[MenuParamKey::kOnDesktop] = true;
    params[MenuParamKey::kWindowId] = view->winId();
    params[MenuParamKey::kIsEmptyArea] = true;
    params[CollectionMenuParams::kOnColletion] = true;
    params[CollectionMenuParams::kColletionView] = reinterpret_cast<qlonglong>(view);

    //find canvasview
    if (auto canvas = getCanvasView()) {
        params[ddplugin_canvas::CanvasMenuParams::kDesktopCanvasView] = reinterpret_cast<qlonglong>(canvas);
    } else {
        fmCritical() << "can not find canvas!";
        return;
    }

    auto canvasScene = dfmplugin_menu_util::menuSceneCreateScene("CanvasMenu");
    if (!canvasScene) {
        fmCritical() << "Create CanvasMenu scene failed";
        return;
    }

    if (!canvasScene->initialize(params)) {
        fmWarning() << "Failed to initialize canvas menu scene";
        delete canvasScene;
        return;
    }

    if (menuPtr)
        delete menuPtr;

    menuPtr = new DMenu(view);
    canvasScene->create(menuPtr);
    canvasScene->updateState(menuPtr);

    if (QAction *act = menuPtr->exec(QCursor::pos())) {
        QList<QUrl> urls { view->model()->rootUrl() };
        dpfSignalDispatcher->publish("ddplugin_organizer", "signal_CollectionView_ReportMenuData", act->text(), urls);
        canvasScene->triggered(act);
    }

    delete canvasScene;
}

void CollectionViewMenu::normalMenu(const QModelIndex &index, const Qt::ItemFlags &indexFlags, const QPoint gridPos)
{
    QList<QUrl> selectUrls;
    // all selected indexes in each view.
    for (const QModelIndex &idx : view->selectionModel()->selectedIndexes()) {
        auto url = view->model()->fileUrl(idx);
        if (url.isValid())
            selectUrls << url;
    }

    auto tgUrl = view->model()->fileUrl(index);
    // first is focus
    if (selectUrls.size() > 1) {
        selectUrls.removeAll(tgUrl);
        selectUrls.prepend(tgUrl);
    }

    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = view->model()->fileUrl(view->model()->rootIndex());
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(selectUrls);
    params[MenuParamKey::kOnDesktop] = true;
    params[MenuParamKey::kWindowId] = view->winId();
    params[MenuParamKey::kIsEmptyArea] = false;
    params[MenuParamKey::kIndexFlags] = QVariant::fromValue(indexFlags);
    params[CollectionMenuParams::kOnColletion] = true;
    params[CollectionMenuParams::kColletionView] = reinterpret_cast<qlonglong>(view);
    params = dfmplugin_menu_util::menuPerfectParams(params);

    //find canvasview
    if (auto canvas = getCanvasView()) {
        params[ddplugin_canvas::CanvasMenuParams::kDesktopCanvasView] = reinterpret_cast<qlonglong>(canvas);
    } else {
        fmCritical() << "can not find canvas!";
        return;
    }

    auto canvasScene = dfmplugin_menu_util::menuSceneCreateScene("CanvasMenu");
    if (!canvasScene) {
        fmCritical() << "Create CanvasMenu scene failed";
        return;
    }

    if (!canvasScene->initialize(params)) {
        fmWarning() << "Failed to initialize canvas menu scene for normal menu";
        delete canvasScene;
        return;
    }

    if (menuPtr)
        delete menuPtr;

    menuPtr = new DMenu(view);
    canvasScene->create(menuPtr);
    canvasScene->updateState(menuPtr);

    if (QAction *act = menuPtr->exec(QCursor::pos())) {
        dpfSignalDispatcher->publish("ddplugin_organizer", "signal_CollectionView_ReportMenuData", act->text(), selectUrls);
        canvasScene->triggered(act);
    }

    delete canvasScene;
}

QWidget *CollectionViewMenu::getCanvasView()
{
    QWidget *canvas = nullptr;
    Surface *surface = nullptr;

    // find surface
    {
        QWidget *cur = view;
        while (auto wid = cur->parentWidget()) {
            if (Surface *isIt = qobject_cast<Surface *>(wid)) {
                surface = isIt;
                break;
            }
            cur = wid;
        }
    }
    if (!surface) {
        fmWarning() << "can not find surface by view" << view;
        return canvas;
    }

    // find root widget by screen name.
    const QString &screen = surface->property(DesktopFrameProperty::kPropScreenName).toString();

    QWidget *root = nullptr;
    for (QWidget *cur : ddplugin_desktop_util::desktopFrameRootWindows()) {
        if (cur->property(DesktopFrameProperty::kPropScreenName).toString() == screen) {
            root = cur;
            break;
        }
    }

    if (!root) {
        fmWarning() << "can not find root frame by screen" << screen;
        return canvas;
    }

    // find canvas
    for (QObject *obj : root->children()) {
        if (QWidget *wid = dynamic_cast<QWidget *>(obj)) {
            QString type = wid->property(DesktopFrameProperty::kPropWidgetName).toString();
            if (type == "canvas") {
                canvas = wid;
                fmDebug() << "Canvas found for menu operations on screen:" << screen;
                break;
            }
        }
    }

    return canvas;
}
