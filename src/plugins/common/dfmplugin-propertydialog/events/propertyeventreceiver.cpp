// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "propertyeventreceiver.h"
#include "utils/propertydialogutil.h"
#include "utils/propertydialogmanager.h"

#include <dfm-framework/dpf.h>

using namespace dfmplugin_propertydialog;

PropertyEventReceiver::PropertyEventReceiver(QObject *parent)
    : QObject(parent)
{
}

PropertyEventReceiver *PropertyEventReceiver::instance()
{
    static PropertyEventReceiver receiver;
    return &receiver;
}

void PropertyEventReceiver::bindEvents()
{
    dpfSlotChannel->connect(DPF_MACRO_TO_STR(DPPROPERTYDIALOG_NAMESPACE), "slot_PropertyDialog_Show",
                            this, &PropertyEventReceiver::handleShowPropertyDialog);

    dpfSlotChannel->connect(DPF_MACRO_TO_STR(DPPROPERTYDIALOG_NAMESPACE), "slot_ViewExtension_Register",
                            this, &PropertyEventReceiver::handleViewExtensionRegister);

    dpfSlotChannel->connect(DPF_MACRO_TO_STR(DPPROPERTYDIALOG_NAMESPACE), "slot_CustomView_Register",
                            this, &PropertyEventReceiver::handleCustomViewRegister);

    dpfSlotChannel->connect(DPF_MACRO_TO_STR(DPPROPERTYDIALOG_NAMESPACE), "slot_BasicViewExtension_Register",
                            this, &PropertyEventReceiver::handleBasicViewExtensionRegister);

    dpfSlotChannel->connect(DPF_MACRO_TO_STR(DPPROPERTYDIALOG_NAMESPACE), "slot_BasicFiledFilter_Add",
                            this, &PropertyEventReceiver::handleBasicFiledFilterAdd);
}

void PropertyEventReceiver::handleShowPropertyDialog(const QList<QUrl> &urls, const QVariantHash &option)
{
    PropertyDialogUtil *fileDialogManager = PropertyDialogUtil::instance();

    QVariantHash showViewOption;
    if (!option.isEmpty()) {   // Need adjust the registered initOption.
        const QString &name = option.value(kOption_Key_Name).toString();   // The `name` was registed when registered view creator.
        showViewOption = PropertyDialogManager::instance().getCreatorOptionByName(name);   // get the registered initial option
        const QStringList &initialOptKeys = showViewOption.keys();
        const QStringList &inputOptKeys = option.keys();
        for (const QString &key : inputOptKeys) {
            if (initialOptKeys.contains(key)) {
                showViewOption.insert(key, option.value(key));   // update the default option by the input option
            }
        }
    }
    fileDialogManager->showPropertyDialog(urls, showViewOption);   // Show the property dialog with updated option
}

bool PropertyEventReceiver::handleViewExtensionRegister(CustomViewExtensionView view, const QString &name, int index)
{
    return PropertyDialogManager::instance().registerExtensionView(view, name, index);
}

bool PropertyEventReceiver::handleCustomViewRegister(CustomViewExtensionView view, const QString &scheme)
{
    return PropertyDialogManager::instance().registerCustomView(view, scheme);
}

bool PropertyEventReceiver::handleBasicViewExtensionRegister(BasicViewFieldFunc func, const QString &scheme)
{
    return PropertyDialogManager::instance().registerBasicViewExtension(func, scheme);
}

bool PropertyEventReceiver::handleBasicFiledFilterAdd(const QString &scheme, const QStringList &enums)
{
    QMetaEnum &&metaState { QMetaEnum::fromType<PropertyFilterType>() };
    QString &&join { enums.join("|") };
    bool ok { false };

    auto &&enumValues { static_cast<PropertyFilterType>(metaState.keysToValue(join.toUtf8().constData(), &ok)) };
    if (ok)
        ok = PropertyDialogManager::instance().addBasicFiledFiltes(scheme, enumValues);

    return ok;
}
