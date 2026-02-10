// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils.h"
#include "reportlog/virtualreportlogplugin.h"
#include "global/virtualglobalplugin.h"
#include "openwith/virtualopenwithplugin.h"
#include "appendcompress/virtualappendcompressplugin.h"
#include "bluetooth/virtualbluetoothplugin.h"
#include "extensionimpl/virtualextensionimplplugin.h"
#include "vaultassist/virtualvaulthelperplugin.h"
#include "testing/virtualtestingplugin.h"
#include "shred/vitrualshredplugin.h"

namespace dfmplugin_utils {
DFM_LOG_REGISTER_CATEGORY(DPUTILS_NAMESPACE)

static constexpr char kReportLog[] { "dfmplugin-reportlog" };
static constexpr char kAppendcompress[] { "dfmplugin-appendcompress" };
static constexpr char kBluetooth[] { "dfmplugin-bluetooth" };
static constexpr char kGlobal[] { "dfmplugin-global" };
static constexpr char kOpenWith[] { "dfmplugin-openwith" };
static constexpr char kExtensionImpl[] { "dfmplugin-extensionimpl" };
static constexpr char kVaultAssist[] { "dfmplugin-vaultassist" };
static constexpr char kTesting[] { "dfmplugin-testing" };
static constexpr char kShred[] { "dfmplugin-shred" };

QSharedPointer<DPF_NAMESPACE::Plugin> Utils::create(const QString &pluginName)
{
    fmInfo() << "Create plugin: " << pluginName;

    if (pluginName == kReportLog) {
        QSharedPointer<DPF_NAMESPACE::Plugin> plugin { new VirtualReportLogPlugin };
        return plugin;
    }

    if (pluginName == kAppendcompress) {
        QSharedPointer<DPF_NAMESPACE::Plugin> plugin { new VirtualAppendCompressPlugin };
        return plugin;
    }

    if (pluginName == kBluetooth) {
        QSharedPointer<DPF_NAMESPACE::Plugin> plugin { new VirtualBluetoothPlugin };
        return plugin;
    }

    if (pluginName == kGlobal) {
        QSharedPointer<DPF_NAMESPACE::Plugin> plugin { new VirtualGlobalPlugin };
        return plugin;
    }

    if (pluginName == kOpenWith) {
        QSharedPointer<DPF_NAMESPACE::Plugin> plugin { new VirtualOpenWithPlugin };
        return plugin;
    }

    if (pluginName == kExtensionImpl) {
        QSharedPointer<DPF_NAMESPACE::Plugin> plugin { new VirtualExtensionImplPlugin };
        return plugin;
    }

    if (pluginName == kVaultAssist) {
        QSharedPointer<DPF_NAMESPACE::Plugin> plugin { new VirtualVaultHelperPlugin };
        return plugin;
    }

    if (pluginName == kTesting) {
        QSharedPointer<DPF_NAMESPACE::Plugin> plugin { new VirtualTestingPlugin };
        return plugin;
    }

    if (pluginName == kShred) {
        QSharedPointer<DPF_NAMESPACE::Plugin> plugin { new VirtualShredPlugin };
        return plugin;
    }

    return {};
}
}   // namespace dfmplugin_utils
