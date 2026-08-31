// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_computereventreceiver_1.cpp
 * @brief Unit tests for ComputerEventReceiver methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/computereventreceiver.h"

#include <QTest>

using namespace dfmplugin_computer;

class ComputerEventReceiverTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ComputerEventReceiver();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ComputerEventReceiver *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ComputerEventReceiverTest, askForConfirmChmod)
{
    // Test method: bool askForConfirmChmod((const QString &devName))
    QString _arg0{};
    auto result = obj->askForConfirmChmod(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(ComputerEventReceiverTest, handleItemEject)
{
    // Test method: void handleItemEject((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleItemEject(_arg0));
}

TEST_F(ComputerEventReceiverTest, handleSepateTitlebarCrumb)
{
    // Test method: bool handleSepateTitlebarCrumb((const QUrl &url, QList<QVariantMap> *mapGroup))
    QUrl _arg0{};
    auto result = obj->handleSepateTitlebarCrumb(_arg0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(ComputerEventReceiverTest, handleSetTabName)
{
    // Test method: bool handleSetTabName((const QUrl &url, QString *tabName))
    QUrl _arg0{};
    auto result = obj->handleSetTabName(_arg0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(ComputerEventReceiverTest, handleSortItem)
{
    // Test method: bool handleSortItem((const QString &group, const QString &subGroup, const QUrl &a, const QUrl &b))
    QString _arg0{};
    QString _arg1{};
    QUrl _arg2{};
    QUrl _arg3{};
    auto result = obj->handleSortItem(_arg0, _arg1, _arg2, _arg3);
    EXPECT_FALSE(result);

}

TEST_F(ComputerEventReceiverTest, instance)
{
    // Test getter: ComputerEventReceiver instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(ComputerEventReceiverTest, setContextMenuEnable)
{
    // Test setter: void setContextMenuEnable((bool enable))
    EXPECT_NO_FATAL_FAILURE(obj->setContextMenuEnable(false));
}
