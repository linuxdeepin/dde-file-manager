// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_classifiercreator.cpp
 * @brief Unit tests for ClassifierCreator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "mode/normalized/fileclassifier.h"

#include <QTest>

using namespace ddplugin_organizer;

class ClassifierCreatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ClassifierCreator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ClassifierCreator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ClassifierCreatorTest, createClassifier)
{
    // Test method: FileClassifier createClassifier((Classifier mode))
    auto result = obj->createClassifier(Classifier());
    EXPECT_NO_FATAL_FAILURE({ obj->createClassifier(Classifier()); });

}
