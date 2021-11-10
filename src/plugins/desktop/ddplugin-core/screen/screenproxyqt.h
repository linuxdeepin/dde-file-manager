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

#include "dfm_desktop_service_global.h"
#include "dfm-base/widgets/abstractscreenproxy.h"

class QScreen;
class DBusDisplay;

DSB_D_BEGIN_NAMESPACE

class ScreenProxyQt : public dfmbase::AbstractScreenProxy
{
    Q_OBJECT
public:
    explicit ScreenProxyQt(QObject *parent = nullptr);
    dfmbase::ScreenPointer primaryScreen() override;
    QVector<dfmbase::ScreenPointer> screens() const override;
    QVector<dfmbase::ScreenPointer> logicScreens() const override;
    dfmbase::ScreenPointer screen(const QString &name) const override;
    qreal devicePixelRatio() const override;
    dfmbase::DisplayMode displayMode() const override;
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
    void connectScreen(dfmbase::ScreenPointer);
    void disconnectScreen(dfmbase::ScreenPointer);
private:
    QMap<QScreen *, dfmbase::ScreenPointer> screenMap;
    DBusDisplay *display = nullptr;
};

DSB_D_END_NAMESPACE
#endif // SCREENPROXYXCB_H
