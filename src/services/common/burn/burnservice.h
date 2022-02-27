/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#ifndef BURNSERVICE_H
#define BURNSERVICE_H

#include "burn_defines.h"

#include <QObject>

#include <dfm-framework/framework.h>

DSC_BEGIN_NAMESPACE

class BurnService final : public dpf::PluginService, dpf::AutoServiceRegister<BurnService>
{
    Q_OBJECT
    Q_DISABLE_COPY(BurnService)
    friend class dpf::QtClassFactory<dpf::PluginService>;

public:
    static QString name()
    {
        return "org.deepin.service.BurnService";
    }

private:
    explicit BurnService(QObject *parent = nullptr);
};

DSC_END_NAMESPACE

#endif   // BURNSERVICE_H
