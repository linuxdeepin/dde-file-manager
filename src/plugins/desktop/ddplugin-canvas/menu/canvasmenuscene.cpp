/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
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
#include "canvasmenuscene_p.h"

#include "displayconfig.h"
#include "canvasmanager.h"
#include "grid/canvasgrid.h"
#include "view/canvasview.h"
#include "model/canvasmodel.h"
#include "model/canvasselectionmodel.h"
#include "delegate/canvasitemdelegate.h"

#include <services/common/menu/menuservice.h>

#include <dfm-framework/framework.h>

#include <QMenu>
#include <QVariant>
#include <QGSettings>
#include <QDBusInterface>
#include <QDBusPendingCall>

DDP_CANVAS_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DSC_USE_NAMESPACE

AbstractMenuScene *CanvasMenuCreator::create()
{
    return new CanvasMenuScene();
}

CanvasMenuScenePrivate::CanvasMenuScenePrivate(CanvasMenuScene *qq)
    : QObject(qq)
{
    // 获取菜单服务
    auto &ctx = dpfInstance.serviceContext();
    extensionMenuServer = ctx.service<MenuService>(MenuService::name());
}

CanvasMenuScene::CanvasMenuScene(QObject *parent)
    : AbstractMenuScene(parent),
      d(new CanvasMenuScenePrivate(this))
{
}

QString CanvasMenuScene::name() const
{
    return CanvasMenuCreator::name();
}

bool CanvasMenuScene::initialize(const QVariantHash &params)
{
    d->currentDir = params.value("currentDir").toString();
    d->focusFile = params.value("focusFile").toString();
    d->selectFiles = params.value("selectFiles").toStringList();
    d->onDesktop = params.value("desktop").toBool();
    d->isEmptyArea = params.value("isEmptyArea").toBool();

    if (d->currentDir.isEmpty())
        return false;

    // default
    if (auto defaultScene = d->extensionMenuServer->createScene("DefaultMenu"))
        subScene.append(defaultScene);

    if (d->isEmptyArea) {
        // new (new doc, new dir)
        if (auto newCreateScene = d->extensionMenuServer->createScene("NewCreateMenu"))
            subScene.append(newCreateScene);

    } else {
        // open with
        if (auto openWithScene = d->extensionMenuServer->createScene("OpenWithMenu"))
            subScene.append(openWithScene);

        // dir (open in new window,open as admin, open in new tab,open new terminal,select all)
        if (auto dirScene = d->extensionMenuServer->createScene("OpenDirMenu"))
            subScene.append(dirScene);

        // file (rename)
        if (auto fileScene = d->extensionMenuServer->createScene("OpenFileMenu"))
            subScene.append(fileScene);
    }

    // 初始化所有子场景
    AbstractMenuScene::initialize(params);

    return true;
}

AbstractMenuScene *CanvasMenuScene::scene(QAction *action) const
{
    if (action == nullptr)
        return nullptr;

    if (d->providSelfActionList.contains(action))
        return const_cast<CanvasMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

bool CanvasMenuScene::create(QMenu *parent)
{
    if (!parent)
        return false;

    d->view = qobject_cast<CanvasView *>(parent->parent());

    if (d->isEmptyArea) {
        this->emptyMenu(parent);
    } else {
        this->normalMenu(parent);
    }

    // 创建子场景菜单
    AbstractMenuScene::create(parent);

    // TODO(Lee) 排序
    return true;
}

void CanvasMenuScene::updateState(QMenu *parent)
{
    Q_UNUSED(parent)
}

bool CanvasMenuScene::triggered(QAction *action)
{
    if (!d->providSelfActionList.contains(action))
        return false;

    auto actionText = action->text();

    // icon size
    if (d->iconSizeMap.contains(actionText)) {

        int iconLv = d->iconSizeMap.value(actionText);
        for (auto v : ddplugin_canvas::CanvasIns->views()) {
            v->itemDelegate()->setIconLevel(iconLv);
            v->updateGrid();
        }

        DispalyIns->setIconLevel(iconLv);
        return true;
    }

    // Display Settings
    if (actionText == tr("Display Settings")) {
        QDBusInterface interface("com.deepin.dde.ControlCenter", "/com/deepin/dde/ControlCenter", "com.deepin.dde.ControlCenter");
        interface.asyncCall("ShowModule", QVariant::fromValue(QString("display")));
        return true;
    }

    // Wallpaper and Screensaver
    if (actionText == tr("Wallpaper and Screensaver")) {
        CanvasIns->onWallperSetting(d->view);
        return true;
    }

    // Auto arrange
    if (actionText == tr("Auto arrange")) {
        bool align = action->isChecked();
        DispalyIns->setAutoAlign(align);
        GridIns->setMode(align ? CanvasGrid::Mode::Align : CanvasGrid::Mode::Custom);
        if (align) {
            GridIns->arrange();
            CanvasIns->update();
        }
        return true;
    }

    return false;
}

void CanvasMenuScene::emptyMenu(QMenu *parent)
{
    QAction *tempAction = parent->addAction(tr("Sort by"));
    tempAction->setMenu(sortBySubActions(parent));
    d->providSelfActionList.append(tempAction);
    d->predicateAction[tr("Sort by")] = tempAction;

    tempAction = parent->addAction(tr("Icon size"));
    tempAction->setMenu(iconSizeSubActions(parent));
    d->providSelfActionList.append(tempAction);
    d->predicateAction[tr("Open as administrator")] = tempAction;

    tempAction = parent->addAction(tr("Auto arrange"));
    d->providSelfActionList.append(tempAction);
    d->predicateAction[tr("Auto arrange")] = tempAction;

    parent->addSeparator();
    tempAction = parent->addAction(tr("Display Settings"));
    d->providSelfActionList.append(tempAction);
    d->predicateAction[tr("Display Settings")] = tempAction;

    tempAction = parent->addAction(tr("Wallpaper and Screensaver"));
    d->providSelfActionList.append(tempAction);
    d->predicateAction[tr("Wallpaper and Screensaver")] = tempAction;
}

void CanvasMenuScene::normalMenu(QMenu *parent) {
    Q_UNUSED(parent)
}

QMenu *CanvasMenuScene::iconSizeSubActions(QMenu *menu)
{
    int mininum = d->view->itemDelegate()->minimumIconLevel();
    int maxinum = d->view->itemDelegate()->maximumIconLevel();

    QMenu *subMenu = new QMenu(menu);
    d->iconSizeMap.clear();

    for (int i = mininum; i <= maxinum; ++i) {
        QAction *tempAction = subMenu->addAction(d->view->itemDelegate()->iconSizeLevelDescription(i));
        d->iconSizeMap.insert(d->view->itemDelegate()->iconSizeLevelDescription(i), i);
        d->providSelfActionList.append(tempAction);
    }
    return subMenu;
}

QMenu *CanvasMenuScene::sortBySubActions(QMenu *menu)
{
    QMenu *subMenu = new QMenu(menu);

    // SortBy
    QAction *tempAction = subMenu->addAction(tr("Name"));
    d->providSelfActionList.append(tempAction);
    d->predicateAction[tr("Name")] = tempAction;

    tempAction = subMenu->addAction(tr("Time modified"));
    d->providSelfActionList.append(tempAction);
    d->predicateAction[tr("Time modified")] = tempAction;

    tempAction = subMenu->addAction(tr("Size"));
    d->providSelfActionList.append(tempAction);
    d->predicateAction[tr("Size")] = tempAction;

    tempAction = subMenu->addAction(tr("Type"));
    d->providSelfActionList.append(tempAction);
    d->predicateAction[tr("Type")] = tempAction;

    return subMenu;
}
