/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
#include "detailmanager.h"

#include <QDebug>
#include <QMap>
#include <QMetaEnum>
#include <QUrl>

using namespace dfmplugin_detailspace;

DetailManager &DetailManager::instance()
{
    static DetailManager ins;
    return ins;
}

/*!
 * /brief Widget extension registration
 * /param view Function pointer to create widget
 * /param index position to insert
 * /return true registration success. false registration failed
 */
bool DetailManager::registerExtensionView(CustomViewExtensionView view, int index)
{
    if (constructList.keys().contains(index) && index != -1) {
        qInfo() << "The current index has registered the associated construction class";
        return false;
    }

    constructList.insert(index, view);
    return true;
}

/*!
 * /brief Cancel widget extension registration
 * /param index position to insert
 */
void DetailManager::unregisterExtensionView(int index)
{
    constructList.remove(index);
}

/*!
 * /brief Create widgets based on registered schemes and function pointers
 * /param url file url
 * /return Returns a mapping table of widgets and display positions
 */
QMap<int, QWidget *> DetailManager::createExtensionView(const QUrl &url)
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

/*!
 * /brief Register the basic information control extension
 * /param func Get function pointer of extension field
 * /param scheme url format
 * /return true registration success. false registration failed
 */
bool DetailManager::registerBasicViewExtension(const QString &scheme, BasicViewFieldFunc func)
{
    if (!basicViewFieldFuncHash.contains(scheme)) {
        basicViewFieldFuncHash.insert(scheme, func);
        return true;
    }

    qInfo() << "The current scheme has registered the associated construction class";
    return false;
}

/*!
 * /brief Cancel the basic information control extension registration
 * /param scheme url format
 */
void DetailManager::unregisterBasicViewExtension(const QString &scheme)
{
    basicViewFieldFuncHash.remove(scheme);
}

/*!
 * /brief Create an extension field based on the registered scheme and function pointer
 * /param url file url
 * /return Returns the mapping table of extension types and extension data
 */
QMap<BasicExpandType, BasicExpandMap> DetailManager::createBasicViewExtensionField(const QUrl &url)
{
    QMap<BasicExpandType, BasicExpandMap> expandField {};
    BasicViewFieldFunc func { basicViewFieldFuncHash.value(url.scheme()) };
    if (func != nullptr) {
        auto &&fields { func(url) };
        if (!fields.isEmpty()) {
            QMetaEnum &&basicType { QMetaEnum::fromType<BasicExpandType>() };
            QMetaEnum &&basicFieldType { QMetaEnum::fromType<BasicFieldExpandEnum>() };

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

/*!
 * /brief Register widgets or basic information field filtering
 * /param scheme url format
 * /param filter filter type
 * /return true registration success. false registration failed
 */
bool DetailManager::addBasicFiledFiltes(const QString &scheme, DetailFilterType filters)
{
    if (!detailFilterHash.contains(scheme)) {
        detailFilterHash.insert(scheme, filters);
        return true;
    }

    qInfo() << "The current scheme has registered the associated construction class";
    return false;
}

/*!
 * /brief Unregister widget or basic information field filtering
 * /param scheme url format
 */
void DetailManager::removeBasicFiledFilters(const QString &scheme)
{
    detailFilterHash.remove(scheme);
}

/*!
 * /brief Get DetailFilterType according to the registered scheme
 * /param url file url
 * /return Return DetailFilterType
 */
DetailFilterType DetailManager::basicFiledFiltes(const QUrl &url)
{
    if (detailFilterHash.isEmpty()) {
        return kNotFilter;
    } else if (!detailFilterHash.contains(url.scheme())) {
        if (detailFilterHash.contains(url.path())) {
            return detailFilterHash.value(url.path());
        }
        return kNotFilter;
    } else {
        return detailFilterHash.value(url.scheme());
    }
}

DetailManager::DetailManager(QObject *parent)
    : QObject(parent)
{
}
