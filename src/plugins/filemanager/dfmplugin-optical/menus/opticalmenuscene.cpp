// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "opticalmenuscene.h"
#include "opticalmenuscene_p.h"
#include "utils/opticalhelper.h"
#include "mastered/masteredmediafileinfo.h"

#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/universalutils.h>

#include <QMenu>

using namespace dfmplugin_optical;
DFMBASE_USE_NAMESPACE

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
    if (!d->selectFiles.isEmpty()) {
        d->focusFile = d->selectFiles.first();
        d->focusFileInfo = DFMBASE_NAMESPACE::InfoFactory::create<FileInfo>(d->focusFile);
    }

    QString backer { MasteredMediaFileInfo(d->currentDir).extraProperties()["mm_backer"].toString() };
    if (backer.isEmpty())
        d->isBlankDisc = true;

    if (!d->initializeParamsIsValid()) {
        fmWarning() << "Menu scene initialization failed - invalid parameters:" << name()
                    << "selectFiles empty:" << d->selectFiles.isEmpty()
                    << "focusFile:" << d->focusFile << "currentDir:" << d->currentDir;
        return false;
    }

    QList<AbstractMenuScene *> currentScene;

    if (auto workspaceScene = dfmplugin_menu_util::menuSceneCreateScene(kWorkspaceMenuSceneName))
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
        "send-to",
        "property",
        "open-in-new-window",
        "open-in-new-tab",
        "open-in-terminal",
        "open-as-administrator",
        "add-bookmark",
        "stage-file-to-burning",
        "set-as-wallpaper",
        "mount-image",
        "tag-color-list",
        "tag-add"
        ""   // for oem
    };
    static const QStringList whiteEmptyActIdList {
        "display-as",
        "sort-by",
        "group-by",
        "open-as-administrator",
        "open-in-terminal",
        "paste",
        "refresh",
        "select-all",
        "property",
        ""   // for oem
    };
    static const QStringList whiteSceneList { "NewCreateMenu", "ClipBoardMenu", "OpenDirMenu", "FileOperatorMenu",
                                              "OpenWithMenu", "ShareMenu", "SortAndDisplayMenu", "PropertyMenu",
                                              "BookmarkMenu", "SendToMenu", "SendToDiscMenu", "OemMenu", "WorkspaceMenu",
                                              "TagMenu" };

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

            // cannot deletes file in disc
            static const QStringList discBlackActIdList { "delete" };
            if (OpticalHelper::burnIsOnDisc(d->focusFile) && discBlackActIdList.contains(id))
                act->setVisible(false);

            // cannot "send-to" for files to be burned
            static const QStringList nativeBlackActIdList { "send-to" };
            if (!OpticalHelper::burnIsOnDisc(d->focusFile) && nativeBlackActIdList.contains(id))
                act->setVisible(false);

            if (d->focusFileInfo && d->focusFileInfo->isAttributes(OptInfoType::kIsDir)) {
                // cannot "open-*" for dirs to be burned
                static const QStringList nativeDirBlackActIdList { "open-as-administrator", "open-in-terminal", "add-bookmark" };
                if (!OpticalHelper::burnIsOnDisc(d->focusFile) && nativeDirBlackActIdList.contains(id))
                    act->setVisible(false);
            }
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
    if (!OpticalHelper::isBurnEnabled()) {
        fmDebug() << "Paste disabled - burn functionality not enabled";
        return false;
    }

    const QString &dev { OpticalHelper::burnDestDevice(currentDir) };
    const QUrl &rootUrl { OpticalHelper::discRoot(dev) };
    if (!UniversalUtils::urlEquals(rootUrl, currentDir)) {
        fmDebug() << "Paste disabled - current dir is not disc root, root:" << rootUrl << "current:" << currentDir;
        return false;
    }

    auto &&clipboard { ClipBoard::instance() };
    return clipboard->clipboardAction() != ClipBoard::kUnknownAction
            && !clipboard->clipboardFileUrlList().isEmpty();
}
