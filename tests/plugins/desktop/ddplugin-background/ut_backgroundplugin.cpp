// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later


#include "backgroundplugin.h"
#include "stubext.h"

#include <gtest/gtest.h>
DDP_BACKGROUND_BEGIN_NAMESPACE

class UT_backgroundplugin : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        bkp = new BackgroundPlugin;
        bkp->initialize();
    }
    virtual void TearDown() override
    {
        delete bkp;
        stub.clear();
    }
    BackgroundPlugin *bkp = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_backgroundplugin, start)
{
    stub.set_lamda(&BackgroundManager::init,[](){
        return;
    });
    EXPECT_TRUE(bkp->start());
    bkp->stop();
    EXPECT_EQ(bkp->backgroundManager,nullptr);
}

DDP_BACKGROUND_END_NAMESPACE
