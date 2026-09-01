// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dfmmimedata.cpp
 * @brief Unit tests for DFMMimeData (dfmmimedata.cpp)
 */

#include <gtest/gtest.h>
#include <QUrl>
#include <QList>

#include <dfm-base/mimedata/dfmmimedata.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/dfm_global_defines.h>
#include <QIcon>
#include <QDir>
#include <mutex>

using namespace dfmbase;

class DFMMimeDataTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        std::call_once(flag, [] {
            UrlRoute::regScheme(Global::Scheme::kFile, QDir::homePath(), QIcon(), false, "file");
            InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
        });
    }
    static std::once_flag flag;
};
std::once_flag DFMMimeDataTest::flag;

TEST_F(DFMMimeDataTest, DefaultConstructorIsInvalid)
{
    DFMMimeData data;
    EXPECT_FALSE(data.isValid());
}

TEST_F(DFMMimeDataTest, SetUrlsAndRetrieve)
{
    DFMMimeData data;
    QList<QUrl> urls { QUrl("file:///home/user/a"), QUrl("file:///home/user/b") };
    data.setUrls(urls);
    auto got = data.urls();
    EXPECT_EQ(got.size(), 2);
}

TEST_F(DFMMimeDataTest, CanTrashCanDeleteDefaults)
{
    DFMMimeData data;
    EXPECT_NO_FATAL_FAILURE({ (void)data.canTrash(); });
    EXPECT_NO_FATAL_FAILURE({ (void)data.canDelete(); });
    EXPECT_NO_FATAL_FAILURE({ (void)data.isTrashFile(); });
}

TEST_F(DFMMimeDataTest, SetAndgetAttribute)
{
    DFMMimeData data;
    data.setAttritube("mykey", QString("myval"));
    EXPECT_EQ(data.attritube("mykey").toString(), QString("myval"));
    EXPECT_EQ(data.attritube("missing", QString("def")).toString(), QString("def"));
}

TEST_F(DFMMimeDataTest, SerializeDeserializeRoundTrip)
{
    DFMMimeData data;
    QList<QUrl> urls { QUrl::fromLocalFile(QDir::tempPath() + "/dfm_x"), QUrl::fromLocalFile(QDir::tempPath() + "/dfm_y") };
    data.setUrls(urls);
    QByteArray bytes;
    EXPECT_NO_FATAL_FAILURE({ bytes = data.toByteArray(); });
    EXPECT_NO_FATAL_FAILURE({ DFMMimeData restored = DFMMimeData::fromByteArray(bytes); (void)restored.urls(); });
}

TEST_F(DFMMimeDataTest, CopyConstructorSharesData)
{
    DFMMimeData data;
    data.setUrls({ QUrl("file:///tmp/a") });
    DFMMimeData copy(data);
    EXPECT_EQ(copy.urls().size(), 1);
}

TEST_F(DFMMimeDataTest, ClearResetsData)
{
    DFMMimeData data;
    data.setUrls({ QUrl("file:///tmp/a") });
    data.clear();
    EXPECT_TRUE(data.urls().isEmpty());
}

TEST_F(DFMMimeDataTest, VersionString)
{
    DFMMimeData data;
    QString v = data.version();
    EXPECT_FALSE(v.isEmpty());
}
