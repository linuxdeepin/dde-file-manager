// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_entryfileinfo.cpp
 * @brief Unit tests for EntryFileInfo (entryfileinfo.cpp)
 */

#include <gtest/gtest.h>
#include <QUrl>
#include <QIcon>
#include <QVariant>
#include <QVariantHash>
#include <mutex>

#include <dfm-base/base/urlroute.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/interfaces/abstractentryfileentity.h>
#include <dfm-base/file/entry/entryfileinfo.h>
#include <dfm-base/interfaces/fileinfo.h>

using namespace dfmbase;

namespace {
class TestEntryEntity : public AbstractEntryFileEntity
{
public:
    explicit TestEntryEntity(const QUrl &url)
        : AbstractEntryFileEntity(url) {}
    QString displayName() const override { return QStringLiteral("TestEntry"); }
    QIcon icon() const override { return QIcon(); }
    bool exists() const override { return true; }
    bool showProgress() const override { return false; }
    bool showTotalSize() const override { return false; }
    bool showUsageSize() const override { return false; }
    EntryOrder order() const override { return EntryOrder::kOrderCustom; }
};
}   // namespace

class EntryFileInfoTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        std::call_once(flag, [] {
            UrlRoute::regScheme(Global::Scheme::kEntry, "/", QIcon(), true, "entry", nullptr);
            EntryEntityFactor::registCreator<TestEntryEntity>("_common_");
            EntryEntityFactor::registCreator<TestEntryEntity>("tag");
        });
    }

    void SetUp() override
    {
        url = QUrl("entry:///home/user/myentry.tag");
    }

    QUrl url;
    static std::once_flag flag;
};

std::once_flag EntryFileInfoTest::flag;

TEST_F(EntryFileInfoTest, ConstructAndEntity)
{
    EntryFileInfo info(url);
    EXPECT_NE(info.entity(), nullptr);
}

TEST_F(EntryFileInfoTest, Order)
{
    EntryFileInfo info(url);
    EXPECT_EQ(info.order(), AbstractEntryFileEntity::EntryOrder::kOrderCustom);
}

TEST_F(EntryFileInfoTest, TargetUrl)
{
    EntryFileInfo info(url);
    EXPECT_NO_FATAL_FAILURE({ (void)info.targetUrl(); });
}

TEST_F(EntryFileInfoTest, IsAccessable)
{
    EntryFileInfo info(url);
    EXPECT_TRUE(info.isAccessable());
}

TEST_F(EntryFileInfoTest, Description)
{
    EntryFileInfo info(url);
    EXPECT_NO_FATAL_FAILURE({ (void)info.description(); });
}

TEST_F(EntryFileInfoTest, ExtraProperty)
{
    EntryFileInfo info(url);
    info.setExtraProperty("k", QVariant(1));
    EXPECT_EQ(info.extraProperty("k").toInt(), 1);
    EXPECT_EQ(info.extraProperty("missing").toInt(), 0);
}

TEST_F(EntryFileInfoTest, Renamable)
{
    EntryFileInfo info(url);
    EXPECT_FALSE(info.renamable());
}

TEST_F(EntryFileInfoTest, DisplayName)
{
    EntryFileInfo info(url);
    EXPECT_EQ(info.displayName(), QString("TestEntry"));
}

TEST_F(EntryFileInfoTest, EditDisplayText)
{
    EntryFileInfo info(url);
    EXPECT_NO_FATAL_FAILURE({ (void)info.editDisplayText(); });
}

TEST_F(EntryFileInfoTest, SizeTotalAndUsageAndFree)
{
    EntryFileInfo info(url);
    EXPECT_EQ(info.sizeTotal(), 0);
    EXPECT_EQ(info.sizeUsage(), 0);
    EXPECT_EQ(info.sizeFree(), 0);
}

TEST_F(EntryFileInfoTest, ShowFlags)
{
    EntryFileInfo info(url);
    EXPECT_FALSE(info.showTotalSize());
    EXPECT_FALSE(info.showUsedSize());
    EXPECT_FALSE(info.showProgress());
}

TEST_F(EntryFileInfoTest, Exists)
{
    EntryFileInfo info(url);
    EXPECT_TRUE(info.exists());
}

TEST_F(EntryFileInfoTest, NameOfBaseNameEmpty)
{
    EntryFileInfo info(url);
    EXPECT_TRUE(info.nameOf(FileInfo::FileNameInfoType::kBaseName).isEmpty());
}

TEST_F(EntryFileInfoTest, NameOfSuffix)
{
    EntryFileInfo info(url);
    EXPECT_EQ(info.nameOf(FileInfo::FileNameInfoType::kSuffix), QString("tag"));
}

TEST_F(EntryFileInfoTest, NameOfDefault)
{
    EntryFileInfo info(url);
    EXPECT_NO_FATAL_FAILURE({ (void)info.nameOf(FileInfo::FileNameInfoType::kFileName); });
}

TEST_F(EntryFileInfoTest, DisplayOfFileDisplayName)
{
    EntryFileInfo info(url);
    EXPECT_EQ(info.displayOf(FileInfo::DisplayInfoType::kFileDisplayName), QString("TestEntry"));
}

TEST_F(EntryFileInfoTest, DisplayOfOther)
{
    EntryFileInfo info(url);
    EXPECT_NO_FATAL_FAILURE({ (void)info.displayOf(FileInfo::DisplayInfoType::kSizeDisplayName); });
}

TEST_F(EntryFileInfoTest, PathOfPathAndFilePath)
{
    EntryFileInfo info(url);
    EXPECT_EQ(info.pathOf(FileInfo::FilePathInfoType::kPath), url.path());
    EXPECT_EQ(info.pathOf(FileInfo::FilePathInfoType::kFilePath), url.path());
}

TEST_F(EntryFileInfoTest, PathOfDefault)
{
    EntryFileInfo info(url);
    EXPECT_NO_FATAL_FAILURE({ (void)info.pathOf(FileInfo::FilePathInfoType::kAbsoluteFilePath); });
}

TEST_F(EntryFileInfoTest, FileIconAndRefresh)
{
    EntryFileInfo info(url);
    EXPECT_NO_FATAL_FAILURE({ (void)info.fileIcon(); });
    EXPECT_NO_FATAL_FAILURE({ info.refresh(); });
}

TEST_F(EntryFileInfoTest, ExtraProperties)
{
    EntryFileInfo info(url);
    EXPECT_NO_FATAL_FAILURE({ (void)info.extraProperties(); });
}
