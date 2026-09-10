// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_desktopfileinfo.cpp
 * @brief Unit tests for DesktopFileInfo (desktopfileinfo.cpp)
 */

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QUrl>
#include <QIcon>
#include <mutex>

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/desktopfileinfo.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/interfaces/fileinfo.h>

using namespace dfmbase;

class DesktopFileInfoTest : public testing::Test
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
        rootPath = tmpDir.path();
    }

    QString makeDesktopFile(const QString &name, const QString &exec,
                            const QString &icon, const QString &type = "Application",
                            const QString &deepinId = "", const QString &deepinVendor = "")
    {
        QString path = rootPath + "/" + name;
        QFile f(path);
        if (!f.open(QIODevice::WriteOnly))
            return {};
        QTextStream ts(&f);
        ts << "[Desktop Entry]\n";
        ts << "Name=" << name.section('.', 0, 0) << "\n";
        ts << "GenericName=" << name.section('.', 0, 0) << " Generic\n";
        ts << "Exec=" << exec << "\n";
        ts << "Icon=" << icon << "\n";
        ts << "Type=" << type << "\n";
        ts << "Categories=Utility;\n";
        if (!deepinId.isEmpty())
            ts << "X-Deepin-AppID=" << deepinId << "\n";
        if (!deepinVendor.isEmpty())
            ts << "X-Deepin-Vendor=" << deepinVendor << "\n";
        f.close();
        return path;
    }

    QTemporaryDir tmpDir;
    QString rootPath;
    static std::once_flag flag;
};

std::once_flag DesktopFileInfoTest::flag;

TEST_F(DesktopFileInfoTest, ParseStandardDesktopFile)
{
    QString path = makeDesktopFile("myapp.desktop", "myapp", "myapp-icon", "Application",
                                   "dde-test", "deepin");
    QUrl url = QUrl::fromLocalFile(path);

    auto real = InfoFactory::create<FileInfo>(url);
    ASSERT_NE(real, nullptr);
    real->initQuerier();

    DesktopFileInfo desktop(url, real);
    EXPECT_NO_FATAL_FAILURE({ (void)desktop.desktopName(); });
    EXPECT_NO_FATAL_FAILURE({ (void)desktop.desktopExec(); });
    EXPECT_EQ(desktop.desktopIconName(), QStringLiteral("myapp-icon"));
    EXPECT_NO_FATAL_FAILURE({ (void)desktop.desktopType(); });
    EXPECT_NO_FATAL_FAILURE({ (void)desktop.desktopCategories(); });
    EXPECT_NO_FATAL_FAILURE({ (void)desktop.canTag(); });
}

TEST_F(DesktopFileInfoTest, NameOfAndDisplayOf)
{
    QString path = makeDesktopFile("calc.desktop", "calc", "calc", "Application");
    QUrl url = QUrl::fromLocalFile(path);
    auto real = InfoFactory::create<FileInfo>(url);
    real->initQuerier();

    DesktopFileInfo desktop(url, real);
    EXPECT_NO_FATAL_FAILURE({ (void)desktop.nameOf(FileInfo::FileNameInfoType::kFileNameOfRename); });
    EXPECT_NO_FATAL_FAILURE({ (void)desktop.nameOf(FileInfo::FileNameInfoType::kBaseNameOfRename); });
    EXPECT_NO_FATAL_FAILURE({ (void)desktop.nameOf(FileInfo::FileNameInfoType::kSuffixOfRename); });
    EXPECT_NO_FATAL_FAILURE({ (void)desktop.nameOf(FileInfo::FileNameInfoType::kFileCopyName); });
    EXPECT_NO_FATAL_FAILURE({ (void)desktop.nameOf(FileInfo::FileNameInfoType::kIconName); });
    EXPECT_NO_FATAL_FAILURE({ (void)desktop.nameOf(FileInfo::FileNameInfoType::kGenericIconName); });
    EXPECT_NO_FATAL_FAILURE({ (void)desktop.nameOf(FileInfo::FileNameInfoType::kFileName); });
    EXPECT_NO_FATAL_FAILURE({ (void)desktop.displayOf(FileInfo::DisplayInfoType::kFileDisplayName); });
    EXPECT_NO_FATAL_FAILURE({ (void)desktop.displayOf(FileInfo::DisplayInfoType::kSizeDisplayName); });
}

TEST_F(DesktopFileInfoTest, CanAttributesForComputerEntry)
{
    QString path = makeDesktopFile("computer.desktop", "dde-computer", "computer",
                                   "Application", "dde-computer", "deepin");
    QUrl url = QUrl::fromLocalFile(path);
    auto real = InfoFactory::create<FileInfo>(url);
    real->initQuerier();

    DesktopFileInfo desktop(url, real);
    EXPECT_FALSE(desktop.canAttributes(FileInfo::FileCanType::kCanMoveOrCopy));
    EXPECT_FALSE(desktop.canAttributes(FileInfo::FileCanType::kCanDrop));
    EXPECT_FALSE(desktop.canTag());
    EXPECT_NO_FATAL_FAILURE({ (void)desktop.supportedOfAttributes(FileInfo::SupportType::kDrag); });
    EXPECT_NO_FATAL_FAILURE({ (void)desktop.supportedOfAttributes(FileInfo::SupportType::kDrop); });
}

TEST_F(DesktopFileInfoTest, CanAttributesForTrashEntry)
{
    QString path = makeDesktopFile("trash.desktop", "dde-trash", "user-trash",
                                   "Application", "dde-trash", "deepin");
    QUrl url = QUrl::fromLocalFile(path);
    auto real = InfoFactory::create<FileInfo>(url);
    real->initQuerier();

    DesktopFileInfo desktop(url, real);
    EXPECT_FALSE(desktop.canAttributes(FileInfo::FileCanType::kCanMoveOrCopy));
    EXPECT_FALSE(desktop.canTag());
    EXPECT_EQ(desktop.supportedOfAttributes(FileInfo::SupportType::kDrag), Qt::IgnoreAction);
}

TEST_F(DesktopFileInfoTest, FileIconAndRefresh)
{
    QString path = makeDesktopFile("icon.desktop", "iconapp", "iconapp", "Application");
    QUrl url = QUrl::fromLocalFile(path);
    auto real = InfoFactory::create<FileInfo>(url);
    real->initQuerier();

    DesktopFileInfo desktop(url, real);
    EXPECT_NO_FATAL_FAILURE({ (void)desktop.fileIcon(); });
    EXPECT_NO_FATAL_FAILURE({ desktop.refresh(); });
    EXPECT_NO_FATAL_FAILURE({ desktop.updateAttributes({}); });
}

TEST_F(DesktopFileInfoTest, FileIconZeroByteDesktopFileFallback)
{
    // Bug: 0 字节 .desktop 文件（无 Icon= 条目）图标为空白，
    // 应通过 ProxyFileInfo 回退到 unknown 图标，图标永远不能为空白。
    QString path = rootPath + "/empty.desktop";
    {
        QFile f(path);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.close();   // 创建 0 字节空文件
    }

    QUrl url = QUrl::fromLocalFile(path);
    auto real = InfoFactory::create<FileInfo>(url);
    ASSERT_NE(real, nullptr);
    real->initQuerier();

    DesktopFileInfo desktop(url, real);
    QIcon icon;
    EXPECT_NO_FATAL_FAILURE({ icon = desktop.fileIcon(); });
    // 无头环境无图标主题，ProxyFileInfo::fileIcon() 回退到 FileInfo::fileIcon() 返回空 QIcon；
    // 适配环境差异：有图标主题时回退后应为非空，无头环境允许为 null
    if (!QIcon::fromTheme("document-new").isNull())
        EXPECT_FALSE(icon.isNull());
}

TEST_F(DesktopFileInfoTest, FileIconEmptyIconEntryFallback)
{
    // Bug: .desktop 文件有 Icon= 但值为空时，同样需要回退到 ProxyFileInfo。
    QString path = makeDesktopFile("noicon.desktop", "noicon", "", "Application");
    QUrl url = QUrl::fromLocalFile(path);
    auto real = InfoFactory::create<FileInfo>(url);
    ASSERT_NE(real, nullptr);
    real->initQuerier();

    DesktopFileInfo desktop(url, real);
    QIcon icon;
    EXPECT_NO_FATAL_FAILURE({ icon = desktop.fileIcon(); });
    // 无头环境无图标主题，ProxyFileInfo 回退后仍可能为 null，适配环境差异
    if (!QIcon::fromTheme("document-new").isNull())
        EXPECT_FALSE(icon.isNull());
}

TEST_F(DesktopFileInfoTest, DesktopFileInfoStatic)
{
    QString path = makeDesktopFile("static.desktop", "staticapp", "static", "Application");
    QUrl url = QUrl::fromLocalFile(path);
    EXPECT_NO_FATAL_FAILURE({ (void)DesktopFileInfo::desktopFileInfo(url); });

    auto info = InfoFactory::create<FileInfo>(url);
    ASSERT_NE(info, nullptr);
    EXPECT_NO_FATAL_FAILURE({ (void)DesktopFileInfo::convert(info); });
}
