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
#include "opticalmenuscene.h"
#include "opticalmenuscene_p.h"
#include "utils/opticalhelper.h"
#include "mastered/masteredmediafileinfo.h"

#include "services/common/menu/menuservice.h"
#include "services/common/menu/menu_defines.h"

DPOPTICAL_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DSC_USE_NAMESPACE

static constexpr char kWorkspaceMenuSceneName[] = "WorkspaceMenu";

AbstractMenuScene *OpticalMenuSceneCreator::create()
{
    return new OpticalMenuScene;
}

OpticalMenuScene::OpticalMenuScene(QObject *parent)
    : AbstractMenuScene(parent), d(new OpticalMenuScenePrivate(this))
{
}

OpticalMenuScene::~OpticalMenuScene()
{
}

QString OpticalMenuScene::name() const
{
    return OpticalMenuSceneCreator::name();
}

bool OpticalMenuScene::initialize(const QVariantHash &params)
{
    d->currentDir = params.value(MenuParamKey::kCurrentDir).toUrl();
    d->isEmptyArea = params.value(MenuParamKey::kIsEmptyArea).toBool();
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    if (!d->selectFiles.isEmpty())
        d->focusFile = d->selectFiles.first();

    QString backer { MasteredMediaFileInfo(d->currentDir).extraProperties()["mm_backer"].toString() };
    if (backer.isEmpty())
        d->isBlankDisc = true;

    if (!d->initializeParamsIsValid()) {
        qWarning() << "menu scene:" << name() << " init failed." << d->selectFiles.isEmpty() << d->focusFile << d->currentDir;
        return false;
    }

    QList<AbstractMenuScene *> currentScene;

    if (auto workspaceScene = MenuService::service()->createScene(kWorkspaceMenuSceneName))
        currentScene.append(workspaceScene);

    // the scene added by binding must be initializeed after 'defalut scene'.
    currentScene.append(subScene);
    setSubscene(currentScene);

    return AbstractMenuScene::initialize(params);
}

void OpticalMenuScene::updateState(QMenu *parent)
{
    AbstractMenuScene::updateState(parent);

    static const QStringList whiteNormalActIdList {
        "open",
        "open-with",
        "delete",
        "copy",
        "create-system-link",
        "send-to-desktop",
        "property",
        "open-in-new-window",
        "open-in-new-tab",
        "open-in-terminal",
        "open-as-administrator",
        "add-bookmark",
        "stage-file-to-burning",
        "set-as-wallpaper",
        "mount-image",
        ""   // for oem
    };
    static const QStringList whiteEmptyActIdList {
        "display-as",
        "sort-by",
        "open-as-administrator",
        "open-in-terminal",
        "paste",
        "select-all",
        "property",
        ""   // for oem
    };
    static const QStringList whiteSceneList { "NewCreateMenu", "ClipBoardMenu", "OpenDirMenu", "FileOperatorMenu",
                                              "OpenWithMenu", "SendToMenu", "SortAndDisplayMenu", "PropertyMenu",
                                              "BookmarkMenu", "SendToDiscMenu", "OemMenu" };

    auto actions = parent->actions();
    std::for_each(actions.begin(), actions.end(), [this](QAction *act) {
        QString id { act->property(ActionPropertyKey::kActionID).toString() };
        QString sceneName { d->findSceneName(act) };

        // scene filter
        if (!whiteSceneList.contains(sceneName))
            act->setVisible(false);

        // empty area filter
        if (d->isEmptyArea) {
            if (id == "paste" && d->enablePaste())
                act->setEnabled(true);
            if (!whiteEmptyActIdList.contains(id))
                act->setVisible(false);
            static const QStringList blankActBlackList { "open-as-administrator", "open-in-terminal" };
            if (d->isBlankDisc && blankActBlackList.contains(id))
                act->setVisible(false);
        }

        // normal filter
        if (!d->isEmptyArea) {
            if (!whiteNormalActIdList.contains(id))
                act->setVisible(false);

            static const QStringList discBlackActIdList { "delete" };
            if (OpticalHelper::burnIsOnDisc(d->focusFile) && discBlackActIdList.contains(id))
                act->setVisible(false);

            static const QStringList nativeBlackActIdList { "create-system-link", "send-to-desktop" };
            if (!OpticalHelper::burnIsOnDisc(d->focusFile) && nativeBlackActIdList.contains(id))
                act->setVisible(false);
        }

        if (act->isSeparator())
            act->setVisible(true);
    });
}

OpticalMenuScenePrivate::OpticalMenuScenePrivate(OpticalMenuScene *qq)
    : AbstractMenuScenePrivate(qq), q(qq)
{
}

QString OpticalMenuScenePrivate::findSceneName(QAction *act) const
{
    QString name;
    auto childScene = q->scene(act);
    if (childScene)
        name = childScene->name();
    return name;
}

bool OpticalMenuScenePrivate::enablePaste() const
{
    auto &&clipboard { ClipBoard::instance() };
    return clipboard->clipboardAction() != ClipBoard::kUnknownAction
            && !clipboard->clipboardFileUrlList().isEmpty();
}
