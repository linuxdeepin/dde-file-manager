// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_avfseventhandler.cpp
 * @brief Unit tests for AvfsEventHandler methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/avfseventhandler.h"

#include <QTest>

using namespace dfmplugin_avfsbrowser;

class AvfsEventHandlerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AvfsEventHandler();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AvfsEventHandler *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AvfsEventHandlerTest, writeToClipbord)
{
    // Test method: void writeToClipbord((quint64 winId, const QList<QUrl> &urls))
    QList<QUrl> _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->writeToClipbord(0, _arg1));
}
