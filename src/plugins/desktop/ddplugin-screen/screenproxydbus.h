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
#ifndef SCREENPROXYDBUS_H
#define SCREENPROXYDBUS_H

#include "ddplugin_screen_global.h"

#include <interfaces/screen/screenglobal.h>
#include <interfaces/screen/abstractscreenproxy.h>

class DBusDisplay;

DDP_SCREEN_BEGIN_NAMESPACE

class ScreenProxyDBus : public dfmbase::AbstractScreenProxy
{
    Q_OBJECT
public:
    explicit ScreenProxyDBus(QObject *parent = nullptr);
    dfmbase::ScreenPointer primaryScreen() override;
    QVector<dfmbase::ScreenPointer> screens() const override;
    QVector<dfmbase::ScreenPointer> logicScreens() const override;
    dfmbase::ScreenPointer screen(const QString &name) const override;
    qreal devicePixelRatio() const override;
    dfmbase::DisplayMode displayMode() const override;
    void reset() override;
protected:
    void processEvent() override;
private slots:
    void onMonitorChanged();
    void onDockChanged();
    void onScreenGeometryChanged(const QRect &rect);
    void onModeChanged();
private:
    void connectScreen(dfmbase::ScreenPointer);
    void disconnectScreen(dfmbase::ScreenPointer);
private:
    QMap<QString, dfmbase::ScreenPointer> screenMap; //dbus-path - screen
    DBusDisplay *display = nullptr;
};

DDP_SCREEN_END_NAMESPACE
#endif // SCREENPROXYDBUS_H
