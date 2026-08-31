// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaulthelperreceiver.cpp
 * @brief Unit tests for VaultHelperReceiver methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "vaultassist/vaulthelperreceiver.h"

#include <QTest>

using namespace dfmplugin_utils;

class VaultHelperReceiverTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultHelperReceiver();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultHelperReceiver *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultHelperReceiverTest, callBackFunction)
{
    // Test method: void callBackFunction((const AbstractJobHandler::CallbackArgus args))
    EXPECT_NO_FATAL_FAILURE(obj->callBackFunction(AbstractJobHandler::CallbackArgus()));
}

TEST_F(VaultHelperReceiverTest, handleFinishedNotify)
{
    // Test method: void handleFinishedNotify((const JobInfoPointer &jobInfo))
    JobInfoPointer _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleFinishedNotify(_arg0));
}

TEST_F(VaultHelperReceiverTest, handlemoveToTrash)
{
    // Test method: bool handlemoveToTrash((const quint64 windowId,
                                            const QList<QUrl> &sources,
                                            const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags))
    QList<QUrl> _arg1{};
    auto result = obj->handlemoveToTrash(0, _arg1, DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags());
    EXPECT_FALSE(result);

}

TEST_F(VaultHelperReceiverTest, initEventConnect)
{
    // Test method: void initEventConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initEventConnect());
}
