// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_plugin.cpp
 * @brief Unit tests for plugin.cpp (DSMRegister / DSMUnRegister).
 *        Since these are extern "C" functions that create DBus objects,
 *        we test them minimally — the DBus registration will fail gracefully
 *        in the test sandbox.
 */

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

#include "stubext.h"
#include <dfm-search/dsearch_global.h>

#include "dfm_test_main.h"
#include "services/textindex/service_textindex_global.h"

using namespace SERVICETEXTINDEX_NAMESPACE;
using namespace DFMSEARCH;

// External C functions from plugin.cpp
extern "C" int DSMRegister(const char *name, void *data);
extern "C" int DSMUnRegister(const char *name, void *data);

class PluginTest : public testing::Test
{
protected:
    void SetUp() override
    {
        ASSERT_TRUE(tmp.isValid());

        // Redirect index directories to temp
        stub.set_lamda(ADDR(Global, contentIndexDirectory),
                       [this]() -> QString {
                           __DBG_STUB_INVOKE__
                           return tmp.path() + "/content-index";
                       });
        stub.set_lamda(ADDR(Global, isContentIndexAvailable),
                       []() -> bool {
                           __DBG_STUB_INVOKE__
                           return true;
                       });
        stub.set_lamda(ADDR(Global, isPathInContentIndexDirectory),
                       [this](const QString &path) -> bool {
                           __DBG_STUB_INVOKE__
                           return path.startsWith(tmp.path());
                       });
        stub.set_lamda(ADDR(Global, ocrTextIndexDirectory),
                       [this]() -> QString {
                           __DBG_STUB_INVOKE__
                           return tmp.path() + "/ocr-index";
                       });
        stub.set_lamda(ADDR(Global, isOcrTextIndexAvailable),
                       []() -> bool {
                           __DBG_STUB_INVOKE__
                           return true;
                       });
        stub.set_lamda(ADDR(Global, isPathInOcrTextIndexDirectory),
                       [this](const QString &path) -> bool {
                           __DBG_STUB_INVOKE__
                           return path.startsWith(tmp.path());
                       });
        stub.set_lamda(ADDR(Global, defaultIndexedDirectory),
                       [this]() -> QStringList {
                           __DBG_STUB_INVOKE__
                           return QStringList { tmp.path() + "/indexed-dir" };
                       });
        stub.set_lamda(ADDR(Global, defaultBlacklistPaths),
                       []() -> QStringList {
                           __DBG_STUB_INVOKE__
                           return QStringList();
                       });
    }

    QTemporaryDir tmp;
    stub_ext::StubExt stub;
};

TEST_F(PluginTest, DSMRegister_ReturnsZero)
{
    // DSMRegister creates DBus objects; should return 0
    // DBus registration may fail in sandbox but the function
    // should still return 0
    int result = DSMRegister("org.deepin.Filemanager.TextIndex", nullptr);
    EXPECT_EQ(result, 0);

    // Clean up via DSMUnRegister
    DSMUnRegister("org.deepin.Filemanager.TextIndex", nullptr);
}

// TEST_F(PluginTest, DSMRegister_CallTwice)
// {
//     int result1 = DSMRegister("org.deepin.Filemanager.TextIndex", nullptr);
//     EXPECT_EQ(result1, 0);
//
//     int result2 = DSMRegister("org.deepin.Filemanager.TextIndex", nullptr);
//     EXPECT_EQ(result2, 0);
//
//     DSMUnRegister("org.deepin.Filemanager.TextIndex", nullptr);
// }

TEST_F(PluginTest, DSMUnRegister_WithoutRegister)
{
    // Calling unregister without register should not crash
    int result = DSMUnRegister("org.deepin.Filemanager.TextIndex", nullptr);
    EXPECT_EQ(result, 0);
}

TEST_F(PluginTest, DSMUnRegister_CalledTwice)
{
    DSMRegister("org.deepin.Filemanager.TextIndex", nullptr);
    DSMUnRegister("org.deepin.Filemanager.TextIndex", nullptr);
    // Second unregister should not crash
    int result = DSMUnRegister("org.deepin.Filemanager.TextIndex", nullptr);
    EXPECT_EQ(result, 0);
}

TEST_F(PluginTest, DSMRegister_WithNullName)
{
    int result = DSMRegister(nullptr, nullptr);
    EXPECT_EQ(result, 0);
    DSMUnRegister(nullptr, nullptr);
}
