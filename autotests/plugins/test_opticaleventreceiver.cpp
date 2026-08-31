// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_opticaleventreceiver.cpp
 * @brief Unit tests for OpticalEventReceiver methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/opticaleventreceiver.h"

#include <QTest>

using namespace dfmplugin_optical;

class OpticalEventReceiverTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OpticalEventReceiver();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OpticalEventReceiver *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OpticalEventReceiverTest, handleCheckDragDropAction)
{
    // Test method: bool handleCheckDragDropAction((const QList<QUrl> &urls, const QUrl &urlTo, Qt::DropAction *action))
    QList<QUrl> _arg0{};
    QUrl _arg1{};
    auto result = obj->handleCheckDragDropAction(_arg0, _arg1, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(OpticalEventReceiverTest, handleDeleteFilesShortcut)
{
    // Test method: bool handleDeleteFilesShortcut((quint64, const QList<QUrl> &urls, const QUrl &rootUrl))
    QList<QUrl> _arg1{};
    QUrl _arg2{};
    auto result = obj->handleDeleteFilesShortcut(0, _arg1, _arg2);
    EXPECT_FALSE(result);

}
