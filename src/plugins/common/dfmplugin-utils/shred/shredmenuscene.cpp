// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shredmenuscene.h"
#include "shredmenuscene_p.h"
#include "shredutils.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/universalutils.h>

#include <QMenu>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_utils;

namespace ShredActionId {
inline constexpr char kFileShredKey[] = "file-shred";
}

AbstractMenuScene *ShredMenuCreator::create()
{
    return new ShredMenuScene();
}

ShredMenuScenePrivate::ShredMenuScenePrivate(ShredMenuScene *qq)
    : AbstractMenuScenePrivate(qq)
{
}

void ShredMenuScenePrivate::updateMenu(QMenu *menu)
{
    auto actions = menu->actions();
    QAction *sendToAct = nullptr;
    for (auto act : actions) {
        if (act->isSeparator())
            continue;

        const auto &p = act->property(ActionPropertyKey::kActionID);
        if (p == "send-to") {
            sendToAct = act;
            break;
        }
    }

    if (sendToAct) {
        actions.removeOne(predicateAction[ShredActionId::kFileShredKey]);
        menu->addActions(actions);

        auto act = menu->insertSeparator(sendToAct);
        menu->insertAction(act, predicateAction[ShredActionId::kFileShredKey]);
        menu->insertSeparator(predicateAction[ShredActionId::kFileShredKey]);
    }
}

ShredMenuScene::ShredMenuScene(QObject *parent)
    : AbstractMenuScene(parent),
      d(new ShredMenuScenePrivate(this))
{
}

QString ShredMenuScene::name() const
{
    return ShredMenuCreator::name();
}

bool ShredMenuScene::initialize(const QVariantHash &params)
{
    d->currentDir = params.value(MenuParamKey::kCurrentDir).toUrl();
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    if (!d->selectFiles.isEmpty())
        d->focusFile = d->selectFiles.first();
    d->isEmptyArea = params.value(MenuParamKey::kIsEmptyArea).toBool();
    d->windowId = params.value(MenuParamKey::kWindowId).toULongLong();

    d->predicateName.insert(ShredActionId::kFileShredKey, tr("File Shred"));
    d->focusFileInfo = DFMBASE_NAMESPACE::InfoFactory::create<FileInfo>(d->focusFile, Global::CreateFileInfoType::kCreateFileInfoAuto);

    return AbstractMenuScene::initialize(params);
}

bool ShredMenuScene::create(QMenu *parent)
{
    if (!ShredUtils::instance()->isShredEnabled())
        return false;

    if (!parent || d->isEmptyArea)
        return false;

    if (UrlRoute::isVirtual(d->currentDir)) {
        QUrl targetUrl;
        UniversalUtils::urlTransformToLocal(d->currentDir, &targetUrl);
        if (targetUrl.scheme() != Global::Scheme::kFile)
            return false;
    }

    if (d->focusFileInfo.isNull() || !ShredUtils::instance()->isValidFile(d->focusFile))
        return false;

    const auto &currentPath = FileUtils::bindPathTransform(d->currentDir.toLocalFile(), false);
    if (currentPath == StandardPaths::location(StandardPaths::kHomePath)) {
        auto iter = std::find_if_not(d->selectFiles.cbegin(), d->selectFiles.cend(),
                                     [](const QUrl &url) {
                                         return ShredUtils::instance()->isValidFile(url);
                                     });
        if (iter != d->selectFiles.cend())
            return false;
    }

    auto act = parent->addAction(d->predicateName.value(ShredActionId::kFileShredKey));
    act->setProperty(ActionPropertyKey::kActionID, QString(ShredActionId::kFileShredKey));
    d->predicateAction.insert(ShredActionId::kFileShredKey, act);

    return AbstractMenuScene::create(parent);
}

void ShredMenuScene::updateState(QMenu *parent)
{
    if (d->predicateAction.isEmpty())
        return;

    d->updateMenu(parent);
}

bool ShredMenuScene::triggered(QAction *action)
{
    if (!d->predicateAction.values().contains(action))
        return AbstractMenuScene::triggered(action);

    QString id = d->predicateAction.key(action);
    if (id == ShredActionId::kFileShredKey) {
        if (d->focusFile.scheme() != Global::Scheme::kFile) {
            QList<QUrl> localUrlList;
            std::transform(d->selectFiles.cbegin(), d->selectFiles.cend(), std::back_inserter(localUrlList),
                           [](const QUrl &url) {
                               QUrl target;
                               UniversalUtils::urlTransformToLocal(url, &target);
                               return target;
                           });
            ShredUtils::instance()->shredfile(localUrlList, d->windowId);
        } else {
            ShredUtils::instance()->shredfile(d->selectFiles, d->windowId);
        }
    }

    return true;
}

AbstractMenuScene *ShredMenuScene::scene(QAction *action) const
{
    if (action == nullptr)
        return nullptr;

    if (!d->predicateAction.key(action).isEmpty())
        return const_cast<ShredMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}
