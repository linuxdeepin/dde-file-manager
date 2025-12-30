// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "propertydialogmanager.h"
#include "utils/computerpropertyhelper.h"

#include <dfm-framework/event/eventhelper.h>

#include <QDebug>
#include <QMetaEnum>
#include <QUrl>

using namespace dfmplugin_propertydialog;

PropertyDialogManager::PropertyDialogManager(QObject *parent)
    : QObject(parent)
{
}

PropertyDialogManager &PropertyDialogManager::instance()
{
    static PropertyDialogManager ins;
    return ins;
}

bool PropertyDialogManager::registerExtensionView(CustomViewExtensionView viewCreator, const QString &name, int index)
{
    fmInfo() << "Model " << name << " register to property dialog, index at " << index;
    while (creatorOptions.keys().contains(index) && index != -1) {   // FIXME(whoever): using a number passed by user as the key is not reliable, refact the mechanism.
        fmInfo() << "The current index has registered the associated construction class" << index << name;
        index++;   // NOTE(xust): this is a temp solution to solve the issue that the share control widget not shown in property dialog.
        //        return false;
    }

    // 1. Different models have different `viewCreator`, `name` and `index`;
    // 2. Here give basic widget a initial expand state : true;
    // 3. The initial expand state of added view is false;
    // 4. When the external model(plugin) call the `slot_PropertyDialog_Show`, it can transfers a option value to
    // adjust the field value of initOption, for example, to set a function pointer ViewIntiCallback to update the UI.
    ViewIntiCallback viewInitiCb { nullptr };
    QVariantHash initOption = {
        { kOption_Key_Name, name },
        { kOption_Key_CreatorCalback, QVariant::fromValue(viewCreator) },
        { kOption_Key_ViewIndex, index },
        { kOption_Key_BasicInfoExpand, true },
        { kOption_Key_ExtendViewExpand, false },
        { kOption_Key_DisableCustomDialog, false },
        { kOption_Key_ViewInitCalback, QVariant::fromValue(viewInitiCb) }
    };
    // Store a initial option, it would be updated in `kOption_Key_ViewInitCalback` if needed.
    creatorOptions.insert(index, initOption);
    return true;
}

QMap<int, QWidget *> PropertyDialogManager::createExtensionView(const QUrl &url, const QVariantHash &option)
{
    QMap<int, QWidget *> temp {};
    auto keys { creatorOptions.keys() };
    for (int index : keys) {
        auto &&values { creatorOptions.values(index) };
        for (const QVariantHash &data : values) {
            QVariantHash showViewOption;
            ViewIntiCallback viewInitCallback = nullptr;

            if (option.isEmpty()) {
                showViewOption = data;
            } else {
                if (option.value(kOption_Key_Name).toString() == data.value(kOption_Key_Name).toString()) {
                    showViewOption = option;
                    viewInitCallback = DPF_NAMESPACE::paramGenerator<ViewIntiCallback>(option.value(kOption_Key_ViewInitCalback));
                } else {
                    showViewOption = data;
                }
            }

            CustomViewExtensionView creatorCallback = DPF_NAMESPACE::paramGenerator<CustomViewExtensionView>(showViewOption.value(kOption_Key_CreatorCalback));
            if (!creatorCallback)
                continue;

            QWidget *g = creatorCallback(url);
            if (g != nullptr) {
                if (viewInitCallback)
                    viewInitCallback(g, showViewOption);

                temp.insert(index, g);
            }
        }
    }

    return temp;
}

bool PropertyDialogManager::registerCustomView(CustomViewExtensionView view, const QString &scheme)
{
    if (viewCreateFunctionHash.keys().contains(scheme)) {
        fmInfo() << "The current index has registered";
        return false;
    }

    viewCreateFunctionHash.insert(scheme, view);
    return true;
}

QWidget *PropertyDialogManager::createCustomView(const QUrl &url)
{
    for (auto creator : viewCreateFunctionHash.values()) {
        if (creator) {
            auto wid = creator(url);
            if (wid)
                return wid;
        }
    }
    return nullptr;
}

bool PropertyDialogManager::registerBasicViewExtension(BasicViewFieldFunc func, const QString &scheme)
{
    if (!basicViewFieldFuncHash.contains(scheme)) {
        basicViewFieldFuncHash.insert(scheme, func);
        return true;
    }

    fmInfo() << "The current scheme has registered the associated construction class";
    return false;
}

QMap<BasicExpandType, BasicExpandMap> PropertyDialogManager::createBasicViewExtensionField(const QUrl &url)
{
    QMap<BasicExpandType, BasicExpandMap> expandField {};
    BasicViewFieldFunc func { basicViewFieldFuncHash.value(url.scheme()) };
    if (func != nullptr) {
        // QMap<QString, QMultiMap<QString, QPair<QString, QString>> &&fields
        auto &&fields { func(url) };
        if (!fields.isEmpty()) {
            QMetaEnum &&basicType { QMetaEnum::fromType<BasicExpandType>() };
            QMetaEnum &&basicFieldType { QMetaEnum::fromType<BasicFieldExpandEnum>() };

            // convert type of fields to QMap<BasicExpandType, BasicExpandMap>
            for (auto basicIter = fields.constBegin(); basicIter != fields.constEnd(); ++basicIter) {
                bool ok { false };
                BasicExpandMap expandMap {};
                auto type = static_cast<BasicExpandType>(basicType.keyToValue(basicIter.key().toUtf8().constData(), &ok));
                if (ok) {
                    const auto &basicMap = basicIter.value();
                    for (auto basicMapItr = basicMap.constBegin(); basicMapItr != basicMap.constEnd(); ++basicMapItr) {
                        ok = false;
                        auto basicField = static_cast<BasicFieldExpandEnum>(basicFieldType.keyToValue(basicMapItr.key().toUtf8().constData(), &ok));
                        if (ok)
                            expandMap.insert(basicField, basicMapItr.value());
                    }

                    expandField.insert(type, expandMap);
                }
            }
        }
    }

    return expandField;
}

bool PropertyDialogManager::addBasicFiledFiltes(const QString &scheme, PropertyFilterType filters)
{
    if (!filePropertyFilterHash.contains(scheme)) {
        filePropertyFilterHash.insert(scheme, filters);
        return true;
    }

    fmInfo() << "The current scheme has registered the associated construction class";
    return false;
}

PropertyFilterType PropertyDialogManager::basicFiledFiltes(const QUrl &url)
{
    if (filePropertyFilterHash.isEmpty())
        return kNotFilter;
    else if (!filePropertyFilterHash.contains(url.scheme()))
        return kNotFilter;
    else
        return filePropertyFilterHash.value(url.scheme());
}

void PropertyDialogManager::addComputerPropertyDialog()
{
    registerCustomView(ComputerPropertyHelper::createComputerProperty, ComputerPropertyHelper::scheme());
}

QVariantHash PropertyDialogManager::getCreatorOptionByName(const QString &name) const
{
    auto keys { creatorOptions.keys() };
    for (int index : keys) {
        auto &&values { creatorOptions.values(index) };
        auto iter = std::find_if(values.begin(), values.end(), [&name](const QVariantHash &data) {
            return (data.value(kOption_Key_Name).toString() == name);
        });

        if (iter != values.end())
            return *iter;
    }

    return QVariantHash();
}
