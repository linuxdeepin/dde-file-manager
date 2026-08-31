// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_protocol_display_utilities.cpp
 * @brief Unit tests for protocol_display_utilities methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "displaycontrol/utilities/protocoldisplayutilities.h"

#include <QTest>

using namespace dfmplugin_smbbrowser;

class protocol_display_utilitiesTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new protocol_display_utilities();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    protocol_display_utilities *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(protocol_display_utilitiesTest, getMountedSmb)
{
    // Test getter: QStringList getMountedSmb()
    auto result = obj->getMountedSmb();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(protocol_display_utilitiesTest, getSmbHostPath)
{
    // Test method: QString getSmbHostPath((const QString &devId))
    QString _arg0{};
    auto result = obj->getSmbHostPath(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(protocol_display_utilitiesTest, getStandardSmbPath)
{
    // Test method: QString getStandardSmbPath((const QString &devId))
    QString _arg0{};
    auto result = obj->getStandardSmbPath(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(protocol_display_utilitiesTest, getStandardSmbPaths)
{
    // Test method: QStringList getStandardSmbPaths((const QStringList &devIds))
    QStringList _arg0{};
    auto result = obj->getStandardSmbPaths(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(protocol_display_utilitiesTest, hasMountedShareOf)
{
    // Test method: bool hasMountedShareOf((const QString &stdHost))
    QString _arg0{};
    auto result = obj->hasMountedShareOf(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(protocol_display_utilitiesTest, makeVEntryUrl)
{
    // Test method: QUrl makeVEntryUrl((const QString &standardSmb))
    QString _arg0{};
    auto result = obj->makeVEntryUrl(_arg0);
    EXPECT_FALSE(result.isValid());

}
