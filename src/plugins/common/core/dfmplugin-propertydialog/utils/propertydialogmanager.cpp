/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "propertydialogmanager.h"
#include "utils/computerpropertyhelper.h"

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

bool PropertyDialogManager::registerExtensionView(CustomViewExtensionView view, int index)
{
    if (constructList.keys().contains(index) && index != -1) {
        qInfo() << "The current index has registered the associated construction class";
        return false;
    }

    constructList.insert(index, view);
    return true;
}

void PropertyDialogManager::unregisterExtensionView(int index)
{
    constructList.remove(index);
}

QMap<int, QWidget *> PropertyDialogManager::createExtensionView(const QUrl &url)
{
    QMap<int, QWidget *> temp {};
    auto keys { constructList.keys() };
    for (int index : keys) {
        auto &&values { constructList.values(index) };
        for (CustomViewExtensionView func : values) {
            QWidget *g = func(url);
            if (g != nullptr)
                temp.insert(index, g);
        }
    }

    return temp;
}

bool PropertyDialogManager::registerCustomView(CustomViewExtensionView view, const QString &scheme)
{
    if (viewCreateFunctionHash.keys().contains(scheme)) {
        qInfo() << "The current index has registered";
        return false;
    }

    viewCreateFunctionHash.insert(scheme, view);
    return true;
}

void PropertyDialogManager::unregisterCustomView(const QString &scheme)
{
    viewCreateFunctionHash.remove(scheme);
}

QWidget *PropertyDialogManager::createCustomView(const QUrl &url)
{
    auto &&scheme = url.scheme();
    if (!viewCreateFunctionHash.contains(scheme))
        return nullptr;

    return viewCreateFunctionHash[scheme](url);
}

bool PropertyDialogManager::registerBasicViewExtension(BasicViewFieldFunc func, const QString &scheme)
{
    if (!basicViewFieldFuncHash.contains(scheme)) {
        basicViewFieldFuncHash.insert(scheme, func);
        return true;
    }

    qInfo() << "The current scheme has registered the associated construction class";
    return false;
}

void PropertyDialogManager::unregisterBasicViewExtension(const QString &scheme)
{
    basicViewFieldFuncHash.remove(scheme);
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

    qInfo() << "The current scheme has registered the associated construction class";
    return false;
}

void PropertyDialogManager::removeBasicFiledFilters(const QString &scheme)
{
    filePropertyFilterHash.remove(scheme);
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
