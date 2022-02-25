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
#ifndef PREVIEWSERVICE_H
#define PREVIEWSERVICE_H

#include "dfm_common_service_global.h"

#include <dfm-framework/framework.h>

DSC_BEGIN_NAMESPACE
class PreviewService final : public dpf::PluginService, dpf::AutoServiceRegister<PreviewService>
{
    Q_OBJECT
    Q_DISABLE_COPY(PreviewService)
    friend class dpf::QtClassFactory<dpf::PluginService>;

private:
    explicit PreviewService(QObject *parent = nullptr);

public:
    static QString name()
    {
        return "org.deepin.service.PreviewService";
    }
};
DSC_END_NAMESPACE
#endif   // PREVIEWSERVICE_H
