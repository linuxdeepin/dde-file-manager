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

private:
    explicit DetailSpaceService(QObject *parent = nullptr);
    virtual ~DetailSpaceService() override;

    QScopedPointer<DetailSpaceServicePrivate> d;
};

DSB_FM_END_NAMESPACE

#endif   // DETAILSPACESERVICE_H
