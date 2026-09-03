// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusArgument>
#include <QDBusMessage>
#include <QApplication>

#include "stubext.h"

#define private public
#define protected public
#include "screen/dbus-private/dbushelper.h"
#include "screen/dbus-private/dbusdock1.h"
#include "screen/dbus-private/dbusdisplay1.h"
#undef private
#undef protected

DDPCORE_USE_NAMESPACE

namespace {

class DbusHelperImpl : public testing::Test
{
public:
    void SetUp() override
    {
        if (!qApp) {
            int argc = 1;
            char *argv[] = { const_cast<char *>("ut") };
            new QApplication(argc, argv);
        }

        stubDBus();
    }

    void TearDown() override
    {
        stub.clear();
    }

protected:
    void stubDBus()
    {
        // sessionBus itself does not open a real connection in unit tests.
        stub.set_lamda(&QDBusConnection::sessionBus, []() -> QDBusConnection {
            __DBG_STUB_INVOKE__
            return QDBusConnection("mock_session_bus");
        });

        // Pretend there is a valid connection interface.
        stub.set_lamda(&QDBusConnection::interface, [](QDBusConnection *) -> QDBusConnectionInterface * {
            __DBG_STUB_INVOKE__
            return reinterpret_cast<QDBusConnectionInterface *>(0x1);
        });

        // Block real D-Bus signal/property connections inside DBusDock/DBusDisplay ctors.
        using ConnectFunc = bool (QDBusConnection::*)(const QString &, const QString &,
                                                        const QString &, const QString &,
                                                        QObject *, const char *);
        stub.set_lamda(static_cast<ConnectFunc>(&QDBusConnection::connect),
                       [](QDBusConnection *, const QString &, const QString &,
                          const QString &, const QString &, QObject *, const char *) -> bool {
            __DBG_STUB_INVOKE__
            return true;
        });

        // isServiceRegistered is used by isDockEnable / isDisplayEnable.
        using IsSvcFunc = QDBusReply<bool> (QDBusConnectionInterface::*)(const QString &) const;
        stub.set_lamda(static_cast<IsSvcFunc>(&QDBusConnectionInterface::isServiceRegistered),
                       [](const QDBusConnectionInterface *, const QString &name) -> QDBusReply<bool> {
            __DBG_STUB_INVOKE__
            QDBusMessage msg = QDBusMessage::createMethodCall("", "", "", "");
            QDBusMessage reply = msg.createReply(QVariantList() << (name == DBusDock::staticServiceName()));
            return QDBusReply<bool>(reply);
        });
    }

    stub_ext::StubExt stub;
};

} // namespace

TEST_F(DbusHelperImpl, Singleton_ReturnsSameInstance)
{
    DBusHelper *one = DBusHelper::ins();
    EXPECT_NE(one, nullptr);
    EXPECT_EQ(one, DBusHelper::ins());
}

TEST_F(DbusHelperImpl, Accessors_ReturnValidInterfaces)
{
    DBusHelper *helper = DBusHelper::ins();
    EXPECT_NE(helper->dock(), nullptr);
    EXPECT_NE(helper->display(), nullptr);
    EXPECT_EQ(helper->dock(), helper->m_dock);
    EXPECT_EQ(helper->display(), helper->m_display);
}

TEST_F(DbusHelperImpl, IsDockEnable_DetectsService)
{
    EXPECT_TRUE(DBusHelper::isDockEnable());

    // Switch stub so every service appears unregistered.
    using IsSvcFunc = QDBusReply<bool> (QDBusConnectionInterface::*)(const QString &) const;
    stub.set_lamda(static_cast<IsSvcFunc>(&QDBusConnectionInterface::isServiceRegistered),
                   [](const QDBusConnectionInterface *, const QString &) -> QDBusReply<bool> {
        QDBusMessage msg = QDBusMessage::createMethodCall("", "", "", "");
        QDBusMessage reply = msg.createReply(QVariantList() << false);
        return QDBusReply<bool>(reply);
    });

    EXPECT_FALSE(DBusHelper::isDockEnable());
}

TEST_F(DbusHelperImpl, IsDisplayEnable_DetectsService)
{
    using IsSvcFunc = QDBusReply<bool> (QDBusConnectionInterface::*)(const QString &) const;
    stub.set_lamda(static_cast<IsSvcFunc>(&QDBusConnectionInterface::isServiceRegistered),
                   [](const QDBusConnectionInterface *, const QString &name) -> QDBusReply<bool> {
        QDBusMessage msg = QDBusMessage::createMethodCall("", "", "", "");
        QDBusMessage reply = msg.createReply(QVariantList() << (name == DBusDisplay::staticServiceName()));
        return QDBusReply<bool>(reply);
    });

    EXPECT_TRUE(DBusHelper::isDisplayEnable());

    stub.set_lamda(static_cast<IsSvcFunc>(&QDBusConnectionInterface::isServiceRegistered),
                   [](const QDBusConnectionInterface *, const QString &) -> QDBusReply<bool> {
        QDBusMessage msg = QDBusMessage::createMethodCall("", "", "", "");
        QDBusMessage reply = msg.createReply(QVariantList() << false);
        return QDBusReply<bool>(reply);
    });

    EXPECT_FALSE(DBusHelper::isDisplayEnable());
}

TEST_F(DbusHelperImpl, StreamOperators_RoundTripDockRect)
{
    qDBusRegisterMetaType<DockRect>();

    DockRect src {};
    src.x = 10;
    src.y = 20;
    src.width = 800;
    src.height = 600;

    QDBusMessage msg = QDBusMessage::createMethodCall("", "", "", "");
    msg << QVariant::fromValue(src);

    DockRect dst = qdbus_cast<DockRect>(msg.arguments().first());

    EXPECT_EQ(dst.x, src.x);
    EXPECT_EQ(dst.y, src.y);
    EXPECT_EQ(dst.width, src.width);
    EXPECT_EQ(dst.height, src.height);
    EXPECT_EQ(QRect(dst), QRect(src));
}

TEST_F(DbusHelperImpl, StreamOperators_RoundTripDisplayRect)
{
    qDBusRegisterMetaType<DisplayRect>();

    DisplayRect src {};
    src.x = 5;
    src.y = 6;
    src.width = 1024;
    src.height = 768;

    QDBusMessage msg = QDBusMessage::createMethodCall("", "", "", "");
    msg << QVariant::fromValue(src);

    DisplayRect dst = qdbus_cast<DisplayRect>(msg.arguments().first());

    EXPECT_EQ(dst.x, src.x);
    EXPECT_EQ(dst.y, src.y);
    EXPECT_EQ(dst.width, src.width);
    EXPECT_EQ(dst.height, src.height);
    EXPECT_EQ(QRect(dst), QRect(src));
}
