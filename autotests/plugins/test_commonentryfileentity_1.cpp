// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_commonentryfileentity_1.cpp
 * @brief Unit tests for CommonEntryFileEntity methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileentity/commonentryfileentity.h"

#include <QTest>

using namespace dfmplugin_computer;

class CommonEntryFileEntityTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CommonEntryFileEntity();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CommonEntryFileEntity *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CommonEntryFileEntityTest, description)
{
    // Test getter: QString description()
    auto result = obj->description();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CommonEntryFileEntityTest, displayName)
{
    // Test getter: QString displayName()
    auto result = obj->displayName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CommonEntryFileEntityTest, exists)
{
    // Test bool getter: exists()
    bool result = obj->exists();
    EXPECT_FALSE(result);

}

TEST_F(CommonEntryFileEntityTest, hasMethod)
{
    // Test method: bool hasMethod((const QString &methodName))
    QString _arg0{};
    auto result = obj->hasMethod(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(CommonEntryFileEntityTest, icon)
{
    // Test getter: QIcon icon()
    auto result = obj->icon();
    EXPECT_TRUE(result.isNull());

}

TEST_F(CommonEntryFileEntityTest, isAccessable)
{
    // Test bool getter: isAccessable()
    bool result = obj->isAccessable();
    EXPECT_FALSE(result);

}

TEST_F(CommonEntryFileEntityTest, order)
{
    // Test getter: AbstractEntryFileEntity::EntryOrder order()
    auto result = obj->order();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(CommonEntryFileEntityTest, reflection)
{
    // Test bool getter: reflection()
    bool result = obj->reflection();
    EXPECT_FALSE(result);

}

TEST_F(CommonEntryFileEntityTest, refresh)
{
    // Test method: void refresh(())
    EXPECT_NO_FATAL_FAILURE(obj->refresh());
}

TEST_F(CommonEntryFileEntityTest, renamable)
{
    // Test bool getter: renamable()
    bool result = obj->renamable();
    EXPECT_FALSE(result);

}

TEST_F(CommonEntryFileEntityTest, setExtraProperty)
{
    // Test setter: void setExtraProperty((const QString &key, const QVariant &val))
    QString _arg0{};
    QVariant _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->setExtraProperty(_arg0, _arg1));
}

TEST_F(CommonEntryFileEntityTest, showProgress)
{
    // Test bool getter: showProgress()
    bool result = obj->showProgress();
    EXPECT_FALSE(result);

}

TEST_F(CommonEntryFileEntityTest, showTotalSize)
{
    // Test bool getter: showTotalSize()
    bool result = obj->showTotalSize();
    EXPECT_FALSE(result);

}

TEST_F(CommonEntryFileEntityTest, showUsageSize)
{
    // Test bool getter: showUsageSize()
    bool result = obj->showUsageSize();
    EXPECT_FALSE(result);

}

TEST_F(CommonEntryFileEntityTest, sizeTotal)
{
    // Test getter: quint64 sizeTotal()
    auto result = obj->sizeTotal();
    EXPECT_EQ(result, 0);

}

TEST_F(CommonEntryFileEntityTest, sizeUsage)
{
    // Test getter: quint64 sizeUsage()
    auto result = obj->sizeUsage();
    EXPECT_EQ(result, 0);

}

TEST_F(CommonEntryFileEntityTest, targetUrl)
{
    // Test getter: QUrl targetUrl()
    auto result = obj->targetUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(CommonEntryFileEntityTest, CommonEntryFileEntity_Destructor)
{
    // Test method:  ~CommonEntryFileEntity(())
    EXPECT_NO_FATAL_FAILURE({ CommonEntryFileEntity *tmp = new CommonEntryFileEntity(); delete tmp; });
}
