/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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
#ifndef BACKGROUNDSERVICE_H
#define BACKGROUNDSERVICE_H

#include "dfm_desktop_service_global.h"
#include "backgroundfactory.h"
#include "dfm-base/widgets/abstractbackground.h"
#include "dfm-base/widgets/abstractscreen.h"

#include <dfm-framework/framework.h>

namespace dfmbase {
class AbstractBackgroundManager;
}

class Core;
DSB_D_BEGIN_NAMESPACE
class BackgroundService final : public dpf::PluginService, dpf::AutoServiceRegister<BackgroundService>
{
    Q_OBJECT
    Q_DISABLE_COPY(BackgroundService)
    friend class dpf::QtClassFactory<dpf::PluginService>;
    friend class ::Core; //权限管控
    explicit BackgroundService(QObject *parent = nullptr);
public:
    static QString name()
    {
        return "org.deepin.service.BackgroundService";
    }

    QMap<QString,dfmbase::BackgroundWidgetPointer> allBackground();
    dfmbase::BackgroundWidgetPointer background(const QString &screenName);
    QMap<QString, QString> allBackgroundPath();
    QString backgroundPath(const QString &screen);

signals:
    void sigBackgroundBuilded(int mode);

private:
    dfmbase::AbstractBackgroundManager *proxy = nullptr;
};
DSB_D_END_NAMESPACE

#endif // BACKGROUNDSERVICE_H
