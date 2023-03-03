// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "smbintcomputermenuscene.h"
#include "smbintcomputermenuscene_p.h"
#include "dfmplugin_smbbrowser_global.h"
#include "smbintegration/smbintegrationmanager.h"

#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/dfm_menu_defines.h"

#include <QMenu>

using namespace dfmplugin_smbbrowser;
DFMBASE_USE_NAMESPACE

AbstractMenuScene *SmbIntComputerMenuCreator::create()
{
    return new SmbIntComputerMenuScene();
}

SmbIntComputerMenuScene::SmbIntComputerMenuScene(QObject *parent)
    : dfmbase::AbstractMenuScene(parent), d(new SmbIntComputerMenuScenePrivate(this))
{
}

SmbIntComputerMenuScene::~SmbIntComputerMenuScene()
{
}

QString SmbIntComputerMenuScene::name() const
{
    return SmbIntComputerMenuCreator::name();
}

bool SmbIntComputerMenuScene::initialize(const QVariantHash &params)
{
    d->windowId = params.value(MenuParamKey::kWindowId).toULongLong();
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();

    if (d->selectFiles.count() == 1 && d->selectFiles.first().path().endsWith(".smbinteg"))
        return true;
    return false;
}

bool SmbIntComputerMenuScene::create(QMenu *parent)
{
    parent->clear();   // smb integration item in computer has its own menu actions.
    d->info.reset(new EntryFileInfo(d->selectFiles.first()));
    QMenu *menu = new QMenu;

    quint64 windowId = d->windowId;
    QUrl url = d->selectFiles.first();
    QUrl temUrl;
    temUrl.setScheme(Global::Scheme::kSmb);
    temUrl.setHost(url.host());
    auto newWindowAct = menu->addAction(QObject::tr("Unmount"), [windowId, url, temUrl]() {
        SmbIntegrationManager::instance()->umountAllProtocolDevice(windowId, url, false);
        SmbIntegrationManager::instance()->removeStashedIntegrationFromConfig(temUrl);
    });
    newWindowAct->setEnabled(true);

    auto newTabAct = menu->addAction(QObject::tr("Clear saved password and unmount"), [windowId, url, temUrl]() {
        SmbIntegrationManager::instance()->umountAllProtocolDevice(windowId, url, true);
        SmbIntegrationManager::instance()->removeStashedIntegrationFromConfig(temUrl);
    });
    newTabAct->setEnabled(true);

    menu->exec(QCursor::pos());
    delete menu;

    return AbstractMenuScene::create(parent);
}

void SmbIntComputerMenuScene::updateState(QMenu *parent)
{
    return AbstractMenuScene::updateState(parent);
}

bool SmbIntComputerMenuScene::triggered(QAction *action)
{
    return AbstractMenuScene::triggered(action);
}

AbstractMenuScene *SmbIntComputerMenuScene::scene(QAction *action) const
{
    if (action == nullptr)
        return nullptr;

    if (!d->predicateAction.key(action).isEmpty())
        return const_cast<SmbIntComputerMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

SmbIntComputerMenuScenePrivate::SmbIntComputerMenuScenePrivate(SmbIntComputerMenuScene *qq)
    : AbstractMenuScenePrivate(qq)
{
}
