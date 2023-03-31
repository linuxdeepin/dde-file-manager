// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filedialogmenuscene.h"

#include <dfm-base/dfm_menu_defines.h>

#include <QMenu>

using namespace filedialog_core;
DFMBASE_USE_NAMESPACE

AbstractMenuScene *FileDialogMenuCreator::create()
{
    return new FileDialogMenuScene();
}

FileDialogMenuScene::FileDialogMenuScene(QObject *parent)
    : AbstractMenuScene(parent)
{
}

QString FileDialogMenuScene::name() const
{
    return FileDialogMenuCreator::name();
}

bool FileDialogMenuScene::initialize(const QVariantHash &params)
{
    Q_UNUSED(params)

    workspaceScene = dynamic_cast<AbstractMenuScene *>(this->parent());
    return AbstractMenuScene::initialize(params);
}

void FileDialogMenuScene::updateState(QMenu *parent)
{
    Q_ASSERT(parent);

    filterAction(parent, false);
    AbstractMenuScene::updateState(parent);
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
    // TODO(zhangs): add whitelist by global config
    static const QStringList whiteActIdList { "new-folder", "new-document", "display-as", "sort-by",
                                              "open", "rename", "delete", "copy", "cut", "paste" };
    static const QStringList whiteSceneList { "NewCreateMenu", "ClipBoardMenu", "OpenDirMenu", "FileOperatorMenu",
                                              "OpenWithMenu", "ShareMenu", "SortAndDisplayMenu" };

    auto actions = parent->actions();
    for (auto act : actions) {
        if (act->isSeparator()) {
            act->setVisible(true);
            continue;
        }

        QString id { act->property(ActionPropertyKey::kActionID).toString() };
        QString sceneName { findSceneName(act) };
        if (!isSubMenu) {
            if (!whiteActIdList.contains(id) || !whiteSceneList.contains(sceneName)) {
                act->setVisible(false);
            } else {
                auto subMenu = act->menu();
                if (subMenu)
                    filterAction(subMenu, true);
            }
        } else {
            if (!whiteSceneList.contains(sceneName))
                act->setVisible(false);
        }
    }
}
