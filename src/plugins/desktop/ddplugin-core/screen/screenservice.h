/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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

#ifndef SCREENSERVICE_H
#define SCREENSERVICE_H

#include "dfm_desktop_service_global.h"
#include "dfm-base/widgets/screenglobal.h"

#include <dfm-framework/framework.h>

namespace dfmbase {
class AbstractScreenProxy;
}

class Core;
DSB_D_BEGIN_NAMESPACE
class ScreenService final : public dpf::PluginService, dpf::AutoServiceRegister<ScreenService>
{
    Q_OBJECT
    Q_DISABLE_COPY(ScreenService)
    friend class dpf::QtClassFactory<dpf::PluginService>;
    friend class ::Core;
    explicit ScreenService(QObject *parent = nullptr);
public:

    static QString name()
    {
        return "org.deepin.service.ScreenService";
    }

    dfmbase::ScreenPointer primaryScreen();
    QVector<dfmbase::ScreenPointer> screens() const;
    QVector<dfmbase::ScreenPointer> logicScreens() const;
    dfmbase::ScreenPointer screen(const QString &name) const;
    qreal devicePixelRatio() const;
    dfmbase::DisplayMode displayMode() const;
    dfmbase::DisplayMode lastChangedMode() const;
    void reset();

signals:
    void screenChanged();
    void displayModeChanged();
    void screenGeometryChanged();
    void screenAvailableGeometryChanged();
private:
    dfmbase::AbstractScreenProxy *proxy = nullptr;
};

DSB_D_END_NAMESPACE

#endif // SCREENSERVICE_H
