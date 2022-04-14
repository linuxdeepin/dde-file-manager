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
#ifndef DELEGATESERVICE_H
#define DELEGATESERVICE_H

#include "delegate_defines.h"

#include <dfm-framework/service/pluginservicecontext.h>

DSC_BEGIN_NAMESPACE

// TODO(zhangs): refactor it

class DelegateService final : public dpf::PluginService, dpf::AutoServiceRegister<DelegateService>
{
    Q_OBJECT
    Q_DISABLE_COPY(DelegateService)
    friend class dpf::QtClassFactory<dpf::PluginService>;

    using KeyType = QString;
    using TransparentHandle = Delegate::IsTransparentCallback;
    using TransparentHandleMap = QMap<KeyType, TransparentHandle>;

    using UrlTransformHandle = Delegate::UrlTransformCallback;
    using UrlTransformHandleMap = QMap<KeyType, UrlTransformHandle>;

public:
    static QString name()
    {
        return "org.deepin.service.DelegateService";
    }
    void registerTransparentHandle(const KeyType &scheme, const TransparentHandle &handle);
    bool isRegistedTransparentHandle(const KeyType &scheme) const;
    TransparentHandleMap getTransparentHandles();
    bool isTransparent(const QUrl &url);

    void registerUrlTransform(const KeyType &scheme, const UrlTransformHandle &handle);
    bool isRegisterUrlTransform(const KeyType &scheme);
    QUrl urlTransform(const QUrl &url);

    static DelegateService *service();

private:
    explicit DelegateService(QObject *parent = nullptr);
    virtual ~DelegateService() = default;

private:
    TransparentHandleMap transparentHandles;
    UrlTransformHandleMap urlTransformHandles;
};

DSC_END_NAMESPACE

#define delegateServIns ::dfm_service_common::DelegateService::service()
#endif   // DELEGATESERVICE_H
