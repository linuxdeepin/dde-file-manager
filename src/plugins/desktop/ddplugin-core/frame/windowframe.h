// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WINDOWFRAME_H
#define WINDOWFRAME_H

#include "ddplugin_core_global.h"

#include <dfm-base/interfaces/abstractdesktopframe.h>
#include <dfm-base/interfaces/screen/abstractscreen.h>

DDPCORE_BEGIN_NAMESPACE
class WindowFramePrivate;
class WindowFrame : public DFMBASE_NAMESPACE::AbstractDesktopFrame
{
    Q_OBJECT
    friend class WindowFramePrivate;

public:
    explicit WindowFrame(QObject *parent = nullptr);
    ~WindowFrame();
    bool init();
    QList<QWidget *> rootWindows() const override;
    void layoutChildren() override;
    QStringList bindedScreens();
public slots:
    void buildBaseWindow();
    void onGeometryChanged();
    void onAvailableGeometryChanged();

private:
    bool needRecreateNativeWindows(const QList<DFMBASE_NAMESPACE::ScreenPointer> &screens) const;
    void cacheScreenTopology(const QList<DFMBASE_NAMESPACE::ScreenPointer> &screens);

private:
    WindowFramePrivate *d;
};

DDPCORE_END_NAMESPACE

#endif   // WINDOWFRAME_H
