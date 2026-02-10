// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "builtininterface.h"
#include "utils/crashhandle.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <signal.h>
#include <QIODevice>

using namespace dfm_upgrade;

class TestCrashHandle : public testing::Test {
protected:
    void SetUp() override {
        // Create test directory
        testDir = QDir::tempPath() + "/crash_handle_test";
        QDir().mkpath(testDir);

        configPath = testDir + "/deepin/dde-file-manager";
        QDir().mkpath(configPath);

        // Set up crash flags
        crashFlag0 = configPath + "/dfm-upgraded.crash.0";
        crashFlag1 = configPath + "/dfm-upgraded.crash.1";

        auto standlocal = static_cast<QList<QString> (*)(QStandardPaths::StandardLocation)>(&QStandardPaths::standardLocations);
        stub.set_lamda(standlocal, [&](QStandardPaths::StandardLocation type) -> QList<QString> {
            if (type == QStandardPaths::GenericCacheLocation) {
                qDebug() << "Mocking crash handle cache directory:" << testDir;
                return {testDir};
            }
            return QStandardPaths::standardLocations(type);
        });
    }

    void TearDown() override {
        // Clean up test files
        QDir(testDir).removeRecursively();
        // Clear all stubs
        stub.clear();
    }

    QString testDir;
    QString crashFlag0;
    QString crashFlag1;
    QString configPath;
    stub_ext::StubExt stub; // Move stub to be a class member
};

TEST_F(TestCrashHandle, upgradeCacheDir)
{
    EXPECT_EQ(configPath, CrashHandle::upgradeCacheDir());
}

TEST_F(TestCrashHandle, isCrashed_WithStubs)
{
    const QString f1 = configPath + "/dfm-upgraded.crash.0";
    const QString f2 = configPath + "/dfm-upgraded.crash.1";

    QList<QString> ex;
    // Use the class member 'stub'
    stub.set_lamda((bool (*)(const QString &fileName))&QFile::exists, [&ex](const QString &fileName) {
        return ex.contains(fileName);
    });

    CrashHandle h;
    EXPECT_FALSE(h.isCrashed());

    ex << f1;
    EXPECT_FALSE(h.isCrashed());

    ex.clear();
    ex << f2;
    EXPECT_FALSE(h.isCrashed());

    ex << f1;
    EXPECT_TRUE(h.isCrashed());
}

TEST_F(TestCrashHandle, clearCrash_WithStubs)
{
    const QString f1 = configPath + "/dfm-upgraded.crash.0";
    const QString f2 = configPath + "/dfm-upgraded.crash.1";

    CrashHandle h;

    QList<QString> ex;
    // Use the class member 'stub'
    stub.set_lamda((bool (*)(const QString &fileName))&QFile::remove, [&ex](const QString &fileName) {
        ex.append(fileName);
        return true;
    });

    h.clearCrash();
    EXPECT_TRUE(ex.contains(f1));
    EXPECT_TRUE(ex.contains(f2));
}

TEST_F(TestCrashHandle, handleSignal_WithStubs)
{
    const QString f1 = configPath + "/dfm-upgraded.crash.0";
    const QString f2 = configPath + "/dfm-upgraded.crash.1";

    // Use the class member 'stub'
    bool unreg = false;
    stub.set_lamda(&CrashHandle::unregSignal, [&unreg]() {
        unreg = true;
    });

    int sig = 0;
    stub.set_lamda((int (*)(int))&raise, [&sig](int s) {
        sig = s;
        return 0;
    });

    bool ex = false;
    stub.set_lamda((bool (*)(const QString &fileName))&QFile::exists, [&ex]() {
        return ex;
    });

    QString opend;
    stub.set_lamda((bool (*)(QFile *, QIODevice::OpenMode))((bool (QFile::*)(QIODevice::OpenMode))&QFile::open), [&opend](QFile *self, QIODevice::OpenMode om) {
        opend = self->fileName();
        return true;
    });

    CrashHandle h;
    h.handleSignal(SIGSEGV);
    EXPECT_TRUE(unreg);
    EXPECT_EQ(sig, SIGSEGV);
    EXPECT_EQ(opend, f1);

    unreg = false;
    sig = 0;
    ex = true;
    opend.clear();

    h.handleSignal(SIGABRT);
    EXPECT_TRUE(unreg);
    EXPECT_EQ(sig, SIGABRT);
    EXPECT_EQ(opend, f2);
}

TEST_F(TestCrashHandle, constructor) {
    CrashHandle handle;
    // Constructor should not throw
    SUCCEED();
}

TEST_F(TestCrashHandle, isCrashed_NoFlags) {
    CrashHandle handle;
    bool result = handle.isCrashed();
    EXPECT_FALSE(result);
}

TEST_F(TestCrashHandle, isCrashed_OneFlag) {
    // Create only one crash flag
    QFile flagFile(crashFlag0);
    if (flagFile.open(QIODevice::WriteOnly)) {
        flagFile.close();
    }

    CrashHandle handle;
    bool result = handle.isCrashed();
    EXPECT_FALSE(result);
}

TEST_F(TestCrashHandle, isCrashed_BothFlags) {
    // Create both crash flags
    QFile flagFile0(crashFlag0);
    if (flagFile0.open(QIODevice::WriteOnly)) {
        flagFile0.close();
    }

    QFile flagFile1(crashFlag1);
    if (flagFile1.open(QIODevice::WriteOnly)) {
        flagFile1.close();
    }

    CrashHandle handle;
    bool result = handle.isCrashed();
    EXPECT_TRUE(result);
}

TEST_F(TestCrashHandle, clearCrash) {
    // Create both crash flags
    QFile flagFile0(crashFlag0);
    if (flagFile0.open(QIODevice::WriteOnly)) {
        flagFile0.close();
    }

    QFile flagFile1(crashFlag1);
    if (flagFile1.open(QIODevice::WriteOnly)) {
        flagFile1.close();
    }

    CrashHandle handle;
    handle.clearCrash();

    // Verify flags are removed
    EXPECT_FALSE(QFile::exists(crashFlag0));
    EXPECT_FALSE(QFile::exists(crashFlag1));
}

TEST_F(TestCrashHandle, regSignal) {
    CrashHandle handle;
    // Should not throw
    handle.regSignal();
    SUCCEED();
}

TEST_F(TestCrashHandle, unregSignal) {
    CrashHandle handle;
    // Should not throw
    handle.unregSignal();
    SUCCEED();
}

TEST_F(TestCrashHandle, handleSignal_CreateFirstFlag) {
    // Use the class member 'stub'
    // Mock QFile::exists to return false initially
    stub.set_lamda((bool (*)(const QString &))&QFile::exists, [](const QString &) {
        return false;
    });

    // Stub QFile::open function using double type conversion
    stub.set_lamda((bool (*)(QFile *, QIODevice::OpenMode))((bool (QFile::*)(QIODevice::OpenMode))&QFile::open), [](QFile *self, QIODevice::OpenMode mode) {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Mock raise to not actually raise signal
    stub.set_lamda((int (*)(int))&raise, [](int) {
        return 0;
    });

    CrashHandle handle;
    // Should not throw
    handle.handleSignal(SIGSEGV);
    SUCCEED();
}

TEST_F(TestCrashHandle, handleSignal_CreateSecondFlag) {
    // Use the class member 'stub'
    // Mock QFile::exists to return true for first flag
    stub.set_lamda((bool (*)(const QString &))&QFile::exists, [this](const QString &path) {
        return path == crashFlag0;
    });

    // Stub QFile::open function using double type conversion
    stub.set_lamda((bool (*)(QFile *, QIODevice::OpenMode))((bool (QFile::*)(QIODevice::OpenMode))&QFile::open), [](QFile *self, QIODevice::OpenMode mode) {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Mock raise to not actually raise signal
    stub.set_lamda((int (*)(int))&raise, [](int) {
        return 0;
    });

    CrashHandle handle;
    // Should not throw
    handle.handleSignal(SIGABRT);
    SUCCEED();
}