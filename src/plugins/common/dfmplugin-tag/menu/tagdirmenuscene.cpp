/*
* Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
*
* Author:     gongheng <gongheng@uniontech.com>
*
* Maintainer: zhengyouge <zhengyouge@uniontech.com>
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
#include "tagdirmenuscene.h"
#include "private/tagdirmenuscene_p.h"

#include "plugins/common/core/dfmplugin-menu/menu_eventinterface_helper.h"

#include "dfm-base/dfm_menu_defines.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/utils/sysinfoutils.h"

#include <DDesktopServices>

#include <QProcess>
#include <QMenu>

DWIDGET_USE_NAMESPACE
using namespace dfmplugin_tag;
DFMBASE_USE_NAMESPACE

TagDirMenuScenePrivate::TagDirMenuScenePrivate(TagDirMenuScene *qq)
    : AbstractMenuScenePrivate(qq),
      q(qq)
{
}

bool TagDirMenuScenePrivate::openFileLocation(const QString &path)
{
    // why? because 'DDesktopServices::showFileItem(realUrl(event->url()))' will call session bus 'org.freedesktop.FileManager1'
    // but cannot find session bus when user is root!
    if (SysInfoUtils::isRootUser()) {
        QStringList urls { path };
        return QProcess::startDetached("dde-file-manager", QStringList() << "--show-item" << urls << "--raw");
    }

    return DDesktopServices::showFileItem(path);
}

void TagDirMenuScenePrivate::updateMenu(QMenu *menu)
{
    QList<QAction *> actions = menu->actions();
    if (isEmptyArea) {
        QList<QAction *>::iterator itAction = actions.begin();
        for (; itAction != actions.end(); ++itAction) {
            if ((*itAction)->isSeparator())
                continue;

            const QString sceneName = q->scene(*itAction)->name();
            if (sceneName == "ExtendMenu" || sceneName == "OemMenu") {
                menu->removeAction(*itAction);
            }
        }
    } else {
        QAction *openLocalAct = nullptr;
        for (auto act : actions) {
            if (act->isSeparator())
                continue;

            const auto &p = act->property(ActionPropertyKey::kActionID);
            if (p == TagActionId::kOpenFileLocation) {
                openLocalAct = act;
                break;
            }
        }

        // insert 'OpenFileLocation' action
        if (openLocalAct) {
            actions.removeOne(openLocalAct);
            actions.insert(1, openLocalAct);
            menu->addActions(actions);
        }
    }
}

AbstractMenuScene *TagDirMenuCreator::create()
{
    return new TagDirMenuScene();
}

TagDirMenuScene::TagDirMenuScene(QObject *parent)
    : AbstractMenuScene(parent), d(new TagDirMenuScenePrivate(this))
{
    d->predicateName[TagActionId::kOpenFileLocation] = tr("Open file location");
}

TagDirMenuScene::~TagDirMenuScene()
{
}

QString TagDirMenuScene::name() const
{
    return TagDirMenuCreator::name();
}

bool TagDirMenuScene::initialize(const QVariantHash &params)
{
    d->currentDir = params.value(MenuParamKey::kCurrentDir).toUrl();
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    if (!d->selectFiles.isEmpty())
        d->focusFile = d->selectFiles.first();
    d->isEmptyArea = params.value(MenuParamKey::kIsEmptyArea).toBool();
    d->windowId = params.value(MenuParamKey::kWindowId).toULongLong();

    const auto &tmpParams = dfmplugin_menu_util::menuPerfectParams(params);
    d->isDDEDesktopFileIncluded = tmpParams.value(MenuParamKey::kIsDDEDesktopFileIncluded, false).toBool();
    d->isSystemPathIncluded = tmpParams.value(MenuParamKey::kIsSystemPathIncluded, false).toBool();

    QList<AbstractMenuScene *> currentScene;
    if (d->isEmptyArea) {
        if (auto newCreateScene = dfmplugin_menu_util::menuSceneCreateScene("SortAndDisplayMenu"))
            currentScene.append(newCreateScene);
    } else {
        if (auto newCreateScene = dfmplugin_menu_util::menuSceneCreateScene("WorkspaceMenu"))
            currentScene.append(newCreateScene);
    }
    setSubscene(currentScene);

    return AbstractMenuScene::initialize(params);
}

bool TagDirMenuScene::create(QMenu *parent)
{
    if (!parent)
        return false;

    if (!d->isEmptyArea) {
        QAction *tempAction = parent->addAction(d->predicateName.value(TagActionId::kOpenFileLocation));
        d->predicateAction[TagActionId::kOpenFileLocation] = tempAction;
        tempAction->setProperty(ActionPropertyKey::kActionID, QString(TagActionId::kOpenFileLocation));
    }

    return AbstractMenuScene::create(parent);
}

void TagDirMenuScene::updateState(QMenu *parent)
{
    if (!parent)
        return;

    AbstractMenuScene::updateState(parent);
    d->updateMenu(parent);
}

AbstractMenuScene *TagDirMenuScene::scene(QAction *action) const
{
    if (action == nullptr)
        return nullptr;

    if (!d->predicateAction.key(action).isEmpty())
        return const_cast<TagDirMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

bool TagDirMenuScene::triggered(QAction *action)
{
    auto actionId = action->property(ActionPropertyKey::kActionID).toString();
    if (d->predicateAction.contains(actionId)) {
        // open file location
        if (actionId == TagActionId::kOpenFileLocation) {
            for (const auto &file : d->selectFiles) {
                auto info = InfoFactory::create<AbstractFileInfo>(file);
                d->openFileLocation(info->pathOf(PathInfoType::kAbsoluteFilePath));
            }

            return true;
        }
    }

    return AbstractMenuScene::triggered(action);
}
