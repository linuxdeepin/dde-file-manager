// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_hidefilehelper.cpp
 * @brief Unit tests for HideFileHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/utils/hidefilehelper.h"

#include <QTest>

using namespace src;

class HideFileHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new HideFileHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    HideFileHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(HideFileHelperTest, HideFileHelper)
{
    // Test constructor: HideFileHelper((const QUrl &dir))
    ASSERT_NE(obj, nullptr);
}

TEST_F(HideFileHelperTest, M_~HideFileHelper)
{
    // Test method:  ~HideFileHelper(())
    EXPECT_NO_FATAL_FAILURE({ HideFileHelper *tmp = new HideFileHelper(); delete tmp; });
}

TEST_F(HideFileHelperTest, save)
{
    // Test bool getter: save()
    bool result = obj->save();
    EXPECT_FALSE(result);

}

TEST_F(HideFileHelperTest, insert)
{
    // Test method: bool insert((const QString &name))
    QString _arg0{};
    auto result = obj->insert(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(HideFileHelperTest, remove)
{
    // Test method: bool remove((const QString &name))
    QString _arg0{};
    auto result = obj->remove(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(HideFileHelperTest, contains)
{
    // Test method: bool contains((const QString &name))
    QString _arg0{};
    auto result = obj->contains(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(HideFileHelperTest, dirUrl)
{
    // Test getter: QUrl dirUrl()
    auto result = obj->dirUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(HideFileHelperTest, fileUrl)
{
    // Test getter: QUrl fileUrl()
    auto result = obj->fileUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(HideFileHelperTest, hideFileList)
{
    // Test getter: QSet<QString> hideFileList()
    auto result = obj->hideFileList();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(HideFileHelperTest, d)
{
    // Test getter: QScopedPointer<HideFileHelperPrivate> d()
    auto result = obj->d();
    EXPECT_EQ(result.get(), nullptr);

}
