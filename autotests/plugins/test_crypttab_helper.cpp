// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_crypttab_helper.cpp
 * @brief Unit tests for crypttab_helper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/diskencrypt/helpers/crypttabhelper.h"

#include <QTest>

using namespace src;

class crypttab_helperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new crypttab_helper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    crypttab_helper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(crypttab_helperTest, addCryptOption)
{
    // Test method: bool addCryptOption((const QString &activeName, const QString &opt))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->addCryptOption(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(crypttab_helperTest, insertCryptItem)
{
    // Test method: bool insertCryptItem((const CryptItem &item))
    CryptItem _arg0{};
    auto result = obj->insertCryptItem(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(crypttab_helperTest, mergeCryptTab)
{
    // Test bool getter: mergeCryptTab()
    bool result = obj->mergeCryptTab();
    EXPECT_FALSE(result);

}

TEST_F(crypttab_helperTest, removeCryptItem)
{
    // Test method: bool removeCryptItem((const QString &activeName))
    QString _arg0{};
    auto result = obj->removeCryptItem(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(crypttab_helperTest, saveCryptItems)
{
    // Test method: void saveCryptItems((const QList<CryptItem> &items, bool doUpdateInitramfs))
    QList<CryptItem> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->saveCryptItems(_arg0, false));
}

TEST_F(crypttab_helperTest, updateCryptTab)
{
    // Test bool getter: updateCryptTab()
    bool result = obj->updateCryptTab();
    EXPECT_FALSE(result);

}

TEST_F(crypttab_helperTest, updateInitramfs)
{
    // Test method: void updateInitramfs(())
    EXPECT_NO_FATAL_FAILURE(obj->updateInitramfs());
}
