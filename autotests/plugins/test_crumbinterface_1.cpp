// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_crumbinterface_1.cpp
 * @brief Unit tests for CrumbInterface methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/crumbinterface.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class CrumbInterfaceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CrumbInterface();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CrumbInterface *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CrumbInterfaceTest, cancelCompletionListTransmission)
{
    // Test method: void cancelCompletionListTransmission(())
    EXPECT_NO_FATAL_FAILURE(obj->cancelCompletionListTransmission());
}

TEST_F(CrumbInterfaceTest, isSupportedScheme)
{
    // Test method: bool isSupportedScheme((const QString &scheme))
    QString _arg0{};
    auto result = obj->isSupportedScheme(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(CrumbInterfaceTest, processAction)
{
    // Test method: void processAction((CrumbInterface::ActionType type))
    EXPECT_NO_FATAL_FAILURE(obj->processAction(CrumbInterface::ActionType()));
}

TEST_F(CrumbInterfaceTest, requestCompletionList)
{
    // Test method: void requestCompletionList((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->requestCompletionList(_arg0));
}

TEST_F(CrumbInterfaceTest, setSupportedScheme)
{
    // Test setter: void setSupportedScheme((const QString &scheme))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setSupportedScheme(_arg0));
}
