// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "extensionlibmenuscene.h"
#include "dfmextmenuimpl.h"
#include "private/extensionlibmenuscene_p.h"
#include "extensionimpl/pluginsload/extensionpluginmanager.h"

#include "plugins/common/core/dfmplugin-menu/menu_eventinterface_helper.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/utils/universalutils.h>

#include <QMenu>

namespace dfmplugin_utils {
DFMBASE_USE_NAMESPACE

dfmbase::AbstractMenuScene *ExtensionLibMenuSceneCreator::create()
{
    return new ExtensionLibMenuScene;
}

ExtensionLibMenuScenePrivate::ExtensionLibMenuScenePrivate(ExtensionLibMenuScene *qq)
    : AbstractMenuScenePrivate(qq),
      q(qq)
{
}

ExtensionLibMenuScene::ExtensionLibMenuScene(QObject *parent)
    : AbstractMenuScene(parent), d(new ExtensionLibMenuScenePrivate(this))
{
}

ExtensionLibMenuScene::~ExtensionLibMenuScene()
{
}

QString ExtensionLibMenuScene::name() const
{
    return ExtensionLibMenuSceneCreator::name();
}

bool ExtensionLibMenuScene::initialize(const QVariantHash &params)
{
    // request load extension plugins
    if (ExtensionPluginManager::instance().currentState() != ExtensionPluginManager::kInitialized)
        emit ExtensionPluginManager::instance().requestInitlaizePlugins();

    // init default info
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

    if (!d->initializeParamsIsValid()) {
        qWarning() << "menu scene:" << name() << " init failed." << d->selectFiles.isEmpty() << d->focusFile << d->currentDir;
        return false;
    }

    return AbstractMenuScene::initialize(params);
}

bool ExtensionLibMenuScene::create(QMenu *parent)
{
    if (!parent)
        return false;

    if (ExtensionPluginManager::instance().currentState() != ExtensionPluginManager::kInitialized) {
        qWarning() << "Extension loading...";
        return false;
    }

    DFMExtMenuImpl *extMenuImpl { new DFMExtMenuImpl(parent) };
    const std::string &newCurrentPath { d->transformedCurrentDir.toLocalFile().toStdString() };
    const std::string &newFocusPath { d->transformedFocusFile.toLocalFile().toStdString() };

    for (auto menu : ExtensionPluginManager::instance().menuPlugins()) {
        menu->initialize(ExtensionPluginManager::instance().pluginMenuProxy());
        if (d->isEmptyArea) {
            menu->buildEmptyAreaMenu(extMenuImpl, newCurrentPath, d->onDesktop);
        } else {
            std::list<std::string> newSelectedPaths;
            std::for_each(d->transformedSelectFiles.cbegin(), d->transformedSelectFiles.cend(), [&newSelectedPaths](const QUrl &url) {
                newSelectedPaths.push_back(url.toLocalFile().toStdString());
            });
            menu->buildNormalMenu(extMenuImpl, newCurrentPath, newFocusPath, newSelectedPaths, d->onDesktop);
        }
    }

    return AbstractMenuScene::create(parent);
}

void ExtensionLibMenuScene::updateState(QMenu *parent)
{
    if (!parent)
        return;

    return AbstractMenuScene::updateState(parent);
}

bool ExtensionLibMenuScene::triggered(QAction *action)
{
    return AbstractMenuScene::triggered(action);
}

AbstractMenuScene *ExtensionLibMenuScene::scene(QAction *action) const
{
    if (action == nullptr)
        return nullptr;

    if (!d->predicateAction.key(action).isEmpty())
        return const_cast<ExtensionLibMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

}   // namespace dfmplugin_utils
