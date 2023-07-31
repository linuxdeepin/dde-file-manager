// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "dfmplugin_trashcore_global.h"
#include "plugins/common/core/dfmplugin-trashcore/trashcore.h"

#include <dfm-base/base/schemefactory.h>

#include <gtest/gtest.h>

DPTRASHCORE_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
class UT_TrashCore : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
    ~UT_TrashCore() override;
};

UT_TrashCore::~UT_TrashCore(){

}

TEST_F(UT_TrashCore, testTrashCore)
{
    TrashCore core;
    core.initialize();
    EXPECT_FALSE(InfoFactory::create<FileInfo>(QUrl("trash:///")).isNull());

    EXPECT_TRUE(core.start());
    core.regCustomPropertyDialog();
}
