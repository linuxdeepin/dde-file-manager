// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "menuhelper.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/utils/protocolutils.h>
#include <dfm-base/base/schemefactory.h>

#include <dfm-io/dfmio_utils.h>

#include <QDebug>

using namespace GlobalDConfDefines::ConfigPath;

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
    if (!enableProtocolDev && ProtocolUtils::isRemoteFile(dirUrl))
        hidden = true;

    // hidden by `dfm.menu.blockdev.enable`
    // NOTE: SMB mounted by cifs that mount point is local but it's a protocol device
    if (!enableBlockDev && DFMIO::DFMUtils::fileIsRemovable(dirUrl) && !ProtocolUtils::isRemoteFile(dirUrl))
        hidden = true;

    return hidden;
}

bool isHiddenMenu(const QString &app)
{
    auto hiddenMenus = DConfigManager::instance()->value(kDefaultCfgPath, "dfm.menu.hidden").toStringList();
    if (!hiddenMenus.isEmpty()) {
        if (hiddenMenus.contains(app) || ((app.startsWith("dde-select-dialog") && hiddenMenus.contains("dde-file-dialog")))) {
            fmDebug() << "menu: hidden menu in app: " << app << hiddenMenus;
            return true;
        }
    }

    if (app == "dde-desktop" || app == "org.deepin.dde-shell")
        return isHiddenDesktopMenu();

    return false;
}

bool isHiddenDesktopMenu()
{
    return Application::appObtuselySetting()->value("ApplicationAttribute", "DisableDesktopContextMenu", false).toBool();
}

bool canOpenSelectedItems(const QList<QUrl> &urls)
{
    constexpr int kMaxScanCount = 1000;
    const int kMaxDirThreshold = DFMGLOBAL_NAMESPACE::kOpenNewWindowMaxCount;

    // 快速检查：如果总数未超过阈值，直接允许
    if (urls.size() <= kMaxDirThreshold)
        return true;

    // 优化：最多只扫描前kMaxScanCount个URL
    int dirCount = 0;
    const int scanLimit = qMin(urls.size(), kMaxScanCount);

    for (int i = 0; i < scanLimit; ++i) {
        auto info = DFMBASE_NAMESPACE::InfoFactory::create<FileInfo>(
                urls[i], Global::CreateFileInfoType::kCreateFileInfoAuto);

        if (info && info->isAttributes(OptInfoType::kIsDir)) {
            if (++dirCount > kMaxDirThreshold) {
                return false;   // 超过目录阈值，需要新窗口
            }
        }
    }

    return true;   // 未超过阈值，可以正常打开
}

}   //  namespace Helper
}   //  namespace dfmplugin_menu
