// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "builtininterface.h"
#include "utils/crashhandle.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <QFile>

using namespace dfm_upgrade;

TEST(CrashHandle, upgradeCacheDir)
{
    QString ret = QStandardPaths::standardLocations(QStandardPaths::GenericCacheLocation).first() + "/deepin/dde-file-manager";
    EXPECT_EQ(ret, CrashHandle::upgradeCacheDir());
}

TEST(CrashHandle, isCrashed)
{
    const QString dir = QStandardPaths::standardLocations(QStandardPaths::GenericCacheLocation).first() + "/deepin/dde-file-manager";
    const QString f1 = dir + "/dfm-upgraded.crash.0";
    const QString f2 = dir + "/dfm-upgraded.crash.1";

    QList<QString> ex;
    stub_ext::StubExt stub;
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

TEST(CrashHandle, clearCrash)
{
    const QString dir = QStandardPaths::standardLocations(QStandardPaths::GenericCacheLocation).first() + "/deepin/dde-file-manager";
    const QString f1 = dir + "/dfm-upgraded.crash.0";
    const QString f2 = dir + "/dfm-upgraded.crash.1";

    CrashHandle h;

    QList<QString> ex;
    stub_ext::StubExt stub;
    stub.set_lamda((bool (*)(const QString &fileName))&QFile::remove, [&ex](const QString &fileName) {
        ex.append(fileName);
        return true;
    });

    h.clearCrash();
    EXPECT_TRUE(ex.contains(f1));
    EXPECT_TRUE(ex.contains(f2));
}

TEST(CrashHandle, handleSignal)
{
    const QString dir = QStandardPaths::standardLocations(QStandardPaths::GenericCacheLocation).first() + "/deepin/dde-file-manager";
    const QString f1 = dir + "/dfm-upgraded.crash.0";
    const QString f2 = dir + "/dfm-upgraded.crash.1";

    stub_ext::StubExt stub;
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
