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
#ifndef REGISTERCREATEPROCESS_H
#define REGISTERCREATEPROCESS_H

#include "propertydialog/property_defines.h"

#include <QWidget>

DSC_BEGIN_NAMESPACE
CPY_BEGIN_NAMESPACE
class RegisterCreateProcess
{
private:
    explicit RegisterCreateProcess();

public:
    static RegisterCreateProcess *instance();

protected:
    //创建函数列表
    QMultiHash<int, createControlViewFunc> constructList {};
    QHash<QString, createControlViewFunc> viewCreateFunctionHash {};
    QHash<QString, basicViewFieldFunc> basicViewFieldFuncHash {};
    QList<QString> propertyPathList {};
    QHash<QString, FilePropertyControlFilter> filePropertyFilterHash {};

public:
    /*!
     * /brief Widget extension registration
     * /param view Function pointer to create widget
     * /param index position to insert
     * /return true registration success. false registration failed
     */
    bool registerControlExpand(createControlViewFunc view, int index = -1);

    /*!
     * /brief Cancel widget extension registration
     * /param index position to insert
     */
    void unregisterControlExpand(int index);

    /*!
     * /brief Register custom properties dialog creation function
     * /param view Custom Properties Dialog Creation Function
     * /param scheme url Url's scheme
     * /return true registration success. false registration failed
     */
    bool registerCustomizePropertyView(createControlViewFunc view, const QString &scheme);

    void unregisterCustomizePropertyView(const QString &scheme);

    /*!
     * /brief Register the basic information control extension
     * /param func Get function pointer of extension field
     * /param scheme url format
     * /return true registration success. false registration failed
     */
    bool registerBasicViewFiledExpand(basicViewFieldFunc func, const QString &scheme);

    /*!
     * /brief Cancel the basic information control extension registration
     * /param scheme url format
     */
    void unregisterBasicViewFiledExpand(const QString &scheme);

    /*!
     * /brief Register widgets or basic information field filtering
     * /param scheme url format
     * /param filter filter type
     * /return true registration success. false registration failed
     */
    bool registerFilterControlField(const QString &scheme, FilePropertyControlFilter filter);

    void unregisterFilterControlField(const QString &scheme);

    bool isContains(const QUrl &url) const;

    /*!
     * /brief Create Custom Extended Properties Dialog
     * /param url Need to use the url of the custom properties dialog
     * /return Return to the Custom Properties dialog
     */
    QWidget *createCustomizePropertyWidget(const QUrl &url);

    /*!
     * @brief Create a property page extension control
     * @param url Requires the url of the property page extension control
     * @return Returns extended controls and positions
     */
    QMap<int, QWidget *> createControlView(const QUrl &url);

    /*!
     * /brief Basic information extension
     * /param url The file url that needs to expand the base information field
     * /return Extension information and extension type
     */
    QMap<BasicExpandType, BasicExpand> basicExpandField(const QUrl &url);

    /*!
     * /brief Get DetailFilterType according to the registered scheme
     * /param url file url
     * /return Return DetailFilterType
     */
    FilePropertyControlFilter contorlFieldFilter(const QUrl &url);
};
CPY_END_NAMESPACE
DSC_END_NAMESPACE
#endif   // REGISTERCREATEPROCESS_H
