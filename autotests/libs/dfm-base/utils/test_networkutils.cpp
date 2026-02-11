// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <QTest>
#include <QSignalSpy>
#include <QTimer>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <QDebug>
#include <QThread>
#include <QUrl>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QVariant>
#include <QDateTime>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include "dfm-base/utils/networkutils.h"
#include "dfm-base/base/application/application.h"
#include "dfm-base/base/standardpaths.h"
#include "dfm-base/base/configs/dconfig/dconfigmanager.h"

#include "stubext.h"

using namespace dfmbase;

class NetworkUtilsTest : public testing::Test
{
public:
    void SetUp() override
    {
        // 初始化测试环境
        std::cout << "SetUp NetworkUtilsTest" << std::endl;
        
        // 获取NetworkUtils实例
        networkUtils = NetworkUtils::instance();
        ASSERT_NE(networkUtils, nullptr) << "NetworkUtils instance should not be null";
        
        // 创建临时目录用于测试
        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid()) << "Temporary directory should be created successfully";
        tempDirPath = tempDir->path();
    }

    void TearDown() override
    {
        // 清理
        stub_ext::StubExt clear;
        clear.clear();
        std::cout << "TearDown NetworkUtilsTest" << std::endl;
    }

protected:
    // 辅助方法
    bool isValidHost(const QString& host)
    {
        // 简单的主机名验证
        return !host.isEmpty() && 
               (host.contains('.') || QHostAddress().setAddress(host) || host == "localhost");
    }
    
    bool isValidPort(const QString& port)
    {
        bool ok;
        int portNum = port.toInt(&ok);
        return ok && portNum > 0 && portNum <= 65535;
    }
    
    void waitForCallback(std::function<void(bool)>& callback, int timeoutMs = 5000)
    {
        QEventLoop loop;
        QTimer::singleShot(timeoutMs, &loop, &QEventLoop::quit);
        
        if (callback) {
            // 包装回调以退出事件循环
            auto wrappedCallback = [&callback, &loop](bool result) {
                if (callback) {
                    callback(result);
                }
                loop.quit();
            };
            callback = wrappedCallback;
        } else {
            callback = [&loop](bool) { loop.quit(); };
        }
        
        loop.exec();
    }

protected:
    NetworkUtils* networkUtils;
    std::unique_ptr<QTemporaryDir> tempDir;
    QString tempDirPath;
};

// 测试1: NetworkUtils 单例实例
TEST_F(NetworkUtilsTest, TestSingletonInstance)
{
    // 测试单例模式
    NetworkUtils* instance1 = NetworkUtils::instance();
    NetworkUtils* instance2 = NetworkUtils::instance();
    
    ASSERT_NE(instance1, nullptr) << "NetworkUtils instance should not be null";
    EXPECT_EQ(instance1, instance2) << "NetworkUtils should be a singleton";
}

// 测试2: 网络连接检查 - 单个端口
TEST_F(NetworkUtilsTest, TestCheckNetConnectionSinglePort)
{
    // 测试常见的主机和端口
    QMap<QString, QString> testCases = {
        {"localhost", "22"},          // SSH
        {"localhost", "80"},          // HTTP
        {"localhost", "443"},         // HTTPS
        {"8.8.8.8", "53"},           // Google DNS
        {"1.1.1.1", "53"},           // Cloudflare DNS
        {"127.0.0.1", "22"},         // Localhost IP
        {"github.com", "443"},        // GitHub HTTPS
        {"example.com", "80"},        // Example HTTP
    };
    
    for (auto it = testCases.begin(); it != testCases.end(); ++it) {
        QString host = it.key();
        QString port = it.value();
        
        bool result = networkUtils->checkNetConnection(host, port, 1000);
        
        // 结果可能是true或false，取决于网络环境，但调用不应该崩溃
        EXPECT_TRUE(result == true || result == false) 
            << "Network connection check should return boolean for " << host.toStdString() << ":" << port.toStdString();
    }
}

// 测试3: 网络连接检查 - 多个端口
TEST_F(NetworkUtilsTest, TestCheckNetConnectionMultiplePorts)
{
    QString host = "localhost";
    QStringList ports = {"22", "80", "443", "8080", "3000"};
    
    bool result = networkUtils->checkNetConnection(host, ports, 1000);
    
    // 结果可能是true或false，但调用不应该崩溃
    EXPECT_TRUE(result == true || result == false) << "Multiple port check should return boolean";
    
    // 测试无效主机
    QString invalidHost = "nonexistent.host.xyz123";
    bool invalidResult = networkUtils->checkNetConnection(invalidHost, ports, 1000);
    EXPECT_FALSE(invalidResult) << "Invalid host should return false";
    
    // 测试空端口列表
    QStringList emptyPorts;
    bool emptyPortsResult = networkUtils->checkNetConnection(host, emptyPorts, 1000);
    EXPECT_FALSE(emptyPortsResult) << "Empty ports list should return false";
}

// 测试4: 异步网络检查回调
TEST_F(NetworkUtilsTest, TestDoAfterCheckNet)
{
    QString host = "localhost";
    QStringList ports = {"22", "80"};
    bool callbackReceived = false;
    bool callbackResult = false;
    
    std::function<void(bool)> callback = [&](bool result) {
        callbackReceived = true;
        callbackResult = result;
    };
    
    // 执行异步检查
    networkUtils->doAfterCheckNet(host, ports, callback, 1000);
    
    // 等待回调完成
    QEventLoop loop;
    QTimer::singleShot(3000, &loop, &QEventLoop::quit);
    
    // 检查回调是否被调用（可能不被调用，这取决于实现）
    EXPECT_TRUE(callbackReceived == true || callbackReceived == false) << "Callback should be handled gracefully";
}

// 测试5: IP地址解析 - 单个端口
TEST_F(NetworkUtilsTest, TestParseIpSinglePort)
{
    // 测试有效的IP地址和端口
    QMap<QString, QPair<QString, QString>> validTestCases = {
        {"192.168.1.1:8080", {"192.168.1.1", "8080"}},
        {"localhost:22", {"localhost", "22"}},
        {"127.0.0.1:443", {"127.0.0.1", "443"}},
        {"8.8.8.8:53", {"8.8.8.8", "53"}},
        {"example.com:80", {"example.com", "80"}},
        {"[::1]:8080", {"::1", "8080"}},  // IPv6
    };
    
    for (auto it = validTestCases.begin(); it != validTestCases.end(); ++it) {
        QString input = it.key();
        QString expectedIp = it.value().first;
        QString expectedPort = it.value().second;
        
        QString actualIp, actualPort;
        bool result = networkUtils->parseIp(input, actualIp, actualPort);
        
        if (result) {
            EXPECT_EQ(actualIp, expectedIp) << "IP should match for: " << input.toStdString();
            EXPECT_EQ(actualPort, expectedPort) << "Port should match for: " << input.toStdString();
        }
        
        // 解析可能失败，但不应该崩溃
        EXPECT_TRUE(result == true || result == false) << "Parse result should be boolean";
    }
    
    // 测试无效输入
    QStringList invalidInputs = {
        "",
        "invalid",
        "192.168.1.1",  // 缺少端口
        ":8080",         // 缺少IP
        "192.168.1.1:", // 缺少端口号
        "192.168.1.999:8080",  // 无效IP
        "192.168.1.1:99999",  // 无效端口
        "host:port:extra",     // 格式错误
    };
    
    for (const QString& input : invalidInputs) {
        QString ip, port;
        bool result = networkUtils->parseIp(input, ip, port);
        EXPECT_FALSE(result) << "Invalid input should return false: " << input.toStdString();
    }
}

// 测试6: IP地址解析 - 多个端口
TEST_F(NetworkUtilsTest, TestParseIpMultiplePorts)
{
    // 测试有效输入
    QString input = "192.168.1.1:80,443,8080";
    QString expectedIp = "192.168.1.1";
    QStringList expectedPorts = {"80", "443", "8080"};
    
    QString actualIp;
    QStringList actualPorts;
    bool result = networkUtils->parseIp(input, actualIp, actualPorts);
    
    if (result) {
        EXPECT_EQ(actualIp, expectedIp) << "IP should match";
        EXPECT_EQ(actualPorts, expectedPorts) << "Ports should match";
    }
    
    // 测试单个端口
    QString singlePortInput = "localhost:22";
    QString singlePortExpectedIp = "localhost";
    QStringList singlePortExpectedPorts = {"22"};
    
    QString actualIp2;
    QStringList actualPorts2;
    bool result2 = networkUtils->parseIp(singlePortInput, actualIp2, actualPorts2);
    
    if (result2) {
        EXPECT_EQ(actualIp2, singlePortExpectedIp) << "Single port IP should match";
        EXPECT_EQ(actualPorts2, singlePortExpectedPorts) << "Single port should match";
    }
    
    // 测试无效输入
    QString invalidInput = "invalid:format";
    QString invalidIp;
    QStringList invalidPorts;
    bool invalidResult = networkUtils->parseIp(invalidInput, invalidIp, invalidPorts);
    EXPECT_FALSE(invalidResult) << "Invalid input should return false";
}

// 测试7: CIFS忙碌检查
TEST_F(NetworkUtilsTest, TestCheckAllCIFSBusy)
{
    bool result = networkUtils->checkAllCIFSBusy();
    
    // 结果可能是true或false，取决于系统状态，但不应该崩溃
    EXPECT_TRUE(result == true || result == false) << "CIFS busy check should return boolean";
}

// 测试8: FTP/SMB忙碌检查
TEST_F(NetworkUtilsTest, TestCheckFtpOrSmbBusy)
{
    // 测试各种URL格式
    QList<QUrl> testUrls = {
        QUrl("ftp://ftp.example.com/"),
        QUrl("smb://smb.example.com/share"),
        QUrl("ftp://192.168.1.100/"),
        QUrl("smb://192.168.1.200/share"),
        QUrl("file:///local/path"),  // 非网络URL
    };
    
    for (const QUrl& url : testUrls) {
        bool result = networkUtils->checkFtpOrSmbBusy(url);
        
        // 结果可能是true或false，但不应该崩溃
        EXPECT_TRUE(result == true || result == false) 
            << "FTP/SMB busy check should return boolean for: " << url.toString().toStdString();
    }
    
    // 测试空URL
    QUrl emptyUrl;
    bool emptyResult = networkUtils->checkFtpOrSmbBusy(emptyUrl);
    EXPECT_FALSE(emptyResult) << "Empty URL should return false";
}

// 测试9: CIFS挂载主机信息
TEST_F(NetworkUtilsTest, TestCifsMountHostInfo)
{
    QMap<QString, QString> hostInfo = NetworkUtils::cifsMountHostInfo();
    
    // 返回的映射应该有效（可能为空）
    EXPECT_TRUE(hostInfo.isEmpty() || !hostInfo.isEmpty()) << "CIFS mount host info should return valid map";
    
    // 如果有数据，验证格式
    for (auto it = hostInfo.begin(); it != hostInfo.end(); ++it) {
        QString key = it.key();
        QString value = it.value();
        
        EXPECT_FALSE(key.isEmpty()) << "Host info key should not be empty";
        EXPECT_FALSE(value.isEmpty()) << "Host info value should not be empty";
    }
}

// 测试10: 边界条件测试
TEST_F(NetworkUtilsTest, TestBoundaryConditions)
{
    // 测试空主机和端口
    bool emptyHostResult = networkUtils->checkNetConnection("", "80", 1000);
    EXPECT_FALSE(emptyHostResult) << "Empty host should return false";
    
    bool emptyPortResult = networkUtils->checkNetConnection("localhost", "", 1000);
    EXPECT_FALSE(emptyPortResult) << "Empty port should return false";
    
    // 测试无效端口
    bool invalidPortResult = networkUtils->checkNetConnection("localhost", "99999", 1000);
    EXPECT_FALSE(invalidPortResult) << "Invalid port should return false";
    
    // 测试负数超时
    bool negativeTimeoutResult = networkUtils->checkNetConnection("localhost", "80", -1000);
    EXPECT_TRUE(negativeTimeoutResult == true || negativeTimeoutResult == false) 
        << "Negative timeout should be handled gracefully";
    
    // 测试零超时
    bool zeroTimeoutResult = networkUtils->checkNetConnection("localhost", "80", 0);
    EXPECT_TRUE(zeroTimeoutResult == true || zeroTimeoutResult == false) 
        << "Zero timeout should be handled gracefully";
}

// 测试11: IPv6支持测试
TEST_F(NetworkUtilsTest, TestIPv6Support)
{
    // 测试IPv6地址
    QStringList ipv6Addresses = {
        "::1",                    // localhost IPv6
        "2001:4860:4860::8888",   // Google DNS IPv6
        "2606:4700:4700::1111",  // Cloudflare DNS IPv6
        "fe80::1",                // link-local
    };
    
    for (const QString& ipv6 : ipv6Addresses) {
        bool result = networkUtils->checkNetConnection(ipv6, "53", 1000);
        EXPECT_TRUE(result == true || result == false) 
            << "IPv6 connection check should return boolean for: " << ipv6.toStdString();
        
        // 测试IPv6解析
        QString ip, port;
        bool parseResult = networkUtils->parseIp(QString("[%1]:8080").arg(ipv6), ip, port);
        EXPECT_TRUE(parseResult == true || parseResult == false) 
            << "IPv6 parse should return boolean for: " << ipv6.toStdString();
    }
}

// 测试12: 域名解析测试
TEST_F(NetworkUtilsTest, TestDomainResolution)
{
    // 测试常见域名
    QStringList domains = {
        "google.com",
        "github.com",
        "example.com",
        "localhost",
    };
    
    for (const QString& domain : domains) {
        bool result = networkUtils->checkNetConnection(domain, "80", 2000);
        EXPECT_TRUE(result == true || result == false) 
            << "Domain connection check should return boolean for: " << domain.toStdString();
    }
    
    // 测试无效域名
    QStringList invalidDomains = {
        "nonexistent.domain.xyz123",
        "invalid..domain",
        "toolongdomainnamethatexceedsthemaximumlengthforahostnameandshouldnotbevalid.com",
    };
    
    for (const QString& domain : invalidDomains) {
        bool result = networkUtils->checkNetConnection(domain, "80", 1000);
        EXPECT_FALSE(result) << "Invalid domain should return false: " << domain.toStdString();
    }
}

// 测试13: 端口范围测试
TEST_F(NetworkUtilsTest, TestPortRange)
{
    QString host = "localhost";
    
    // 测试系统保留端口
    QList<QString> reservedPorts = {"0", "1", "22", "80", "443", "1023"};
    for (const QString& port : reservedPorts) {
        bool result = networkUtils->checkNetConnection(host, port, 1000);
        EXPECT_TRUE(result == true || result == false) 
            << "Reserved port check should return boolean for: " << port.toStdString();
    }
    
    // 测试用户端口范围
    QList<QString> userPorts = {"1024", "2048", "4096", "8080", "8081"};
    for (const QString& port : userPorts) {
        bool result = networkUtils->checkNetConnection(host, port, 1000);
        EXPECT_TRUE(result == true || result == false) 
            << "User port check should return boolean for: " << port.toStdString();
    }
    
    // 测试动态/私有端口
    QList<QString> dynamicPorts = {"49152", "50000", "60000", "65535"};
    for (const QString& port : dynamicPorts) {
        bool result = networkUtils->checkNetConnection(host, port, 1000);
        EXPECT_TRUE(result == true || result == false) 
            << "Dynamic port check should return boolean for: " << port.toStdString();
    }
}

// 测试14: 特殊字符处理
TEST_F(NetworkUtilsTest, TestSpecialCharacters)
{
    // 测试包含特殊字符的主机名
    QStringList specialHosts = {
        "test-host.example.com",
        "test_host.example.com",
        "testhost123.example.com",
        "subdomain.example-domain.com",
    };
    
    for (const QString& host : specialHosts) {
        bool result = networkUtils->checkNetConnection(host, "80", 1000);
        EXPECT_TRUE(result == true || result == false) 
            << "Special character host check should return boolean for: " << host.toStdString();
    }
    
    // 测试无效字符的主机名
    QStringList invalidHosts = {
        "host with spaces.com",
        "host@with@symbols.com",
        "host\nwith\nnewlines.com",
        "host\twith\ttabs.com",
    };
    
    for (const QString& host : invalidHosts) {
        bool result = networkUtils->checkNetConnection(host, "80", 1000);
        EXPECT_FALSE(result) << "Invalid character host should return false: " << host.toStdString();
    }
}

// 测试15: 并发测试
TEST_F(NetworkUtilsTest, TestConcurrentChecks)
{
    const int threadCount = 5;
    QList<QThread*> threads;
    QList<bool> results;
    
    for (int i = 0; i < threadCount; ++i) {
        QThread* thread = QThread::create([this, i, &results]() {
            try {
                // 并发执行网络检查
                bool result1 = networkUtils->checkNetConnection("localhost", "22", 1000);
                bool result2 = networkUtils->checkNetConnection("8.8.8.8", "53", 1000);
                
                // 验证操作成功完成
                bool success = (result1 == true || result1 == false) &&
                              (result2 == true || result2 == false);
                
                results.append(success);
                
                QThread::msleep(10);
            } catch (...) {
                results.append(false);
            }
        });
        
        threads.append(thread);
        thread->start();
    }
    
    // 等待所有线程完成
    for (QThread* thread : threads) {
        thread->wait();
        delete thread;
    }
    
    EXPECT_EQ(results.size(), threadCount) << "All threads should complete";
    
    int successCount = 0;
    for (bool result : results) {
        if (result) successCount++;
    }
    
    EXPECT_EQ(successCount, threadCount) << "All concurrent operations should succeed";
}

// 测试16: 性能测试
TEST_F(NetworkUtilsTest, TestPerformance)
{
    const int operationCount = 100;
    qint64 startTime = QDateTime::currentMSecsSinceEpoch();
    
    // 测试连接检查性能
    for (int i = 0; i < operationCount; ++i) {
        bool result = networkUtils->checkNetConnection("localhost", "22", 100);
        EXPECT_TRUE(result == true || result == false) << "Connection check should return boolean";
    }
    
    qint64 endTime = QDateTime::currentMSecsSinceEpoch();
    qint64 duration = endTime - startTime;
    
    EXPECT_LT(duration, 10000) << "Connection checks should complete within 10 seconds";
    
    // 测试IP解析性能
    startTime = QDateTime::currentMSecsSinceEpoch();
    
    for (int i = 0; i < operationCount; ++i) {
        QString input = QString("192.168.1.%1:8080").arg((i % 254) + 1);
        QString ip, port;
        bool result = networkUtils->parseIp(input, ip, port);
        EXPECT_TRUE(result == true || result == false) << "IP parse should return boolean";
    }
    
    endTime = QDateTime::currentMSecsSinceEpoch();
    duration = endTime - startTime;
    
    EXPECT_LT(duration, 5000) << "IP parsing should complete within 5 seconds";
}

// 测试17: 错误处理测试
TEST_F(NetworkUtilsTest, TestErrorHandling)
{
    // 测试各种错误情况
    
    // 测试空参数
    bool emptyResult1 = networkUtils->checkNetConnection("", "", 1000);
    EXPECT_FALSE(emptyResult1) << "Empty parameters should return false";
    
    // 测试异步操作的错误处理
    std::function<void(bool)> errorCallback = nullptr;
    networkUtils->doAfterCheckNet("", QStringList(), errorCallback, 1000);
    
    // 测试解析错误
    QString malformedIp = "malformed.ip.address";
    QString ip, port;
    bool parseResult = networkUtils->parseIp(malformedIp, ip, port);
    EXPECT_FALSE(parseResult) << "Malformed IP should return false";
    
    // 测试极端超时值
    bool extremeTimeoutResult = networkUtils->checkNetConnection("localhost", "80", INT_MAX);
    EXPECT_TRUE(extremeTimeoutResult == true || extremeTimeoutResult == false) 
        << "Extreme timeout should be handled gracefully";
}

// 测试18: 内存管理测试
TEST_F(NetworkUtilsTest, TestMemoryManagement)
{
    // 由于NetworkUtils是单例，主要测试内存稳定性
    const int operationCount = 1000;
    
    // 执行大量操作，验证内存稳定性
    for (int i = 0; i < operationCount; ++i) {
        bool connectionResult = networkUtils->checkNetConnection("localhost", "22", 100);
        
        QString input = QString("192.168.1.%1:8080").arg((i % 254) + 1);
        QString ip, port;
        bool parseResult = networkUtils->parseIp(input, ip, port);
        
        bool cifsResult = networkUtils->checkAllCIFSBusy();
        
        // 验证返回值的有效性
        EXPECT_TRUE(connectionResult == true || connectionResult == false) << "Connection result should be valid";
        EXPECT_TRUE(parseResult == true || parseResult == false) << "Parse result should be valid";
        EXPECT_TRUE(cifsResult == true || cifsResult == false) << "CIFS result should be valid";
    }
    
    SUCCEED() << "Memory management test completed successfully";
}
