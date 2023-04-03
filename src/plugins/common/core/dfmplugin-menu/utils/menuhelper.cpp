// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "menuhelper.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>

#include <dfm-io/dfmio_utils.h>

#include <QGSettings>
#include <QDebug>

namespace dfmplugin_menu {
namespace Helper {

DFMBASE_USE_NAMESPACE

bool isHiddenExtMenu(const QUrl &dirUrl)
{
    Q_ASSERT(dirUrl.isValid());

    const auto &hiddenMenus { DConfigManager::instance()->value(kDefaultCfgPath, "dfm.menu.hidden").toStringList() };
    bool enableProtocolDev { DConfigManager::instance()
                                     ->value(kDefaultCfgPath, "dfm.menu.protocoldev.enable", true)
                                     .toBool() };
    bool enableBlockDev { DConfigManager::instance()
                                  ->value(kDefaultCfgPath, "dfm.menu.blockdev.enable", true)
                                  .toBool() };

    // hidden by `dfm.menu.hidden`
    bool hidden { false };
    if (hiddenMenus.contains("extension-menu"))
        hidden = true;

    // hidden by `dfm.menu.protocoldev.enable`
    if (!enableProtocolDev && FileUtils::isGvfsFile(dirUrl))
        hidden = true;

    // hidden by `dfm.menu.blockdev.enable`
    // NOTE: SMB mounted by cifs that mount point is local but it's a protocol device
    if (!enableBlockDev && DFMIO::DFMUtils::fileIsRemovable(dirUrl) && !FileUtils::isGvfsFile(dirUrl))
        hidden = true;

    return hidden;
}

bool isHiddenMenu(const QString &app)
{
    auto hiddenMenus = DConfigManager::instance()->value(kDefaultCfgPath, "dfm.menu.hidden").toStringList();
    if (!hiddenMenus.isEmpty()) {
        if (hiddenMenus.contains(app) || ((app.startsWith("dde-select-dialog") && hiddenMenus.contains("dde-file-dialog")))) {
            qDebug() << "menu: hidden menu in app: " << app << hiddenMenus;
            return true;
        }
    }

    if (app == "dde-desktop")
        return isHiddenDesktopMenu();

    return false;
}

bool isHiddenDesktopMenu()
{
    // the gsetting control is higher than json profile. it doesn't check json profile if there is gsetting value.
    if (QGSettings::isSchemaInstalled("com.deepin.dde.filemanager.desktop")) {
        QGSettings set("com.deepin.dde.filemanager.desktop", "/com/deepin/dde/filemanager/desktop/");
        QVariant var = set.get("contextMenu");
        if (var.isValid())
            return !var.toBool();
    }

    return Application::appObtuselySetting()->value("ApplicationAttribute", "DisableDesktopContextMenu", false).toBool();
}

}   //  namespace Helper
}   //  namespace dfmplugin_menu
