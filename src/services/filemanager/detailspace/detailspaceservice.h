/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef DETAILSPACESERVICE_H
#define DETAILSPACESERVICE_H

#include "detailspace_defines.h"

#include <dfm-framework/framework.h>

DSB_FM_BEGIN_NAMESPACE

class DetailSpaceServicePrivate;
class DetailSpaceService final : public dpf::PluginService, dpf::AutoServiceRegister<DetailSpaceService>
{
    Q_OBJECT
    Q_DISABLE_COPY(DetailSpaceService)
    friend class dpf::QtClassFactory<dpf::PluginService>;

public:
    static QString name()
    {
        return "org.deepin.service.DetailSpaceService";
    }

    static DetailSpaceService *serviceInstance();

private:
    explicit DetailSpaceService(QObject *parent = nullptr);
    virtual ~DetailSpaceService() override;

    QScopedPointer<DetailSpaceServicePrivate> d;

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
     * /brief Create widgets based on registered schemes and function pointers
     * /param url file url
     * /return Returns a mapping table of widgets and display positions
     */
    QMap<int, QWidget *> createControlView(const QUrl &url);

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
     * /brief Create an extension field based on the registered scheme and function pointer
     * /param url file url
     * /return Returns the mapping table of extension types and extension data
     */
    QMap<BasicExpandType, BasicExpandMap> createBasicExpandField(const QUrl &url);

    /*!
     * /brief Register widgets or basic information field filtering
     * /param scheme url format
     * /param filter filter type
     * /return true registration success. false registration failed
     */
    bool registerFilterControlField(const QString &scheme, DetailFilterTypes filter);

    /*!
     * /brief Unregister widget or basic information field filtering
     * /param scheme url format
     */
    void unregisterFilterControlField(const QString &scheme);

    /*!
     * /brief Get DetailFilterType according to the registered scheme
     * /param url file url
     * /return Return DetailFilterType
     */
    DetailFilterTypes contorlFieldFilter(const QUrl &url);
};

DSB_FM_END_NAMESPACE
#define detailServIns ::DSB_FM_NAMESPACE::DetailSpaceService::serviceInstance()
#endif   // DETAILSPACESERVICE_H
