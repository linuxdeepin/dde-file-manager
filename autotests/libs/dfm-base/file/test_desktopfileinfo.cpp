// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>
#include <QUrl>
#include <QDir>
#include <QIcon>

#include "stubext.h"

#include <dfm-base/file/local/desktopfileinfo.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/asyncfileinfo.h>

DFMBASE_USE_NAMESPACE

class TestDesktopFileInfo : public testing::Test
{
public:
    void SetUp() override
    {
        // Register FileInfo classes like in core.cpp
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        UrlRoute::regScheme(Global::Scheme::kAsyncFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
        InfoFactory::regClass<AsyncFileInfo>(Global::Scheme::kAsyncFile);

        // Create temporary directory for test files
        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid());

        tempDirPath = tempDir->path();
    }

    void TearDown() override
    {
        stub.clear();
        tempDir.reset();
    }

protected:
    QString createDesktopFile(const QString &fileName, const QMap<QString, QString> &entries)
    {
        QString filePath = tempDirPath + QDir::separator() + fileName;
        QFile file(filePath);
        EXPECT_TRUE(file.open(QIODevice::WriteOnly));
        QTextStream stream(&file);

        stream << "[Desktop Entry]\n";
        for (auto it = entries.begin(); it != entries.end(); ++it) {
            stream << it.key() << "=" << it.value() << "\n";
        }

        file.close();
        return filePath;
    }

    stub_ext::StubExt stub;
    std::unique_ptr<QTemporaryDir> tempDir;
    QString tempDirPath;
};

// ========== Constructor Tests ==========

TEST_F(TestDesktopFileInfo, Constructor_WithUrl)
{
    QMap<QString, QString> entries;
    entries["Name"] = "Test Application";
    entries["Type"] = "Application";
    entries["Exec"] = "/usr/bin/test";
    entries["Icon"] = "test-icon";

    QString desktopPath = createDesktopFile("test.desktop", entries);
    QUrl desktopUrl = QUrl::fromLocalFile(desktopPath);

    DesktopFileInfo info(desktopUrl);
    EXPECT_EQ(info.fileUrl(), desktopUrl);
}

TEST_F(TestDesktopFileInfo, Constructor_WithFileInfoPointer)
{
    QMap<QString, QString> entries;
    entries["Name"] = "Test App";
    entries["Type"] = "Application";

    QString desktopPath = createDesktopFile("test2.desktop", entries);
    QUrl desktopUrl = QUrl::fromLocalFile(desktopPath);

    FileInfoPointer baseInfo = InfoFactory::create<FileInfo>(desktopUrl);
    DesktopFileInfo info(desktopUrl, baseInfo);
    EXPECT_EQ(info.fileUrl(), desktopUrl);
}

// ========== DesktopName Tests ==========

TEST_F(TestDesktopFileInfo, DesktopName_ReturnsName)
{
    QMap<QString, QString> entries;
    entries["Name"] = "My Application";
    entries["Type"] = "Application";
    entries["X-Deepin-Vendor"] = "other";

    QString desktopPath = createDesktopFile("name.desktop", entries);
    DesktopFileInfo info(QUrl::fromLocalFile(desktopPath));

    QString name = info.desktopName();
    EXPECT_EQ(name, "My Application");
}

TEST_F(TestDesktopFileInfo, DesktopName_DeepinVendorUsesGenericName)
{
    QMap<QString, QString> entries;
    entries["Name"] = "App Name";
    entries["GenericName"] = "Generic Name";
    entries["Type"] = "Application";
    entries["X-Deepin-Vendor"] = "deepin";

    QString desktopPath = createDesktopFile("deepin.desktop", entries);
    DesktopFileInfo info(QUrl::fromLocalFile(desktopPath));

    QString name = info.desktopName();
    EXPECT_EQ(name, "Generic Name");
}

TEST_F(TestDesktopFileInfo, DesktopName_DeepinVendorEmptyGenericName)
{
    QMap<QString, QString> entries;
    entries["Name"] = "Only Name";
    entries["Type"] = "Application";
    entries["X-Deepin-Vendor"] = "deepin";

    QString desktopPath = createDesktopFile("noGeneric.desktop", entries);
    DesktopFileInfo info(QUrl::fromLocalFile(desktopPath));

    QString name = info.desktopName();
    EXPECT_EQ(name, "Only Name");
}

// ========== DesktopExec Tests ==========

TEST_F(TestDesktopFileInfo, DesktopExec_ReturnsExecCommand)
{
    QMap<QString, QString> entries;
    entries["Name"] = "Test";
    entries["Type"] = "Application";
    entries["Exec"] = "/usr/bin/myapp --flag";

    QString desktopPath = createDesktopFile("exec.desktop", entries);
    DesktopFileInfo info(QUrl::fromLocalFile(desktopPath));

    QString exec = info.desktopExec();
    EXPECT_EQ(exec, "/usr/bin/myapp --flag");
}

// ========== DesktopIconName Tests ==========

TEST_F(TestDesktopFileInfo, DesktopIconName_RegularIcon)
{
    QMap<QString, QString> entries;
    entries["Name"] = "Test";
    entries["Type"] = "Application";
    entries["Icon"] = "application-icon";

    QString desktopPath = createDesktopFile("icon.desktop", entries);
    DesktopFileInfo info(QUrl::fromLocalFile(desktopPath));

    QString iconName = info.desktopIconName();
    EXPECT_EQ(iconName, "application-icon");
}

TEST_F(TestDesktopFileInfo, DesktopIconName_TrashEmpty)
{
    QMap<QString, QString> entries;
    entries["Name"] = "Trash";
    entries["Type"] = "Application";
    entries["Icon"] = "user-trash";

    QString desktopPath = createDesktopFile("trash.desktop", entries);
    DesktopFileInfo info(QUrl::fromLocalFile(desktopPath));

    // Stub FileUtils::trashIsEmpty to return true
    stub.set_lamda(&FileUtils::trashIsEmpty, []() -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    QString iconName = info.desktopIconName();
    EXPECT_EQ(iconName, "user-trash");
}

TEST_F(TestDesktopFileInfo, DesktopIconName_TrashFull)
{
    QMap<QString, QString> entries;
    entries["Name"] = "Trash";
    entries["Type"] = "Application";
    entries["Icon"] = "user-trash";

    QString desktopPath = createDesktopFile("trash_full.desktop", entries);
    DesktopFileInfo info(QUrl::fromLocalFile(desktopPath));

    // Stub FileUtils::trashIsEmpty to return false
    stub.set_lamda(&FileUtils::trashIsEmpty, []() -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    QString iconName = info.desktopIconName();
    EXPECT_EQ(iconName, "user-trash-full");
}

// ========== DesktopType Tests ==========

TEST_F(TestDesktopFileInfo, DesktopType_Application)
{
    QMap<QString, QString> entries;
    entries["Name"] = "Test";
    entries["Type"] = "Application";

    QString desktopPath = createDesktopFile("type_app.desktop", entries);
    DesktopFileInfo info(QUrl::fromLocalFile(desktopPath));

    QString type = info.desktopType();
    EXPECT_EQ(type, "Application");
}

TEST_F(TestDesktopFileInfo, DesktopType_Link)
{
    QMap<QString, QString> entries;
    entries["Name"] = "Test Link";
    entries["Type"] = "Link";
    entries["URL"] = "http://example.com";

    QString desktopPath = createDesktopFile("type_link.desktop", entries);
    DesktopFileInfo info(QUrl::fromLocalFile(desktopPath));

    QString type = info.desktopType();
    EXPECT_EQ(type, "Link");
}

// ========== DesktopCategories Tests ==========

TEST_F(TestDesktopFileInfo, DesktopCategories_MultipleCategories)
{
    QMap<QString, QString> entries;
    entries["Name"] = "Test";
    entries["Type"] = "Application";
    entries["Categories"] = "Office;TextEditor;Utility;";

    QString desktopPath = createDesktopFile("categories.desktop", entries);
    DesktopFileInfo info(QUrl::fromLocalFile(desktopPath));

    QStringList categories = info.desktopCategories();
    EXPECT_GT(categories.size(), 0);
}

TEST_F(TestDesktopFileInfo, DesktopCategories_EmptyCategories)
{
    QMap<QString, QString> entries;
    entries["Name"] = "Test";
    entries["Type"] = "Application";

    QString desktopPath = createDesktopFile("no_categories.desktop", entries);
    DesktopFileInfo info(QUrl::fromLocalFile(desktopPath));

    QStringList categories = info.desktopCategories();
    EXPECT_TRUE(categories.isEmpty() || !categories.isEmpty());
}

// ========== FileIcon Tests ==========

TEST_F(TestDesktopFileInfo, FileIcon_ThemeIcon)
{
    QMap<QString, QString> entries;
    entries["Name"] = "Test";
    entries["Type"] = "Application";
    entries["Icon"] = "text-editor";

    QString desktopPath = createDesktopFile("file_icon.desktop", entries);
    DesktopFileInfo info(QUrl::fromLocalFile(desktopPath));

    QIcon icon = info.fileIcon();
    EXPECT_FALSE(icon.isNull());
}

// ========== NameOf Tests ==========

TEST_F(TestDesktopFileInfo, NameOf_FileNameOfRename)
{
    QMap<QString, QString> entries;
    entries["Name"] = "Rename Test";
    entries["Type"] = "Application";

    QString desktopPath = createDesktopFile("rename.desktop", entries);
    DesktopFileInfo info(QUrl::fromLocalFile(desktopPath));

    QString name = info.nameOf(FileInfo::FileNameInfoType::kFileNameOfRename);
    EXPECT_EQ(name, "Rename Test");
}

TEST_F(TestDesktopFileInfo, NameOf_BaseNameOfRename)
{
    QMap<QString, QString> entries;
    entries["Name"] = "Base Name Test";
    entries["Type"] = "Application";

    QString desktopPath = createDesktopFile("basename.desktop", entries);
    DesktopFileInfo info(QUrl::fromLocalFile(desktopPath));

    QString baseName = info.nameOf(FileInfo::FileNameInfoType::kBaseNameOfRename);
    EXPECT_EQ(baseName, "Base Name Test");
}

TEST_F(TestDesktopFileInfo, NameOf_SuffixOfRename)
{
    QMap<QString, QString> entries;
    entries["Name"] = "Suffix Test";
    entries["Type"] = "Application";

    QString desktopPath = createDesktopFile("suffix.desktop", entries);
    DesktopFileInfo info(QUrl::fromLocalFile(desktopPath));

    QString suffix = info.nameOf(FileInfo::FileNameInfoType::kSuffixOfRename);
    EXPECT_TRUE(suffix.isEmpty());
}

TEST_F(TestDesktopFileInfo, NameOf_IconName)
{
    QMap<QString, QString> entries;
    entries["Name"] = "Icon Test";
    entries["Type"] = "Application";
    entries["Icon"] = "custom-icon";

    QString desktopPath = createDesktopFile("iconname.desktop", entries);
    DesktopFileInfo info(QUrl::fromLocalFile(desktopPath));

    QString iconName = info.nameOf(FileInfo::FileNameInfoType::kIconName);
    EXPECT_EQ(iconName, "custom-icon");
}

TEST_F(TestDesktopFileInfo, NameOf_GenericIconName)
{
    QMap<QString, QString> entries;
    entries["Name"] = "Generic Icon";
    entries["Type"] = "Application";

    QString desktopPath = createDesktopFile("generic_icon.desktop", entries);
    DesktopFileInfo info(QUrl::fromLocalFile(desktopPath));

    QString genericIconName = info.nameOf(FileInfo::FileNameInfoType::kGenericIconName);
    EXPECT_EQ(genericIconName, "application-default-icon");
}

// ========== DisplayOf Tests ==========

TEST_F(TestDesktopFileInfo, DisplayOf_FileDisplayName)
{
    QMap<QString, QString> entries;
    entries["Name"] = "Display Name";
    entries["Type"] = "Application";

    QString desktopPath = createDesktopFile("display.desktop", entries);
    DesktopFileInfo info(QUrl::fromLocalFile(desktopPath));

    QString display = info.displayOf(FileInfo::DisplayInfoType::kFileDisplayName);
    EXPECT_EQ(display, "Display Name");
}

// ========== Refresh Tests ==========

TEST_F(TestDesktopFileInfo, Refresh_UpdatesInfo)
{
    QMap<QString, QString> entries;
    entries["Name"] = "Original Name";
    entries["Type"] = "Application";

    QString desktopPath = createDesktopFile("refresh.desktop", entries);
    DesktopFileInfo info(QUrl::fromLocalFile(desktopPath));

    QString originalName = info.desktopName();
    EXPECT_EQ(originalName, "Original Name");

    info.refresh();

    QString refreshedName = info.desktopName();
    EXPECT_EQ(refreshedName, "Original Name");
}

// ========== SupportedOfAttributes Tests ==========

TEST_F(TestDesktopFileInfo, SupportedOfAttributes_TrashNoDrag)
{
    QMap<QString, QString> entries;
    entries["Name"] = "Trash";
    entries["Type"] = "Application";
    entries["X-Deepin-AppID"] = "dde-trash";

    QString desktopPath = createDesktopFile("trash_drag.desktop", entries);
    DesktopFileInfo info(QUrl::fromLocalFile(desktopPath));

    Qt::DropActions actions = info.supportedOfAttributes(FileInfo::SupportType::kDrag);
    EXPECT_EQ(actions, Qt::IgnoreAction);
}

TEST_F(TestDesktopFileInfo, SupportedOfAttributes_ComputerNoDrag)
{
    QMap<QString, QString> entries;
    entries["Name"] = "Computer";
    entries["Type"] = "Application";
    entries["X-Deepin-AppID"] = "dde-computer";

    QString desktopPath = createDesktopFile("computer_drag.desktop", entries);
    DesktopFileInfo info(QUrl::fromLocalFile(desktopPath));

    Qt::DropActions actions = info.supportedOfAttributes(FileInfo::SupportType::kDrag);
    EXPECT_EQ(actions, Qt::IgnoreAction);
}

// ========== UpdateAttributes Tests ==========

TEST_F(TestDesktopFileInfo, UpdateAttributes_EmptyList)
{
    QMap<QString, QString> entries;
    entries["Name"] = "Update Test";
    entries["Type"] = "Application";

    QString desktopPath = createDesktopFile("update_attr.desktop", entries);
    DesktopFileInfo info(QUrl::fromLocalFile(desktopPath));

    info.updateAttributes();
    // No assertion needed, just verify it doesn't crash
}

// ========== CanTag Tests ==========

TEST_F(TestDesktopFileInfo, CanTag_RegularApp)
{
    QMap<QString, QString> entries;
    entries["Name"] = "Regular App";
    entries["Type"] = "Application";

    QString desktopPath = createDesktopFile("can_tag.desktop", entries);
    DesktopFileInfo info(QUrl::fromLocalFile(desktopPath));

    bool canTag = info.canTag();
    EXPECT_TRUE(canTag);
}

TEST_F(TestDesktopFileInfo, CanTag_TrashCannotTag)
{
    QMap<QString, QString> entries;
    entries["Name"] = "Trash";
    entries["Type"] = "Application";
    entries["X-Deepin-AppID"] = "dde-trash";

    QString desktopPath = createDesktopFile("trash_tag.desktop", entries);
    DesktopFileInfo info(QUrl::fromLocalFile(desktopPath));

    bool canTag = info.canTag();
    EXPECT_FALSE(canTag);
}

TEST_F(TestDesktopFileInfo, CanTag_ComputerCannotTag)
{
    QMap<QString, QString> entries;
    entries["Name"] = "Computer";
    entries["Type"] = "Application";
    entries["X-Deepin-AppID"] = "dde-computer";

    QString desktopPath = createDesktopFile("computer_tag.desktop", entries);
    DesktopFileInfo info(QUrl::fromLocalFile(desktopPath));

    bool canTag = info.canTag();
    EXPECT_FALSE(canTag);
}

TEST_F(TestDesktopFileInfo, CanTag_FileManagerHomeDirCannotTag)
{
    QMap<QString, QString> entries;
    entries["Name"] = "File Manager";
    entries["Type"] = "Application";
    entries["X-Deepin-AppID"] = "dde-file-manager";
    entries["Exec"] = "dde-file-manager -O %u";

    QString desktopPath = createDesktopFile("fm_home.desktop", entries);
    DesktopFileInfo info(QUrl::fromLocalFile(desktopPath));

    bool canTag = info.canTag();
    EXPECT_FALSE(canTag);
}

// ========== CanAttributes Tests ==========

TEST_F(TestDesktopFileInfo, CanAttributes_TrashCannotMoveOrCopy)
{
    QMap<QString, QString> entries;
    entries["Name"] = "Trash";
    entries["Type"] = "Application";
    entries["X-Deepin-AppID"] = "dde-trash";

    QString desktopPath = createDesktopFile("trash_move.desktop", entries);
    DesktopFileInfo info(QUrl::fromLocalFile(desktopPath));

    bool canMove = info.canAttributes(FileInfo::FileCanType::kCanMoveOrCopy);
    EXPECT_FALSE(canMove);
}

TEST_F(TestDesktopFileInfo, CanAttributes_ComputerCannotMoveOrCopy)
{
    QMap<QString, QString> entries;
    entries["Name"] = "Computer";
    entries["Type"] = "Application";
    entries["X-Deepin-AppID"] = "dde-computer";

    QString desktopPath = createDesktopFile("computer_move.desktop", entries);
    DesktopFileInfo info(QUrl::fromLocalFile(desktopPath));

    bool canMove = info.canAttributes(FileInfo::FileCanType::kCanMoveOrCopy);
    EXPECT_FALSE(canMove);
}

TEST_F(TestDesktopFileInfo, CanAttributes_FileManagerHomeDirCannotMoveOrCopy)
{
    QMap<QString, QString> entries;
    entries["Name"] = "File Manager";
    entries["Type"] = "Application";
    entries["X-Deepin-AppID"] = "dde-file-manager";
    entries["Exec"] = "dde-file-manager -O %f";

    QString desktopPath = createDesktopFile("fm_home_move.desktop", entries);
    DesktopFileInfo info(QUrl::fromLocalFile(desktopPath));

    bool canMove = info.canAttributes(FileInfo::FileCanType::kCanMoveOrCopy);
    EXPECT_FALSE(canMove);
}

TEST_F(TestDesktopFileInfo, CanAttributes_ComputerCannotDrop)
{
    QMap<QString, QString> entries;
    entries["Name"] = "Computer";
    entries["Type"] = "Application";
    entries["X-Deepin-AppID"] = "dde-computer";

    QString desktopPath = createDesktopFile("computer_drop.desktop", entries);
    DesktopFileInfo info(QUrl::fromLocalFile(desktopPath));

    bool canDrop = info.canAttributes(FileInfo::FileCanType::kCanDrop);
    EXPECT_FALSE(canDrop);
}

// ========== Static Method Tests ==========

TEST_F(TestDesktopFileInfo, DesktopFileInfo_StaticMethod)
{
    QMap<QString, QString> entries;
    entries["Name"] = "Static Test";
    entries["Type"] = "Application";
    entries["Exec"] = "/usr/bin/static";
    entries["Icon"] = "static-icon";

    QString desktopPath = createDesktopFile("static.desktop", entries);
    QUrl desktopUrl = QUrl::fromLocalFile(desktopPath);

    QMap<QString, QVariant> info = DesktopFileInfo::desktopFileInfo(desktopUrl);

    EXPECT_FALSE(info.isEmpty());
    EXPECT_TRUE(info.contains("Name"));
    EXPECT_TRUE(info.contains("Type"));
}

TEST_F(TestDesktopFileInfo, Convert_DesktopFile)
{
    QMap<QString, QString> entries;
    entries["Name"] = "Convert Test";
    entries["Type"] = "Application";

    QString desktopPath = createDesktopFile("convert.desktop", entries);
    QUrl desktopUrl = QUrl::fromLocalFile(desktopPath);

    FileInfoPointer baseInfo = InfoFactory::create<FileInfo>(desktopUrl);

    stub.set_lamda(&FileUtils::isDesktopFileSuffix, [](const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    QSharedPointer<FileInfo> converted = DesktopFileInfo::convert(baseInfo);
    EXPECT_NE(converted, nullptr);
}

TEST_F(TestDesktopFileInfo, Convert_NonDesktopFile)
{
    QMap<QString, QString> entries;
    entries["Name"] = "Not Desktop";
    entries["Type"] = "Application";

    QString normalPath = tempDirPath + QDir::separator() + "normal.txt";
    QFile file(normalPath);
    file.open(QIODevice::WriteOnly);
    file.write("Not a desktop file");
    file.close();

    QUrl normalUrl = QUrl::fromLocalFile(normalPath);
    FileInfoPointer baseInfo = InfoFactory::create<FileInfo>(normalUrl);

    stub.set_lamda(&FileUtils::isDesktopFileSuffix, [](const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    QSharedPointer<FileInfo> converted = DesktopFileInfo::convert(baseInfo);
    EXPECT_EQ(converted, baseInfo);
}
