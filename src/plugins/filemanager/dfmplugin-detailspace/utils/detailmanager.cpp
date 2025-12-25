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
 * \brief Register extension view with reusable pattern
 * \param create Function to create the widget (called once)
 * \param update Function to update widget data (called on each URL change)
 * \param shouldShow Function to determine visibility (called on each URL change)
 * \param index Position to insert (-1 for append)
 * \return true if registration succeeded
 */
bool DetailManager::registerExtensionView(ViewExtensionCreateFunc create,
                                          ViewExtensionUpdateFunc update,
                                          ViewExtensionShouldShowFunc shouldShow,
                                          int index)
{
    if (!create || !update || !shouldShow) {
        fmWarning() << "Invalid extension view functions provided";
        return false;
    }

    ViewExtensionInfo info;
    info.create = create;
    info.update = update;
    info.shouldShow = shouldShow;
    info.index = index;

    m_extensionInfos.append(info);
    return true;
}

QList<ViewExtensionInfo> DetailManager::extensionInfos() const
{
    return m_extensionInfos;
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

    fmWarning() << "The current scheme has registered the associated construction class, scheme:" << scheme;
    return false;
}

bool DetailManager::registerBasicViewExtensionRoot(const QString &scheme, BasicViewFieldFunc func)
{
    if (!basicViewFieldFuncHashRoot.contains(scheme)) {
        basicViewFieldFuncHashRoot.insert(scheme, func);
        return true;
    }

    fmWarning() << "The current scheme has registered the associated construction class, root scheme:" << scheme;
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

    fmWarning() << "The current scheme has registered the associated construction class, scheme:" << scheme;
    return false;
}

bool DetailManager::addRootBasicFiledFiltes(const QString &scheme, DetailFilterType filters)
{
    if (!detailFilterHashRoot.contains(scheme)) {
        detailFilterHashRoot.insert(scheme, filters);
        return true;
    }

    fmWarning() << "The current scheme has registered the associated construction class, root scheme:" << scheme;
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
