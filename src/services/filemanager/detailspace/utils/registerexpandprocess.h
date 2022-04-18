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

#ifndef REGISTEREXPANDPROCESS_H
#define REGISTEREXPANDPROCESS_H

#include "dfm_filemanager_service_global.h"
#include "detailspace/detailspace_defines.h"

#include <QWidget>

DSB_FM_BEGIN_NAMESPACE
DTSP_BEGIN_NAMESPACE
class RegisterExpandProcess
{
private:
    explicit RegisterExpandProcess();

public:
    static RegisterExpandProcess *instance();

protected:
    //! 创建函数列表
    QHash<int, createControlViewFunc> constructList {};
    QHash<QString, basicViewFieldFunc> basicViewFieldFuncHash {};
    QHash<QString, DetailFilterType> detailFilterHash {};

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
     * /brief Register the basic information control extension
     * /param func Get function pointer of extension field
     * /param scheme url format
     * /return true registration success. false registration failed
     */
    bool registerBasicViewExpand(basicViewFieldFunc func, const QString &scheme);

    /*!
     * /brief Cancel the basic information control extension registration
     * /param scheme url format
     */
    void unregisterBasicViewExpand(const QString &scheme);

    /*!
     * /brief Register widgets or basic information field filtering
     * /param scheme url format
     * /param filter filter type
     * /return true registration success. false registration failed
     */
    bool registerFilterControlField(const QString &scheme, DetailFilterType filter);

    /*!
     * /brief Unregister widget or basic information field filtering
     * /param scheme url format
     */
    void unregisterFilterControlField(const QString &scheme);

    /*!
     * /brief Create widgets based on registered schemes and function pointers
     * /param url file url
     * /return Returns a mapping table of widgets and display positions
     */
    QMap<int, QWidget *> createControlView(const QUrl &url);

    /*!
     * /brief Create an extension field based on the registered scheme and function pointer
     * /param url file url
     * /return Returns the mapping table of extension types and extension data
     */
    QMap<BasicExpandType, BasicExpandMap> createBasicExpandField(const QUrl &url);

    /*!
     * /brief Get DetailFilterType according to the registered scheme
     * /param url file url
     * /return Return DetailFilterType
     */
    DetailFilterType contorlFieldFilter(const QUrl &url);
};
DTSP_END_NAMESPACE
DSB_FM_END_NAMESPACE
#endif   //! REGISTEREXPANDPROCESS_H
