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
#ifndef TITLEBARSERVICE_H
#define TITLEBARSERVICE_H

#include "dfm_filemanager_service_global.h"
#include "titlebar_defines.h"

#include <dfm-framework/framework.h>

DSB_FM_BEGIN_NAMESPACE

class TitleBarServicePrivate;
class TitleBarService final : public dpf::PluginService, dpf::AutoServiceRegister<TitleBarService>
{
    Q_OBJECT
    Q_DISABLE_COPY(TitleBarService)
    friend class dpf::QtClassFactory<dpf::PluginService>;

public:
    static QString name()
    {
        return "org.deepin.service.TitleBarService";
    }

    bool addCustomCrumbar(const TitleBar::CustomCrumbInfo &info);

private:
    explicit TitleBarService(QObject *parent = nullptr);
    virtual ~TitleBarService() override;
    QScopedPointer<TitleBarServicePrivate> d;
};

DSB_FM_END_NAMESPACE

#endif   // TITLEBARSERVICE_H
