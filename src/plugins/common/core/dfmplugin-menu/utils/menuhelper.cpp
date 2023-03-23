// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "menuhelper.h"

#include "dfm-base/base/configs/dconfig/dconfigmanager.h"
#include "dfm-base/utils/fileutils.h"

#include <dfm-io/dfmio_utils.h>

namespace dfmplugin_menu {
namespace Helper {

DFMBASE_USE_NAMESPACE

bool isHiddenExtMenuByDConfig(const QUrl &dirUrl)
{
    Q_ASSERT(dirUrl.isValid());

    const auto &hiddenMenus { DConfigManager::instance()->value(kDefaultCfgPath, "dfm.menu.hidden").toStringList() };
    bool enableProtocolDev { DConfigManager::instance()
                                     ->value(kDefaultCfgPath, "dfd.menu.protocoldev.enable", true)
                                     .toBool() };
    bool enableBlockDev { DConfigManager::instance()
                                  ->value(kDefaultCfgPath, "dfd.menu.blockdev.enable", true)
                                  .toBool() };

    // hidden by `dfm.menu.hidden`
    bool hidden { false };
    if (hiddenMenus.contains("extension-menu"))
        hidden = true;

    // hidden by `dfd.menu.protocoldev.enable`
    if (!enableProtocolDev && FileUtils::isGvfsFile(dirUrl))
        hidden = true;

    // hidden by `dfd.menu.blockdev.enable`
    // NOTE: SMB mounted by cifs that mount point is local but it's a protocol device
    if (!enableBlockDev && DFMIO::DFMUtils::fileIsRemovable(dirUrl) && !FileUtils::isGvfsFile(dirUrl))
        hidden = true;

    return hidden;
}

}   //  namespace Helper
}   //  namespace dfmplugin_menu
