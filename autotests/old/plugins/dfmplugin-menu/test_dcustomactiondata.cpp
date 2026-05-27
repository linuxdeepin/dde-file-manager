// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "extendmenuscene/extendmenu/dcustomactiondata.h"
#include "extendmenuscene/extendmenu/dcustomactiondefine.h"

#include <gtest/gtest.h>

using namespace dfmplugin_menu;

class UT_DCustomActionData : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        data = new DCustomActionData();
    }

    virtual void TearDown() override
    {
        delete data;
        data = nullptr;
        stub.clear();
    }

protected:
    DCustomActionData *data { nullptr };
    stub_ext::StubExt stub;
};

class UT_DCustomActionEntry : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        entry = new DCustomActionEntry();
    }

    virtual void TearDown() override
    {
        delete entry;
        entry = nullptr;
        stub.clear();
    }

protected:
    DCustomActionEntry *entry { nullptr };
    stub_ext::StubExt stub;
};

// DCustomActionData 测试

TEST_F(UT_DCustomActionData, IsMenu_WithChildren_ReturnsTrue)
{
    data->childrenActions.append(DCustomActionData());
    EXPECT_TRUE(data->isMenu());
}

TEST_F(UT_DCustomActionData, IsMenu_WithoutChildren_ReturnsFalse)
{
    EXPECT_FALSE(data->isMenu());
}

TEST_F(UT_DCustomActionData, IsAction_WithCommand)
{
    data->actionCommand = "test command";
    EXPECT_TRUE(data->isAction());
}

TEST_F(UT_DCustomActionData, Name_ReturnsActionName)
{
    data->actionName = "TestAction";
    EXPECT_EQ(data->name(), "TestAction");
}

TEST_F(UT_DCustomActionData, Icon_ReturnsActionIcon)
{
    data->actionIcon = "test-icon";
    EXPECT_EQ(data->icon(), "test-icon");
}

TEST_F(UT_DCustomActionData, Command_ReturnsActionCommand)
{
    data->actionCommand = "test command";
    EXPECT_EQ(data->command(), "test command");
}

TEST_F(UT_DCustomActionData, ParentPath_ReturnsActionParentPath)
{
    data->actionParentPath = "/parent/path";
    EXPECT_EQ(data->parentPath(), "/parent/path");
}

TEST_F(UT_DCustomActionData, Separator_ReturnsActionSeparator)
{
    data->actionSeparator = DCustomActionDefines::Separator::kTop;
    EXPECT_EQ(data->separator(), DCustomActionDefines::Separator::kTop);
}

TEST_F(UT_DCustomActionData, Acitons_ReturnsChildrenActions)
{
    DCustomActionData child;
    child.actionName = "ChildAction";
    data->childrenActions.append(child);

    auto children = data->acitons();
    EXPECT_EQ(children.size(), 1);
    EXPECT_EQ(children.first().name(), "ChildAction");
}

TEST_F(UT_DCustomActionData, NameArg_ReturnsActionNameArg)
{
    data->actionNameArg = DCustomActionDefines::ActionArg::kBaseName;
    EXPECT_EQ(data->nameArg(), DCustomActionDefines::ActionArg::kBaseName);
}

TEST_F(UT_DCustomActionData, CommandArg_ReturnsActionCmdArg)
{
    data->actionCmdArg = DCustomActionDefines::ActionArg::kFilePath;
    EXPECT_EQ(data->commandArg(), DCustomActionDefines::ActionArg::kFilePath);
}

TEST_F(UT_DCustomActionData, Position_WithComboType_ReturnsCorrectPosition)
{
    data->comboPos[DCustomActionDefines::ComboType::kBlankSpace] = 10;
    EXPECT_EQ(data->position(DCustomActionDefines::ComboType::kBlankSpace), 10);
}

TEST_F(UT_DCustomActionData, Position_ReturnsActionPosition)
{
    data->actionPosition = 5;
    EXPECT_EQ(data->position(), 5);
}

TEST_F(UT_DCustomActionData, CopyConstructor_CopiesAllFields)
{
    data->actionName = "Test";
    data->actionIcon = "icon";
    data->actionCommand = "command";
    data->actionPosition = 5;
    data->actionSeparator = DCustomActionDefines::Separator::kBottom;

    DCustomActionData copy(*data);

    EXPECT_EQ(copy.name(), "Test");
    EXPECT_EQ(copy.icon(), "icon");
    EXPECT_EQ(copy.command(), "command");
    EXPECT_EQ(copy.position(), 5);
    EXPECT_EQ(copy.separator(), DCustomActionDefines::Separator::kBottom);
}

TEST_F(UT_DCustomActionData, AssignmentOperator_CopiesAllFields)
{
    data->actionName = "Test";
    data->actionIcon = "icon";
    data->actionCommand = "command";
    data->actionPosition = 5;

    DCustomActionData copy;
    copy = *data;

    EXPECT_EQ(copy.name(), "Test");
    EXPECT_EQ(copy.icon(), "icon");
    EXPECT_EQ(copy.command(), "command");
    EXPECT_EQ(copy.position(), 5);
}

// DCustomActionEntry 测试

TEST_F(UT_DCustomActionEntry, Package_ReturnsPackageName)
{
    entry->packageName = "TestPackage";
    EXPECT_EQ(entry->package(), "TestPackage");
}

TEST_F(UT_DCustomActionEntry, Version_ReturnsPackageVersion)
{
    entry->packageVersion = "1.0.0";
    EXPECT_EQ(entry->version(), "1.0.0");
}

TEST_F(UT_DCustomActionEntry, Comment_ReturnsPackageComment)
{
    entry->packageComment = "Test Comment";
    EXPECT_EQ(entry->comment(), "Test Comment");
}

TEST_F(UT_DCustomActionEntry, FileCombo_ReturnsActionFileCombo)
{
    entry->actionFileCombo = dfmplugin_menu::DCustomActionDefines::kBlankSpace;
    EXPECT_EQ(entry->fileCombo(), dfmplugin_menu::DCustomActionDefines::kBlankSpace);
}

TEST_F(UT_DCustomActionEntry, MimeTypes_ReturnsActionMimeTypes)
{
    entry->actionMimeTypes = QStringList { "text/plain", "image/png" };
    auto mimeTypes = entry->mimeTypes();
    EXPECT_EQ(mimeTypes.size(), 2);
    EXPECT_TRUE(mimeTypes.contains("text/plain"));
    EXPECT_TRUE(mimeTypes.contains("image/png"));
}

TEST_F(UT_DCustomActionEntry, ExcludeMimeTypes_ReturnsActionExcludeMimeTypes)
{
    entry->actionExcludeMimeTypes = QStringList { "text/html" };
    auto excludeMimeTypes = entry->excludeMimeTypes();
    EXPECT_EQ(excludeMimeTypes.size(), 1);
    EXPECT_TRUE(excludeMimeTypes.contains("text/html"));
}

TEST_F(UT_DCustomActionEntry, SurpportSchemes_ReturnsActionSupportSchemes)
{
    entry->actionSupportSchemes = QStringList { "file", "trash" };
    auto schemes = entry->surpportSchemes();
    EXPECT_EQ(schemes.size(), 2);
    EXPECT_TRUE(schemes.contains("file"));
    EXPECT_TRUE(schemes.contains("trash"));
}

TEST_F(UT_DCustomActionEntry, NotShowIn_ReturnsActionNotShowIn)
{
    entry->actionNotShowIn = QStringList { "Desktop" };
    auto notShowIn = entry->notShowIn();
    EXPECT_EQ(notShowIn.size(), 1);
    EXPECT_TRUE(notShowIn.contains("Desktop"));
}

TEST_F(UT_DCustomActionEntry, SupportStuffix_ReturnsActionSupportSuffix)
{
    entry->actionSupportSuffix = QStringList { "*.txt", "*.md" };
    auto suffix = entry->supportStuffix();
    EXPECT_EQ(suffix.size(), 2);
    EXPECT_TRUE(suffix.contains("*.txt"));
    EXPECT_TRUE(suffix.contains("*.md"));
}

TEST_F(UT_DCustomActionEntry, Data_ReturnsActionData)
{
    entry->actionData.actionName = "TestData";
    auto data = entry->data();
    EXPECT_EQ(data.name(), "TestData");
}

TEST_F(UT_DCustomActionEntry, CopyConstructor_CopiesAllFields)
{
    entry->packageName = "Package";
    entry->packageVersion = "1.0";
    entry->packageComment = "Comment";
    entry->actionMimeTypes = QStringList { "text/plain" };

    DCustomActionEntry copy(*entry);

    EXPECT_EQ(copy.package(), "Package");
    EXPECT_EQ(copy.version(), "1.0");
    EXPECT_EQ(copy.comment(), "Comment");
    EXPECT_EQ(copy.mimeTypes().size(), 1);
}

TEST_F(UT_DCustomActionEntry, AssignmentOperator_CopiesAllFields)
{
    entry->packageName = "Package";
    entry->packageVersion = "1.0";
    entry->packageComment = "Comment";

    DCustomActionEntry copy;
    copy = *entry;

    EXPECT_EQ(copy.package(), "Package");
    EXPECT_EQ(copy.version(), "1.0");
    EXPECT_EQ(copy.comment(), "Comment");
}
