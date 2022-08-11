/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#include "collectionviewmenu.h"
#include "collectionview.h"
#include "models/fileproxymodel.h"
#include "menus/organizermenu_defines.h"
#include "private/surface.h"

#include "desktoputils/ddpugin_eventinterface_helper.h"
#include "plugins/common/core/dfmplugin-menu/menu_eventinterface_helper.h"
#include "plugins/desktop/core/ddplugin-canvas/menu/canvasmenu_defines.h"

#include "dfm-base/dfm_menu_defines.h"
#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/dfm_desktop_defines.h"
#include "dfm-base/base/application/application.h"
#include "dfm-base/base/application/settings.h"

#include <QGSettings>
#include <QMenu>
#include <QItemSelectionModel>

DFMGLOBAL_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

using namespace ddplugin_organizer;

CollectionViewMenu::CollectionViewMenu(CollectionView *parent)
    : QObject(parent)
    , view(parent)
{

}

bool CollectionViewMenu::disableMenu()
{
    // same as canvas
    if (QGSettings::isSchemaInstalled("com.deepin.dde.filemanager.desktop")) {
        QGSettings set("com.deepin.dde.filemanager.desktop", "/com/deepin/dde/filemanager/desktop/");
        QVariant var = set.get("contextMenu");
        if (var.isValid())
            return !var.toBool();
    }

    return Application::appObtuselySetting()->value("ApplicationAttribute", "DisableDesktopContextMenu", false).toBool();
}

void CollectionViewMenu::emptyAreaMenu()
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = view->model()->fileUrl(view->model()->rootIndex()).toString();
    params[MenuParamKey::kOnDesktop] = true;
    params[MenuParamKey::kWindowId] = view->winId();
    params[MenuParamKey::kIsEmptyArea] = true;
    params[CollectionMenuParams::kOnColletion] = true;

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

    if (QAction *act = menu.exec(QCursor::pos()))
        canvasScene->triggered(act);

    delete canvasScene;
}

void CollectionViewMenu::normalMenu(const QModelIndex &index, const Qt::ItemFlags &indexFlags, const QPoint gridPos)
{
    QList<QUrl> selectUrls;
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
    params[MenuParamKey::kCurrentDir] = view->model()->fileUrl(view->model()->rootIndex()).toString();
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(selectUrls);
    params[MenuParamKey::kOnDesktop] = true;
    params[MenuParamKey::kWindowId] = view->winId();
    params[MenuParamKey::kIsEmptyArea] = false;
    params[MenuParamKey::kIndexFlags] = QVariant::fromValue(indexFlags);
    params[CollectionMenuParams::kOnColletion] = true;
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

    if (QAction *act = menu.exec(QCursor::pos()))
        canvasScene->triggered(act);

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
