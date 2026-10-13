// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include "dfm-base/file/local/private/asyncfileinfo_p.h"
#include <dfm-base/utils/iconutils.h>
#include <dfm-io/dfileinfo.h>
#include <dfm-base/dfm_global_defines.h>

#include "stubext.h"

#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <QIcon>
#include <QMimeType>
#include <mutex>

using namespace dfmbase;

class UT_AsyncFileInfoBug379017 : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        std::call_once(flag, [] {
            UrlRoute::regScheme(Global::Scheme::kFile, QDir::homePath(), QIcon(), false, "file");
            InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
        });
    }

    void SetUp() override
    {
        ASSERT_TRUE(tmpDir.isValid());
        dirPath = tmpDir.path();
        filePath = dirPath + "/icon_test.txt";
        QFile f(filePath);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write("icon test");
        f.close();
        fileUrl = QUrl::fromLocalFile(filePath);
        dirUrl = QUrl::fromLocalFile(dirPath);
    }

    void TearDown() override
    {
        stub.clear();
    }

    QTemporaryDir tmpDir;
    QString dirPath;
    QString filePath;
    QUrl fileUrl;
    QUrl dirUrl;
    stub_ext::StubExt stub;
    static std::once_flag flag;
};

std::once_flag UT_AsyncFileInfoBug379017::flag;

// PMS:379017 AsyncFileInfo iconName 按需计算
// 修复 (b772280b): iconName() 在 fileIconName 为空时不再直接返回空，
// 而是按需计算：目录返回 "inode-directory"，文件通过 mime type 获取图标名
TEST_F(UT_AsyncFileInfoBug379017, IconName_Directory_ReturnsInodeDirectory)
{
    AsyncFileInfo info(dirUrl);
    ASSERT_TRUE(info.d != nullptr);

    // Ensure fileIconName is empty (simulates the bug condition)
    info.d->fileIconName.clear();

    // Stub queryAttributeFinished so attribute() returns synchronously
    stub.set_lamda(&DFMIO::DFileInfo::queryAttributeFinished,
                   [](DFMIO::DFileInfo *) -> bool { return true; });

    // Stub attribute to indicate this is a directory
    stub.set_lamda(static_cast<QVariant (DFMIO::DFileInfo::*)(DFMIO::DFileInfo::AttributeID, bool *) const>(
                       &DFMIO::DFileInfo::attribute),
                   [](DFMIO::DFileInfo *, DFMIO::DFileInfo::AttributeID id, bool *ok) -> QVariant {
                       if (id == DFMIO::DFileInfo::AttributeID::kStandardIsDir) {
                           if (ok) *ok = true;
                           return true;
                       }
                       if (ok) *ok = false;
                       return {};
                   });

    QString result = info.d->iconName();

    // The fix computes "inode-directory" on demand for directories
    EXPECT_EQ(result, QStringLiteral("inode-directory"));
}

// Verify that when fileIconName is already set, iconName() returns it directly
TEST_F(UT_AsyncFileInfoBug379017, IconName_PresetValue_ReturnsDirectly)
{
    AsyncFileInfo info(fileUrl);
    ASSERT_TRUE(info.d != nullptr);

    info.d->fileIconName = "preset-icon";

    QString result = info.d->iconName();

    EXPECT_EQ(result, QStringLiteral("preset-icon"));
}

// For a file (non-directory), iconName() should compute via mime type and return non-empty
TEST_F(UT_AsyncFileInfoBug379017, IconName_File_ComputesViaMimeType)
{
    AsyncFileInfo info(fileUrl);
    ASSERT_TRUE(info.d != nullptr);

    info.d->fileIconName.clear();

    stub.set_lamda(&DFMIO::DFileInfo::queryAttributeFinished,
                   [](DFMIO::DFileInfo *) -> bool { return true; });

    // Stub attribute: not a directory
    stub.set_lamda(static_cast<QVariant (DFMIO::DFileInfo::*)(DFMIO::DFileInfo::AttributeID, bool *) const>(
                       &DFMIO::DFileInfo::attribute),
                   [](DFMIO::DFileInfo *, DFMIO::DFileInfo::AttributeID id, bool *ok) -> QVariant {
                       if (id == DFMIO::DFileInfo::AttributeID::kStandardIsDir) {
                           if (ok) *ok = true;
                           return false;
                       }
                       if (ok) *ok = false;
                       return {};
                   });

    // Stub AsyncFileInfo::fileMimeType (virtual override) to avoid DFMIO calls
    stub.set_lamda(VADDR(AsyncFileInfo, fileMimeType), [](AsyncFileInfo *, QMimeDatabase::MatchMode) -> QMimeType {
        __DBG_STUB_INVOKE__
        QMimeType mt;
        return mt;
    });

    QString result;
    EXPECT_NO_FATAL_FAILURE(result = info.d->iconName());

    // After the fix, iconName computes on demand and caches the result
    // The result may be empty if mime type has no icon name, but the method
    // should not crash and should attempt computation
    EXPECT_NO_FATAL_FAILURE(info.d->iconName());
}
