// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCREENPROXYDBUS_H
#define SCREENPROXYDBUS_H

#include "ddplugin_core_global.h"

#include <dfm-base/interfaces/screen/abstractscreenproxy.h>

DDPCORE_BEGIN_NAMESPACE

class ScreenProxyDBus : public DFMBASE_NAMESPACE::AbstractScreenProxy
{
    Q_OBJECT
public:
    explicit ScreenProxyDBus(QObject *parent = nullptr);
    DFMBASE_NAMESPACE::ScreenPointer primaryScreen() override;
    QList<DFMBASE_NAMESPACE::ScreenPointer> screens() const override;
    QList<DFMBASE_NAMESPACE::ScreenPointer> logicScreens() const override;
    DFMBASE_NAMESPACE::ScreenPointer screen(const QString &name) const override;
    qreal devicePixelRatio() const override;
    DFMBASE_NAMESPACE::DisplayMode displayMode() const override;
    void reset() override;
protected:
    void processEvent() override;
private slots:
    void onMonitorChanged();
    void onDockChanged();
    void onScreenGeometryChanged(const QRect &rect);
    void onModeChanged();
private:
    void connectScreen(DFMBASE_NAMESPACE::ScreenPointer);
    void disconnectScreen(DFMBASE_NAMESPACE::ScreenPointer);
private:
    QMap<QString, DFMBASE_NAMESPACE::ScreenPointer> screenMap; //dbus-path - screen
};

DDPCORE_END_NAMESPACE
#endif // SCREENPROXYDBUS_H
