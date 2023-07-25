// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "plugins/common/core/dfmplugin-fileoperations/fileoperations.h"

#include <gtest/gtest.h>

DPFILEOPERATIONS_USE_NAMESPACE
class UT_Fileoperations : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
    ~UT_Fileoperations() override;
};

UT_Fileoperations::~UT_Fileoperations(){

}

TEST_F(UT_Fileoperations, testFileoperations)
{
    FileOperations op;
    EXPECT_NO_FATAL_FAILURE(op.start());
    EXPECT_TRUE(op.start());

    op.initialize();
}
