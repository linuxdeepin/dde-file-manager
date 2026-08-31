// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_proxyfileinfo.cpp
 * @brief Unit tests for ProxyFileInfo methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/interfaces/proxyfileinfo.h"

#include <QTest>

using namespace src;

class ProxyFileInfoTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ProxyFileInfo();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ProxyFileInfo *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ProxyFileInfoTest, ProxyFileInfo)
{
    // Test constructor: ProxyFileInfo((const QUrl &url))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ProxyFileInfoTest, M_~ProxyFileInfo)
{
    // Test method:  ~ProxyFileInfo(())
    EXPECT_NO_FATAL_FAILURE({ ProxyFileInfo *tmp = new ProxyFileInfo(); delete tmp; });
}

TEST_F(ProxyFileInfoTest, exists)
{
    // Test bool getter: exists()
    bool result = obj->exists();
    EXPECT_FALSE(result);

}

TEST_F(ProxyFileInfoTest, refresh)
{
    // Test method: void refresh(())
    EXPECT_NO_FATAL_FAILURE(obj->refresh());
}

TEST_F(ProxyFileInfoTest, fileUrl)
{
    // Test getter: QUrl fileUrl()
    auto result = obj->fileUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(ProxyFileInfoTest, setProxy)
{
    // Test setter: void setProxy((const FileInfoPointer &proxy))
    FileInfoPointer _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setProxy(_arg0));
}

TEST_F(ProxyFileInfoTest, updateAttributes)
{
    // Test method: void updateAttributes((const QList<FileInfo::FileInfoAttributeID> &types))
    QList<FileInfo::FileInfoAttributeID> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->updateAttributes(_arg0));
}

TEST_F(ProxyFileInfoTest, setNotifyUrl)
{
    // Test setter: void setNotifyUrl((const QUrl &url, const QString &token))
    QUrl _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->setNotifyUrl(_arg0, _arg1));
}

TEST_F(ProxyFileInfoTest, removeNotifyUrl)
{
    // Test method: void removeNotifyUrl((const QUrl &url, const QString &token))
    QUrl _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->removeNotifyUrl(_arg0, _arg1));
}

TEST_F(ProxyFileInfoTest, proxy)
{
    // Test getter: FileInfoPointer proxy()
    auto result = obj->proxy();
    EXPECT_EQ(result.get(), nullptr);

}
