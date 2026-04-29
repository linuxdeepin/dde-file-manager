// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "virtualopenwithplugin.h"
#include "openwith/openwithhelper.h"
#include "openwith/openwithwidget.h"

using namespace dfmplugin_utils;

using CustomViewExtensionView = std::function<QWidget *(const QUrl &url)>;
using ViewExtensionUpdateFunc = std::function<void(QWidget *widget, const QUrl &url)>;
Q_DECLARE_METATYPE(CustomViewExtensionView)
Q_DECLARE_METATYPE(ViewExtensionUpdateFunc)

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
    CustomViewExtensionView create { OpenWithHelper::createOpenWithWidget };
    ViewExtensionUpdateFunc update { [](QWidget *widget, const QUrl &url) {
        auto *openWidget = qobject_cast<OpenWithWidget *>(widget);
        if (!openWidget)
            return;
        openWidget->selectFileUrl(url);
    } };
    dpfSlotChannel->push("dfmplugin_propertydialog", "slot_ViewExtensionWithUpdate_Register", create, update, QString("Virtual"), 2);
}
