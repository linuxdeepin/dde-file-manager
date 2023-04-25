// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "screen/screenqt.h"
#include "screen/dbus-private/dbushelper.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <QApplication>
#include <QSharedPointer>
#include <QMap>
#include <QScreen>
#include <QTimer>
#include <qpa/qplatformscreen.h>

DDPCORE_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

TEST(ScreenQt, util)
{
    if (auto pr = qApp->primaryScreen()) {
        ScreenQt sc(pr);
        EXPECT_EQ(sc.name(), pr->name());
        EXPECT_EQ(sc.geometry(), pr->geometry());
        EXPECT_EQ(sc.handleGeometry(), pr->handle()->geometry());
        EXPECT_EQ(sc.screen(), pr);
    }
}

namespace  {
class ScreenOperation : public testing::Test
{
public:
    ScreenOperation() : Test() {

    }
    ~ScreenOperation() {
        delete scqt;
    }

    void SetUp() override {
        if (auto pr = qApp->primaryScreen()) {
            scqt = new ScreenQt(pr);
            stub.set_lamda(ADDR(DBusDock,hideMode),[this](){
                return dockhide;
            });
            stub.set_lamda(ADDR(DBusHelper,isDockEnable),[this](){
                return dockEnable;
            });
            stub.set_lamda(ADDR(DBusDock,frontendWindowRect),[this](){
                return dockRect;
            });
            stub.set_lamda(ADDR(DBusDock,position),[this](){
                return dockPos;
            });
            stub.set_lamda(VADDR(ScreenQt, geometry),[this](){
                return QRect(0,0, 900, 900);
            });
            stub.set_lamda(VADDR(ScreenQt, handleGeometry),[this](){
                return QRect(0,0, 900, 900);
            });
            stub.set_lamda(ADDR(QScreen, devicePixelRatio),[this](){
                return 1.0;
            });
        }
    }
    stub_ext::StubExt stub;
    ScreenQt *scqt = nullptr;
    int dockhide = 0;
    bool dockEnable = true;
    DockRect  dockRect;
    int dockPos = 0;
};
}

TEST_F(ScreenOperation, availableGeometry_hide)
{
    if (!scqt)
        return;

    dockEnable = false;

    EXPECT_EQ(scqt->availableGeometry(), QRect(0,0, 900, 900));

    dockEnable = true;
    dockhide = 1;
    EXPECT_EQ(scqt->availableGeometry(), QRect(0,0, 900, 900));
}

TEST_F(ScreenOperation, availableGeometry_invaild)
{
    if (!scqt)
        return;

    dockPos = -1;
    EXPECT_EQ(scqt->availableGeometry(), QRect(0,0, 900, 900));

    dockPos = 4;
    EXPECT_EQ(scqt->availableGeometry(), QRect(0,0, 900, 900));
}

TEST_F(ScreenOperation, availableGeometry_top)
{
    if (!scqt)
        return;

    dockPos = 0;
    dockRect = DockRect{0,0,900,50};

    QRect geo(0,0, 900, 900);
    QRect dock = dockRect.operator QRect();
    geo.setY(dock.bottom());
    EXPECT_EQ(scqt->availableGeometry(), geo);

    dockRect = DockRect{900,0,900,50};
    EXPECT_EQ(scqt->availableGeometry(), QRect(0,0, 900, 900));
}

TEST_F(ScreenOperation, availableGeometry_right)
{
    if (!scqt)
        return;

    dockPos = 1;
    dockRect = DockRect{850,0,50,800};

    QRect geo(0,0, 900, 900);
    QRect dock = dockRect.operator QRect();

    geo.setWidth(dock.left() - geo.left());
    EXPECT_EQ(scqt->availableGeometry(), geo);
}

TEST_F(ScreenOperation, availableGeometry_bottom)
{
    if (!scqt)
        return;

    dockPos = 2;
    dockRect = DockRect{0, 850, 900, 50};

    QRect geo(0, 0, 900, 900);
    QRect dock = dockRect.operator QRect();

    geo.setHeight(dock.top() - geo.top());
    EXPECT_EQ(scqt->availableGeometry(), geo);
}

TEST_F(ScreenOperation, availableGeometry_left)
{
    if (!scqt)
        return;

    dockPos = 3;
    dockRect = DockRect{0, 0, 50, 900};

    QRect geo(0, 0, 900, 900);
    QRect dock = dockRect.operator QRect();

    geo.setX(dock.right());
    EXPECT_EQ(scqt->availableGeometry(), geo);
}
