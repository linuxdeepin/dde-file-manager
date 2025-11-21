// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFM_PLUGIN_DEFINES_H
#define DFM_PLUGIN_DEFINES_H

#include <dfm-base/dfm_base_global.h>

#include <QStringList>

DFMBASE_BEGIN_NAMESPACE

// TODO: auto register

namespace Plugins {
namespace Core {
inline constexpr std::initializer_list<const char *> kCommon {
    "dfmplugin-bookmark", "dfmplugin-fileoperations", "dfmplugin-menu",
    "dfmplugin-propertydialog", "dfmplugin-trashcore", "dfmplugin-utils"
};
inline constexpr std::initializer_list<const char *> kCommonVirtual {
    "dfmplugin-appendcompress", "dfmplugin-bluetooth", "dfmplugin-extensionimpl",
    "dfmplugin-global", "dfmplugin-openwith", "dfmplugin-reportlog",
    "dfmplugin-testing", "dfmplugin-vaultassist", "dfmplugin-shred"
};
inline constexpr std::initializer_list<const char *> kFileManager {
    "dfmplugin-core", "dfmplugin-optical", "dfmplugin-computer",
    "dfmplugin-detailspace", "dfmplugin-sidebar", "dfmplugin-titlebar",
    "dfmplugin-workspace", "dfmplugin-smbbrowser", "dfmplugin-disk-encrypt"
};

inline constexpr std::initializer_list<const char *> kDesktop {
    "ddplugin-core", "ddplugin-canvas", "ddplugin-background", "ddplugin-organizer"
};
}   // namespace Core

namespace Edge {
inline constexpr std::initializer_list<const char *> kCommon {
    "dfmplugin-emblem", "dfmplugin-tag",
    "dfmplugin-dirshare", "dfmplugin-burn"
};
inline constexpr std::initializer_list<const char *> kFileManager {
    "dfmplugin-trash", "dfmplugin-recent", "dfmplugin-avfsbrowser",
    "dfmplugin-search", "dfmplugin-myshares", "dfmplugin-vault"
};

inline constexpr std::initializer_list<const char *> kDesktop {
    "ddplugin-wallpapersetting"
};
}   // namespace Edge

namespace Utils {
inline QStringList filemanagerCorePlugins()
{
    QStringList result;
    std::copy(Plugins::Core::kCommon.begin(), Plugins::Core::kCommon.end(), std::back_inserter(result));
    std::copy(Plugins::Core::kCommonVirtual.begin(), Plugins::Core::kCommonVirtual.end(), std::back_inserter(result));
    std::copy(Plugins::Core::kFileManager.begin(), Plugins::Core::kFileManager.end(), std::back_inserter(result));
    return result;
}

inline QStringList filemanagerAllPlugins()
{
    QStringList result { filemanagerCorePlugins() };
    std::copy(Plugins::Edge::kCommon.begin(), Plugins::Edge::kCommon.end(), std::back_inserter(result));
    std::copy(Plugins::Edge::kFileManager.begin(), Plugins::Edge::kFileManager.end(), std::back_inserter(result));
    return result;
}

inline QStringList desktopCorePlugins()
{
    QStringList result;
    std::copy(Plugins::Core::kCommon.begin(), Plugins::Core::kCommon.end(), std::back_inserter(result));
    std::copy(Plugins::Core::kCommonVirtual.begin(), Plugins::Core::kCommonVirtual.end(), std::back_inserter(result));
    std::copy(Plugins::Core::kDesktop.begin(), Plugins::Core::kDesktop.end(), std::back_inserter(result));
    return result;
}

inline QStringList desktopAllPlugins()
{
    QStringList result { desktopCorePlugins() };
    std::copy(Plugins::Edge::kCommon.begin(), Plugins::Edge::kCommon.end(), std::back_inserter(result));
    std::copy(Plugins::Edge::kDesktop.begin(), Plugins::Edge::kDesktop.end(), std::back_inserter(result));
    return result;
}

}   // namespace Utils
}   // namespace Plugins

DFMBASE_END_NAMESPACE

#endif   // DFM_PLUGIN_DEFINES_H
