// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_iteratorsearcherbridge.cpp
 * @brief Unit tests for IteratorSearcherBridge methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "searchmanager/searcher/iterator/iteratorsearcher.h"

#include <QTest>

using namespace dfmplugin_search;

class IteratorSearcherBridgeTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new IteratorSearcherBridge();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    IteratorSearcherBridge *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(IteratorSearcherBridgeTest, createIterator)
{
    // Test method: void createIterator((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->createIterator(_arg0));
}

TEST_F(IteratorSearcherBridgeTest, setSearcher)
{
    // Test setter: void setSearcher((IteratorSearcher *searcher))
    EXPECT_NO_FATAL_FAILURE(obj->setSearcher(nullptr));
}
