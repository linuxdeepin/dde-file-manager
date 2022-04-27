/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "filedialogmenuscene.h"

#include "services/common/menu/menu_defines.h"

#include <QMenu>

DSC_USE_NAMESPACE
DIALOGCORE_USE_NAMESPACE
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
    return true;
}

void FileDialogMenuScene::updateState(QMenu *parent)
{
    Q_ASSERT(parent);
    // TODO(zhangs): add whitelist by global config
    static QString lineActId { "separator-line" };
    static const QStringList whiteActIdList { "new-folder", "new-document", "display-as", "sort-by",
                                              "open", "rename", "delete", "copy", "cut", "paste" };
    static const QStringList whiteSceneList { "NewCreateMenu", "ClipBoardMenu", "OpenDirMenu", "FileOperatorMenu",
                                              "OpenWithMenu", "SendToMenu", "SortAndDisplayMenu" };

    auto actions = parent->actions();
    std::for_each(actions.begin(), actions.end(), [this](QAction *act) {
        QString id { act->property(ActionPropertyKey::kActionID).toString() };
        QString sceneName { findSceneName(act) };
        if (!whiteActIdList.contains(id) || !whiteSceneList.contains(sceneName))
            act->setVisible(false);
        if (id == lineActId)
            act->setVisible(true);
    });
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
