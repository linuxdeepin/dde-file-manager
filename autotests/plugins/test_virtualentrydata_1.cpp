// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_virtualentrydata_1.cpp
 * @brief Unit tests for VirtualEntryData methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "tools/upgrade/units/beans/virtualentrydata.h"

#include <QTest>

using namespace src;

class VirtualEntryDataTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VirtualEntryData();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VirtualEntryData *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VirtualEntryDataTest, VirtualEntryData)
{
    // Test constructor: VirtualEntryData((const VirtualEntryData &other))
    ASSERT_NE(obj, nullptr);
}

TEST_F(VirtualEntryDataTest, operator=)
{
    // Test getter: VirtualEntryData operator=()
    EXPECT_NO_FATAL_FAILURE({ obj->operator=(); });
}

TEST_F(VirtualEntryDataTest, getKey)
{
    // Test getter: QString getKey()
    auto result = obj->getKey();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(VirtualEntryDataTest, setKey)
{
    // Test setter: void setKey((const QString &newKey))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setKey(_arg0));
}

TEST_F(VirtualEntryDataTest, getProtocol)
{
    // Test getter: QString getProtocol()
    auto result = obj->getProtocol();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(VirtualEntryDataTest, setProtocol)
{
    // Test setter: void setProtocol((const QString &newProtocol))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setProtocol(_arg0));
}

TEST_F(VirtualEntryDataTest, getHost)
{
    // Test getter: QString getHost()
    auto result = obj->getHost();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(VirtualEntryDataTest, setHost)
{
    // Test setter: void setHost((const QString &newHost))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setHost(_arg0));
}

TEST_F(VirtualEntryDataTest, getPort)
{
    // Test getter: int getPort()
    auto result = obj->getPort();
    EXPECT_EQ(result, 0);

}

TEST_F(VirtualEntryDataTest, setPort)
{
    // Test setter: void setPort((int newPort))
    EXPECT_NO_FATAL_FAILURE(obj->setPort(0));
}

TEST_F(VirtualEntryDataTest, getDisplayName)
{
    // Test getter: QString getDisplayName()
    auto result = obj->getDisplayName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(VirtualEntryDataTest, setDisplayName)
{
    // Test setter: void setDisplayName((const QString &newDisplayName))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setDisplayName(_arg0));
}

TEST_F(VirtualEntryDataTest, getTargetPath)
{
    // Test getter: QString getTargetPath()
    auto result = obj->getTargetPath();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(VirtualEntryDataTest, setTargetPath)
{
    // Test setter: void setTargetPath((const QString &targetPath))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setTargetPath(_arg0));
}

TEST_F(VirtualEntryDataTest, key)
{
    // Test getter: QString key()
    auto result = obj->key();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(VirtualEntryDataTest, protocol)
{
    // Test getter: QString protocol()
    auto result = obj->protocol();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(VirtualEntryDataTest, host)
{
    // Test getter: QString host()
    auto result = obj->host();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(VirtualEntryDataTest, port)
{
    // Test getter: int port()
    auto result = obj->port();
    EXPECT_EQ(result, 0);

}

TEST_F(VirtualEntryDataTest, displayName)
{
    // Test getter: QString displayName()
    auto result = obj->displayName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(VirtualEntryDataTest, targetPath)
{
    // Test getter: QString targetPath()
    auto result = obj->targetPath();
    EXPECT_TRUE(result.isEmpty());

}
