/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#include "fileviewmenuhelper.h"
#include "views/fileview.h"
#include "models/filesortfilterproxymodel.h"
#include "menus/workspacemenuscene.h"
#include "utils/workspacehelper.h"

#include "plugins/common/core/dfmplugin-menu/menu_eventinterface_helper.h"

#include "dfm-base/dfm_menu_defines.h"
#include "dfm-base/utils/systempathutil.h"
#include "dfm-base/utils/fileutils.h"
#include "dfm-base/widgets/dfmwindow/filemanagerwindowsmanager.h"

#include <dfm-framework/dpf.h>

#include <QMenu>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_workspace;

FileViewMenuHelper::FileViewMenuHelper(FileView *parent)
    : QObject(parent),
      view(parent)
{
}

void FileViewMenuHelper::showEmptyAreaMenu()
{
    auto scene = dfmplugin_menu_util::menuSceneCreateScene(currentMenuScene());
    if (!scene) {
        qWarning() << "Create scene failed, scene name: " << currentMenuScene();
        return;
    }

    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = view->rootUrl();
    params[MenuParamKey::kOnDesktop] = false;
    params[MenuParamKey::kIsEmptyArea] = true;
    params[MenuParamKey::kWindowId] = FMWindowsIns.findWindowId(view);

    if (!scene->initialize(params)) {
        delete scene;
        return;
    }

    QMenu menu(this->view);
    scene->create(&menu);
    scene->updateState(&menu);

    QAction *act = menu.exec(QCursor::pos());
    if (act)
        scene->triggered(act);
    delete scene;
}

void FileViewMenuHelper::showNormalMenu(const QModelIndex &index, const Qt::ItemFlags &indexFlags)
{
    auto scene = dfmplugin_menu_util::menuSceneCreateScene(currentMenuScene());
    if (!scene) {
        qWarning() << "Create scene failed, scene name: " << currentMenuScene();
        return;
    }

    QList<QUrl> selectUrls = view->selectedUrlList();
    QUrl tgUrl;

    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = view->rootUrl();

    const AbstractFileInfoPointer &focusFileInfo = view->model()->itemFileInfo(index);
    if (focusFileInfo) {
        tgUrl = focusFileInfo->urlOf(UrlInfoType::kUrl);
        // first is focus
        selectUrls.removeAll(tgUrl);
        selectUrls.prepend(tgUrl);
    }
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(selectUrls);
    params[MenuParamKey::kIndexFlags] = QVariant::fromValue(indexFlags);
    params[MenuParamKey::kOnDesktop] = false;
    params[MenuParamKey::kIsEmptyArea] = false;
    params[MenuParamKey::kWindowId] = FMWindowsIns.findWindowId(view);
    params = dfmplugin_menu_util::menuPerfectParams(params);

    if (!scene->initialize(params)) {
        delete scene;
        return;
    }

    QMenu menu(this->view);
    scene->create(&menu);
    scene->updateState(&menu);

    QAction *act = menu.exec(QCursor::pos());
    if (act)
        scene->triggered(act);
    delete scene;
}

QString FileViewMenuHelper::currentMenuScene() const
{
    QString scene = WorkspaceHelper::instance()->findMenuScene(view->rootUrl().scheme());
    return scene.isEmpty() ? WorkspaceMenuCreator::name() : scene;
}
