// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "private/oemmenuscene_p.h"

#include "utils/menuhelper.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/universalutils.h>

#include <dfm-framework/dpf.h>

#include <QMenu>
#include <QDebug>

using namespace dfmplugin_menu;
DFMBASE_USE_NAMESPACE

AbstractMenuScene *OemMenuCreator::create()
{
    std::call_once(loadFlag, [this]() {
        oemMenu = new OemMenu(this);
        oemMenu->loadDesktopFile();
        fmInfo() << "oem menus *.desktop loaded.";
    });

    return new OemMenuScene(oemMenu);
}

OemMenuScenePrivate::OemMenuScenePrivate(OemMenuScene *qq)
    : AbstractMenuScenePrivate(qq)
{
}

QList<QAction *> OemMenuScenePrivate::childActions(QAction *action)
{
    QList<QAction *> actions;

    if (action->menu()) {
        auto tempChildActions = action->menu()->actions();
        for (auto childAction : tempChildActions) {
            actions << childAction;
            actions << childActions(childAction);
        }
    }

    return actions;
}

OemMenuScene::OemMenuScene(OemMenu *oem, QObject *parent)
    : AbstractMenuScene(parent),
      d(new OemMenuScenePrivate(this))
{
    Q_ASSERT(oem);
    d->oemMenu = oem;
}

QString OemMenuScene::name() const
{
    return OemMenuCreator::name();
}

bool OemMenuScene::initialize(const QVariantHash &params)
{
    d->currentDir = params.value(MenuParamKey::kCurrentDir).toUrl();
    UniversalUtils::urlTransformToLocal(d->currentDir, &d->transformedCurrentDir);
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    UniversalUtils::urlsTransformToLocal(d->selectFiles, &d->transformedSelectFiles);
    Q_ASSERT(d->selectFiles.size() == d->transformedSelectFiles.size());
    if (!d->selectFiles.isEmpty())
        d->focusFile = d->selectFiles.first();
    if (!d->transformedSelectFiles.isEmpty())
        d->transformedFocusFile = d->transformedSelectFiles.first();

    d->onDesktop = params.value(MenuParamKey::kOnDesktop).toBool();
    d->isEmptyArea = params.value(MenuParamKey::kIsEmptyArea).toBool();
    d->indexFlags = params.value(MenuParamKey::kIndexFlags).value<Qt::ItemFlags>();
    d->windowId = params.value(MenuParamKey::kWindowId).toULongLong();

    if (!d->initializeParamsIsValid()) {
        fmWarning() << "menu scene:" << name() << " init failed." << d->selectFiles.isEmpty() << d->focusFile << d->currentDir;
        return false;
    }

    if (!d->isEmptyArea) {
        QString errString;
        d->focusFileInfo = DFMBASE_NAMESPACE::InfoFactory::create<FileInfo>(d->focusFile, Global::CreateFileInfoType::kCreateFileInfoAuto, &errString);
        if (d->focusFileInfo.isNull()) {
            fmDebug() << errString;
            return false;
        }
    }

    return AbstractMenuScene::initialize(params);
}

AbstractMenuScene *OemMenuScene::scene(QAction *action) const
{
    if (!action)
        return nullptr;

    if (d->oemActions.contains(action) || d->oemChildActions.contains(action))
        return const_cast<OemMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

bool OemMenuScene::create(QMenu *parent)
{
    d->oemActions.clear();
    d->oemChildActions.clear();

    if (d->isEmptyArea)
        d->oemActions = d->oemMenu->emptyActions(d->currentDir, d->onDesktop);
    else
#ifdef MENU_CHECK_FOCUSONLY
        d->oemActions = d->oemMenu->focusNormalActions(d->focusFile, d->selectFiles, d->onDesktop);
#else
        d->oemActions = d->oemMenu->normalActions(d->selectFiles, d->onDesktop);
#endif

    for (auto action : d->oemActions) {
        // reset status
        action->setVisible(true);
        action->setEnabled(true);
        parent->addAction(action);
        d->oemChildActions.append(d->childActions(action));
    }

    return AbstractMenuScene::create(parent);
}

void OemMenuScene::updateState(QMenu *parent)
{
    if (!parent)
        return;

    AbstractMenuScene::updateState(parent);
}

bool OemMenuScene::triggered(QAction *action)
{
    if (!d->oemActions.contains(action) && !d->oemChildActions.contains(action))
        return AbstractMenuScene::triggered(action);

    QPair<QString, QStringList> runable = d->oemMenu->makeCommand(
            action, d->transformedCurrentDir, d->transformedFocusFile, d->transformedSelectFiles);
    if (!runable.first.isEmpty())
        return UniversalUtils::runCommand(runable.first, runable.second);

    return AbstractMenuScene::triggered(action);
}
