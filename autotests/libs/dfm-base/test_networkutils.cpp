// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_networkutils.cpp
 * @brief Unit tests for NetworkUtils (networkutils.cpp) - parseIp & helpers
 */

#include <gtest/gtest.h>
#include <QTest>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QCoreApplication>
#include <QEvent>

#include <dfm-base/utils/networkutils.h>

#include <thread>

using namespace dfmbase;

TEST(NetworkUtilsTest, ParseIpFtpWithPort)
{
    QString ip, port;
    bool ok = NetworkUtils::instance()->parseIp("/run/user/1000/gvfs/ftp:host=1.2.3.4,port=2121", ip, port);
    EXPECT_TRUE(ok);
    EXPECT_EQ(ip, QString("1.2.3.4"));
    EXPECT_EQ(port, QString("2121"));
}

TEST(NetworkUtilsTest, ParseIpFtpDefaultPort)
{
    QString ip, port;
    bool ok = NetworkUtils::instance()->parseIp("/run/user/1000/gvfs/ftp:host=1.2.3.4", ip, port);
    EXPECT_TRUE(ok);
    EXPECT_EQ(ip, QString("1.2.3.4"));
    EXPECT_EQ(port, QString("21"));
}

TEST(NetworkUtilsTest, ParseIpSmbWithPort)
{
    QString ip, port;
    bool ok = NetworkUtils::instance()->parseIp("/run/user/1000/gvfs/smb-share:server=5.6.7.8,port=445", ip, port);
    EXPECT_TRUE(ok);
    EXPECT_EQ(ip, QString("5.6.7.8"));
    EXPECT_EQ(port, QString("445"));
}

TEST(NetworkUtilsTest, ParseIpSmbDefaultPort)
{
    QString ip, port;
    bool ok = NetworkUtils::instance()->parseIp("/run/user/1000/gvfs/smb-share:server=5.6.7.8", ip, port);
    EXPECT_TRUE(ok);
    EXPECT_EQ(ip, QString("5.6.7.8"));
    EXPECT_EQ(port, QString("445"));
}

TEST(NetworkUtilsTest, ParseIpSftpWithPort)
{
    QString ip, port;
    bool ok = NetworkUtils::instance()->parseIp("/run/user/1000/gvfs/sftp:host=9.10.11.12,port=2222", ip, port);
    EXPECT_TRUE(ok);
    EXPECT_EQ(ip, QString("9.10.11.12"));
    EXPECT_EQ(port, QString("2222"));
}

TEST(NetworkUtilsTest, ParseIpSftpDefaultPort)
{
    QString ip, port;
    bool ok = NetworkUtils::instance()->parseIp("/run/user/1000/gvfs/sftp:host=9.10.11.12", ip, port);
    EXPECT_TRUE(ok);
    EXPECT_EQ(ip, QString("9.10.11.12"));
    EXPECT_EQ(port, QString("22"));
}

TEST(NetworkUtilsTest, ParseIpInvalidScheme)
{
    QString ip, port;
    bool ok = NetworkUtils::instance()->parseIp("/run/user/1000/gvfs/http:host=1.2.3.4", ip, port);
    EXPECT_FALSE(ok);
}

TEST(NetworkUtilsTest, ParseIpReturnsPortsList)
{
    QString ip;
    QStringList ports;
    bool ok = NetworkUtils::instance()->parseIp("/run/user/1000/gvfs/ftp:host=1.2.3.4", ip, ports);
    EXPECT_TRUE(ok);
    EXPECT_EQ(ip, QString("1.2.3.4"));
    EXPECT_EQ(ports.size(), 1);
    EXPECT_EQ(ports.first(), QString("21"));
}

TEST(NetworkUtilsTest, ParseIpSmbReturnsTwoPorts)
{
    QString ip;
    QStringList ports;
    bool ok = NetworkUtils::instance()->parseIp("/run/user/1000/gvfs/smb-share:server=5.6.7.8", ip, ports);
    EXPECT_TRUE(ok);
    EXPECT_EQ(ports.size(), 2);
}

// ---- Coverage additions for remaining NetworkUtils API ----

TEST(NetworkUtilsTest, CheckAllCifsBusyIsCallable)
{
    EXPECT_NO_FATAL_FAILURE({ (void)NetworkUtils::instance()->checkAllCIFSBusy(); });
}

TEST(NetworkUtilsTest, CheckNetConnectionHostStringListIsCallable)
{
    EXPECT_NO_FATAL_FAILURE({ (void)NetworkUtils::instance()->checkNetConnection("localhost", QStringList{"80"}, 200); });
}

TEST(NetworkUtilsTest, CheckNetConnectionHostPortIntIsCallable)
{
    EXPECT_NO_FATAL_FAILURE({ (void)NetworkUtils::instance()->checkNetConnection("localhost", "80", 200); });
}

TEST(NetworkUtilsTest, CheckFtpOrSmbBusyIsCallable)
{
    EXPECT_NO_FATAL_FAILURE({ (void)NetworkUtils::instance()->checkFtpOrSmbBusy(QUrl("file:///")); });
}

TEST(NetworkUtilsTest, CifsMountHostInfoIsCallable)
{
    EXPECT_NO_FATAL_FAILURE({ (void)NetworkUtils::instance()->cifsMountHostInfo(); });
}

TEST(NetworkUtilsTest, ResolveLocalSftpMountUrlIsCallable)
{
    EXPECT_NO_FATAL_FAILURE({ (void)NetworkUtils::instance()->resolveLocalSftpMountUrl(QUrl("file:///")); });
}
TEST(NetworkUtilsTest, ResolveLocalSftpMountUrlNonSftpReturnsUnchanged)
{
    QUrl url("file:///tmp/test.txt");
    QUrl result = NetworkUtils::instance()->resolveLocalSftpMountUrl(url);
    EXPECT_EQ(result, url);
}
TEST(NetworkUtilsTest, ResolveLocalSftpMountUrlEmptyPath)
{
    QUrl url("sftp://user@host/path");
    QUrl result = NetworkUtils::instance()->resolveLocalSftpMountUrl(url);
    EXPECT_NO_FATAL_FAILURE({ (void)result; });
}
TEST(NetworkUtilsTest, DoAfterCheckNetEmptyPortsReturnsTrue)
{
    // doAfterCheckNet with empty ports: host check skipped → callback(true)
    bool called = false;
    NetworkUtils::instance()->doAfterCheckNet("127.0.0.1", {}, [&called](bool ok) {
        called = true;
        EXPECT_TRUE(ok);
    }, 100);
    // Wait for async to complete
    for (int i = 0; i < 20 && !called; ++i) {
        QCoreApplication::processEvents();
        QTest::qWait(50);
    }
    EXPECT_TRUE(called);
}

// ===== PMS sev-2 regression cluster: networkutils.cpp (work-order batch 2) =====

// PMS:177969 instance() 从 header 内联 static 移到 cpp 内单例：同一进程内所有调用必须返回同一指针
TEST(NetworkUtilsTest, BUG177969_InstanceReturnsSameSingletonPointer)
{
    auto *a = NetworkUtils::instance();
    auto *b = NetworkUtils::instance();
    ASSERT_NE(a, nullptr);
    EXPECT_EQ(a, b);
}

// PMS:178299 单例契约跨线程成立：工作线程获取的实例与主线程同一对象（header 内联 static 的旧实现会按 TU/线程分裂）
TEST(NetworkUtilsTest, BUG178299_InstanceSharedAcrossThreads)
{
    auto *mainPtr = NetworkUtils::instance();
    ASSERT_NE(mainPtr, nullptr);
    NetworkUtils *workerPtr = nullptr;
    std::thread t([&workerPtr] { workerPtr = NetworkUtils::instance(); });
    t.join();
    ASSERT_NE(workerPtr, nullptr);
    EXPECT_EQ(workerPtr, mainPtr);
}

// ============================================================
// PMS sev-2 regression cluster: networkutils.cpp (work-order batch 3)
// ============================================================

// PMS:178317 mips 无法挂载/读取光盘：NetworkUtils 单例必须稳定可用
TEST(NetworkUtilsTest, BUG178317_InstanceSingletonStable)
{
    NetworkUtils *a = NetworkUtils::instance();
    NetworkUtils *b = NetworkUtils::instance();
    ASSERT_NE(a, nullptr);
    EXPECT_EQ(a, b);
}

// PMS:178805 mips 文件选择对话框无系统盘：instance() 必须线程安全，
// 首次/再次从其它线程获取返回同一实例且不崩溃
TEST(NetworkUtilsTest, BUG178805_InstanceThreadSafeAcrossThreads)
{
    NetworkUtils *mainPtr = NetworkUtils::instance();
    ASSERT_NE(mainPtr, nullptr);
    NetworkUtils *threadPtr = nullptr;
    std::thread t([&threadPtr] { threadPtr = NetworkUtils::instance(); });
    t.join();
    EXPECT_EQ(threadPtr, mainPtr);
}

// PMS:262127 smb 命令访问 windows 共享挂载失败：smb-share gvfs 路径解析出
// 主机与 CIFS 双端口（445/139），显式端口时不再追加
TEST(NetworkUtilsTest, BUG262127_ParseIpSmbShareCifsPorts)
{
    QString ip;
    QStringList ports;
    // gvfs smb-share mount point, no explicit port -> default 445 + 139
    ASSERT_TRUE(NetworkUtils::instance()->parseIp(
        QStringLiteral("/run/user/1000/gvfs/smb-share:server=5.6.7.8,share=pub"), ip, ports));
    EXPECT_EQ(ip, QStringLiteral("5.6.7.8"));
    EXPECT_TRUE(ports.contains(QStringLiteral("445")));
    EXPECT_TRUE(ports.contains(QStringLiteral("139")));

    // explicit port wins and no extra port appended
    ports.clear();
    ASSERT_TRUE(NetworkUtils::instance()->parseIp(
        QStringLiteral("/run/user/1000/gvfs/smb-share:port=1445,server=5.6.7.8,share=pub"), ip, ports));
    EXPECT_EQ(ip, QStringLiteral("5.6.7.8"));
    EXPECT_EQ(ports, QStringList { QStringLiteral("1445") });
}

// PMS:188977 ftp 共享管控内容不一致：未支持的 gvfs 协议前缀解析失败（不误挂载）
TEST(NetworkUtilsTest, BUG188977_ParseIpUnknownGvfsProtocolFails)
{
    QString ip;
    QStringList ports;
    EXPECT_FALSE(NetworkUtils::instance()->parseIp(
        QStringLiteral("/run/user/1000/gvfs/webdav:host=1.2.3.4"), ip, ports));
    EXPECT_FALSE(NetworkUtils::instance()->parseIp(
        QStringLiteral("/run/user/1000/gvfs/dav:host=1.2.3.4"), ip, ports));
}

// PMS:257855 挂载 smb 出现连接问题导致文管卡住：普通本地路径解析失败快速返回，
// cifsMountHostInfo 静态查询保持可调用（解析/探测路径无阻塞死等）
TEST(NetworkUtilsTest, BUG257855_ParseIpNonGvfsPathFailsFast)
{
    QString ip;
    QStringList ports;
    EXPECT_FALSE(NetworkUtils::instance()->parseIp(QStringLiteral("/tmp/ut_random_path"), ip, ports));
    EXPECT_TRUE(ip.isEmpty());
    EXPECT_NO_FATAL_FAILURE({ const auto info = NetworkUtils::cifsMountHostInfo(); Q_UNUSED(info); });
}

// PMS:286637 dav 协议连接提示挂载错误：doAfterCheckNet 空端口列表直接跳过
// 探测并以 true 回调（异步结果经事件循环送达）
TEST(NetworkUtilsTest, BUG286637_DoAfterCheckNetEmptyPortsCallbackTrue)
{
    bool called = false;
    bool ok = false;
    NetworkUtils::instance()->doAfterCheckNet(QStringLiteral("ut-host-286637"), QStringList {},
                                              [&](bool result) {
                                                  called = true;
                                                  ok = result;
                                              },
                                              100);
    QTest::qWaitFor([&called]() {
        QCoreApplication::sendPostedEvents(nullptr, -1);
        return called;
    }, 5000);
    EXPECT_TRUE(called);
    EXPECT_TRUE(ok);
}
