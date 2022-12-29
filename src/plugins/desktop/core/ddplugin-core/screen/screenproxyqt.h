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

#ifndef SCREENPROXYQT_H
#define SCREENPROXYQT_H

#include "ddplugin_core_global.h"

#include "interfaces/screen/abstractscreenproxy.h"

class QScreen;

DDPCORE_BEGIN_NAMESPACE

class ScreenProxyQt : public DFMBASE_NAMESPACE::AbstractScreenProxy
{
    Q_OBJECT
public:
    explicit ScreenProxyQt(QObject *parent = nullptr);
    DFMBASE_NAMESPACE::ScreenPointer primaryScreen() override;
    QVector<DFMBASE_NAMESPACE::ScreenPointer> screens() const override;
    QVector<DFMBASE_NAMESPACE::ScreenPointer> logicScreens() const override;
    DFMBASE_NAMESPACE::ScreenPointer screen(const QString &name) const override;
    qreal devicePixelRatio() const override;
    DFMBASE_NAMESPACE::DisplayMode displayMode() const override;
    void reset() override;
public slots:
    void onPrimaryChanged();
private slots:
    void onScreenAdded(QScreen *screen);
    void onScreenRemoved(QScreen *screen);
    void onScreenGeometryChanged(const QRect &);
    void onScreenAvailableGeometryChanged(const QRect &);
    void onDockChanged();
protected:
    void processEvent() override;
private:
    void connectScreen(DFMBASE_NAMESPACE::ScreenPointer);
    void disconnectScreen(DFMBASE_NAMESPACE::ScreenPointer);
private:
    QMap<QScreen *, DFMBASE_NAMESPACE::ScreenPointer> screenMap;
};

DDPCORE_END_NAMESPACE
#endif // SCREENPROXYXCB_H
