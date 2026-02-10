// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "packetwritingmenuscene.h"
#include "packetwritingmenuscene_p.h"
#include "utils/opticalhelper.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/schemefactory.h>

#include <QUrl>
#include <QMenu>

DPOPTICAL_BEGIN_NAMESPACE
DFMBASE_USE_NAMESPACE

PacketWritingMenuScenePrivate::PacketWritingMenuScenePrivate(PacketWritingMenuScene *qq)
    : AbstractMenuScenePrivate(qq), q(qq)

{
}

QString PacketWritingMenuScenePrivate::findSceneName(QAction *act) const
{
    QString name;
    auto childScene = workspaceScene->scene(act);
    if (childScene)
        name = childScene->name();
    return name;
}

bool PacketWritingMenuScenePrivate::isContainSubDirFile(const QString &mnt) const
{
    if (selectFiles.isEmpty() || mnt.isEmpty()) {
        fmDebug() << "No subdirectory files check - selectFiles empty:" << selectFiles.isEmpty() << "mount point empty:" << mnt.isEmpty();
        return false;
    }

    return std::any_of(selectFiles.begin(), selectFiles.end(), [mnt](const QUrl &url) {
        const QString &directory {
            url.adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash).toLocalFile()
        };
        if (directory == mnt)
            return false;
        return true;
    });
}

AbstractMenuScene *PacketWritingMenuCreator::create()
{
    return new PacketWritingMenuScene;
}

PacketWritingMenuScene::PacketWritingMenuScene(QObject *parent)
    : AbstractMenuScene(parent), d(new PacketWritingMenuScenePrivate(this))
{
}

PacketWritingMenuScene::~PacketWritingMenuScene()
{
}

QString PacketWritingMenuScene::name() const
{
    return PacketWritingMenuCreator::name();
}

bool PacketWritingMenuScene::initialize(const QVariantHash &params)
{
    d->currentDir = params.value(MenuParamKey::kCurrentDir).toUrl();
    d->workspaceScene = dynamic_cast<AbstractMenuScene *>(this->parent());
    const QString &currentPath { d->currentDir.toLocalFile() };
    auto treeSelectedUrls = params.value(MenuParamKey::kTreeSelectFiles).value<QList<QUrl>>();
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    if (!d->currentDir.isLocalFile()
        || !DeviceProxyManager::instance()->isFileFromOptical(currentPath))
        return AbstractMenuScene::initialize(params);

    // currentdir is not mountpoint
    QString dev { DeviceUtils::getMountInfo(currentPath, false) };
    QString curMnt { OpticalHelper::findMountPoint(currentPath) };
    fmDebug() << "Initial device info - dev:" << dev << "mount point:" << curMnt;

    if (dev.isEmpty()) {
        dev = DeviceUtils::getMountInfo(curMnt, false);
        d->isWorkingSubDir = true;
    }

    // work in subdir if treemode
    if (!d->isWorkingSubDir && d->isContainSubDirFile(curMnt))
        d->isWorkingSubDir = true;

    if (DeviceUtils::isPWUserspaceOpticalDiscDev(dev)) {
        d->isPackeWritingDir = true;
        d->isEmptyArea = params.value(MenuParamKey::kIsEmptyArea).toBool();
    }

    return AbstractMenuScene::initialize(params);
}

void PacketWritingMenuScene::updateState(QMenu *parent)
{
    if (!d->isPackeWritingDir) {
        fmDebug() << "Not a packet writing directory, using default menu state update";
        AbstractMenuScene::updateState(parent);
        return;
    }

    static const QStringList whiteNormalActIdList {
        "open",
        "open-with",
        "delete",
        "copy",
        "rename",
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
                                              "BookmarkMenu", "SendToMenu", "SendToDiscMenu", "OemMenu", "WorkspaceMenu" };
    auto actions = parent->actions();
    std::for_each(actions.begin(), actions.end(), [this](QAction *act) {
        if (act->isSeparator()) {
            act->setVisible(true);
            return;
        }
        QString id { act->property(ActionPropertyKey::kActionID).toString() };
        QString sceneName { d->findSceneName(act) };

        // scene filter
        if (!whiteSceneList.contains(sceneName)) {
            act->setVisible(false);
            return;
        }

        // empty area filter
        if (d->isEmptyArea) {
            if (!whiteEmptyActIdList.contains(id)) {
                act->setVisible(false);
                return;
            }

            static QStringList subdirBlackList { "paste" };
            if (d->isWorkingSubDir && subdirBlackList.contains(id)) {
                act->setVisible(false);
                return;
            }
        }

        // normal filter
        if (!d->isEmptyArea) {
            if (!whiteNormalActIdList.contains(id)) {
                act->setVisible(false);
                return;
            }

            static QStringList subdirBlackList { "rename", "delete" };
            if (d->isWorkingSubDir && subdirBlackList.contains(id)) {
                act->setDisabled(true);
                return;
            }
        }
    });

    AbstractMenuScene::updateState(parent);
}

DPOPTICAL_END_NAMESPACE
