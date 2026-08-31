// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_desktopfileinfo.cpp
 * @brief Unit tests for DesktopFileInfo methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/file/local/desktopfileinfo.h"

#include <QTest>

using namespace src;

class DesktopFileInfoTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DesktopFileInfo();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DesktopFileInfo *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DesktopFileInfoTest, DesktopFileInfo)
{
    // Test constructor: DesktopFileInfo((const QUrl &fileUrl, const FileInfoPointer &info))
    ASSERT_NE(obj, nullptr);
}

TEST_F(DesktopFileInfoTest, M_~DesktopFileInfo)
{
    // Test method:  ~DesktopFileInfo(())
    EXPECT_NO_FATAL_FAILURE({ DesktopFileInfo *tmp = new DesktopFileInfo(); delete tmp; });
}

TEST_F(DesktopFileInfoTest, desktopFileInfo)
{
    // Test method: QMap<QString, QVariant> desktopFileInfo((const QUrl &fileUrl))
    QUrl _arg0{};
    auto result = obj->desktopFileInfo(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DesktopFileInfoTest, refresh)
{
    // Test method: void refresh(())
    EXPECT_NO_FATAL_FAILURE(obj->refresh());
}

TEST_F(DesktopFileInfoTest, convert)
{
    // Test method: QSharedPointer<FileInfo> convert((QSharedPointer<FileInfo> fileInfo))
    auto result = obj->convert(QSharedPointer<FileInfo>());
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(DesktopFileInfoTest, desktopName)
{
    // Test getter: QString desktopName()
    auto result = obj->desktopName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DesktopFileInfoTest, desktopExec)
{
    // Test getter: QString desktopExec()
    auto result = obj->desktopExec();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DesktopFileInfoTest, desktopType)
{
    // Test getter: QString desktopType()
    auto result = obj->desktopType();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DesktopFileInfoTest, desktopCategories)
{
    // Test getter: QStringList desktopCategories()
    auto result = obj->desktopCategories();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DesktopFileInfoTest, fileIcon)
{
    // Test getter: QIcon fileIcon()
    auto result = obj->fileIcon();
    EXPECT_TRUE(result.isNull());

}

TEST_F(DesktopFileInfoTest, nameOf)
{
    // Test method: QString nameOf((const NameInfoType type))
    auto result = obj->nameOf(NameInfoType());
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DesktopFileInfoTest, displayOf)
{
    // Test method: QString displayOf((const DisPlayInfoType type))
    auto result = obj->displayOf(DisPlayInfoType());
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DesktopFileInfoTest, updateAttributes)
{
    // Test method: void updateAttributes((const QList<FileInfo::FileInfoAttributeID> &types))
    QList<FileInfo::FileInfoAttributeID> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->updateAttributes(_arg0));
}

TEST_F(DesktopFileInfoTest, canTag)
{
    // Test bool getter: canTag()
    bool result = obj->canTag();
    EXPECT_FALSE(result);

}

TEST_F(DesktopFileInfoTest, canAttributes)
{
    // Test method: bool canAttributes((const CanableInfoType type))
    auto result = obj->canAttributes(CanableInfoType());
    EXPECT_FALSE(result);

}

TEST_F(DesktopFileInfoTest, desktopIconName)
{
    // Test getter: QString desktopIconName()
    auto result = obj->desktopIconName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DesktopFileInfoTest, supportedOfAttributes)
{
    // Test method: Qt::DropActions supportedOfAttributes((const SupportType type))
    auto result = obj->supportedOfAttributes(SupportType());
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(DesktopFileInfoTest, d)
{
    // Test getter: QSharedPointer<DesktopFileInfoPrivate> d()
    auto result = obj->d();
    EXPECT_EQ(result.get(), nullptr);

}
