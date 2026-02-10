// SPDX-FileCopyrightText: 2020 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTSCREENPROXY_H
#define ABSTRACTSCREENPROXY_H

#include <dfm-base/dfm_desktop_defines.h>
#include <dfm-base/interfaces/screen/abstractscreen.h>

#include <QObject>
#include <QMultiMap>

class QTimer;

namespace dfmbase {

class AbstractScreenProxy : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(AbstractScreenProxy)
public:
    enum Event { kScreen,
                 kMode,
                 kGeometry,
                 kAvailableGeometry };

public:
    explicit AbstractScreenProxy(QObject *parent = nullptr);

    virtual ScreenPointer primaryScreen() = 0;
    virtual QList<ScreenPointer> screens() const = 0;
    virtual QList<ScreenPointer> logicScreens() const = 0;
    virtual ScreenPointer screen(const QString &name) const = 0;
    virtual qreal devicePixelRatio() const = 0;
    virtual DisplayMode displayMode() const = 0;
    virtual DisplayMode lastChangedMode() const;
    virtual void reset() = 0;

protected:
    virtual void processEvent() = 0;

protected:
    void appendEvent(Event);
Q_SIGNALS:
    void screenChanged();
    void displayModeChanged();
    void screenGeometryChanged();
    void screenAvailableGeometryChanged();

protected:
    DisplayMode lastMode = DisplayMode::kCustom;
    QMultiMap<Event, qint64> events;

private:
    QTimer *eventShot = nullptr;
};

}

#endif   // ABSTRACTSCREENPROXY_H
