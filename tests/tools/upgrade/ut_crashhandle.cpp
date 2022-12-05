/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
