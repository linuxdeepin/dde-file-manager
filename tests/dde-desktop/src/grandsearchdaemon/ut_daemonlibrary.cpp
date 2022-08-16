// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "daemonlibrary.h"

#include "stub.h"
#include "stubext.h"

#include <gtest/gtest.h>

#include <QLibrary>

using namespace GrandSearch;

TEST(DaemonLibrary, constructor)
{
    const QString &path = "/test/path";
    DaemonLibrary lib(path);
    EXPECT_EQ(lib.libPath, path);
}

TEST(DaemonLibrary, load_with_repeat)
{
    DaemonLibrary lib("/test/path");
    stub_ext::StubExt stub;
    bool load = false;
    stub.set_lamda(&QLibrary::load, [&load](){
        load = true;
        return true;
    });

    ASSERT_EQ(lib.library, nullptr);
    QLibrary *fake = reinterpret_cast<QLibrary *>(0x1);
    lib.library = fake;

    EXPECT_TRUE(lib.load());
    EXPECT_FALSE(load);
    EXPECT_EQ(lib.library, fake);
}

TEST(DaemonLibrary, load_false)
{
    DaemonLibrary lib("/test/path");
    stub_ext::StubExt stub;
    bool load = false;
    stub.set_lamda(&QLibrary::load, [&load](){
        load = true;
        return false;
    });

    ASSERT_EQ(lib.library, nullptr);

    EXPECT_FALSE(lib.load());
    EXPECT_TRUE(load);
    EXPECT_EQ(lib.library, nullptr);
}

static bool resolveVersion = false;
static bool resolveStart = false;
static bool resolveStop = false;

static int startDaemon(int argc, char *argv[]) {
    return argc;
}

static int stopDaemon() {
    return 999;
}

const char *verDaemon(){
    return "ver";
}

TEST(DaemonLibrary, resolve_ok)
{
    DaemonLibrary lib("/test/path");
    stub_ext::StubExt stub;
    stub.set_lamda(&QLibrary::load, [](){
        return true;
    });

    int resolve = 0;
    stub.set_lamda((QFunctionPointer (QLibrary::*)(const char *))&QLibrary::resolve, [&resolve](QLibrary *self, const char *func)-> QFunctionPointer {
        if (QString(func) == QString("grandSearchDaemonAppVersion")) {
            resolve = resolve | 1;
            return (QFunctionPointer)&verDaemon;
        } else if (QString(func) == QString("startGrandSearchDaemon")) {
            resolve = resolve | 2;
            return (QFunctionPointer)&startDaemon;
        } else if (QString(func) == QString("stopGrandSearchDaemon")) {
            resolve = resolve | 4;
            return (QFunctionPointer)&stopDaemon;
        }
        return (QFunctionPointer)nullptr;
    });

    ASSERT_TRUE(lib.load());
    EXPECT_NE(lib.libPath, nullptr);
    EXPECT_EQ(resolve, 7);
    EXPECT_EQ(lib.verFunc, &verDaemon);
    EXPECT_EQ(lib.startFunc, &startDaemon);
    EXPECT_EQ(lib.stopFunc, &stopDaemon);
}

TEST(DaemonLibrary, call_resolved)
{
    DaemonLibrary lib("/test/path");
    stub_ext::StubExt stub;
    stub.set_lamda(&QLibrary::load, [](){
        return true;
    });

    int resolve = 0;
    stub.set_lamda((QFunctionPointer (QLibrary::*)(const char *))&QLibrary::resolve, [&resolve](QLibrary *self, const char *func)-> QFunctionPointer {
        if (QString(func) == QString("grandSearchDaemonAppVersion")) {
            resolve = resolve | 1;
            return (QFunctionPointer)&verDaemon;
        } else if (QString(func) == QString("startGrandSearchDaemon")) {
            resolve = resolve | 2;
            return (QFunctionPointer)&startDaemon;
        } else if (QString(func) == QString("stopGrandSearchDaemon")) {
            resolve = resolve | 4;
            return (QFunctionPointer)&stopDaemon;
        }
        return (QFunctionPointer)nullptr;
    });

    ASSERT_TRUE(lib.load());
    EXPECT_EQ(lib.start(10, nullptr), 10);
    EXPECT_EQ(lib.stop(), 999);
    EXPECT_EQ(lib.version(), QString("ver"));
}

TEST(DaemonLibrary, resolve_ver_null)
{
    DaemonLibrary lib("/test/path");
    stub_ext::StubExt stub;
    stub.set_lamda(&QLibrary::load, [](){
        return true;
    });

    int resolve = 0;
    stub.set_lamda((QFunctionPointer (QLibrary::*)(const char *))&QLibrary::resolve, [&resolve](QLibrary *self, const char *func)-> QFunctionPointer {
        if (QString(func) == QString("grandSearchDaemonAppVersion")) {
            resolve = resolve | 1;
            return (QFunctionPointer)nullptr;
        } else if (QString(func) == QString("startGrandSearchDaemon")) {
            resolve = resolve | 2;
            return (QFunctionPointer)&startDaemon;
        } else if (QString(func) == QString("stopGrandSearchDaemon")) {
            resolve = resolve | 4;
            return (QFunctionPointer)&stopDaemon;
        }
        return (QFunctionPointer)nullptr;
    });

    ASSERT_FALSE(lib.load());
    EXPECT_EQ(lib.library, nullptr);
    EXPECT_EQ(resolve, 1);
    EXPECT_EQ(lib.verFunc, nullptr);
    EXPECT_EQ(lib.startFunc, nullptr);
    EXPECT_EQ(lib.stopFunc, nullptr);
}

TEST(DaemonLibrary, resolve_start_null)
{
    DaemonLibrary lib("/test/path");
    stub_ext::StubExt stub;
    stub.set_lamda(&QLibrary::load, [](){
        return true;
    });

    int resolve = 0;
    stub.set_lamda((QFunctionPointer (QLibrary::*)(const char *))&QLibrary::resolve, [&resolve](QLibrary *self, const char *func)-> QFunctionPointer {
        if (QString(func) == QString("grandSearchDaemonAppVersion")) {
            resolve = resolve | 1;
            return (QFunctionPointer)&verDaemon;
        } else if (QString(func) == QString("startGrandSearchDaemon")) {
            resolve = resolve | 2;
            return (QFunctionPointer)nullptr;
        } else if (QString(func) == QString("stopGrandSearchDaemon")) {
            resolve = resolve | 4;
            return (QFunctionPointer)&stopDaemon;
        }
        return (QFunctionPointer)nullptr;
    });

    ASSERT_FALSE(lib.load());
    EXPECT_EQ(lib.library, nullptr);
    EXPECT_EQ(resolve, 3);
    EXPECT_EQ(lib.verFunc, &verDaemon);
    EXPECT_EQ(lib.startFunc, nullptr);
    EXPECT_EQ(lib.stopFunc, nullptr);
}

TEST(DaemonLibrary, resolve_stop_null)
{
    DaemonLibrary lib("/test/path");
    stub_ext::StubExt stub;
    stub.set_lamda(&QLibrary::load, [](){
        return true;
    });

    int resolve = 0;
    stub.set_lamda((QFunctionPointer (QLibrary::*)(const char *))&QLibrary::resolve, [&resolve](QLibrary *self, const char *func)-> QFunctionPointer {
        if (QString(func) == QString("grandSearchDaemonAppVersion")) {
            resolve = resolve | 1;
            return (QFunctionPointer)&verDaemon;
        } else if (QString(func) == QString("startGrandSearchDaemon")) {
            resolve = resolve | 2;
            return (QFunctionPointer)&startDaemon;
        } else if (QString(func) == QString("stopGrandSearchDaemon")) {
            resolve = resolve | 4;
            return (QFunctionPointer)nullptr;
        }
        return (QFunctionPointer)nullptr;
    });

    ASSERT_FALSE(lib.load());
    EXPECT_EQ(lib.library, nullptr);
    EXPECT_EQ(resolve, 7);
    EXPECT_EQ(lib.verFunc, &verDaemon);
    EXPECT_EQ(lib.startFunc, startDaemon);
    EXPECT_EQ(lib.stopFunc, nullptr);
}

TEST(DaemonLibrary, unload)
{
    DaemonLibrary lib("/test/path");

    stub_ext::StubExt stub;
    bool unload = false;
    stub.set_lamda(&QLibrary::unload, [&unload]() {
        unload = true;
        return true;
    });

    ASSERT_EQ(lib.library, nullptr);
    lib.unload();
    EXPECT_FALSE(unload);

    lib.library = new QLibrary;
    unload = false;
    lib.unload();
    EXPECT_TRUE(unload);
    EXPECT_EQ(lib.library, nullptr);
}
