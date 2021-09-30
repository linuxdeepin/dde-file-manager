/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *             wangchunlin<wangchunlin@uniontech.com>
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

#ifndef SCREENMANAGER_H
#define SCREENMANAGER_H

#include "abstractscreenmanager.h"
#include <QVector>
#include <QMap>

class QScreen;
class ScreenObject;
class DBusDisplay;
class ScreenManager : public AbstractScreenManager
{
    Q_OBJECT
public:
    explicit ScreenManager(QObject *parent = nullptr);
    ~ScreenManager() override;
    ScreenPointer primaryScreen() override;
    QVector<ScreenPointer> screens() const override;
    QVector<ScreenPointer> logicScreens() const override;
    ScreenPointer screen(const QString &name) const override;
    qreal devicePixelRatio() const override;
    DisplayMode displayMode() const override;
    DisplayMode lastChangedMode() const override;
    void reset() override;
public slots:
    void onPrimaryChanged();
protected:
    void processEvent() override;
protected slots:
    void onScreenAdded(QScreen *screen);
    void onScreenRemoved(QScreen *screen);
    void onScreenGeometryChanged(const QRect &);
    void onScreenAvailableGeometryChanged(const QRect &);
    void onDockChanged();
private:
    void init();
    void connectScreen(ScreenPointer);
    void disconnectScreen(ScreenPointer);
protected:
    QMap<QScreen *, ScreenPointer> m_screens;
    DBusDisplay *m_display = nullptr;
};

#endif // SCREENMANAGER_H
