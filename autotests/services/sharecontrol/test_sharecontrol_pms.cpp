// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// sharecontrol 服务单元测试：服务无可链接目标，此处 unity-include 被测服务
// 实现源码，并对 DBus / QProcess / polkit / smb 工具等外部依赖打桩隔离。

#include <gtest/gtest.h>
#include <stubext.h>

#include <QDBusConnection>
#include <QDBusContext>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDBusUnixFileDescriptor>
#include <QDataStream>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>

#include <unistd.h>

#include "services/sharecontrol/sharecontroldbus.h"

// unity-include 被测实现（服务无可链接目标）
#include "services/sharecontrol/plugin.cpp"
#include "services/sharecontrol/sharecontroldbus.cpp"
#include "services/common/polkit/policykithelper.cpp"

SERVICESHARECONTROL_USE_NAMESPACE

namespace {
// ------------------------- QProcess 桩状态 -------------------------
QVector<QByteArray> g_outs;
QVector<QByteArray> g_errs;
QVector<int> g_exits;
QMap<quint64, int> g_idxOf;
int g_next { 0 };
bool g_waitFinished { true };
bool g_waitStarted { true };
int g_startCalls { 0 };

// ------------------------- 其它桩状态 -------------------------
int g_polkitOk { 0 };
uint g_serviceUid { 0 };
int g_serviceUidCalls { 0 };
bool g_connConnected { false };
int g_connIsConnectedCalls { 0 };
int g_registerObjectCalls { 0 };
int g_decryptCalls { 0 };

void resetGlobals()
{
    g_outs.clear();
    g_errs.clear();
    g_exits.clear();
    g_idxOf.clear();
    g_next = 0;
    g_waitFinished = true;
    g_waitStarted = true;
    g_startCalls = 0;
    g_polkitOk = 0;
    g_serviceUid = 0;
    g_serviceUidCalls = 0;
    g_connConnected = false;
    g_connIsConnectedCalls = 0;
    g_registerObjectCalls = 0;
    g_decryptCalls = 0;
}

int procIdx(void *proc)
{
    const int i = g_idxOf.value(reinterpret_cast<quint64>(proc), qMax(g_next - 1, 0));
    return qMin(i, qMax(g_outs.size() - 1, 0));
}
}   // namespace

class ShareControlServiceTest : public testing::Test
{
public:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        resetGlobals();

        // QDBusContext::message() 在无 DBus 上下文时会解引用空指针，桩为空消息
        static QDBusMessage s_emptyDBusMsg;
        stub.set_lamda(static_cast<const QDBusMessage &(QDBusContext::*)() const>(&QDBusContext::message),
                       [](QDBusContext *) -> const QDBusMessage & {
            return s_emptyDBusMsg;
        });


        // 隔离真实 DBus：systemBus / connectToBus 返回未连接对象
        stub.set_lamda(static_cast<QDBusConnection (*)()>(&QDBusConnection::systemBus),
                       []() -> QDBusConnection {
            return QDBusConnection(QString());
        });
        stub.set_lamda(
                static_cast<QDBusConnection (*)(QDBusConnection::BusType, const QString &)>(&QDBusConnection::connectToBus),
                [](QDBusConnection::BusType, const QString &) -> QDBusConnection {
            return QDBusConnection(QString());
        });
        stub.set_lamda(static_cast<bool (QDBusConnection::*)() const>(&QDBusConnection::isConnected),
                       [](QDBusConnection *) -> bool {
            ++g_connIsConnectedCalls;
            return g_connConnected;
        });
        stub.set_lamda(
                static_cast<bool (QDBusConnection::*)(const QString &, QObject *, QDBusConnection::RegisterOptions)>(&QDBusConnection::registerObject),
                [](QDBusConnection *, const QString &, QObject *, QDBusConnection::RegisterOptions) -> bool {
            ++g_registerObjectCalls;
            return true;
        });
        // interface() 桩：默认构造的连接 d 指针为空，真实调用会返回 nullptr，
        // 返回哑指针配合 serviceUid 桩使用（不会解引用）。
        stub.set_lamda(static_cast<QDBusConnectionInterface *(QDBusConnection::*)() const>(&QDBusConnection::interface),
                       [](QDBusConnection *) -> QDBusConnectionInterface * {
            return reinterpret_cast<QDBusConnectionInterface *>(quintptr(0x1));
        });
        // serviceUid 桩：避免对真实总线发起 GetNameOwner 类调用
        stub.set_lamda(&QDBusConnectionInterface::serviceUid,
                       [](QDBusConnectionInterface *, const QString &) -> QDBusReply<uint> {
            ++g_serviceUidCalls;
            QDBusMessage req = QDBusMessage::createMethodCall(
                    "org.freedesktop.DBus", "/", "org.freedesktop.DBus", "GetServiceUid");
            return QDBusReply<uint>(req.createReply(QVariant(uint(g_serviceUid))));
        });

        // polkit 授权桩（PolicyKitHelper 已 unity-include 进本 TU）
        stub.set_lamda(&ServiceCommon::PolicyKitHelper::checkAuthorization,
                       [](ServiceCommon::PolicyKitHelper *, const QString &, const QString &) -> bool {
            return g_polkitOk == 1;
        });

        // dfm-base 密码解密桩
        stub.set_lamda(&dfmbase::FileUtils::decryptString, [](const QString &str) -> QString {
            ++g_decryptCalls;
            return str;
        });

        // QProcess 桩：不启动真实 smbcontrol/smbpasswd/pdbedit
        stub.set_lamda(
                static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODeviceBase::OpenMode)>(&QProcess::start),
                [](QProcess *self, const QString &, const QStringList &, QIODeviceBase::OpenMode) {
            ++g_startCalls;
            g_idxOf[reinterpret_cast<quint64>(self)] = g_next++;
        });
        stub.set_lamda(static_cast<bool (QProcess::*)(int)>(&QProcess::waitForFinished),
                       [](QProcess *, int) -> bool { return g_waitFinished; });
        stub.set_lamda(static_cast<bool (QProcess::*)(int)>(&QProcess::waitForStarted),
                       [](QProcess *, int) -> bool { return g_waitStarted; });
        stub.set_lamda(static_cast<int (QProcess::*)() const>(&QProcess::exitCode),
                       [](QProcess *self) -> int {
            return g_exits.value(procIdx(self), 0);
        });
        stub.set_lamda(static_cast<QByteArray (QProcess::*)()>(&QProcess::readAllStandardOutput),
                       [](QProcess *self) -> QByteArray {
            return g_outs.value(procIdx(self), QByteArray());
        });
        stub.set_lamda(static_cast<QByteArray (QProcess::*)()>(&QProcess::readAllStandardError),
                       [](QProcess *self) -> QByteArray {
            return g_errs.value(procIdx(self), QByteArray());
        });
    }

    void TearDown() override
    {
        stub.clear();
        resetGlobals();
    }

    // 构造已授权的凭据管道（QDataStream 序列化 username/password）
    static QDBusUnixFileDescriptor makeCredentials(const QString &username, const QString &password)
    {
        int fds[2];
        if (pipe(fds) != 0)
            return QDBusUnixFileDescriptor();
        QByteArray buf;
        QDataStream w(&buf, QIODevice::WriteOnly);
        w << username << password;
        (void)!::write(fds[1], buf.constData(), buf.size());
        ::close(fds[1]);
        QDBusUnixFileDescriptor fd(fds[0]);
        ::close(fds[0]);
        return fd;
    }
};

/******************************************************************************/
/****************************** 生命周期 **************************************/
/******************************************************************************/

TEST_F(ShareControlServiceTest, DBus_Lifecycle_NotConnectedEarlyReturn)
{
    // Arrange
    g_connConnected = false;

    // Act
    ShareControlDBus dbus("ut.sharecontrol.service");

    // Assert
    EXPECT_GT(g_connIsConnectedCalls, 0);
    EXPECT_EQ(g_registerObjectCalls, 0);   // 未连接 → 不注册对象
    EXPECT_NE(dbus.adapter, nullptr);   // 适配器在连接前已创建
}

TEST_F(ShareControlServiceTest, DBus_Lifecycle_ConnectedRegistersObject)
{
    // Arrange
    g_connConnected = true;

    // Act
    ShareControlDBus dbus("ut.sharecontrol.service");

    // Assert
    EXPECT_EQ(g_registerObjectCalls, 1);
}

/******************************************************************************/
/*********************** CloseSmbShareByShareName *****************************/
/******************************************************************************/

TEST_F(ShareControlServiceTest, CloseSmbShare_ShowFalse_SkipsWithoutAuth)
{
    // Arrange
    ShareControlDBus dbus("ut.sharecontrol.service");
    g_polkitOk = 0;   // 未授权也应直接成功（无需鉴权的跳过分支）

    // Act
    const bool ok = dbus.CloseSmbShareByShareName("share", false);

    // Assert
    EXPECT_TRUE(ok);
    EXPECT_EQ(g_startCalls, 0);
}

TEST_F(ShareControlServiceTest, CloseSmbShare_AuthFailed_ReturnsFalse)
{
    // Arrange
    ShareControlDBus dbus("ut.sharecontrol.service");
    g_polkitOk = 0;

    // Act
    const bool ok = dbus.CloseSmbShareByShareName("share", true);

    // Assert
    EXPECT_FALSE(ok);
    EXPECT_EQ(g_startCalls, 0);
}

TEST_F(ShareControlServiceTest, CloseSmbShare_BusNotConnected_ReturnsFalse)
{
    // Arrange
    ShareControlDBus dbus("ut.sharecontrol.service");
    g_polkitOk = 1;
    g_connConnected = false;

    // Act
    const bool ok = dbus.CloseSmbShareByShareName("share", true);

    // Assert
    EXPECT_FALSE(ok);
    EXPECT_EQ(g_serviceUidCalls, 0);
}

TEST_F(ShareControlServiceTest, CloseSmbShare_RootCaller_ExecutesSmbcontrol)
{
    // Arrange
    ShareControlDBus dbus("ut.sharecontrol.service");
    g_polkitOk = 1;
    g_connConnected = true;
    g_serviceUid = 0;   // root 调用者 → 属主校验放行
    g_waitFinished = true;
    g_outs = { QByteArray("") };
    g_exits = { 0 };

    // Act
    const bool ok = dbus.CloseSmbShareByShareName("MyShare", true);

    // Assert
    EXPECT_TRUE(ok);
    EXPECT_EQ(g_startCalls, 1);
    EXPECT_EQ(g_serviceUidCalls, 1);
}

TEST_F(ShareControlServiceTest, CloseSmbShare_OtherOwner_ReturnsFalse)
{
    // Arrange
    ShareControlDBus dbus("ut.sharecontrol.service");
    g_polkitOk = 1;
    g_connConnected = true;
    g_serviceUid = 1000;   // 非 root 且与文件属主（不存在文件 ownerId=-1）不同

    // Act
    const bool ok = dbus.CloseSmbShareByShareName("MyShare", true);

    // Assert
    EXPECT_FALSE(ok);
    EXPECT_EQ(g_startCalls, 0);
}

TEST_F(ShareControlServiceTest, CloseSmbShare_PathTraversal_ReturnsFalse)
{
    // Arrange
    ShareControlDBus dbus("ut.sharecontrol.service");
    g_polkitOk = 1;
    g_connConnected = true;
    g_serviceUid = 0;

    // Act
    const bool ok = dbus.CloseSmbShareByShareName("../../etc/passwd", true);

    // Assert
    EXPECT_FALSE(ok);
    EXPECT_EQ(g_startCalls, 0);
}

TEST_F(ShareControlServiceTest, CloseSmbShare_SmbcontrolTimeout_ReturnsFalse)
{
    // Arrange
    ShareControlDBus dbus("ut.sharecontrol.service");
    g_polkitOk = 1;
    g_connConnected = true;
    g_serviceUid = 0;
    g_waitFinished = false;

    // Act
    const bool ok = dbus.CloseSmbShareByShareName("MyShare", true);

    // Assert
    EXPECT_FALSE(ok);
    EXPECT_EQ(g_startCalls, 1);
}

/******************************************************************************/
/************************* SetUserSharePassword *******************************/
/******************************************************************************/

TEST_F(ShareControlServiceTest, SetUserSharePassword_AuthFailed_ReturnsFalse)
{
    // Arrange
    ShareControlDBus dbus("ut.sharecontrol.service");
    g_polkitOk = 0;

    // Act
    const bool ok = dbus.SetUserSharePassword(QDBusUnixFileDescriptor());

    // Assert
    EXPECT_FALSE(ok);
    EXPECT_EQ(g_startCalls, 0);
}

TEST_F(ShareControlServiceTest, SetUserSharePassword_InvalidFd_ReturnsFalse)
{
    // Arrange
    ShareControlDBus dbus("ut.sharecontrol.service");
    g_polkitOk = 1;

    // Act
    const bool ok = dbus.SetUserSharePassword(QDBusUnixFileDescriptor());

    // Assert
    EXPECT_FALSE(ok);
}

TEST_F(ShareControlServiceTest, SetUserSharePassword_EmptyPipe_ReturnsFalse)
{
    // Arrange
    ShareControlDBus dbus("ut.sharecontrol.service");
    g_polkitOk = 1;
    int fds[2];
    ASSERT_EQ(pipe(fds), 0);
    ::close(fds[1]);
    QDBusUnixFileDescriptor fd(fds[0]);
    ::close(fds[0]);

    // Act
    const bool ok = dbus.SetUserSharePassword(fd);

    // Assert
    EXPECT_FALSE(ok);
}

TEST_F(ShareControlServiceTest, SetUserSharePassword_MalformedStream_ReturnsFalse)
{
    // Arrange
    ShareControlDBus dbus("ut.sharecontrol.service");
    g_polkitOk = 1;
    int fds[2];
    ASSERT_EQ(pipe(fds), 0);
    (void)!::write(fds[1], "zz", 2);
    ::close(fds[1]);
    QDBusUnixFileDescriptor fd(fds[0]);
    ::close(fds[0]);

    // Act
    const bool ok = dbus.SetUserSharePassword(fd);

    // Assert
    EXPECT_FALSE(ok);
}

TEST_F(ShareControlServiceTest, SetUserSharePassword_InvalidUsername_ReturnsFalse)
{
    // Arrange
    ShareControlDBus dbus("ut.sharecontrol.service");
    g_polkitOk = 1;

    // Act
    const bool ok = dbus.SetUserSharePassword(makeCredentials("bad user!", "pw"));

    // Assert
    EXPECT_FALSE(ok);
    EXPECT_EQ(g_startCalls, 0);   // 用户名校验失败 → 不启动 smbpasswd
}

TEST_F(ShareControlServiceTest, SetUserSharePassword_ValidCredentials_Succeeds)
{
    // Arrange
    ShareControlDBus dbus("ut.sharecontrol.service");
    g_polkitOk = 1;
    g_waitStarted = true;
    g_waitFinished = true;
    g_outs = { QByteArray("") };
    g_exits = { 0 };

    // Act
    const bool ok = dbus.SetUserSharePassword(makeCredentials("ut_user-1", "s3cret"));

    // Assert
    EXPECT_TRUE(ok);
    EXPECT_EQ(g_startCalls, 1);
    EXPECT_EQ(g_decryptCalls, 1);
}

TEST_F(ShareControlServiceTest, SetUserSharePassword_SmbpasswdFailsToStart_ReturnsFalse)
{
    // Arrange
    ShareControlDBus dbus("ut.sharecontrol.service");
    g_polkitOk = 1;
    g_waitStarted = false;

    // Act
    const bool ok = dbus.SetUserSharePassword(makeCredentials("ut_user-1", "s3cret"));

    // Assert
    EXPECT_FALSE(ok);
}

TEST_F(ShareControlServiceTest, SetUserSharePassword_SmbpasswdHangs_ReturnsFalse)
{
    // Arrange
    ShareControlDBus dbus("ut.sharecontrol.service");
    g_polkitOk = 1;
    g_waitStarted = true;
    g_waitFinished = false;

    // Act
    const bool ok = dbus.SetUserSharePassword(makeCredentials("ut_user-1", "s3cret"));

    // Assert
    EXPECT_FALSE(ok);
}

/******************************************************************************/
/************************* IsUserSharePasswordSet *****************************/
/******************************************************************************/

TEST_F(ShareControlServiceTest, IsUserSharePasswordSet_UserListed_ReturnsTrue)
{
    // Arrange
    ShareControlDBus dbus("ut.sharecontrol.service");
    g_outs = { QByteArray("ut_user-1:1000:UT User:/home/ut:/bin/bash\nother:1001:x:/home/o:/bin/sh\n") };
    g_exits = { 0 };
    g_waitFinished = true;

    // Act
    const bool set = dbus.IsUserSharePasswordSet("ut_user-1");

    // Assert
    EXPECT_TRUE(set);
    EXPECT_EQ(g_startCalls, 1);
}

TEST_F(ShareControlServiceTest, IsUserSharePasswordSet_UserMissing_ReturnsFalse)
{
    // Arrange
    ShareControlDBus dbus("ut.sharecontrol.service");
    g_outs = { QByteArray("other:1001:x:/home/o:/bin/sh\n") };
    g_exits = { 0 };
    g_waitFinished = true;

    // Act
    const bool set = dbus.IsUserSharePasswordSet("ut_user-1");

    // Assert
    EXPECT_FALSE(set);
}

TEST_F(ShareControlServiceTest, IsUserSharePasswordSet_PdbeditFails_ReturnsFalse)
{
    // Arrange
    ShareControlDBus dbus("ut.sharecontrol.service");
    g_waitFinished = false;

    // Act
    const bool set = dbus.IsUserSharePasswordSet("ut_user-1");

    // Assert
    EXPECT_FALSE(set);
}

/******************************************************************************/
/***************************** 内部方法直测 ***********************************/
/******************************************************************************/

TEST_F(ShareControlServiceTest, CheckAuthentication_DefaultDenied_StubbedGranted)
{
    // Arrange
    ShareControlDBus dbus("ut.sharecontrol.service");

    // Act
    const bool denied = dbus.checkAuthentication();
    g_polkitOk = 1;
    const bool granted = dbus.checkAuthentication();

    // Assert
    EXPECT_FALSE(denied);
    EXPECT_TRUE(granted);
}

TEST_F(ShareControlServiceTest, IsValidUsername_FullMatrix)
{
    // Arrange
    ShareControlDBus dbus("ut.sharecontrol.service");
    const QString len32(QString::fromLatin1("abcdefghijklmnopqrstuvwxyz012345"));   // 恰 32 字符
    const QString len33(QString::fromLatin1("abcdefghijklmnopqrstuvwxyz0123456"));   // 33 字符

    // Act / Assert
    EXPECT_FALSE(dbus.isValidUsername(""));
    EXPECT_FALSE(dbus.isValidUsername(len33));
    EXPECT_TRUE(dbus.isValidUsername(len32));
    EXPECT_TRUE(dbus.isValidUsername("ut_user-1"));
    EXPECT_TRUE(dbus.isValidUsername("a"));
    EXPECT_FALSE(dbus.isValidUsername("-leading"));   // 不允许以连字符开头
    EXPECT_FALSE(dbus.isValidUsername("has space"));
    EXPECT_FALSE(dbus.isValidUsername("has;semicolon"));
    EXPECT_FALSE(dbus.isValidUsername("double--hyphen"));   // 会被解释为选项
}

/******************************************************************************/
/****************************** 插件入口 **************************************/
/******************************************************************************/

TEST_F(ShareControlServiceTest, DsmRegisterAndUnRegister_Lifecycle)
{
    // Arrange / Act
    const int reg = DSMRegister("ut.sharecontrol.service", nullptr);

    // Assert
    EXPECT_EQ(reg, 0);

    // Act
    const int unreg = DSMUnRegister("ut.sharecontrol.service", nullptr);
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);

    // Assert
    EXPECT_EQ(unreg, 0);
}

/******************************************************************************/
/****************************** 日志类别覆盖 **********************************/
/******************************************************************************/

TEST_F(ShareControlServiceTest, LogCategory_RegisteredAndFetchable)
{
    // Arrange
    const QLoggingCategory *viaHelper = &service_sharecontrol::__getLogCategoryName();

    // Act
    const QLoggingCategory *registered = &service_sharecontrol::__logservice_sharecontrol();

    // Assert
    EXPECT_EQ(viaHelper, registered);
    EXPECT_STREQ(registered->categoryName(), "org.deepin.dde.filemanager.plugin.service_sharecontrol");
}
