// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "collectionviewmenu.h"
#include "collectionview.h"
#include "models/collectionmodel.h"
#include "menus/organizermenu_defines.h"
#include "private/surface.h"

#include "desktoputils/ddpugin_eventinterface_helper.h"
#include "plugins/common/core/dfmplugin-menu/menu_eventinterface_helper.h"
#include "plugins/desktop/core/ddplugin-canvas/menu/canvasmenu_defines.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/dfm_desktop_defines.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>

#include <QGSettings>
#include <QMenu>
#include <QItemSelectionModel>

DFMGLOBAL_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

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
        qCritical() << "can not find canvas!";
        return;
    }

    auto canvasScene = dfmplugin_menu_util::menuSceneCreateScene("CanvasMenu");
    if (!canvasScene) {
        qCritical() << "Create CanvasMenu scene failed";
        return;
    }

    if (!canvasScene->initialize(params)) {
        delete canvasScene;
        return;
    }

    QMenu menu(view);
    canvasScene->create(&menu);
    canvasScene->updateState(&menu);

    if (QAction *act = menu.exec(QCursor::pos())) {
        QList<QUrl> urls { view->model()->rootUrl() };
        dpfSignalDispatcher->publish("ddplugin_organizer", "signal_CollectionView_ReportMenuData", act->text(), urls);
        canvasScene->triggered(act);
    }

    delete canvasScene;
}

void CollectionViewMenu::normalMenu(const QModelIndex &index, const Qt::ItemFlags &indexFlags, const QPoint gridPos)
{
    QList<QUrl> selectUrls;
    QList<FileInfoPointer> selectInfos;
    for (const QModelIndex &idx : view->selectedIndexes()) {
        auto url = view->model()->fileUrl(idx);
        if (url.isValid())
            selectUrls << url;
        auto info = view->model()->fileInfo(idx);
        if (info)
            selectInfos.append(info);
    }

    auto tgUrl = view->model()->fileUrl(index);
    auto focusInfo = view->model()->fileInfo(index);
    if (!focusInfo)
        return;

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
        qCritical() << "can not find canvas!";
        return;
    }

    auto canvasScene = dfmplugin_menu_util::menuSceneCreateScene("CanvasMenu");
    if (!canvasScene) {
        qCritical() << "Create CanvasMenu scene failed";
        return;
    }

    if (!canvasScene->initialize(params)) {
        delete canvasScene;
        return;
    }

    QMenu menu(view);
    canvasScene->create(&menu);
    canvasScene->updateState(&menu);

    if (QAction *act = menu.exec(QCursor::pos())) {
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
        qWarning() << "can not find surface by view" << view;
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
        qWarning() << "can not find root frame by screen" << screen;
        return canvas;
    }

    // find canvas
    for (QObject *obj : root->children()) {
        if (QWidget *wid = dynamic_cast<QWidget *>(obj)) {
            QString type = wid->property(DesktopFrameProperty::kPropWidgetName).toString();
            if (type == "canvas") {
                canvas = wid;
                qDebug() << "CollectionViewMenu find canvas" << wid << screen;
                break;
            }
        }
    }

    return canvas;
}
