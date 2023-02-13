// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "desktopdbusinterface.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <QDBusInterface>
#include <QDBusPendingCall>

using namespace dde_desktop;

TEST(DesktopDBusInterface, Refresh)
{
    DesktopDBusInterface ifs;
    stub_ext::StubExt stub;
    QString srv;
    QString addr;
    QString sifs;
    QString me;
    bool silent = false;
    stub.set_lamda((QDBusPendingCall (QDBusAbstractInterface::*)(const QString &method,
                                                                 const QVariant &arg1,
                                                                 const QVariant &arg2,
                                                                 const QVariant &arg3,
                                                                 const QVariant &arg4,
                                                                 const QVariant &arg5,
                                                                 const QVariant &arg6,
                                                                 const QVariant &arg7,
                                                                 const QVariant &arg8))
                   &QDBusAbstractInterface::asyncCall,
                   [&srv, &addr, &sifs, &silent, &me](QDBusAbstractInterface *self, const QString &method,
                                                   const QVariant &arg1,
                                                   const QVariant &arg2,
                                                   const QVariant &arg3,
                                                   const QVariant &arg4,
                                                   const QVariant &arg5,
                                                   const QVariant &arg6,
                                                   const QVariant &arg7,
                                                   const QVariant &arg8) {
        srv = self->service();
        addr = self->path();
        sifs = self->interface();
        me = method;
        silent = arg1.toBool();
        return QDBusPendingCall::fromError(QDBusError());
    });

    ifs.Refresh();
    EXPECT_EQ(srv, QString("com.deepin.dde.desktop"));
    EXPECT_EQ(addr, QString("/org/deepin/dde/desktop/canvas"));
    EXPECT_EQ(sifs, QString("org.deepin.dde.desktop.canvas"));

    EXPECT_EQ(me, QString("Refresh"));
    EXPECT_TRUE(silent);

    me.clear();
    silent = true;
    ifs.Refresh(false);
    EXPECT_EQ(me, QString("Refresh"));
    EXPECT_FALSE(silent);
}

TEST(DesktopDBusInterface, ShowWallpaperChooser)
{
    DesktopDBusInterface ifs;
    stub_ext::StubExt stub;
    QString srv;
    QString addr;
    QString sifs;
    QString me;
    QString screen;
    stub.set_lamda((QDBusPendingCall (QDBusAbstractInterface::*)(const QString &method,
                                                                 const QVariant &arg1,
                                                                 const QVariant &arg2,
                                                                 const QVariant &arg3,
                                                                 const QVariant &arg4,
                                                                 const QVariant &arg5,
                                                                 const QVariant &arg6,
                                                                 const QVariant &arg7,
                                                                 const QVariant &arg8))
                   &QDBusAbstractInterface::asyncCall,
                   [&srv, &addr, &sifs, &screen, &me](QDBusAbstractInterface *self, const QString &method,
                                                   const QVariant &arg1,
                                                   const QVariant &arg2,
                                                   const QVariant &arg3,
                                                   const QVariant &arg4,
                                                   const QVariant &arg5,
                                                   const QVariant &arg6,
                                                   const QVariant &arg7,
                                                   const QVariant &arg8) {
        srv = self->service();
        addr = self->path();
        sifs = self->interface();
        me = method;
        screen = arg1.toString();
        return QDBusPendingCall::fromError(QDBusError());
    });

    ifs.ShowWallpaperChooser();
    EXPECT_EQ(srv, QString("com.deepin.dde.desktop"));
    EXPECT_EQ(addr, QString("/org/deepin/dde/desktop/wallpapersettings"));
    EXPECT_EQ(sifs, QString("org.deepin.dde.desktop.wallpapersettings"));

    EXPECT_EQ(me, QString("ShowWallpaperChooser"));
    EXPECT_TRUE(screen.isEmpty());

    me.clear();
    screen.clear();
    ifs.ShowWallpaperChooser("test");
    EXPECT_EQ(me, QString("ShowWallpaperChooser"));
    EXPECT_EQ(screen, QString("test"));
}

TEST(DesktopDBusInterface, ShowScreensaverChooser)
{
    DesktopDBusInterface ifs;
    stub_ext::StubExt stub;
    QString srv;
    QString addr;
    QString sifs;
    QString me;
    QString screen;
    stub.set_lamda((QDBusPendingCall (QDBusAbstractInterface::*)(const QString &method,
                                                                 const QVariant &arg1,
                                                                 const QVariant &arg2,
                                                                 const QVariant &arg3,
                                                                 const QVariant &arg4,
                                                                 const QVariant &arg5,
                                                                 const QVariant &arg6,
                                                                 const QVariant &arg7,
                                                                 const QVariant &arg8))
                   &QDBusAbstractInterface::asyncCall,
                   [&srv, &addr, &sifs, &screen, &me](QDBusAbstractInterface *self, const QString &method,
                                                   const QVariant &arg1,
                                                   const QVariant &arg2,
                                                   const QVariant &arg3,
                                                   const QVariant &arg4,
                                                   const QVariant &arg5,
                                                   const QVariant &arg6,
                                                   const QVariant &arg7,
                                                   const QVariant &arg8) {
        srv = self->service();
        addr = self->path();
        sifs = self->interface();
        me = method;
        screen = arg1.toString();
        return QDBusPendingCall::fromError(QDBusError());
    });

    ifs.ShowScreensaverChooser();
    EXPECT_EQ(srv, QString("com.deepin.dde.desktop"));
    EXPECT_EQ(addr, QString("/org/deepin/dde/desktop/wallpapersettings"));
    EXPECT_EQ(sifs, QString("org.deepin.dde.desktop.wallpapersettings"));

    EXPECT_EQ(me, QString("ShowScreensaverChooser"));
    EXPECT_TRUE(screen.isEmpty());

    me.clear();
    screen.clear();
    ifs.ShowScreensaverChooser("test");
    EXPECT_EQ(me, QString("ShowScreensaverChooser"));
    EXPECT_EQ(screen, QString("test"));
}
