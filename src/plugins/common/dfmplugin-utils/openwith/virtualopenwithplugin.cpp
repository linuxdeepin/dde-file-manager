// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "virtualopenwithplugin.h"
#include "openwith/openwithhelper.h"

using namespace dfmplugin_utils;

using CustomViewExtensionView = std::function<QWidget *(const QUrl &url)>;
Q_DECLARE_METATYPE(CustomViewExtensionView)

void VirtualOpenWithPlugin::initialize()
{
    eventReceiver->initEventConnect();
}

bool VirtualOpenWithPlugin::start()
{
    auto propertyDialogPlugin { DPF_NAMESPACE::LifeCycle::pluginMetaObj("dfmplugin-propertydialog") };
    if (propertyDialogPlugin
            && (propertyDialogPlugin->pluginState() == DPF_NAMESPACE::PluginMetaObject::kInitialized
                || propertyDialogPlugin->pluginState() == DPF_NAMESPACE::PluginMetaObject::kStarted)) {
        regViewToPropertyDialog();
    } else {
        connect(DPF_NAMESPACE::Listener::instance(), &DPF_NAMESPACE::Listener::pluginInitialized,
                this, [this](const QString &iid, const QString &name) {
                    Q_UNUSED(iid)
                    if (name == "dfmplugin-propertydialog")
                        regViewToPropertyDialog();
                },
                Qt::DirectConnection);
    }

    return true;
}

void VirtualOpenWithPlugin::regViewToPropertyDialog()
{
    CustomViewExtensionView func { OpenWithHelper::createOpenWithWidget };
    dpfSlotChannel->push("dfmplugin_propertydialog", "slot_ViewExtension_Register", func, "Virtual", 2);
}
