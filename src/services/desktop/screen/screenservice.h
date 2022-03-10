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

#include "services/desktop/dd_service_global.h"

#include <interfaces/screen/screenglobal.h>
#include <interfaces/screen/abstractscreen.h>
#include <dfm-framework/framework.h>

class Core;
DSB_D_BEGIN_NAMESPACE
class ScreenServicePrivate;
class ScreenService final : public dpf::PluginService, dpf::AutoServiceRegister<ScreenService>
{
    Q_OBJECT
    Q_DISABLE_COPY(ScreenService)
    friend class dpf::QtClassFactory<dpf::PluginService>;
    friend class ::Core;
    friend class ScreenServicePrivate;
public:
    static QString name()
    {
        return "org.deepin.service.Desktop.ScreenService";
    }
    DFMBASE_NAMESPACE::ScreenPointer primaryScreen();
    QVector<DFMBASE_NAMESPACE::ScreenPointer> screens() const;
    QVector<DFMBASE_NAMESPACE::ScreenPointer> logicScreens() const;
    DFMBASE_NAMESPACE::ScreenPointer screen(const QString &name) const;
    qreal devicePixelRatio() const;
    DFMBASE_NAMESPACE::DisplayMode displayMode() const;
    DFMBASE_NAMESPACE::DisplayMode lastChangedMode() const;
    void reset();

signals:
    void screenChanged();
    void displayModeChanged();
    void screenGeometryChanged();
    void screenAvailableGeometryChanged();
protected:
    explicit ScreenService(QObject *parent = nullptr);
    ~ScreenService();
private:
    ScreenServicePrivate *d;
};

DSB_D_END_NAMESPACE

#endif // SCREENSERVICE_H
