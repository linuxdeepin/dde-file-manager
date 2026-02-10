// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCREENPROXYQT_H
#define SCREENPROXYQT_H

#include "ddplugin_core_global.h"

#include <dfm-base/interfaces/screen/abstractscreenproxy.h>

class QScreen;

DDPCORE_BEGIN_NAMESPACE

class ScreenProxyQt : public DFMBASE_NAMESPACE::AbstractScreenProxy
{
    Q_OBJECT
public:
    explicit ScreenProxyQt(QObject *parent = nullptr);
    DFMBASE_NAMESPACE::ScreenPointer primaryScreen() override;
    QList<DFMBASE_NAMESPACE::ScreenPointer> screens() const override;
    QList<DFMBASE_NAMESPACE::ScreenPointer> logicScreens() const override;
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
    bool checkUsedScreens();

private:
    void connectScreen(DFMBASE_NAMESPACE::ScreenPointer);
    void disconnectScreen(DFMBASE_NAMESPACE::ScreenPointer);

private:
    QMap<QScreen *, DFMBASE_NAMESPACE::ScreenPointer> screenMap;
};

DDPCORE_END_NAMESPACE
#endif   // SCREENPROXYXCB_H
