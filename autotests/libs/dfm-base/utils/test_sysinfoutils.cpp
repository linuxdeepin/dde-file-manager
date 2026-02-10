// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QByteArray>
#include <QMimeData>
#include <QDir>
#include <QProcessEnvironment>
#include <QDBusInterface>
#include <QDBusReply>
#include <QCoreApplication>

#include <dfm-base/utils/sysinfoutils.h>
#include <dfm-base/dfm_global_defines.h>

// Include stub headers
#include "stubext.h"

#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <fcntl.h>

using namespace dfmbase;

class TestSysInfoUtils : public testing::Test {
protected:
    void SetUp() override {
        // Setup code before each test
        stub.clear();
    }

    void TearDown() override {
        // Cleanup code after each test
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
};

// Test getUser function
TEST_F(TestSysInfoUtils, TestGetUser) {
    QString user = SysInfoUtils::getUser();
    
    // User should not be empty in most cases
    EXPECT_FALSE(user.isEmpty());
    
    // Should be consistent across calls
    QString user2 = SysInfoUtils::getUser();
    EXPECT_EQ(user, user2);
}

// Test getAllUsersOfHome function
TEST_F(TestSysInfoUtils, TestGetAllUsersOfHome) {
    QStringList users = SysInfoUtils::getAllUsersOfHome();
    
    // Should return a list (could be empty)
    EXPECT_TRUE(users.contains("root") || users.isEmpty());
    
    // Should not contain "." or ".."
    EXPECT_FALSE(users.contains("."));
    EXPECT_FALSE(users.contains(".."));
}

// Test getHostName function
TEST_F(TestSysInfoUtils, TestGetHostName) {
    QString hostname = SysInfoUtils::getHostName();
    
    // Hostname might be empty if gethostname fails, but shouldn't crash
    // If not empty, it should be a valid hostname
    if (!hostname.isEmpty()) {
        EXPECT_GT(hostname.length(), 0);
        EXPECT_FALSE(hostname.contains(" "));
    }
    
    // Should be consistent across calls
    QString hostname2 = SysInfoUtils::getHostName();
    EXPECT_EQ(hostname, hostname2);
}

// Test getUserId function
TEST_F(TestSysInfoUtils, TestGetUserId) {
    int userId = SysInfoUtils::getUserId();
    
    // User ID should be non-negative
    EXPECT_GE(userId, 0);
    EXPECT_LT(userId, 65536);  // Max UID
    
    // Should be consistent across calls
    int userId2 = SysInfoUtils::getUserId();
    EXPECT_EQ(userId, userId2);
}

// Test isRootUser function
TEST_F(TestSysInfoUtils, TestIsRootUser) {
    bool isRoot = SysInfoUtils::isRootUser();
    int userId = SysInfoUtils::getUserId();
    
    // Root status should match user ID
    EXPECT_EQ(isRoot, (userId == 0));
}

// Test isServerSys function
TEST_F(TestSysInfoUtils, TestIsServerSys) {
    bool isServer = SysInfoUtils::isServerSys();
    
    // Should not crash
    (void)isServer;  // Suppress unused variable warning
}

// Test isDesktopSys function
TEST_F(TestSysInfoUtils, TestIsDesktopSys) {
    bool isDesktop = SysInfoUtils::isDesktopSys();
    bool isServer = SysInfoUtils::isServerSys();
    
    // Desktop should be opposite of server
    EXPECT_EQ(isDesktop, !isServer);
}

// Test isOpenAsAdmin function
TEST_F(TestSysInfoUtils, TestIsOpenAsAdmin) {
    bool isOpenAsAdmin = SysInfoUtils::isOpenAsAdmin();
    
    // Should not crash
    (void)isOpenAsAdmin;  // Suppress unused variable warning
}

// Test isDeveloperModeEnabled function
TEST_F(TestSysInfoUtils, TestIsDeveloperModeEnabled) {
    // Mock QDBusInterface to avoid actual D-Bus calls
    // Note: We cannot stub constructor directly, so we just test the function behavior
    bool isDeveloperMode = SysInfoUtils::isDeveloperModeEnabled();
    
    // Should not crash
    (void)isDeveloperMode;  // Suppress unused variable warning
}

// Test isDeveloperModeEnabled with valid reply
TEST_F(TestSysInfoUtils, TestIsDeveloperModeEnabledValidReply) {
    // Reset static variables by forcing re-initialization
    // Note: This is a limitation of testing static variables
    
    // Note: Due to static caching in isDeveloperModeEnabled(), 
    // we cannot fully test different return values without process restart
    
    // Mock QDBusInterface constructor is not possible, so we just test the function
    bool isDeveloperMode = SysInfoUtils::isDeveloperModeEnabled();
    
    // Result depends on first call
    (void)isDeveloperMode;
}

// Test isProfessional function
TEST_F(TestSysInfoUtils, TestIsProfessional) {
    bool isProfessional = SysInfoUtils::isProfessional();
    
    // Should not crash
    (void)isProfessional;  // Suppress unused variable warning
}

// Test isDeepin23 function
TEST_F(TestSysInfoUtils, TestIsDeepin23) {
    bool isDeepin23 = SysInfoUtils::isDeepin23();
    
    // Should not crash
    (void)isDeepin23;  // Suppress unused variable warning
}

// Test isSameUser function with matching user
TEST_F(TestSysInfoUtils, TestIsSameUserMatching) {
    QMimeData data;
    int userId = SysInfoUtils::getUserId();
    
    // Set user ID in mime data
    SysInfoUtils::setMimeDataUserId(&data);
    
    // Should return true for same user
    EXPECT_TRUE(SysInfoUtils::isSameUser(&data));
}

// Test isSameUser function with different mime data
TEST_F(TestSysInfoUtils, TestIsSameUserDifferent) {
    QMimeData data;
    
    // Don't set user ID
    EXPECT_FALSE(SysInfoUtils::isSameUser(&data));
    
    // Set different format
    data.setData("test/format", QByteArray("test"));
    EXPECT_FALSE(SysInfoUtils::isSameUser(&data));
}

// Test setMimeDataUserId function
TEST_F(TestSysInfoUtils, TestSetMimeDataUserId) {
    QMimeData data;
    int userId = SysInfoUtils::getUserId();
    QString userIdStr = QString::number(userId);
    
    SysInfoUtils::setMimeDataUserId(&data);
    
    // Check if the data was set
    EXPECT_TRUE(data.hasFormat(DFMGLOBAL_NAMESPACE::Mime::kDataUserIDKey));
    
    QByteArray userIdData = data.data(DFMGLOBAL_NAMESPACE::Mime::kDataUserIDKey);
    EXPECT_EQ(QString::fromUtf8(userIdData), userIdStr);
    
    // Check if user-specific format was also set
    QString userKey = QString(DFMGLOBAL_NAMESPACE::Mime::kDataUserIDKey) + "_" + userIdStr;
    EXPECT_TRUE(data.hasFormat(userKey));
}

// Test getMemoryUsage function with current process
TEST_F(TestSysInfoUtils, TestGetMemoryUsageCurrentProcess) {
    // Test with current process ID
    int currentPid = QCoreApplication::applicationPid();
    float memoryUsage = SysInfoUtils::getMemoryUsage(currentPid);
    
    // Memory usage should be non-negative
    EXPECT_GE(memoryUsage, 0.0f);
}

// Test getMemoryUsage function with invalid PID
TEST_F(TestSysInfoUtils, TestGetMemoryUsageInvalidPid) {
    // Test with invalid PID
    int invalidPid = -1;
    float memoryUsage = SysInfoUtils::getMemoryUsage(invalidPid);
    
    // Should return 0 for invalid PID
    EXPECT_EQ(memoryUsage, 0.0f);
}

// Test getMemoryUsage function with high PID
TEST_F(TestSysInfoUtils, TestGetMemoryUsageHighPid) {
    // Test with very high PID that likely doesn't exist
    int highPid = 999999;
    float memoryUsage = SysInfoUtils::getMemoryUsage(highPid);
    
    // Should return 0 for non-existent PID
    EXPECT_EQ(memoryUsage, 0.0f);
}

// Mock open/read to test getMemoryUsage edge cases
TEST_F(TestSysInfoUtils, TestGetMemoryUsageWithMock) {
    // Mock open to fail
    stub.set_lamda((int(*)(const char*, int))&::open, [](const char*, int) {
        return -1;  // Error
    });
    
    int pid = 1;
    float memoryUsage = SysInfoUtils::getMemoryUsage(pid);
    
    // Should return 0 when open fails
    EXPECT_EQ(memoryUsage, 0.0f);
}

// Test getOriginalUserHome function without PKEXEC_UID
TEST_F(TestSysInfoUtils, TestGetOriginalUserHomeWithoutPkexec) {
    // Mock QProcessEnvironment to not contain PKEXEC_UID
    stub.set_lamda(ADDR(QProcessEnvironment, contains), [](QProcessEnvironment*, const QString &key) {
        return key != "PKEXEC_UID";
    });
    
    QString homePath = SysInfoUtils::getOriginalUserHome();
    
    // Should return current user's home
    EXPECT_EQ(homePath, QDir::homePath());
}

// Test getOriginalUserHome function with PKEXEC_UID
TEST_F(TestSysInfoUtils, TestGetOriginalUserHomeWithPkexec) {
    // Mock QProcessEnvironment
    stub.set_lamda((bool(QProcessEnvironment::*)(const QString&) const)&QProcessEnvironment::contains, [](QProcessEnvironment*, const QString &key) {
        return key == "PKEXEC_UID";
    });
    
    stub.set_lamda((QString(QProcessEnvironment::*)(const QString&, const QString&) const)&QProcessEnvironment::value, [](QProcessEnvironment*, const QString &key, const QString &) -> QString {
        if (key == "PKEXEC_UID")
            return "1000";  // Non-root user ID
        return QString();
    });
    
    // Mock getpwuid
    static struct passwd mockPw;
    mockPw.pw_dir = const_cast<char*>("/home/testuser");
    
    stub.set_lamda((struct passwd*(*)(uid_t))&::getpwuid, [](uid_t) {
        return &mockPw;
    });
    
    QString homePath = SysInfoUtils::getOriginalUserHome();
    
    // Should return the mock user's home
    EXPECT_TRUE(homePath.contains("testuser"));
}

// Test getOriginalUserHome with invalid PKEXEC_UID
TEST_F(TestSysInfoUtils, TestGetOriginalUserHomeWithInvalidPkexec) {
    // Mock QProcessEnvironment
    stub.set_lamda((bool(QProcessEnvironment::*)(const QString&) const)&QProcessEnvironment::contains, [](QProcessEnvironment*, const QString &key) {
        return key == "PKEXEC_UID";
    });
    
    stub.set_lamda((QString(QProcessEnvironment::*)(const QString&, const QString&) const)&QProcessEnvironment::value, [](QProcessEnvironment*, const QString &key, const QString &) -> QString {
        if (key == "PKEXEC_UID")
            return "invalid";  // Invalid UID
        return QString();
    });
    
    QString homePath = SysInfoUtils::getOriginalUserHome();
    
    // Should return current user's home for invalid UID
    EXPECT_EQ(homePath, QDir::homePath());
}

// Test getOriginalUserHome with root PKEXEC_UID
TEST_F(TestSysInfoUtils, TestGetOriginalUserHomeWithRootPkexec) {
    // Mock QProcessEnvironment
    stub.set_lamda((bool(QProcessEnvironment::*)(const QString&) const)&QProcessEnvironment::contains, [](QProcessEnvironment*, const QString &key) {
        return key == "PKEXEC_UID";
    });
    
    stub.set_lamda((QString(QProcessEnvironment::*)(const QString&, const QString&) const)&QProcessEnvironment::value, [](QProcessEnvironment*, const QString &key, const QString &) -> QString {
        if (key == "PKEXEC_UID")
            return "0";  // Root UID
        return QString();
    });
    
    QString homePath = SysInfoUtils::getOriginalUserHome();
    
    // Should return current user's home for root UID
    EXPECT_EQ(homePath, QDir::homePath());
}

// Test getOriginalUserHome with getpwuid failure
TEST_F(TestSysInfoUtils, TestGetOriginalUserHomeWithGetpwuidFailure) {
    // Mock QProcessEnvironment
    stub.set_lamda((bool(QProcessEnvironment::*)(const QString&) const)&QProcessEnvironment::contains, [](QProcessEnvironment*, const QString &key) {
        return key == "PKEXEC_UID";
    });
    
    stub.set_lamda((QString(QProcessEnvironment::*)(const QString&, const QString&) const)&QProcessEnvironment::value, [](QProcessEnvironment*, const QString &key, const QString &) -> QString {
        if (key == "PKEXEC_UID")
            return "1000";  // Valid looking UID
        return QString();
    });
    
    // Mock getpwuid to return nullptr (failure)
    stub.set_lamda((struct passwd*(*)(uid_t))&::getpwuid, [](uid_t) {
        return nullptr;
    });
    
    QString homePath = SysInfoUtils::getOriginalUserHome();
    
    // Should return current user's home when getpwuid fails
    EXPECT_EQ(homePath, QDir::homePath());
}