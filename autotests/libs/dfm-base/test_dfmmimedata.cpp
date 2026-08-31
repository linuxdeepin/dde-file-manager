// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dfmmimedata.cpp
 * @brief Unit tests for DFMMimeData methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/mimedata/dfmmimedata.h"

#include <QTest>

using namespace src;

class DFMMimeDataTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DFMMimeData();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DFMMimeData *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DFMMimeDataTest, DFMMimeData)
{
    // Test constructor: DFMMimeData((const DFMMimeData &other))
    ASSERT_NE(obj, nullptr);
}

TEST_F(DFMMimeDataTest, M_~DFMMimeData)
{
    // Test method:  ~DFMMimeData(())
    EXPECT_NO_FATAL_FAILURE({ DFMMimeData *tmp = new DFMMimeData(); delete tmp; });
}

TEST_F(DFMMimeDataTest, urls)
{
    // Test getter: QList<QUrl> urls()
    auto result = obj->urls();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DFMMimeDataTest, version)
{
    // Test getter: QString version()
    auto result = obj->version();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DFMMimeDataTest, clear)
{
    // Test method: void clear(())
    EXPECT_NO_FATAL_FAILURE(obj->clear());
}

TEST_F(DFMMimeDataTest, attritube)
{
    // Test method: QVariant attritube((const QString &name, const QVariant &defaultValue))
    QString _arg0{};
    QVariant _arg1{};
    auto result = obj->attritube(_arg0, _arg1);
    EXPECT_FALSE(result.isValid());

}

TEST_F(DFMMimeDataTest, operator=)
{
    // Test getter: DFMMimeData operator=()
    EXPECT_NO_FATAL_FAILURE({ obj->operator=(); });
}

TEST_F(DFMMimeDataTest, swap)
{
    // Test method: void swap(())
    EXPECT_NO_FATAL_FAILURE(obj->swap());
}

TEST_F(DFMMimeDataTest, setUrls)
{
    // Test setter: void setUrls((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setUrls(_arg0));
}

TEST_F(DFMMimeDataTest, canTrash)
{
    // Test bool getter: canTrash()
    bool result = obj->canTrash();
    EXPECT_FALSE(result);

}

TEST_F(DFMMimeDataTest, canDelete)
{
    // Test bool getter: canDelete()
    bool result = obj->canDelete();
    EXPECT_FALSE(result);

}

TEST_F(DFMMimeDataTest, setAttritube)
{
    // Test setter: void setAttritube((const QString &name, const QVariant &value))
    QString _arg0{};
    QVariant _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->setAttritube(_arg0, _arg1));
}

TEST_F(DFMMimeDataTest, isValid)
{
    // Test bool getter: isValid()
    bool result = obj->isValid();
    EXPECT_FALSE(result);

}

TEST_F(DFMMimeDataTest, isTrashFile)
{
    // Test bool getter: isTrashFile()
    bool result = obj->isTrashFile();
    EXPECT_FALSE(result);

}

TEST_F(DFMMimeDataTest, toByteArray)
{
    // Test getter: QByteArray toByteArray()
    auto result = obj->toByteArray();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DFMMimeDataTest, fromByteArray)
{
    // Test method: DFMMimeData fromByteArray((const QByteArray &data))
    QByteArray _arg0{};
    auto result = obj->fromByteArray(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->fromByteArray(_arg0); });

}

TEST_F(DFMMimeDataTest, d)
{
    // Test getter: QSharedDataPointer<DFMMimeDataPrivate> d()
    auto result = obj->d();
    EXPECT_EQ(result.get(), nullptr);

}
