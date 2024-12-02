// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
        fmInfo() << "The current index has registered the associated construction class";
        return false;
    }

    constructList.insert(index, view);
    return true;
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
    if (!basicViewFieldFuncHashNormal.contains(scheme)) {
        basicViewFieldFuncHashNormal.insert(scheme, func);
        return true;
    }

    fmInfo() << "The current scheme has registered the associated construction class";
    return false;
}

bool DetailManager::registerBasicViewExtensionRoot(const QString &scheme, BasicViewFieldFunc func)
{
    if (!basicViewFieldFuncHashRoot.contains(scheme)) {
        basicViewFieldFuncHashRoot.insert(scheme, func);
        return true;
    }

    fmInfo() << "The current scheme has registered the associated construction class";
    return false;
}

/*!
 * /brief Create an extension field based on the registered scheme and function pointer
 * /param url file url
 * /return Returns the mapping table of extension types and extension data
 */
QMap<BasicExpandType, BasicExpandMap> DetailManager::createBasicViewExtensionField(const QUrl &url)
{
    QMap<BasicExpandType, BasicExpandMap> expandField {};

    BasicViewFieldFunc func = nullptr;
    if (url.path() == "/")
        func = basicViewFieldFuncHashRoot.value(url.scheme());
    else
        func = basicViewFieldFuncHashNormal.value(url.scheme());
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
    if (!detailFilterHashNormal.contains(scheme)) {
        detailFilterHashNormal.insert(scheme, filters);
        return true;
    }

    fmInfo() << "The current scheme has registered the associated construction class";
    return false;
}

bool DetailManager::addRootBasicFiledFiltes(const QString &scheme, DetailFilterType filters)
{
    if (!detailFilterHashRoot.contains(scheme)) {
        detailFilterHashRoot.insert(scheme, filters);
        return true;
    }

    fmInfo() << "The current scheme has registered the associated construction class";
    return false;
}

/*!
 * /brief Get DetailFilterType according to the registered scheme
 * /param url file url
 * /return Return DetailFilterType
 */
DetailFilterType DetailManager::basicFiledFiltes(const QUrl &url)
{
    if (url.path() == "/" && detailFilterHashRoot.count(url.scheme()) > 0)
        return detailFilterHashRoot.value(url.scheme());

    if (detailFilterHashNormal.isEmpty()) {
        return kNotFilter;
    } else if (!detailFilterHashNormal.contains(url.scheme())) {
        if (detailFilterHashNormal.contains(url.path())) {
            return detailFilterHashNormal.value(url.path());
        }
        return kNotFilter;
    } else {
        return detailFilterHashNormal.value(url.scheme());
    }
}

DetailManager::DetailManager(QObject *parent)
    : QObject(parent)
{
}
