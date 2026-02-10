// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filedialogmenuscene.h"
#include "private/filedialogmenuscene_p.h"

#include "plugins/common/dfmplugin-menu/menuscene/action_defines.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/dfm_menu_defines.h>
#include <dfm-framework/dpf.h>

#include <QMenu>

using namespace filedialog_core;
using namespace dfmplugin_menu;
DFMBASE_USE_NAMESPACE

AbstractMenuScene *FileDialogMenuCreator::create()
{
    return new FileDialogMenuScene();
}

FileDialogMenuScene::FileDialogMenuScene(QObject *parent)
    : AbstractMenuScene(parent), d(new FileDialogMenuScenePrivate(this))
{
}

QString FileDialogMenuScene::name() const
{
    return FileDialogMenuCreator::name();
}

bool FileDialogMenuScene::initialize(const QVariantHash &params)
{
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    if (!d->selectFiles.isEmpty()) {
        d->focusFile = d->selectFiles.first();
        d->focusFileInfo = InfoFactory::create<FileInfo>(d->focusFile);
    }
    d->windowId = params.value(MenuParamKey::kWindowId).toULongLong();

    workspaceScene = dynamic_cast<AbstractMenuScene *>(this->parent());
    return AbstractMenuScene::initialize(params);
}

void FileDialogMenuScene::updateState(QMenu *parent)
{
    Q_ASSERT(parent);

    filterAction(parent, false);
    AbstractMenuScene::updateState(parent);
}

bool FileDialogMenuScene::actionFilter(AbstractMenuScene *caller, QAction *action)
{
    if (!caller || !action)
        return false;

    auto actionId = action->property(ActionPropertyKey::kActionID).toString();
    if (actionId == ActionID::kOpen) {
        auto focusFileInfo = d->focusFileInfo;
        if (!focusFileInfo || !focusFileInfo->isAttributes(OptInfoType::kIsDir))
            return false;
        QUrl cdUrl = d->focusFile;
        FileInfoPointer infoPtr = InfoFactory::create<FileInfo>(cdUrl);
        if (infoPtr && infoPtr->isAttributes(OptInfoType::kIsSymLink))
            cdUrl = QUrl::fromLocalFile(infoPtr->pathOf(PathInfoType::kSymLinkTarget));

        dpfSignalDispatcher->publish(GlobalEventType::kChangeCurrentUrl, d->windowId, cdUrl);
        return true;
    }

    return false;
}

QString FileDialogMenuScene::findSceneName(QAction *act) const
{
    QString name;
    if (workspaceScene) {
        auto childScene = workspaceScene->scene(act);
        if (childScene)
            name = childScene->name();
    }
    return name;
}

void FileDialogMenuScene::filterAction(QMenu *parent, bool isSubMenu)
{
    static const QStringList whiteActIdList { ActionID::kNewFolder, ActionID::kNewDoc,
                                              ActionID::kDisplayAs, ActionID::kSortBy, ActionID::kGroupBy,
                                              ActionID::kOpen, ActionID::kRename,
                                              ActionID::kDelete, ActionID::kCopy,
                                              ActionID::kCut, ActionID::kPaste };
    static const QStringList whiteSceneList { "NewCreateMenu", "ClipBoardMenu", "OpenDirMenu", "FileOperatorMenu",
                                              "OpenWithMenu", "ShareMenu", "SortAndDisplayMenu" };
    static const QStringList extSceneList { "ExtendMenu", "OemMenu", "ExtensionLibMenu" };

    auto actions = parent->actions();
    for (auto act : actions) {
        if (act->isSeparator()) {
            act->setVisible(true);
            continue;
        }

        QString id { act->property(ActionPropertyKey::kActionID).toString() };
        QString sceneName { findSceneName(act) };

        if (extSceneList.contains(sceneName)) {
            act->setVisible(true);
            continue;
        }

        if (isSubMenu) {
            if (!whiteSceneList.contains(sceneName))
                act->setVisible(false);
            continue;
        }

        if (!whiteActIdList.contains(id) || !whiteSceneList.contains(sceneName)) {
            act->setVisible(false);
        } else {
            auto subMenu = act->menu();
            if (subMenu)
                filterAction(subMenu, true);
        }
    }
}

FileDialogMenuScenePrivate::FileDialogMenuScenePrivate(FileDialogMenuScene *qq)
    : AbstractMenuScenePrivate(qq), q(qq)
{
}
